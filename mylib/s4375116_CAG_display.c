 /**
 **************************************************************
 * @file mylib/s4375116_CAG_display.c
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

#include "board.h"
#include "processor_hal.h"
#include "debug_log.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "oled_pixel.h"
#include "oled_string.h"
#include "fonts.h"

#include "s4375116_oled.h"
#include "s4375116_CAG_grid.h"
#include "s4375116_CAG_simulator.h"
#include "s4375116_CAG_display.h"

/**
 * @brief Draw border bounding the grid on the oled
 * 
 */
void draw_grid_borders(void) {
    int i = 0;
    //Draw Horizontal lines of boundary box
    for (i=0; i < GRID_WIDTH * CELL_SIZE; i++) {

        ssd1306_DrawPixel(i, 0, SSD1306_WHITE);					//top line
        ssd1306_DrawPixel(i, GRID_HEIGHT*CELL_SIZE-1, SSD1306_WHITE);	//bottom line
    }

    //Draw Vertical lines of boundary box
    for (i=0; i < GRID_HEIGHT * CELL_SIZE; i++) {

        ssd1306_DrawPixel(0, i, SSD1306_WHITE);					//left line
        ssd1306_DrawPixel(GRID_WIDTH*CELL_SIZE-1, i, SSD1306_WHITE);	//right line

    }
}

/**
 * @brief clear Oled display
 * 
 */
void clear_display(void) {
	ssd1306_Fill(Black);
	ssd1306_UpdateScreen();
} 

/**
 * @brief Draws the received grid on the oled
 * 
 * @param RcvdMsg 
 */
void display_to_oled(CagDisplayTextMsg RcvdMsg) {
	//Clear Screen
	ssd1306_Fill(Black);
	for(int y = 0; y < GRID_HEIGHT; y++) {                                           
		for(int x = 0; x < GRID_WIDTH; x++) {  
			// draw the cell which is 2 by 2 pixels
		    ssd1306_DrawPixel(x*CELL_SIZE, y*CELL_SIZE, RcvdMsg.grid[y][x]);
			ssd1306_DrawPixel(x*CELL_SIZE+1, y*CELL_SIZE, RcvdMsg.grid[y][x]);
			ssd1306_DrawPixel(x*CELL_SIZE, y*CELL_SIZE+1, RcvdMsg.grid[y][x]);
			ssd1306_DrawPixel(x*CELL_SIZE+1, y*CELL_SIZE+1, RcvdMsg.grid[y][x]);
		}                                                                       
	} 	
	ssd1306_UpdateScreen();
}

/**
 * @brief Initialises OLED hardware pins
 * and creates the cyclic executive for the oled
 * 
 */
void s4375116TaskCAGDisplay(void) {

	portDISABLE_INTERRUPTS();	//Disable interrupts
	
	s4375116_reg_oled_init();

	portENABLE_INTERRUPTS();	//Enable interrupts

	EventBits_t uxBits;

	CagDisplayTextMsg RcvdMsg; // message struct which holds the grid
	
	//queue to receive the game of life grid
	CAGDisplayMessageQueue = xQueueCreate(1, sizeof(RcvdMsg));		// Create queue of length 10 Message items 

	for(;;) {

		if (CAGDisplayMessageQueue != NULL) {	// Check if queue exists

			// get updated grid from CAG simulator
			if (xQueueReceive( CAGDisplayMessageQueue, &RcvdMsg, 10 )) {
				portDISABLE_INTERRUPTS();	//Disable interrupts
				//drw grid on oled
				display_to_oled(RcvdMsg);
				portENABLE_INTERRUPTS();	//Enable interrupts

        	}
		}

		// Check if clear grid Event flag is set and that the simulator task has been deleted (otherwise the display will be cleared by the simulator)
    	uxBits = xEventGroupWaitBits(gridctrlEventGroup, GRID_CTRL_EVENT, pdTRUE, pdFALSE, 10);
		if (((uxBits & EVT_CLR_GRID) != 0) && (xSimulatorCagTaskHandle == NULL)) {
			
			clear_display();        
			uxBits = xEventGroupClearBits(gridctrlEventGroup, EVT_CLR_GRID);
			
		}

		// Delay the task for 1000ms.
		vTaskDelay(100);
	}
}

/**
 * @brief Creates the task that runs the OLED cylic executive.
 * 
 */
void s4375116_tsk_cag_display_init(void) {

	xTaskCreate( (void *) &s4375116TaskCAGDisplay, (const signed char *) "CAGDISPLAY", CAGDISPLAYTASK_STACK_SIZE, NULL, CAGDISPLAYTASK_PRIORITY, NULL );

}