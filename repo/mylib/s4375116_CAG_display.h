 /**
 **************************************************************
 * @file mylib/s4375116_CAG_display.h
 * @author Sami Kaab - s4375116 
 * @date 12052022
 * @brief task functions to display game of life on OLED
 * REFERENCE: csse3010_project.pdf
 ***************************************************************
 * EXTERNAL FUNCTIONS
 ***************************************************************
 * s4375116_tsk_cag_display_init - create the task that displays the CAG_simulation output
 ***************************************************************
 */

#ifndef S4375116_CAG_DISPLAY_H
#define S4375116_CAG_DISPLAY_H

#define GRID_HEIGHT	16	// cells
#define GRID_WIDTH	64	// cells
#define CELL_SIZE	2	// a cell is 2 by 2 pixels

// Task Priorities (Idle Priority is the lowest priority)
#define CAGDISPLAYTASK_PRIORITY					( tskIDLE_PRIORITY + 2 )

// Task Stack Allocations (must be a multiple of the minimal stack size)
#define CAGDISPLAYTASK_STACK_SIZE		( configMINIMAL_STACK_SIZE * 5 )

// struct to hold messgae contiaining grid from CAG simulator
struct cagDisplayTextMsg {
	char grid[GRID_HEIGHT][GRID_WIDTH];
};
typedef struct cagDisplayTextMsg CagDisplayTextMsg;

QueueHandle_t CAGDisplayMessageQueue;	// Queue used to receive message from cag simulator

void s4375116_tsk_cag_display_init(void);

#endif