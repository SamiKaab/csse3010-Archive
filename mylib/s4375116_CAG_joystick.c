 /**
 **************************************************************
 * @file mylib/s4375116_CAG_joystick.c
 * @author Sami Kaab - s4375116 
 * @date 21052022
 * @brief task functions to implement the joystick control of the cag simulator
 * REFERENCE: csse3010_project.pdf
 ***************************************************************
 * EXTERNAL FUNCTIONS
 ***************************************************************
 * s4375116_tsk_cag_joystick_init - create the task that runs the joystick task
 ***************************************************************
 */

#include "board.h"
#include "processor_hal.h"
#include "debug_log.h"
#include "fonts.h"


#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"

#include "oled_pixel.h"
#include "oled_string.h"

#include "s4375116_joystick.h"
#include "s4375116_CAG_joystick.h"
#include "s4375116_CAG_grid.h"

unsigned int adcYThreshold10 = 4080;
unsigned int adcYThreshold5 = 3000;
unsigned int adcYThreshold2 = 1000;
unsigned int adcXThresholdPause = 1000;
unsigned int adcXThresholdStart = 3000;
int adcXOffset = 35;
int adcYOffset = 35;

/**
 * @brief initialise the necessary hardware components
 * 
 */
void cag_joystick_hardware_init(void) {
		
	portDISABLE_INTERRUPTS();	//Disable interrupts
	
    BRD_debuguart_init();  //Initialise UART for debug log output
	// s4375116_reg_joystick_pb_init(); //initialise joystick pushbutton
	// s4375116_reg_joystick_init(); //initialise joystick adc x and y

	portENABLE_INTERRUPTS();	//Enable interrupts
}

/**
 * @brief cyclic executive for the joystick cag task
 * 
 */
void s4375116TaskCAGJoystick(void) {

	cag_joystick_hardware_init();

	EventBits_t uxBits;

	adcMessage_t rcvdAdcMessage; // message struct which holds the joystick adc values
	//queue to receive the joystick adc values
	joystickMessageQueue = xQueueCreate(50, sizeof(rcvdAdcMessage));

	// Create Event Group
	joystickctrlEventGroup = xEventGroupCreate();
	

	unsigned int adcValueX = 0; // Holds the adc value for the x axis
	unsigned int adcValueY = 0; // Holds the adc value for the y axis

	unsigned int lastAdcValueX = 0; // Holds the adc value for the x axis
	unsigned int lastAdcValueY = 0; // Holds the adc value for the y axis

	uxBits = xEventGroupClearBits(gridctrlEventGroup, EVT_CLR_GRID);

	for(;;) {

		//check if joystick pushbutton was pressed
		if (LTpbSemaphore != NULL) {	// Check if semaphore exists
			/* See if we can obtain the PB semaphore. If the semaphore is not available
			wait 10 ticks to see if it becomes free. */
			if( xSemaphoreTake( LTpbSemaphore, 10 ) == pdTRUE ) {
	
				uxBits = xEventGroupSetBits(gridctrlEventGroup, EVT_CLR_GRID);

			}

		}

		//read adc value for the queue
		if (joystickMessageQueue != NULL) {	// Check if queue exists
			// Check for item received - block atmost for 10 ticks
			if (xQueueReceive( joystickMessageQueue, &rcvdAdcMessage, 10 )) {
				// Get x and y adc value from the joystick
				adcValueX = rcvdAdcMessage.adcX;
				adcValueY = rcvdAdcMessage.adcY;
			}
		} 


		//deal with adc x values
		if (adcValueX > adcXThresholdStart && lastAdcValueX < adcXThresholdStart) {
			// the joystick is has gone to its maximum x position and the simulator is pause, so start it again
			uxBits = xEventGroupSetBits(joystickctrlEventGroup, EVT_START);
		} else if (adcValueX < adcXThresholdPause && lastAdcValueX > adcXThresholdPause) {
			// the joystick is has gone to its minimum x position and the simulator is running, so pause it
			uxBits = xEventGroupSetBits(joystickctrlEventGroup, EVT_STOP);
		}

		//deal with adc y values
		if (adcValueY > adcYThreshold10 && lastAdcValueY < adcYThreshold10) {
			uxBits = xEventGroupSetBits(joystickctrlEventGroup, EVT_UPDATE_TIME_10);
		} else if (adcValueY > adcYThreshold5 && adcValueY < adcYThreshold10 && (lastAdcValueY < adcYThreshold5 || lastAdcValueY > adcYThreshold10)) {
			uxBits = xEventGroupSetBits(joystickctrlEventGroup, EVT_UPDATE_TIME_5);
		} else if (adcValueY > adcYThreshold2 && adcValueY < adcYThreshold5 && (lastAdcValueY < adcYThreshold2 || lastAdcValueY > adcYThreshold5)) {
			uxBits = xEventGroupSetBits(joystickctrlEventGroup, EVT_UPDATE_TIME_2);
		} else if (adcValueY < adcYThreshold2 && lastAdcValueY > adcYThreshold2) {
			uxBits = xEventGroupSetBits(joystickctrlEventGroup, EVT_UPDATE_TIME_1);
		}
		// update adc values
		lastAdcValueX = adcValueX;
		lastAdcValueY = adcValueY;

		// Delay the task for 100ms.
		vTaskDelay(100);
	}
}

/**
 * @brief Creates the task that runs the OLED cylic executive.
 * 
 */
void s4375116_tsk_cag_joystick_init(void) {
	
	xTaskCreate( (void *) &s4375116TaskCAGJoystick, (const signed char *) "CAGJOYSTICK", CAGJOYSTICKTASK_STACK_SIZE, NULL, CAGJOYSTICKTASK_PRIORITY, &xJoystickCagTaskHandle );

}