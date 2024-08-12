 /**
 **************************************************************
 * @file mylib/s4375116_cli_task.h
 * @author Sami Kaab - s4375116 
 * @date 22052022
 * @brief task functions to get and parse cli commands
 * REFERENCE: csse3010_project.pdf
 ***************************************************************
 * EXTERNAL FUNCTIONS
 ***************************************************************
 * s4375116_cli_init() - creates the task that runs the cli cyclic executive
 ***************************************************************
 */

#ifndef S4375116_CLI_TASK_H
#define S4375116_CLI_TASK_H

// Task Priorities (Idle Priority is the lowest priority)
#define CLI_PRIORITY					( tskIDLE_PRIORITY + 2 )

// Task Stack Allocations (must be a multiple of the minimal stack size)
#define CLI_TASK_STACK_SIZE		( configMINIMAL_STACK_SIZE * 2 )

void s4375116_cli_init(void);


#endif