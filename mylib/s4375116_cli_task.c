 /**
 **************************************************************
 * @file mylib/s4375116_cli_task.c
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

#include "board.h"
#include "processor_hal.h"
#include "debug_log.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "FreeRTOS_CLI.h"
#include "oled_pixel.h"
#include "oled_string.h"
#include "fonts.h"
#include <string.h>
#include "s4375116_oled.h"
#include "s4375116_CAG_display.h"
#include "s4375116_cli_mnemonic.h"
#include "s4375116_cli_task.h"
#include "s4375116_CAG_grid.h"


void CLI_Task(void);
void s4375116_cli_init(void);
void cli_hardware_init( void );

int cliMode = 0;

/*
 * Initialise CLI
 */
void s4375116_cli_init(void) {

	/* Create CLI Receiving task */
	xTaskCreate( (void *) &CLI_Task, (const signed char *) "CLI", CLI_TASK_STACK_SIZE, NULL, CLI_PRIORITY, NULL );

}

/*
 * CLI Receiving Task.
 */
void CLI_Task(void) {

	cli_hardware_init();


	int i;
	char cRxedChar;
	char cInputString[100];
	int InputIndex = 0;
	char *pcOutputString;
	BaseType_t xReturned;


	/* Initialise pointer to CLI output buffer. */
	memset(cInputString, 0, sizeof(cInputString));
	pcOutputString = FreeRTOS_CLIGetOutputBuffer();

	for (;;) {

		//check if onboard pushbutton was pressed
		if (gridModeSemaphore != NULL) {	// Check if semaphore exists
			/* See if we can obtain the PB semaphore. If the semaphore is not available
			wait 10 ticks to see if it becomes free. */
			if( xSemaphoreTake( gridModeSemaphore, 10 ) == pdTRUE ) {
				
				cliMode = !cliMode;
				debug_log("Mode: %s\r\n", cliMode ? "Mnemonic" : "Grid");

			}
		}

		if (cliMode) {
			/* Receive character from terminal */
			cRxedChar = debug_getc();

			/* Process if character if not Null */
			if (cRxedChar != '\0') {

				/* Echo character */
				debug_putc(cRxedChar);
				

				/* Process only if return is received. */
				if (cRxedChar == '\r') {

					//Put new line and transmit buffer
					debug_putc('\n');
					debug_flush();

					/* Put null character in command input string. */
					cInputString[InputIndex] = '\0';

					xReturned = pdTRUE;
					/* Process command input string. */
					while (xReturned != pdFALSE) {

						/* Returns pdFALSE, when all strings have been returned */
						xReturned = FreeRTOS_CLIProcessCommand( cInputString, pcOutputString, configCOMMAND_INT_MAX_OUTPUT_SIZE );

						/* Display CLI command output string (not thread safe) */
						portENTER_CRITICAL();
						for (i = 0; i < (int) strlen(pcOutputString); i++) {
							debug_putc(*(pcOutputString + i));
						}
						portEXIT_CRITICAL();

						vTaskDelay(5);	//Must delay between debug_printfs.
					}

					memset(cInputString, 0, sizeof(cInputString));
					InputIndex = 0;

				} else {

					debug_flush();		//Transmit USB buffer

					if( cRxedChar == '\r' ) {

						/* Ignore the character. */
					} else if( cRxedChar == 127 ) {
						
						/* Backspace was pressed.  Erase the last character in the
						string - if any.*/
						if( InputIndex > 0 ) {
							InputIndex--;
							debug_log("\033[1D");//move currsor back
							debug_putc(' ');
							debug_log("\033[1D");//move currsor back
							cInputString[ InputIndex ] = '\0';
						}

					} else {

						/* A character was entered.  Add it to the string
						entered so far.  When a \n is entered the complete
						string will be passed to the command interpreter. */
						if( InputIndex < 20 ) {
							cInputString[ InputIndex ] = cRxedChar;
							InputIndex++;
						}
					}
				}
			}
		}

		

		vTaskDelay(50);
		
	}
}

/*
 * Hardware Initialisation.
 */
void cli_hardware_init( void ) {

	portDISABLE_INTERRUPTS();	//Disable interrupts

	BRD_LEDInit();				//Initialise Green LED
	BRD_debuguart_init();  		//Initialise UART for debug log output

	portENABLE_INTERRUPTS();	//Enable interrupts
}
