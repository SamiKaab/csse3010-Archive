 /** 
 **************************************************************
 * @file mylib/s4375116_joystick.h
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

#ifndef S4375116_JOYSTICK_H
#define s4375116_JOYSTICK_H

#define S4375116_REG_JOYSTICK_X_ZERO_CAL_OFFSET 0
#define S4375116_REG_JOYSTICK_Y_ZERO_CAL_OFFSET 0
#define S4375116_REG_JOYSTICK_X_READ()  s4375116_joystick_readxy(AdcHandleX)
#define S4375116_REG_JOYSTICK_Y_READ()  s4375116_joystick_readxy(AdcHandleY)

// Task Priorities (Idle Priority is the lowest priority)
#define JOYSTICKTASK_PRIORITY					( tskIDLE_PRIORITY + 2 )
// Task Stack Allocations (must be a multiple of the minimal stack size)
#define JOYSTICKTASK_STACK_SIZE		( configMINIMAL_STACK_SIZE * 2 )

ADC_HandleTypeDef AdcHandleX;
ADC_ChannelConfTypeDef AdcChanConfigX;
ADC_HandleTypeDef AdcHandleY;
ADC_ChannelConfTypeDef AdcChanConfigY;
SemaphoreHandle_t LTpbSemaphore;	// Semaphore for pushbutton interrupt
SemaphoreHandle_t RTpbSemaphore;	// Semaphore for pushbutton interrupt


void s4375116_reg_joystick_pb_init(void);
void s4375116_reg_joystick_pb_isr(void);
int s4375116_reg_joystick_press_get(void);
void s4375116_reg_joystick_press_reset(void);
void s4375116_reg_joystick_init(void);
int s4375116_joystick_readxy(ADC_HandleTypeDef AdcHandle);
void s4375116_tsk_joystick_init(void);
        
#endif