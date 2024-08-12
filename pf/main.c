/**
  ******************************************************************************
  * @file    pf/main.c
  * @author  Sami Kaab s43751167
  * @date    06052022
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
#include "s4375116_CAG_display.h"
#include "s4375116_CAG_simulator.h"
#include "s4375116_CAG_grid.h"
#include "s4375116_CAG_joystick.h"
#include "s4375116_cli_mnemonic.h"
#include "s4375116_cli_task.h"
#include "s4375116_CAG_mnemonic.h"
/*
 * Main program
 */
int main(void) {

	HAL_Init(); 		// Initialise Board

	s4375116_tsk_cag_simulator_init();
	s4375116_tsk_cag_display_init();
	s4375116_tsk_cag_grid_init();
	s4375116_tsk_joystick_init();
	s4375116_tsk_cag_joystick_init();

	s4375116_cli_init();
	s4375116_tsk_cag_mnemonic_init();

	/* Start the scheduler.

	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used here. */

	vTaskStartScheduler();

	return 0;
}