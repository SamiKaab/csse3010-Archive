 /** 
 **************************************************************
 * @file mylib/s4375116_pantilt.c
 * @author Sami Kaab - s4375116 
 * @date 18032022
 * @brief Pan and Tilt Register Driver Functions
 * REFERENCE: csse3010_mylib_reg_pantilt.pdf,  stm32f429zi_reference.pdf
 ***************************************************************
 * EXTERNAL FUNCTIONS 
 ***************************************************************
 * s4375116_reg_pantilt_init() - Initialize pins D6 and D7 to be pwm output
 * s4375116_pantilt_angle_write() - Move specied motor to specified angle 
 * s4375116_pantilt_angle_read() - Read specified motor angle
 *************************************************************** 
 */

#include "board.h"
#include "processor_hal.h"
#include "debug_log.h"
#include "s4375116_pantilt.h"

#define PAN 								0
#define TILT 								1
#define MAX_DUTYCYCLE 						12.25 	// Max dutycycle of the servo motor
#define MED_DUTYCYCLE 						7.25 	// Duty cycle of servo motor for 0 degrees
#define MIN_DUTYCYCLE 						2.25	// Min dutycycle of the servo motor
#define MAX_ABS_ANGLE 						90		// Absolut max angle the motor can go to

// Convert provided angle to corresponding dutycycle of the motor
#define ANGLE2DUTYCYCLE(angle) (uint16_t)((angle * (MAX_DUTYCYCLE-MIN_DUTYCYCLE)/(2 * MAX_ABS_ANGLE)) + MED_DUTYCYCLE)
// Convert provided dutycycle to corresponding angle of the motor
#define DUTYCYCLE2ANGLE(duty) (int)(((int)duty - MED_DUTYCYCLE) * ((2 * MAX_ABS_ANGLE)/(MAX_DUTYCYCLE-MIN_DUTYCYCLE)))

#define TIMER_RUNNING_FREQ  				50000	//Frequency (in Hz)
#define PWM_PULSE_WIDTH_SECONDS				0.02	//20ms
#define PWM_PULSE_WIDTH_TICKS				2000	//(((PWM_PULSE_WIDTH_SECONDS)/(1/TIMER_RUNNING_FREQ))/5)	//Period in timer ticks
#define PWM_PERCENT2TICKS_DUTYCYCLE(value)		(uint16_t)(value*PWM_PULSE_WIDTH_TICKS/100)		//Duty Cycle on time in timer ticks

#define PWM_TICKS2PERCENT_DUTYCYCLE(ticks)		(float)(100 * ticks / PWM_PULSE_WIDTH_TICKS)	//Dutycycle in timer ticks to percent

// Write PWM dutycyle to the pin controlling the pan and tilt motor
#define S4375116_REG_PANTILT_PAN_WRITE(angle)	TIM1->CCR2=(uint16_t)PWM_PERCENT2TICKS_DUTYCYCLE(ANGLE2DUTYCYCLE(angle))	
#define S4375116_REG_PANTILT_TILT_WRITE(angle) 	TIM1->CCR1=(uint16_t)PWM_PERCENT2TICKS_DUTYCYCLE(ANGLE2DUTYCYCLE(angle))

// Read and convert PWM dutycyle from the pin controlling the pan and tilt motors to angle
#define S4375116_REG_PANTILT_PAN_READ()		(int)DUTYCYCLE2ANGLE(PWM_TICKS2PERCENT_DUTYCYCLE(TIM1->CCR2))	
#define S4375116_REG_PANTILT_TILT_READ() 	(int)DUTYCYCLE2ANGLE(PWM_TICKS2PERCENT_DUTYCYCLE(TIM1->CCR1))

/*
 * Initialize pins D6 and D7 to be pwm output
 */
void s4375116_reg_pantilt_init(void) {

	// Enable GPIOC Clock
	__GPIOE_CLK_ENABLE();

	// Initialise pins PE9 (D6) and PE11 (D5)
    GPIOE->OSPEEDR |= (GPIO_SPEED_FAST << 2);								//Set fast speed.
	GPIOE->PUPDR &= ~((0x03 << (9 * 2)) | (0x03 << (11 * 2)));				//Clear bits for no push/pull
	GPIOE->MODER &= ~((0x03 << (9 * 2)) | (0x03 << (11 * 2)));				//Clear bits
	GPIOE->MODER |= ((GPIO_MODE_AF_PP << (9 * 2)) | (GPIO_MODE_AF_PP << (11 * 2)));  			//Set Alternate Function Push Pull Mode
	
	GPIOE->AFR[1] &= ~(((0xF0) << ((9 - 8) * 4)) | ((0xF0) << ((11 - 8) * 4)));					//Clear Alternate Function for pin (lower ARF register)
	GPIOE->AFR[1] |= ((GPIO_AF1_TIM1 << ((9 - 8) * 4)) | (GPIO_AF1_TIM1 << ((11 - 8) * 4)));	//Set Alternate Function for pin (lower ARF register) 

	// Timer 1 clock enable
	__TIM1_CLK_ENABLE();

	/* Compute the prescaler value
	   Set the clock prescaler to 50kHz
	   SystemCoreClock is the system clock frequency */
	TIM1->PSC = ((SystemCoreClock / 2) / TIMER_RUNNING_FREQ) - 1;

	// Counting direction: 0 = up-counting, 1 = down-counting (Timer Control Register 1)
	TIM1->CR1 &= ~TIM_CR1_DIR; 

	TIM1->ARR = PWM_PULSE_WIDTH_TICKS; 		//Set pulse period to ~1s 
	// For channel 1
	TIM1->CCR1 = PWM_PERCENT2TICKS_DUTYCYCLE(50);	//Set pulse width to 0% duty cycle.

	TIM1->CCMR1 &= ~(TIM_CCMR1_OC2M); 	// Clear OC2M (Channel 2) 
	TIM1->CCMR1 |= (0x6 << 12); 		// Enable PWM Mode 1, upcounting, on Channel 2 (page 570 of ref. manual)
	TIM1->CCMR1 |= (TIM_CCMR1_OC2PE); 	// Enable output preload bit for channel 2 
	
	TIM1->CR1  |= (TIM_CR1_ARPE); 	// Set Auto-Reload Preload Enable 
	TIM1->CCER |= TIM_CCER_CC2E; 	// Set CC2E Bit
	TIM1->CCER &= ~TIM_CCER_CC2NE; 	// Clear CC2NE Bit for active high output
	
	// For channel 2
	TIM1->CCR2 = PWM_PERCENT2TICKS_DUTYCYCLE(50);	//Set pulse width to 0% duty cycle.
	TIM1->CCMR1 &= ~(TIM_CCMR1_OC1M); 	// Clear OC2M (Channel 2) 
	TIM1->CCMR1 |= (0x6 << 4); 		// Enable PWM Mode 1, upcounting, on Channel 2 (page 570 of ref. manual)
	TIM1->CCMR1 |= (TIM_CCMR1_OC1PE); 	// Enable output preload bit for channel 2 
	
	TIM1->CR1  |= (TIM_CR1_ARPE); 	// Set Auto-Reload Preload Enable 
	TIM1->CCER |= TIM_CCER_CC1E; 	// Set CC2E Bit
	TIM1->CCER &= ~TIM_CCER_CC1NE; 	// Clear CC2NE Bit for active high output
	
	/* Set Main Output Enable (MOE) bit
	   Set Off-State Selection for Run mode (OSSR) bit
	   Set Off-State Selection for Idle mode (OSSI) bit */
	TIM1->BDTR |= TIM_BDTR_MOE | TIM_BDTR_OSSR | TIM_BDTR_OSSI; 

	TIM1->CR1 |= TIM_CR1_CEN;	// Enable the counter
}

/*
 * Moves specified motor to specified angle after checking 
 * it does not go beyond set limit and adding the offset.   
 */
void s4375116_pantilt_angle_write(int type, int angle) {

	// Absolute cutoff angle beyond wich not to go
	int angle_cap = 85; // 

	// Write angle to specified motor
	switch (type) {

		case PAN :

			// debug_log("pan angle = %d\t", angle);

			// Add offset to angle to align motor
			angle += S4375116_REG_PANTILT_PAN_90_CAL_OFFSET;

			// Check angle is not greater than absolute max
			if (angle > angle_cap) {

				angle = angle_cap;
			} else if (angle < -angle_cap) {

				angle = -angle_cap;
			}

			S4375116_REG_PANTILT_PAN_WRITE(angle);	//Set duty cyle	
			break;

		case TILT:

			// debug_log("tilt angle = %d\r\n", angle);

			// Add offset to angle to align motor
			angle += S4375116_REG_PANTILT_TILT_90_CAL_OFFSET;
		
			// Check angle is not greater than absolute max
			if (angle > angle_cap) {

				angle = angle_cap;

			} else if (angle < -angle_cap) {
		
				angle = -angle_cap;
			}
			
			S4375116_REG_PANTILT_TILT_WRITE(angle);	//Set duty cyle
			break;
		
		default:
			break;
	}
}

/*
 * Reads the converted angle from the duticycle of the specified motor and returns it after adding required offset*/
int s4375116_pantilt_angle_read(int type) {

	int angle = 0;

	switch(type) {

		case PAN:

			// Get converted angle from PWM register and add offsets
			angle = S4375116_REG_PANTILT_PAN_READ() - S4375116_REG_PANTILT_PAN_90_CAL_OFFSET + 20;
			// debug_log("%d\n\r", angle);
			break;
		case TILT:

			// Get converted angle from PWM register and add offsets
			angle = S4375116_REG_PANTILT_TILT_READ() - S4375116_REG_PANTILT_TILT_90_CAL_OFFSET + 20;
			// debug_log("%d\n\r", angle);
			
			break;
	}
	return angle;
} 
