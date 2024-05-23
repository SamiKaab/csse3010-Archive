 /**
 **************************************************************
 * @file mylib/s4375116_CAG_simulator.h
 * @author Sami Kaab - s4375116 
 * @date 13052022
 * @brief some of the functions necessary to control the simulator using cli commands
 * REFERENCE: csse3010_project.pdf
 ***************************************************************
 * EXTERNAL FUNCTIONS
 ***************************************************************
 * s4375116_tsk_cag_mnemonic_init() - create the task to control the simulator using cli commands
 ***************************************************************
 */


#ifndef S4375116_CAG_MNEMONIC_H
#define S4375116_CAG_MNEMONIC_H

#include "semphr.h"

// Task Priorities 
#define CAGMNEMONICTASK_PRIORITY			( tskIDLE_PRIORITY + 2 )

// Task Stack Allocations 
#define CAGMNEMONICTASK_STACK_SIZE		( configMINIMAL_STACK_SIZE * 2 )

SemaphoreHandle_t drawLifeSemaphore;	// Semaphore for pushbutton 
SemaphoreHandle_t deleteTaskSemaphore;	// Semaphore for pushbutton 


void s4375116_tsk_cag_mnemonic_init(void);
// void s4375116_processLifeCommand(const char* cCmd_string, int type);

#endif