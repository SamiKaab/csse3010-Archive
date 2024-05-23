 /**
 **************************************************************
 * @file mylib/s4375116_irremote.c
 * @author Sami Kaab - s4375116 
 * @date 08042022
 * @brief IR receiver HAL Driver functions
 * REFERENCE: csse3010_mylib_reg_irremote.pdf, csse3010_stage3.pdf,
 * sourcelib/examples/timer/inputcapture/nucleo-f429/main.c
 ***************************************************************
 * EXTERNAL FUNCTIONS
 ***************************************************************
 * s4375116_reg_irremote_init() - Initialise input capture for ir receiver
 * s4375116_reg_irremote_recv() - Convert received pulses to a bit pattern 
 * s4375116_reg_irremote_readkey() - Convert bit pattern to key
 ***************************************************************
 */

#include "board.h"
#include "processor_hal.h"
#include "debug_log.h"
#include "s4375116_irremote.h"

static uint32_t bitPattern=0;         // The bit pattern received from the IR remote
static uint32_t lastIntTriggerTicks;  // last time the interrupt was triggered
int pulseCount = 0;                   // keep track of the number of pulses received since start of ir message

/**
 * @brief initialises pin D36 to be an input capture trigger by falling edges
 * 
 */
void s4375116_reg_irremote_init(void) {

  //D36->PB10

	BRD_debuguart_init();  //Initialise UART for debug log output

	// Enable GPIOB Clock
	__GPIOB_CLK_ENABLE();

  GPIOB->OSPEEDR |= (GPIO_SPEED_FAST << 1);		//Set fast speed.
	GPIOB->PUPDR &= ~(0x03 << (10 * 2));				//Clear bits for no push/pull
	GPIOB->MODER &= ~(0x03 << (10 * 2));				//Clear bits
	GPIOB->MODER |= (GPIO_MODE_AF_PP << (10 * 2));  	//Set Alternate Function Push Pull Mode
	
	GPIOB->AFR[1] &= ~((0x0F) << ((10-8) * 4));			//Clear Alternate Function for pin (lower ARF register)
	GPIOB->AFR[1] |= (GPIO_AF1_TIM2 << ((10-8) * 4));	//Set Alternate Function for pin (lower ARF register) 

	// Timer 3 clock enable
	__TIM2_CLK_ENABLE();

	// Compute the prescaler value to set the timer counting frequency to 50kHz
	// SystemCoreClock is the system clock frequency
	TIM2->PSC = ((SystemCoreClock / 2) / TIMER_COUNTER_FREQ) - 1;

	// Counting direction: 0 = up-counting, 1 = down-counting (Timer Control Register 1)
	TIM2->CR1 &= ~TIM_CR1_DIR; 

	// Set the direction as input and select the active input
	// CC1S[1:0] for channel 1;
	// 00 = output
	// 01 = input, CC1 is mapped on timer Input 1
	// 10 = input, CC1 is mapped on timer Input 2
	// 11 = input, CC1 is mapped on slave timer
	TIM2->CCMR2 &= ~TIM_CCMR2_CC3S;
	TIM2->CCMR2 |= TIM_CCMR2_CC3S_0; 
											
	// Disable digital filtering by clearing IC1F[3:0] bits
	// because we want to capture every event
	TIM2->CCMR2 &= ~TIM_CCMR2_IC3F;
	
	// Select the edge of the active transition
	// Detect only rising edges in this example
	// CC4NP:CC4P bits
	// 00 = rising edge,
	// 01 = falling edge,
	// 10 = reserved,
	// 11 = both edges
	TIM2->CCER |= (TIM_CCER_CC3P);		// falling edges.
	
	// Program the input prescaler
	// To capture each valid transition, set the input prescaler to zero;
	// IC4PSC[1:0] bits (input capture 1 prescaler)
	TIM2->CCMR2 &= ~TIM_CCMR2_IC3PSC; // Clear filtering because we need to capture every event
	
	// Enable Capture for channel 3
	TIM2->CCER |= TIM_CCER_CC3E;    
	
	// Enable related interrupts
	TIM2->DIER |= TIM_DIER_CC3IE;		// Enable Capture interrupts for channel 4
	TIM2->DIER |= TIM_DIER_UIE;			// Enable update interrupt to occur on timer counter overflow or underflow.
	
	//Enable priority (10) and interrupt callback. Do not set a priority lower than 5.
	HAL_NVIC_SetPriority(TIM2_IRQn, 10, 0);
	HAL_NVIC_EnableIRQ(TIM2_IRQn);

	TIM2->CR1 |= TIM_CR1_CEN;				// Enable the counter
}

/*
 * Timer update interrupt callback - convert received pulse to bit pattern base on
 * the length beetween pulses
 */
void s4375116_reg_irremote_recv(void) {

	uint32_t current = 0;
  uint32_t dt;

	// Check if overlfow has taken place
	if((TIM2->SR & TIM_SR_UIF) != TIM_SR_UIF) {	 

		TIM2->SR &= ~TIM_SR_UIF; 	// Clear the UIF Flag
  }

	// Check if input capture has taken place 
	if((TIM2->SR & TIM_SR_CC3IF) == TIM_SR_CC3IF) { 

    keyPressedFlag = 1; // Key is being received/detected raise flag
    pulseCount++;       // Pulse received
    current = TIM2->CCR3;  // Reading CCR1 clears CC4IF
    dt = (current - lastIntTriggerTicks)/2000; // convert to ms (2.5ms = 25)
    if (dt > 90){ // the start or end of a frame
      
      bitPattern = bitPattern & 0x00FFFFFF; //keep the first 6 bytes
      pulseCount = -1;
      bitPattern = 0;
    } else if ((pulseCount>=0) && (pulseCount<32)) {
      
      if(dt >= 15) {

        bitPattern |= (uint32_t)1 << (31 - pulseCount); // put a 1 here MSB
      } else {

        // skip (received a zero)
      }
    } 

    lastIntTriggerTicks = current;
	}
}

/*
 * Interrupt handler for Timer 2 Interrupt Handler
 */ 
void TIM2_IRQHandler(void) {

	s4375116_reg_irremote_recv();   // Callback for overflow and input capture.
}

/**
 * @brief reset key pressed flag, sets the value of the key pressed base on the
 * Hex command received and return 1 if a corresponding key was found and 0 otherwise.
 * 
 * @param value 
 * @return int 
 */
int s4375116_reg_irremote_readkey(char *value) {

  keyPressedFlag = 0;

  switch (bitPattern) {

    case 0x00FF6897:
    
      *value = '0';
      break;
    
    case 0x00FF30CF:
    
      *value = '1';
      break;
    
    case 0x00FF18E7:
    
      *value = '2';
      break;
    
    case 0x00FF7A85:
    
      *value = '3';
      break;
    
    case 0x00FF10EF:
    
      *value = '4';
      break;
    
    case 0x00FF38C7:
    
      *value = '5';
      break;
    
    case 0x00FF5AA5:
    
      *value = '6';
      break;
    
    case 0x00FF42BD:
    
      *value = '7';
      break;
    
    case 0x00FF4AB5:

      *value = '8';
      break;

    case 0x00FF52AD:

      *value = '9';
      break;
    
    default:

      return 0;
      break;

    }

  return 1;
}