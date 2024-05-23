/**
  ******************************************************************************
  * @file    s4/main.c
  * @author  Sami Kaab s43751167
  * @date    14042022
  * @brief    
  ******************************************************************************
  */

#include "board.h"
#include "processor_hal.h"
#include "debug_log.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include <string.h>

#include "s4375116_oled.h"
#include "s4375116_joystick.h"

// Task Priorities 
#define TIMERDISPLAYTASK_PRIORITY			( tskIDLE_PRIORITY + 2 )
#define LEFTTIMERTASK_PRIORITY				( tskIDLE_PRIORITY + 3 )
#define RIGHTTIMERTASK_PRIORITY				( tskIDLE_PRIORITY + 3 )


// Task Stack Allocations 
#define TIMERDISPLAYTASK_STACK_SIZE		( configMINIMAL_STACK_SIZE * 2 )
#define LEFTTIMERTASK_STACK_SIZE		( configMINIMAL_STACK_SIZE * 2 )
#define RIGHTTIMERTASK_STACK_SIZE		( configMINIMAL_STACK_SIZE * 2 )



void TaskTimerDisplay( void );
void takeTask( void );
void TaskTimerLeft(void);
void TaskTimerRight(void);


struct dualTimerMsg {
	char type; //type is either l or r
	unsigned short timerValue;
};  // message to hold left and write timer value 

QueueHandle_t queueTimerDisplay;
SemaphoreHandle_t lTSSemaphore;
SemaphoreHandle_t rTSSemaphore;

/*
 * Main program
 */
int main(void) {


	HAL_Init(); 		// Initialise Board

	// Start the oled task
	s4375116_tsk_oled_init();
	s4375116_tsk_joystick_init();

	xTaskCreate( (void *) &TaskTimerDisplay, (const signed char *) "TIMERDISPLAY", TIMERDISPLAYTASK_STACK_SIZE, NULL, TIMERDISPLAYTASK_PRIORITY, NULL );
	xTaskCreate( (void *) &TaskTimerLeft, (const signed char *) "LEFTTIMER", LEFTTIMERTASK_STACK_SIZE, NULL, LEFTTIMERTASK_PRIORITY, NULL );
	xTaskCreate( (void *) &TaskTimerRight, (const signed char *) "RIGHTTIMER", RIGHTTIMERTASK_STACK_SIZE, NULL, RIGHTTIMERTASK_PRIORITY, NULL );

	/* Start the scheduler.

	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used here. */

	vTaskStartScheduler();

	return 0;
}

/**
 * @brief creates cylclic executive for left timer: counts up every second and send number to queue
 * 
 */
void TaskTimerLeft(void) {

	struct dualTimerMsg leftTimer;
	leftTimer.type = 'l';
	leftTimer.timerValue = 0;

	uint8_t unpaused = 1;// check whether the timer should be paused
	int counter = 0;

	
	for (;;) {

		if (LTpbSemaphore != NULL) {	// Check if semaphore exists

			/* See if we can obtain the PB semaphore. If the semaphore is not available
			wait 10 ticks to see if it becomes free. */
			if( xSemaphoreTake( LTpbSemaphore, 10 ) == pdTRUE ) {

				// We were able to obtain the semaphore and can now access the shared resource. 

				// Invert mode
				unpaused = ~unpaused & 0x01;
			}
		}
		if (queueTimerDisplay != NULL) {	// Check if queue exists 
			leftTimer.type = 'l';
			xQueueSendToFront(queueTimerDisplay, ( void * ) &leftTimer, ( portTickType ) 10 );
			if(unpaused) {
				counter++;
				if(!(counter%100)) {
					leftTimer.timerValue++;
				}	
			}
			leftTimer.timerValue = leftTimer.timerValue%1000;
			// HAL_Delay(10);
		}

		// Wait for 1ms 
		vTaskDelay(1);
	}
}

/**
 * @brief creates cylclic executive for left timer: counts up every second and send number to queue
 * 
 */
void TaskTimerRight(void) {

	struct dualTimerMsg rightTimer;
	rightTimer.type = 'r';
	rightTimer.timerValue = 0;

	uint8_t unpaused = 1;
	int counter = 0;


	for (;;) {

		if (RTpbSemaphore != NULL) {	// Check if semaphore exists

			/* See if we can obtain the PB semaphore. If the semaphore is not available
			wait 10 ticks to see if it becomes free. */
			if( xSemaphoreTake( RTpbSemaphore, 10 ) == pdTRUE ) {

				// We were able to obtain the semaphore and can now access the shared resource. 

				// Invert mode
				unpaused = ~unpaused & 0x01;
			}
		}

		if (queueTimerDisplay != NULL) {	// Check if queue exists 
			rightTimer.type = 'r';
			xQueueSendToFront(queueTimerDisplay, ( void * ) &rightTimer, ( portTickType ) 10 );
			if(unpaused) {
				counter++;
				if(!(counter%1)) {
					rightTimer.timerValue++;
				}	
			}
			rightTimer.timerValue = rightTimer.timerValue%100;
			// HAL_Delay(10);
		}

		// Wait for 1ms 
		vTaskDelay(1);
	}
}

/*
 * receive messages from left and right timer and compute and sent oled message
 */
void TaskTimerDisplay( void ) {
	

	struct dualTimerMsg leftRightTimer;// left timer struc instance
	queueTimerDisplay = xQueueCreate(10, sizeof(leftRightTimer));


	OledTextMsg SendMsg;
	SendMsg.startX = 10;
	SendMsg.startY = 10;
	sprintf(SendMsg.displayText, "%u:%u", 0, 0);
	unsigned short leftTimerValue = 0;
	unsigned short rightTimerValue = 0;
	int presscount = 1;

	lTSSemaphore = xSemaphoreCreateBinary(); //left timer stop semaphore
	rTSSemaphore = xSemaphoreCreateBinary(); //right timer stop semaphore
	BaseType_t xHigherPriorityTaskWoken; //RTOS related


	for (;;) {


		if (queueTimerDisplay != NULL) {	// Check if queue exists

			// Check for item received - block atmost for 10 ticks
			if (xQueueReceive( queueTimerDisplay, &leftRightTimer, 10 )) {
				if (leftRightTimer.type == 'l') {
					leftTimerValue = leftRightTimer.timerValue;
				}
				
        	}
			// Check for item received - block atmost for 10 ticks
			if (xQueueReceive( queueTimerDisplay, &leftRightTimer, 10 )) {

				
				if (leftRightTimer.type == 'r') {
					rightTimerValue = leftRightTimer.timerValue;
				}
        	}
		}

		if (OledMessageQueue != NULL) {	// Check if queue exists 
		
			sprintf(SendMsg.displayText, "%u:%u", leftTimerValue,rightTimerValue);
			xQueueSendToFront(OledMessageQueue, ( void * ) &SendMsg, ( portTickType ) 10 );
			
			
		}

		// Wait for 1000ms 
		vTaskDelay(1);
	}
}
