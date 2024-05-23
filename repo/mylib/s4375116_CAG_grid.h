 /**
 **************************************************************
 * @file mylib/s4375116_CAG_grid.h
 * @author Sami Kaab - s4375116 
 * @date 14052022
 * @brief task function to get single character input from user and control simulator and display
 * REFERENCE: csse3010_project.pdf
 ***************************************************************
 * EXTERNAL FUNCTIONS
 ***************************************************************
 * s4375116_tsk_cag_grid_init - create the task that controls the simulator with user input
 ***************************************************************
 */

#ifndef S4375116_CAG_GRID_H
#define S4375116_CAG_GRID_H



#include "board.h"
#include "processor_hal.h"
#include "debug_log.h"
#include "fonts.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"
#include "semphr.h"

#include "oled_pixel.h"
#include "oled_string.h"

#include "s4375116_CAG_display.h"
#include "s4375116_CAG_simulator.h"
#include "s4375116_lta1000g.h"

// Task Priorities 
#define CAGGRIDTASK_PRIORITY			( tskIDLE_PRIORITY + 2 )

// Task Stack Allocations 
#define CAGGRIDTASK_STACK_SIZE		( configMINIMAL_STACK_SIZE * 2 )

#define EVT_MV_UP			1 << 0		//W Event Flag
#define EVT_MV_LEFT			1 << 1		//A Event Flag
#define EVT_MV_DOWN			1 << 2		//S Event Flag
#define EVT_MV_RIGHT	    1 << 3		//D Event Flag
#define EVT_SLCT_CELL		1 << 4		//X Event Flag
#define EVT_USLCT_CELL		1 << 5		//Z Event Flag
#define EVT_START_STOP		1 << 6		//P Event Flag
#define EVT_MV_ORIGIN		1 << 7		//O Event Flag
#define EVT_CLR_GRID		1 << 8		//C Event Flag


#define GRID_CTRL_EVENT ( EVT_MV_UP | EVT_MV_LEFT | EVT_MV_DOWN | EVT_MV_RIGHT | EVT_SLCT_CELL | EVT_USLCT_CELL | EVT_START_STOP | EVT_MV_ORIGIN | EVT_CLR_GRID )//Control Event Group Mask

EventGroupHandle_t gridctrlEventGroup;		//Control Event Group
SemaphoreHandle_t gridModeSemaphore;	// Semaphore for pushbutton 
EventBits_t uxBits;
caMessage_t sendCaMessage;



void s4375116_tsk_cag_grid_init(void);

#endif