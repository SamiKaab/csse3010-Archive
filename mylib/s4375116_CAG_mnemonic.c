 /**
 **************************************************************
 * @file mylib/s4375116_CAG_simulator.c
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

#include "s4375116_oled.h"
#include "s4375116_CAG_display.h"
#include "s4375116_CAG_simulator.h"
#include "s4375116_CAG_grid.h"
#include "s4375116_CAG_joystick.h"
#include "s4375116_CAG_mnemonic.h"
#include "s4375116_cli_mnemonic.h"


/**
 * @brief cyclic executive for running game of life simulation and sending grid to display
 * 
 */
void s4375116TaskCAGMnemonic( void ) {

    portDISABLE_INTERRUPTS();	//Disable interrupts
	
    BRD_debuguart_init();  //Initialise UART for debug log output

	portENABLE_INTERRUPTS();	//Enable interrupts

    /* Register CLI commands */
	s4375116_cli_mnemonic_init();
	
	drawLifeSemaphore = xSemaphoreCreateBinary();
	deleteTaskSemaphore = xSemaphoreCreateBinary();

	for (;;) {

		// Wait for 100ms 
		vTaskDelay(100);
	}
}

/**
 * @brief create the task that runs the game of life simulation
 * 
 */
void s4375116_tsk_cag_mnemonic_init(void) {

	xTaskCreate( (void *) &s4375116TaskCAGMnemonic, (const signed char *) "CAGMNEMONIC", CAGMNEMONICTASK_STACK_SIZE, NULL, CAGMNEMONICTASK_PRIORITY, NULL );

}