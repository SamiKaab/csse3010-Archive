 /** 
 **************************************************************
 * @file mylib/s4375116_joystick.c
 * @author Sami Kaab - s4375116 
 * @date 27022022
 * @brief Joystick Pushbutton Register Driver Functions
 * REFERENCE: csse3010_mylib_reg_joystick_pushbutton.pdf, csse3010_mylib_reg_pantilt.pdf, 
 ***************************************************************
 * EXTERNAL FUNCTIONS 
 ***************************************************************
 * s4375116_reg_joystick_pb_init() - Enables the Joystick pushbutton source
 * s4375116_reg_joystick_pb_isr() - Joystick Pushbutton Interrupt service routine
 * s4375116_reg_joystick_press_get() - Returns the value of the Joystick pushbutton press
 * s4375116_reg_joystick_press_reset() - Reset the Joystick event counter value to 0
 * s4375116_reg_joystick_init() - Initialise pins as ADC inputs from the joystick
 * s4375116_joystick_readxy() - Read ADC value from one of the joystick axes (x or y)
 * s4375116_tsk_joystick_init() - Create Joystick Task
 *************************************************************** 
 */

#include "board.h"
#include "processor_hal.h"
#include "debug_log.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"

#include "s4375116_joystick.h"
#include "s4375116_CAG_joystick.h"


/* The number of times the joystick push button has been pressed */
static int joystick_press_counter;
/* The last time (ms) a rising edge was detected */
uint32_t previous_rising_edge_trigger = 0;
/* The last time (ms) a rising edge was detected */
uint32_t previous_falling_edge_trigger = 0;

/* 
 * Initialise GPIO pin A0 (PA3) as a rising and falling edge interrupt
 */
void s4375116_reg_joystick_pb_init(void) {

	BRD_debuguart_init();  //Initialise UART for debug log output

    // Enable GPIOA Clock
	__GPIOA_CLK_ENABLE();

    GPIOA->OSPEEDR |= (GPIO_SPEED_FAST << 3);	//Set fast speed.
	GPIOA->PUPDR &= ~(0x03 << (3 * 2));		//Clear bits for no push/pull
	GPIOA->MODER &= ~(0x03 << (3 * 2));		//Clear bits for input mode

	// Enable EXTI clock
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

	//Select trigger source (port A, pin 3) on EXTICR3.
	SYSCFG->EXTICR[0] &= ~SYSCFG_EXTICR1_EXTI3;
	SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI3_PA;
	SYSCFG->EXTICR[0] &= ~(0x000F);

	EXTI->RTSR |= EXTI_RTSR_TR3;	//enable rising dedge
	EXTI->FTSR |= EXTI_FTSR_TR3;	//enable falling edge
	EXTI->IMR |= EXTI_IMR_IM3;		//Enable external interrupt

	//Enable priority (10) and interrupt callback. Do not set a priority lower than 5.
	HAL_NVIC_SetPriority(EXTI3_IRQn, 10, 0);
	HAL_NVIC_EnableIRQ(EXTI3_IRQn);
}

/*
 * Joystick Push Button callback
 * Increments the value of the Joystick pushbutton event counter by 1
 */
void s4375116_reg_joystick_pb_isr(void) {

	BaseType_t xHigherPriorityTaskWoken; //RTOS related
	
	// Check that PA3 is high (rising edge)
	if ((GPIOA->IDR & (0x0001 << 3))) {

		previous_rising_edge_trigger = HAL_GetTick();
		
		// debug_log("rise %d\r\n", previous_falling_edge_trigger);
		// debug_log("dt %d\r\n", previous_rising_edge_trigger-previous_falling_edge_trigger);
		
		// Check that the push button has been pressed for long enough
		if (previous_falling_edge_trigger-previous_rising_edge_trigger > 110) {

			joystick_press_counter++;

			/* -----------	RTOS : give semaphore	-----------------------*/
			// Is it time for another Task() to run?
			xHigherPriorityTaskWoken = pdFALSE;

			if (LTpbSemaphore != NULL) {	// Check if semaphore exists 
				xSemaphoreGiveFromISR( LTpbSemaphore, &xHigherPriorityTaskWoken );		// Give PB Semaphore from ISR
			}
			if (RTpbSemaphore != NULL) {	// Check if semaphore exists 
				xSemaphoreGiveFromISR( RTpbSemaphore, &xHigherPriorityTaskWoken );		// Give PB Semaphore from ISR
			}

			// Perform context switching, if required.
			portYIELD_FROM_ISR( xHigherPriorityTaskWoken );

		}

	} else { // PA3 is low (falling edge)

		previous_falling_edge_trigger = HAL_GetTick();
		// debug_log("fall %d\r\n", previous_falling_edge_trigger);	
	}
}

/*
 * Returns the value of the Joystick pushbutton event counter.
 */
int s4375116_reg_joystick_press_get(void) {
	
    return joystick_press_counter;
}

/*
 * Resets the value of the Joystick pushbutton event counter to 0
 */
void s4375116_reg_joystick_press_reset(void) {

    joystick_press_counter = 0;
}

/*
 * Interrupt handler (ISR) for EXTI 15 to 10 IRQ Handler
 * Note ISR should only execute a callback
 */ 
void EXTI3_IRQHandler(void) {

	NVIC_ClearPendingIRQ(EXTI3_IRQn);

	// PR: Pending register
	if ((EXTI->PR & EXTI_PR_PR3) == EXTI_PR_PR3) {

		// cleared by writing a 1 to this bit
		EXTI->PR |= EXTI_PR_PR3;	//Clear interrupt flag.

		s4375116_reg_joystick_pb_isr();   // Callback for PA3
	}
}

/*
 *  Initialise pins A1(PC0) and A2(PC3) to be ADC inputs 
 */
void s4375116_reg_joystick_init(void) {
	
	// Enable the GPIO C Clock
  	__GPIOC_CLK_ENABLE();
  
	// Initalise PC0(A1) PC3(A2) as an Analog input.
  	GPIOC->MODER |= ((0x03 << (3 * 2)) | (0x03 << (0 * 2)));	//Set bits for Analog input mode

  	GPIOC->OSPEEDR &= ~((0x03<<(3 * 2)) | (0x03<<(0 * 2)));
	GPIOC->OSPEEDR |= ((0x02<<(3 * 2)) | (0x02<<(0 * 2)));  	// Fast speed

	GPIOC->PUPDR &= ~((0x03 << (3 * 2)) | (0x03 << (0 * 2)));	//Clear bits for no push/pull

	// Set up ADC for A2
	__ADC1_CLK_ENABLE();						//Enable ADC1 clock

	AdcHandleX.Instance = (ADC_TypeDef *)(ADC1_BASE);						//Use ADC1 (For PC3)
	AdcHandleX.Init.ClockPrescaler        = ADC_CLOCKPRESCALER_PCLK_DIV2;	//Set clock prescaler
	AdcHandleX.Init.Resolution            = ADC_RESOLUTION12b;				//Set 12-bit data resolution
	AdcHandleX.Init.ScanConvMode          = DISABLE;
	AdcHandleX.Init.ContinuousConvMode    = DISABLE;
	AdcHandleX.Init.DiscontinuousConvMode = DISABLE;
	AdcHandleX.Init.NbrOfDiscConversion   = 0;
	AdcHandleX.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;	//No Trigger
	AdcHandleX.Init.ExternalTrigConv      = ADC_EXTERNALTRIGCONV_T1_CC1;		//No Trigger
	AdcHandleX.Init.DataAlign             = ADC_DATAALIGN_RIGHT;				//Right align data
	AdcHandleX.Init.NbrOfConversion       = 1;
	AdcHandleX.Init.DMAContinuousRequests = DISABLE;
	AdcHandleX.Init.EOCSelection          = DISABLE;

	HAL_ADC_Init(&AdcHandleX);		//Initialise ADC

	// Configure ADC Channel
	AdcChanConfigX.Channel = ADC_CHANNEL_13;					//PC3 has Analog Channel 1 connected
	AdcChanConfigX.Rank         = 1;
	AdcChanConfigX.SamplingTime = ADC_SAMPLETIME_3CYCLES;
	AdcChanConfigX.Offset       = 0;

	HAL_ADC_ConfigChannel(&AdcHandleX, &AdcChanConfigX);		//Initialise ADC Channel

	
	// Set up ADC for A1(PC0)
	__ADC2_CLK_ENABLE();						//Enable ADC1 clock

	AdcHandleY.Instance = (ADC_TypeDef *)(ADC2_BASE);						//Use ADC2 (For PC0)
	AdcHandleY.Init.ClockPrescaler        = ADC_CLOCKPRESCALER_PCLK_DIV2;	//Set clock prescaler
	AdcHandleY.Init.Resolution            = ADC_RESOLUTION12b;				//Set 12-bit data resolution
	AdcHandleY.Init.ScanConvMode          = DISABLE;
	AdcHandleY.Init.ContinuousConvMode    = DISABLE;
	AdcHandleY.Init.DiscontinuousConvMode = DISABLE;
	AdcHandleY.Init.NbrOfDiscConversion   = 0;
	AdcHandleY.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;	//No Trigger
	AdcHandleY.Init.ExternalTrigConv      = ADC_EXTERNALTRIGCONV_T1_CC1;		//No Trigger
	AdcHandleY.Init.DataAlign             = ADC_DATAALIGN_RIGHT;				//Right align data
	AdcHandleY.Init.NbrOfConversion       = 1;
	AdcHandleY.Init.DMAContinuousRequests = DISABLE;
	AdcHandleY.Init.EOCSelection          = DISABLE;

	HAL_ADC_Init(&AdcHandleY);		//Initialise ADC

	// Configure ADC Channel
	AdcChanConfigY.Channel = ADC_CHANNEL_10;					//PC0 has Analog Channel 1 connected
	AdcChanConfigY.Rank         = 1;
	AdcChanConfigY.SamplingTime = ADC_SAMPLETIME_3CYCLES;
	AdcChanConfigY.Offset       = 0;

	HAL_ADC_ConfigChannel(&AdcHandleY, &AdcChanConfigY);		//Initialise ADC Channel	//Initialise ADC Channel
}

/**
 * Takes in the adc handle of one of the joystick's axes,
 * and returns the corresponding value
 */
int s4375116_joystick_readxy(ADC_HandleTypeDef AdcHandle) {

	unsigned int adc_value;	//holds the value of the specified adc 

	HAL_ADC_Start(&AdcHandle); // Start ADC conversion

	// Wait for ADC conversion to finish
	while (HAL_ADC_PollForConversion(&AdcHandle, 10) != HAL_OK);

	// Check which ADC to read from
	if (AdcHandle.Instance == AdcHandleX.Instance) {

	    adc_value = ADC1->DR;		// Read ADC1 Data Register for converted value.
		adc_value += S4375116_REG_JOYSTICK_X_ZERO_CAL_OFFSET; // Add offset to captured value
		// debug_log("x=%d ", adc_value);
	} else {

		adc_value = ADC2->DR;		// Read ADC2 Data Register for converted value.
		adc_value += S4375116_REG_JOYSTICK_Y_ZERO_CAL_OFFSET; // Add offset to captured value
		// debug_log("y=%d\n\r", adc_value);
	}
	return adc_value;
}

/**
 * @brief Control task to creat push button semaphore and initialise hardware
 * 
 */
void s4375116TaskJoystick(void) {

	LTpbSemaphore = xSemaphoreCreateBinary();
	RTpbSemaphore = xSemaphoreCreateBinary();

	portDISABLE_INTERRUPTS();	//Disable interrupts

	s4375116_reg_joystick_pb_init();
	s4375116_reg_joystick_init(); //initialise joystick adc x and y


	portENABLE_INTERRUPTS();	//Enable interrupts

	adcMessage_t sendAdcMessage; //message struct sent to CAG_joystick


	for(;;) {

		// Get x and y adc value from the joystick
		sendAdcMessage.adcX = S4375116_REG_JOYSTICK_X_READ();
		sendAdcMessage.adcY = S4375116_REG_JOYSTICK_Y_READ();
		// debug_log("adc x=%d,adc y=%d\n\r",sendAdcMessage.adcX, sendAdcMessage.adcY);
		//send adc values to cag joystick
		if (joystickMessageQueue != NULL) {	// Check if queue exists 
			xQueueSendToFront(joystickMessageQueue, ( void * ) &sendAdcMessage, ( portTickType ) 10 );
		}

		vTaskDelay(100);
	}
}

/**
 * @brief Creates the task that runs the OLED cylic executive.
 * 
 */
void s4375116_tsk_joystick_init(void) {

	xTaskCreate( (void *) &s4375116TaskJoystick, (const signed char *) "JOYSTICK", JOYSTICKTASK_STACK_SIZE, NULL, JOYSTICKTASK_PRIORITY, NULL );

}