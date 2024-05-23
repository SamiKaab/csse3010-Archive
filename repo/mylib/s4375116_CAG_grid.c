 /**
 **************************************************************
 * @file mylib/s4375116_CAG_grid.c
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


#include "s4375116_CAG_grid.h"

/* The last time (ms) a rising edge was detected */
static TickType_t previous_pb_rising_edge_trigger = 0;
/* The last time (ms) a rising edge was detected */
static TickType_t previous_pb_falling_edge_trigger = 0;
/* variable to keep track of the grid mode and whether we should read input */
static uint8_t gridMode = 1;

/*
 * Initialise onboard push button Hardware
 */
void onboard_pushbutton_hardware_init(void) {

	// Enable GPIO Clock
	__GPIOC_CLK_ENABLE();

    GPIOC->OSPEEDR |= (GPIO_SPEED_FAST << 13);	//Set fast speed.
	GPIOC->PUPDR &= ~(0x03 << (13 * 2));			//Clear bits for no push/pull
	GPIOC->MODER &= ~(0x03 << (13 * 2));			//Clear bits for input mode

	// Enable EXTI clock
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

	//select trigger source (port c, pin 13) on EXTICR4.
	SYSCFG->EXTICR[3] &= ~SYSCFG_EXTICR4_EXTI13;
	SYSCFG->EXTICR[3] |= SYSCFG_EXTICR4_EXTI13_PC;

	EXTI->RTSR |= EXTI_RTSR_TR13;	//enable rising dedge
	EXTI->FTSR |= EXTI_FTSR_TR13;	//disable falling edge
	EXTI->IMR |= EXTI_IMR_IM13;		//Enable external interrupt

	//Enable priority (10) and interrupt callback. Do not set a priority lower than 5.
	HAL_NVIC_SetPriority(EXTI15_10_IRQn, 10, 0);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/*
 * Push Button callback
 */
void pb_callback(uint16_t GPIO_Pin) {

	BaseType_t xHigherPriorityTaskWoken; //RTOS related

	// Check if the pushbutton pin was pressed.
	if ((GPIOC->IDR & (0x0001 << 13))) {
		previous_pb_rising_edge_trigger = xTaskGetTickCountFromISR();

		if (previous_pb_falling_edge_trigger-previous_pb_rising_edge_trigger > 400) {

			xHigherPriorityTaskWoken = pdFALSE;

			BRD_LEDGreenToggle(); 		//Toggle Blue LED
			gridMode = !gridMode;
			
			// give semaphore to clear the grid
			if (gridModeSemaphore != NULL) {	// Check if semaphore exists 
				xSemaphoreGiveFromISR( gridModeSemaphore, &xHigherPriorityTaskWoken );		
			}

			// Perform context switching, if required.
			portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
		}


	} else { // C13 is low (falling edge)

		previous_pb_falling_edge_trigger = xTaskGetTickCountFromISR();
	}
}

/*
 * Interrupt handler (ISR) for EXTI 15 to 10 IRQ Handler
 * Note ISR should only execute a callback
 */ 
void EXTI15_10_IRQHandler(void) {

	NVIC_ClearPendingIRQ(EXTI15_10_IRQn);

	// PR: Pending register
	if ((EXTI->PR & EXTI_PR_PR13) == EXTI_PR_PR13) {

		// cleared by writing a 1 to this bit
		EXTI->PR |= EXTI_PR_PR13;	//Clear interrupt flag.

		pb_callback(13);   // Callback for C13
	}
}

/**
 * @brief initialise the necessary hardware components
 * 
 */
void cag_grid_hardware_init(void) {
		
	portDISABLE_INTERRUPTS();	//Disable interrupts
	
    BRD_debuguart_init();  //Initialise UART for debug log output
	
	BRD_LEDInit();		//Initialise LEDS
	s4375116_reg_lta1000g_init();
	onboard_pushbutton_hardware_init();

	// Turn off LEDs
	BRD_LEDRedOff();
	BRD_LEDGreenOn();
	BRD_LEDBlueOff();

	portENABLE_INTERRUPTS();	//Enable interrupts
}

/**
 * @brief add the message to the queue
 * 
 * @param sendCaMessage the messgae struct
 */
void send_message_to_simulator(void) {
	//send grid to display
	if (CAGSimulatorMessageQueue != NULL) {	// Check if queue exists 
		xQueueSendToBack(CAGSimulatorMessageQueue, ( void * ) &sendCaMessage, ( portTickType ) 10 );
	}
}

/**
 * @brief sets the appropriate event bit depending on which character has been entered
 * 
 * @param currChar the char entered
 * @param sendCaMessage a pointer to the message struct
 * @param uxBits a pointer to the event bits
 */
void process_input(char currChar) {
	switch (currChar)
	{
	case 'W':
		if (sendCaMessage.y > 0) {
			(sendCaMessage.y)--;
		}
		debug_log("Move UP (%d,%d)\r\n", sendCaMessage.x, sendCaMessage.y);
		uxBits = xEventGroupSetBits(gridctrlEventGroup, EVT_MV_UP);

		break;
	case 'A':
		if (sendCaMessage.x > 0) {
			(sendCaMessage.x)--;
		}
		debug_log("Move LEFT (%d,%d)\r\n", sendCaMessage.x, sendCaMessage.y);
		uxBits = xEventGroupSetBits(gridctrlEventGroup, EVT_MV_LEFT);
		break;
	case 'S':
		if (sendCaMessage.y < GRID_HEIGHT) {
			(sendCaMessage.y)++;
		}
		debug_log("Move DOWN (%d,%d)\r\n", sendCaMessage.x, sendCaMessage.y);
		uxBits = xEventGroupSetBits(gridctrlEventGroup, EVT_MV_DOWN);
		break;
	case 'D':
		if (sendCaMessage.x < GRID_WIDTH) {
			(sendCaMessage.x)++;
		}
		debug_log("Move RIGHT (%d,%d)\r\n", sendCaMessage.x, sendCaMessage.y);
		uxBits = xEventGroupSetBits(gridctrlEventGroup, EVT_MV_RIGHT);
		break;
	case 'X':
		sendCaMessage.type = 0x11;
		send_message_to_simulator();
		uxBits = xEventGroupSetBits(gridctrlEventGroup, EVT_SLCT_CELL);
		break;
	case 'Z':
		sendCaMessage.type = 0x10;
		send_message_to_simulator();
		uxBits = xEventGroupSetBits(gridctrlEventGroup, EVT_USLCT_CELL);
		break;
	case 'P':
		uxBits = xEventGroupSetBits(gridctrlEventGroup, EVT_START_STOP);
		break;
	case 'O':
		sendCaMessage.x = 0;
		sendCaMessage.y = 0;
		debug_log("Move to Origin\r\n");
		uxBits = xEventGroupSetBits(gridctrlEventGroup, EVT_MV_ORIGIN);
		break;
	case 'C':
		uxBits = xEventGroupSetBits(gridctrlEventGroup, EVT_CLR_GRID);
		break;
	default:
		debug_log("Invalid Character, use: W, A, S, D, P, X, Z, O or C\r\n");
		break;
	}
}


/**
 * @brief cyclic executive for the cag grid
 * 
 */
void s4375116TaskCAGGrid(void) {

	cag_grid_hardware_init(); // initialise hardware

	s4375116_reg_lta1000g_write(0); // initialise LED Bar to 0

	char currChar;
	sendCaMessage.x = 0;
	sendCaMessage.y = 0;
	sendCaMessage.type = 0;

	// Create Event Group
	gridctrlEventGroup = xEventGroupCreate();
	gridModeSemaphore = xSemaphoreCreateBinary();

	debug_log("Mode: Grid\r\n");

	for(;;) {
		
		if(gridMode) {
			currChar = BRD_debuguart_getc();// get char from keyboard input
		
			//Check that the key received is not null 
			if (currChar != '\0') {
				debug_log("%c -> ",currChar);
				process_input(currChar);
				//display current position on the led bar
				s4375116_reg_lta1000g_write((sendCaMessage.x << 4) | sendCaMessage.y);

			}
		}

		// Delay the task for 1000ms.
		vTaskDelay(100);
	}
}

/**
 * @brief creates the task to run the cyclic executive for the grid control
 * 
 */
void s4375116_tsk_cag_grid_init(void) {

	xTaskCreate( (void *) &s4375116TaskCAGGrid, (const signed char *) "CAGGRID", CAGGRIDTASK_STACK_SIZE, NULL, CAGGRIDTASK_PRIORITY, NULL );

}