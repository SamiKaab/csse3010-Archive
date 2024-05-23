 /**
 **************************************************************
 * @file mylib/s4375116_CAG_joystick.h
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

#ifndef S4375116_CAG_JOYSTICK_H
#define S4375116_CAG_JOYSTICK_H

// Task Priorities (Idle Priority is the lowest priority)
#define CAGJOYSTICKTASK_PRIORITY					( tskIDLE_PRIORITY + 2 )

// Task Stack Allocations (must be a multiple of the minimal stack size)
#define CAGJOYSTICKTASK_STACK_SIZE		( configMINIMAL_STACK_SIZE * 2 )

#define EVT_START			1 << 0
#define EVT_STOP			1 << 1
#define EVT_UPDATE_TIME_1	1 << 2
#define EVT_UPDATE_TIME_2	1 << 3
#define EVT_UPDATE_TIME_5	1 << 4
#define EVT_UPDATE_TIME_10	1 << 5

#define JOYSTICK_CTRL_EVENT ( EVT_START | EVT_STOP | EVT_UPDATE_TIME_1 | EVT_UPDATE_TIME_2 | EVT_UPDATE_TIME_5 | EVT_UPDATE_TIME_10)//Control Event Group Mask

EventGroupHandle_t joystickctrlEventGroup;		//Control Event Group

// message containing the joystick adc x and y value
struct adcMessage {
	int adcX;
	int adcY;
};
typedef struct adcMessage adcMessage_t;

QueueHandle_t joystickMessageQueue;	// Queue used to receive adc values from the joystick
TaskHandle_t xJoystickCagTaskHandle; // handle for the joystick task


void s4375116_tsk_cag_joystick_init(void);

#endif