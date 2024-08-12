 /**
 **************************************************************
 * @file mylib/s4375116_cli_mnemonic.c
 * @author Sami Kaab - s4375116 
 * @date 22052022
 * @brief cli command functions used to control the cag simulation
 * REFERENCE: csse3010_project.pdf
 ***************************************************************
 * EXTERNAL FUNCTIONS
 ***************************************************************
 * s4375116_cli_mnemonic_init() - registers the cli commands
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
#include "s4375116_CAG_mnemonic.h"
#include "s4375116_CAG_grid.h"
#include "s4375116_CAG_joystick.h"
#include "s4375116_CAG_simulator.h"


static BaseType_t prvStillCommand(char *cCmd_string, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvOscCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvGliderCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvClearCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvPauseCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvStartCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvDelCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
static BaseType_t prvCreCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );

CLI_Command_Definition_t xStill = {	// Structure that defines the "still" command line command.
	"still",														// Comamnd String
	"still: Draw a still life of specified type and at specified location:\r\n still type x y\r\n\r\n",								// Help String (Displayed when "help' is typed)
	prvStillCommand,												// Command Callback that implements the command
	3															// Number of input parameters
};

CLI_Command_Definition_t xOsc = {	// Structure that defines the "osc" command line command.
	"osc",														// Comamnd String
	"osc: Draw an oscillator life of specified type and at a specified location:\r\n osc type x y\r\n\r\n",								// Help String (Displayed when "help' is typed)
	prvOscCommand,												// Command Callback that implements the command
	3															// Number of input parameters
};

CLI_Command_Definition_t xGlider = {	// Structure that defines the "osc" command line command.
	"glider",														// Comamnd String
	"glider: Draw a glider at a specified location:\n\r glider x y.\r\n\r\n",								// Help String (Displayed when "help' is typed)
	prvGliderCommand,												// Command Callback that implements the command
	2																// Number of input parameters
};

CLI_Command_Definition_t xClear = {	// Structure that defines the "Clear" command line command.
	"clear",														// Comamnd String
	"clear: Clear the Oled display and reset the simulation.\r\n\r\n",	// Help String (Displayed when "help' is typed)
	prvClearCommand,												// Command Callback that implements the command
	0																// Number of input parameters
};

CLI_Command_Definition_t xPause = {	// Structure that defines the "Pause" command line command.
	"pause",														// Comamnd String
	"pause: Pause the simulation.\r\n\r\n",								// Help String (Displayed when "help' is typed)
	prvPauseCommand,												// Command Callback that implements the command
	0																// Number of input parameters
};

CLI_Command_Definition_t xStart = {	// Structure that defines the "Start" command line command.
	"start",														// Comamnd String
	"start: Start the simulation.\r\n\r\n",								// Help String (Displayed when "help' is typed)
	prvStartCommand,												// Command Callback that implements the command
	0																// Number of input parameters
};

CLI_Command_Definition_t xDel = {	// Structure that defines the "del" command line command.
	"del",														// Comamnd String
	"del: delete the specified task.\r\n del type\r\n\r\n",								// Help String (Displayed when "help' is typed)
	prvDelCommand,												// Command Callback that implements the command
	1																// Number of input parameters
};

CLI_Command_Definition_t xCre = {	// Structure that defines the "del" command line command.
	"cre",														// Comamnd String
	"cre: create the specified task.\r\n cre type\r\n\r\n",								// Help String (Displayed when "help' is typed)
	prvCreCommand,												// Command Callback that implements the command
	1																// Number of input parameters
};

/*
 * Initialise CLI
 */
void s4375116_cli_mnemonic_init(void) {

	/* Register CLI commands */
	FreeRTOS_CLIRegisterCommand(&xStill);
	FreeRTOS_CLIRegisterCommand(&xOsc);
	FreeRTOS_CLIRegisterCommand(&xGlider);
	FreeRTOS_CLIRegisterCommand(&xClear);
	FreeRTOS_CLIRegisterCommand(&xPause);
	FreeRTOS_CLIRegisterCommand(&xStart);
	FreeRTOS_CLIRegisterCommand(&xDel);
	FreeRTOS_CLIRegisterCommand(&xCre);

}

/*
 * still Command.
 */
static BaseType_t prvStillCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {

	int8_t *pcParameter1, *pcParameter2, *pcParameter3;
	BaseType_t xParameter1StringLength, xParameter2StringLength, xParameter3StringLength, xResult;
	caMessage_t sendCaMessage;

    /* Obtain the name of the source file, and the length of its name, from
    the command string. The name of the source file is the first parameter. */
    pcParameter1 = FreeRTOS_CLIGetParameter
                        (
                          /* The command string itself. */
                          pcCommandString,
                          /* Return the first parameter. */
                          1,
                          /* Store the parameter string length. */
                          &xParameter1StringLength
                        );

    /* Obtain the name of the destination file, and the length of its name. */
    pcParameter2 = FreeRTOS_CLIGetParameter( pcCommandString,
                                             2,
                                             &xParameter2StringLength );
	/* Obtain the name of the destination file, and the length of its name. */
    pcParameter3 = FreeRTOS_CLIGetParameter( pcCommandString,
                                             3,
                                             &xParameter3StringLength );
    // /* Terminate both file names. */
    // pcParameter1[ xParameter1StringLength ] = 0x00;
    // pcParameter2[ xParameter2StringLength ] = 0x00;
    // pcParameter3[ xParameter3StringLength ] = 0x00;


	int type2 = atoi (pcParameter1);
	int x = atoi (pcParameter2);
	int y = atoi (pcParameter3);
	int type = 2;


	sendCaMessage.type = type<<0x04 | type2; 
	sendCaMessage.x = x;
	sendCaMessage.y = y;
	
	xSemaphoreGive( drawLifeSemaphore );
	if (CAGSimulatorMessageQueue != NULL) {	// Check if queue exists 
		xQueueSendToFront(CAGSimulatorMessageQueue, ( void * ) &sendCaMessage, ( portTickType ) 10 );
	}

	xWriteBufferLen = sprintf((char *) pcWriteBuffer, "");
	return pdFALSE;
}

/*
 * Osc Command.
 */
static BaseType_t prvOscCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {

	int8_t *pcParameter1, *pcParameter2, *pcParameter3;
	BaseType_t xParameter1StringLength, xParameter2StringLength, xParameter3StringLength, xResult;
	caMessage_t sendCaMessage;

    /* Obtain the name of the source file, and the length of its name, from
    the command string. The name of the source file is the first parameter. */
    pcParameter1 = FreeRTOS_CLIGetParameter
                        (
                          /* The command string itself. */
                          pcCommandString,
                          /* Return the first parameter. */
                          1,
                          /* Store the parameter string length. */
                          &xParameter1StringLength
                        );

    /* Obtain the name of the destination file, and the length of its name. */
    pcParameter2 = FreeRTOS_CLIGetParameter( pcCommandString,
                                             2,
                                             &xParameter2StringLength );
	/* Obtain the name of the destination file, and the length of its name. */
    pcParameter3 = FreeRTOS_CLIGetParameter( pcCommandString,
                                             3,
                                             &xParameter3StringLength );


	int type2 = atoi (pcParameter1);
	int x = atoi (pcParameter2);
	int y = atoi (pcParameter3);
	int type = 3;

	sendCaMessage.type = type<<0x04 | type2; 
	sendCaMessage.x = x;
	sendCaMessage.y = y;
	
	xSemaphoreGive( drawLifeSemaphore );
	if (CAGSimulatorMessageQueue != NULL) {	// Check if queue exists 
		xQueueSendToFront(CAGSimulatorMessageQueue, ( void * ) &sendCaMessage, ( portTickType ) 10 );
	}
	xWriteBufferLen = sprintf((char *) pcWriteBuffer, "");
	return pdFALSE;
}

/*
 * Glider Command.
 */
static BaseType_t prvGliderCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {

	int8_t *pcParameter1, *pcParameter2, *pcParameter3;
	BaseType_t xParameter1StringLength, xParameter2StringLength, xResult;
	caMessage_t sendCaMessage;

    /* Obtain the name of the source file, and the length of its name, from
    the command string. The name of the source file is the first parameter. */
    pcParameter1 = FreeRTOS_CLIGetParameter
                        (
                          /* The command string itself. */
                          pcCommandString,
                          /* Return the first parameter. */
                          1,
                          /* Store the parameter string length. */
                          &xParameter1StringLength
                        );

    /* Obtain the name of the destination file, and the length of its name. */
    pcParameter2 = FreeRTOS_CLIGetParameter( pcCommandString,
                                             2,
                                             &xParameter2StringLength );
	int type2 = 0;
	int x = atoi (pcParameter1);
	int y = atoi (pcParameter2);
	int type = 4;


	sendCaMessage.type = type<<0x04 | type2; 
	sendCaMessage.x = x;
	sendCaMessage.y = y;
	
	xSemaphoreGive( drawLifeSemaphore );
	if (CAGSimulatorMessageQueue != NULL) {	// Check if queue exists 
		xQueueSendToFront(CAGSimulatorMessageQueue, ( void * ) &sendCaMessage, ( portTickType ) 10 );
	}
	xWriteBufferLen = sprintf((char *) pcWriteBuffer, "");
	return pdFALSE;
}

/*
 * Clear Command.
 */
static BaseType_t prvClearCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {

	long lParam_len;
	const char *cCmd_string;

	uxBits = xEventGroupSetBits(gridctrlEventGroup, EVT_CLR_GRID);

	/* Return pdFALSE, as there are no more strings to return */
	/* Only return pdTRUE, if more strings need to be printed */
	xWriteBufferLen = sprintf((char *) pcWriteBuffer, "");
	return pdFALSE;
}

/*
 * Pause Command.
 */
static BaseType_t prvPauseCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {

	long lParam_len;
	const char *cCmd_string;


	uxBits = xEventGroupSetBits(joystickctrlEventGroup, EVT_STOP);
	/* Return pdFALSE, as there are no more strings to return */
	/* Only return pdTRUE, if more strings need to be printed */
	xWriteBufferLen = sprintf((char *) pcWriteBuffer, "");
	return pdFALSE;
}

/*
 * Start Command.
 */
static BaseType_t prvStartCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {

	long lParam_len;
	const char *cCmd_string;


	uxBits = xEventGroupSetBits(joystickctrlEventGroup, EVT_START);
	/* Return pdFALSE, as there are no more strings to return */
	/* Only return pdTRUE, if more strings need to be printed */
	xWriteBufferLen = sprintf((char *) pcWriteBuffer, "");
	return pdFALSE;
}



/*
 * delete tasks Command.
 */
static BaseType_t prvDelCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {

	

	int8_t *pcParameter1;
	BaseType_t xParameter1StringLength;
	caMessage_t sendCaMessage;

    /* Obtain the name of the source file, and the length of its name, from
    the command string. The name of the source file is the first parameter. */
    pcParameter1 = FreeRTOS_CLIGetParameter
                        (
                          /* The command string itself. */
                          pcCommandString,
                          /* Return the first parameter. */
                          1,
                          /* Store the parameter string length. */
                          &xParameter1StringLength
                        );

	int task = atoi (pcParameter1);
	
	TaskHandle_t xTaskToDelete;

	if (task == 0 && xSimulatorCagTaskHandle != NULL) {
		xTaskToDelete = xSimulatorCagTaskHandle;
		xSimulatorCagTaskHandle = NULL; // so that we can create the task again
		vTaskDelete( xTaskToDelete );
		debug_log("Deleted the simulator task\n\r");
	} else if( task == 1 && xJoystickCagTaskHandle != NULL ) {// check character is a number
		xTaskToDelete = xJoystickCagTaskHandle;
		xJoystickCagTaskHandle = NULL; // so that we can create the task again
		vTaskDelete( xTaskToDelete );
		debug_log("Deleted the joystick task\n\r");

	}
	/* Return pdFALSE, as there are no more strings to return */
	/* Only return pdTRUE, if more strings need to be printed */
	xWriteBufferLen = sprintf((char *) pcWriteBuffer, "");
	return pdFALSE;
}

/*
 * delete tasks Command.
 */
static BaseType_t prvCreCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString ) {



	int8_t *pcParameter1;
	BaseType_t xParameter1StringLength;
	caMessage_t sendCaMessage;

    /* Obtain the name of the source file, and the length of its name, from
    the command string. The name of the source file is the first parameter. */
    pcParameter1 = FreeRTOS_CLIGetParameter
                        (
                          /* The command string itself. */
                          pcCommandString,
                          /* Return the first parameter. */
                          1,
                          /* Store the parameter string length. */
                          &xParameter1StringLength
                        );

	int task = atoi (pcParameter1);

	if (task == 0 && xSimulatorCagTaskHandle == NULL) {
		s4375116_tsk_cag_simulator_init();
		debug_log("Created the simulator task\n\r");
	} else if( task == 1 && xJoystickCagTaskHandle == NULL ) {// check character is a number
		s4375116_tsk_cag_joystick_init();
		debug_log("Created the joystick task\n\r");
	}
	
	/* Return pdFALSE, as there are no more strings to return */
	/* Only return pdTRUE, if more strings need to be printed */
	xWriteBufferLen = sprintf((char *) pcWriteBuffer, "");
	return pdFALSE;
}
