 /**
 **************************************************************
 * @file mylib/s4375116_CAG_simulator.c
 * @author Sami Kaab - s4375116 
 * @date 13052022
 * @brief the functions necessary to simulate the game of life and send the output the the CAG display
 * REFERENCE: csse3010_project.pdf
 ***************************************************************
 * EXTERNAL FUNCTIONS
 ***************************************************************
 * s4375116_tsk_cag_simulator_init() - create the game of life simulator controlling task
 ***************************************************************
 */

#include "board.h"
#include "processor_hal.h"
#include "debug_log.h"
#include "fonts.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "oled_pixel.h"
#include "oled_string.h"

#include "s4375116_oled.h"
#include "s4375116_CAG_display.h"
#include "s4375116_CAG_simulator.h"
#include "s4375116_CAG_grid.h"
#include "s4375116_CAG_joystick.h"
#include "s4375116_CAG_mnemonic.h"

static uint8_t stop = 1;
static uint8_t updateTime = 1;

/**
 * @brief Clear the grid by replacing all its values with zeroes
 * 
 */
void clear_grid(void){                                                           
    for(int y = 0; y < GRID_HEIGHT; y++) {                                           
        for(int x = 0; x < GRID_WIDTH; x++) {                                        
            GRID[y][x] = 0;                                                     
        }                                                                       
    }                                                                           
}

/**
 * @brief Initialise the array containg the sum of live neighbours of a cell to zeroe.
 * 
 */
void clear_pattern(void){                                                           
    for(int y = 0; y < GRID_HEIGHT; y++) {                                           
        for(int x = 0; x < GRID_WIDTH; x++) {                                        
            PATTERN[y][x] = 0;                                                     
        }                                                                       
    }                                                                           
}                

/**
 * @brief print the given grid to the consol
 * 
 * @param grid 
 * @param showNums 
 */
void printGrid(int grid[GRID_HEIGHT][GRID_WIDTH], int showNums){                                           
    int alive;     
    for(int x = 0; x < GRID_WIDTH; x++) {                                       
        debug_log("___");                                                          
    }                           

    for(int y = 0; y < GRID_HEIGHT; y++) {                                           
        for(int x = 0; x < GRID_WIDTH; x++) {                                        
            alive = grid[y][x];                             
            if (!showNums) {
                if(alive) {                                                         
                    debug_log(" # ");                                                  
                } else {                                                            
                    debug_log("   ");                                                  
                }
            } else {
                debug_log(" %d ",alive);
            }         
        }                                                                
        debug_log("|\n\r");                                                           
    }   
    for(int x = 0; x < GRID_WIDTH; x++) {                                       
        debug_log("___");                                                          
    }      
    debug_log("\n\r");                                                                           
}                                                                               

/**
 * @brief Compute the matrix which values are to the sum of live neighbours surrounding the corresponding cell in the Grid
 * 
 */
void update_pattern(void){                                                         
    int xi = 0;                                                                 
    int yj = 0;                                                                 
    int sum = 0; 
	//iterate through every cell
    for(int y = 0; y < GRID_HEIGHT; y++) {                                         
        for(int x = 0; x < GRID_WIDTH; x++) {                                      
            sum = 0;             
			//iterate through cell directly surrounding current cell                                               
            for(int j = -1; j < 2; j++) {                                       
                for(int i = -1; i < 2; i++) {                                   
                    xi = x + i;                                                 
                    yj = y + j;     	                      
					// check that the neighbouring cell is not out of bound and that it is not the current cell                 
                    if((yj >= 0) && (yj < GRID_HEIGHT) && (xi >= 0) && (xi < GRID_WIDTH) && ((x != xi) || (y != yj))) {                   
                        sum = sum + GRID[yj][xi]; // add value of the neighbouring cell to the pattern sum
                    } 
                }                                                               
            }        
			PATTERN[y][x] = sum;                                                
        }           
	}
}                                                                               

/**
 * @brief Use the calculated pattern of the current grid to compute the next state of the grid
 * 
 */
void update_GRID(void) {                                                        
    int alive = 0;   
    int sum = 0;           
    //iterate through every cell                                                
    for(int y = 0; y < GRID_HEIGHT; y++) {                                           
        for(int x = 0; x < GRID_WIDTH; x++) {                                        
            alive = GRID[y][x];
            sum = PATTERN[y][x];
            // if the cell is alive and does not have exactly 2 or 3 live neighbours
            if (alive && (sum < 2 || sum > 3)) {            
                //cell dies of over or under population
                GRID[y][x] = 0;                                                 
            } else if (!alive && sum == 3) { // a dead cell with exactly 3 live neighbours....
                //...comes to life               
                GRID[y][x] = 1;                                                 
            } // else live cells lives and dead cells stay dead                                              
        }                                                                       
    }                                                                           
}                                                                               

/**
 * @brief Copy the game of life grid to the specified message struct.
 * 
 * @param message Struct sent to the CAG display
 */
void send_grid_to_display(void) {
    for(int y = 0; y < GRID_HEIGHT; y++) {                                           
        for(int x = 0; x < GRID_WIDTH; x++) {                                        
            if (GRID[y][x] == 1) {            
                sendGridMsg.grid[y][x] = 1;                                                 
            } else {                          
                sendGridMsg.grid[y][x] = 0;                                                 
            }                                                                   
        }                                                                       
    }
    //send grid to display
    if (CAGDisplayMessageQueue != NULL) {	// Check if queue exists 
        xQueueSendToFront(CAGDisplayMessageQueue, ( void * ) &sendGridMsg, ( portTickType ) 10 );
    }
}

void add_life(int x, int y,int *grid) {                                                     
                                                                                                                                        
    for (int j = 0; j < LIFE_DIM; j++){                                                
        for (int i = 0; i < LIFE_DIM; i++){    
            int val = *((grid+j*LIFE_DIM) + i);
            if(val){
                GRID[j + y][i + x] = val;
            }                                    
                
        }                                                                       
    }                                                                           
} 

/**
 * @brief sets the appropriate value in grid depending on the message received
 * 
 * @param rcvdCaMessage te message struct
 */
void process_grid_message(void) {
    if(rcvdCaMessage.x < GRID_WIDTH && rcvdCaMessage.x >= 0 && rcvdCaMessage.y < GRID_HEIGHT && rcvdCaMessage.y >= 0) {
        if (rcvdCaMessage.type == 0x10) {
            GRID[rcvdCaMessage.y][rcvdCaMessage.x] = 0;
            debug_log("Kill cell at (%d, %d)\n\r",rcvdCaMessage.x,rcvdCaMessage.y);
        } else if (rcvdCaMessage.type == 0x11) {
            GRID[rcvdCaMessage.y][rcvdCaMessage.x] = 1;
            debug_log("Spawn cell at (%d, %d)\n\r",rcvdCaMessage.x,rcvdCaMessage.y);
        } else if (rcvdCaMessage.type == 0x20) {
            add_life(rcvdCaMessage.x,rcvdCaMessage.y,(int *)BLOCK);
            debug_log("Spawn Block at (%d, %d)\n\r",rcvdCaMessage.x,rcvdCaMessage.y);
        } else if (rcvdCaMessage.type == 0x21) {
            add_life(rcvdCaMessage.x,rcvdCaMessage.y,(int *)BEEHIVE);
            debug_log("Spawn Beehive at (%d, %d)\n\r",rcvdCaMessage.x,rcvdCaMessage.y);
        } else if (rcvdCaMessage.type == 0x22) {
            add_life(rcvdCaMessage.x,rcvdCaMessage.y,(int *)LOAF);
            debug_log("Spawn Loaf at (%d, %d)\n\r",rcvdCaMessage.x,rcvdCaMessage.y);
        } else if (rcvdCaMessage.type == 0x30) {
            add_life(rcvdCaMessage.x,rcvdCaMessage.y,(int *)BLINKER);
            debug_log("Spawn Blinker at (%d, %d)\n\r",rcvdCaMessage.x,rcvdCaMessage.y);
        } else if (rcvdCaMessage.type == 0x31) {
            add_life(rcvdCaMessage.x,rcvdCaMessage.y,(int *)TOAD);
            debug_log("Spawn Toad at (%d, %d)\n\r",rcvdCaMessage.x,rcvdCaMessage.y);
        } else if (rcvdCaMessage.type == 0x32) {
            add_life(rcvdCaMessage.x,rcvdCaMessage.y,(int *)BEACON);
            debug_log("Spawn Beacon at (%d, %d)\n\r",rcvdCaMessage.x,rcvdCaMessage.y);
        }  else if (rcvdCaMessage.type == 0x40) {
            add_life(rcvdCaMessage.x,rcvdCaMessage.y,(int *)GLIDER);
            debug_log("Spawn Glider at (%d, %d)\n\r",rcvdCaMessage.x,rcvdCaMessage.y);
        }  else {
            debug_log("Invalid life form\r\n");
        }
    } else {
        debug_log("Invalid position\r\n");
    }
}

/**
 * @brief Reads from the cag simulator message queue and processes the message
 * 
 * @param rcvdCaMessage the message struct
 */
void read_queue(void){
    // receive message from CAG_grid or ....to add later!
    if (CAGSimulatorMessageQueue != NULL) {	// Check if queue exists

        // Check for item received - block atmost for 10 ticks
        if (xQueueReceive( CAGSimulatorMessageQueue, &rcvdCaMessage, 10 )) {
            process_grid_message();
        }
    } 
}


void print_binary(int num,int nbbits) {
  debug_log("\n\r");
  for(int i = nbbits-1; i >= 0; i--) {
    if((num >> i) & 0x1){
      debug_log("1");
    } else {
      debug_log("0");
    }

  }
  debug_log("\n\r");
}

/**
 * @brief handle event bits by executing the correct function based on which bit is set
 * 
 * @param uxBits pointer to an event bit
 * @param rcvdCaMessage the message struct
 */
void handle_event_bits (void) {
    uxBits = xEventGroupWaitBits(gridctrlEventGroup, GRID_CTRL_EVENT, pdTRUE, pdFALSE, 10);
    // Check if clear grid Event flag is set
    if ((uxBits & EVT_CLR_GRID) != 0) {
        clear_grid(); 
        send_grid_to_display();
        uxBits = xEventGroupClearBits(gridctrlEventGroup, EVT_CLR_GRID);		//Clear LED ON event flat.
        debug_log("Grid Cleared\r\n");
 }
    // Check if start stop grid Event flag is set
    if ((uxBits & EVT_START_STOP) != 0) {
        stop = !stop;
        uxBits = xEventGroupClearBits(gridctrlEventGroup, EVT_START_STOP);		//Clear LED ON event flat.
        debug_log("Simulation %s\r\n", stop ? "stopped" : "running");
        }
    // Check if select cell grid Event flag is set
    if ((uxBits & EVT_SLCT_CELL) != 0) {
        read_queue();
        send_grid_to_display();
        uxBits = xEventGroupClearBits(gridctrlEventGroup, EVT_SLCT_CELL);		//Clear LED ON event flat.
    }
    // Check if unselect cell grid Event flag is set
    if ((uxBits & EVT_USLCT_CELL) != 0) {
        read_queue(); //read message from cag grid
        send_grid_to_display();
        uxBits = xEventGroupClearBits(gridctrlEventGroup, EVT_USLCT_CELL);		//Clear LED ON event flat.
    }

    uxBits = xEventGroupWaitBits(joystickctrlEventGroup, JOYSTICK_CTRL_EVENT, pdTRUE, pdFALSE, 10);
    
    // Check if start joystick Event flag is set
    if ((uxBits & EVT_START) != 0) {
        stop = 0;
        uxBits = xEventGroupClearBits(joystickctrlEventGroup, EVT_START);		//Clear LED ON event flat.
        debug_log("Simulation running\r\n");
    }
    // Check if stop joystick Event flag is set
    if ((uxBits & EVT_STOP) != 0) {
        stop = 1; // stop becomes true
        uxBits = xEventGroupClearBits(joystickctrlEventGroup, EVT_STOP);		//Clear LED ON event flat.
        debug_log("Simulation stopped\r\n");
    }
    // Check if set update time to 1 joystick Event flag is set
    if ((uxBits & EVT_UPDATE_TIME_1) != 0) {
        updateTime = 1;
        uxBits = xEventGroupClearBits(joystickctrlEventGroup, EVT_UPDATE_TIME_1);		//Clear LED ON event flat.
        debug_log("The simulation updates every second\n\r");
    }
    // Check if set update time to 2 joystick Event flag is set
    if ((uxBits & EVT_UPDATE_TIME_2) != 0) {
        updateTime = 2;
        uxBits = xEventGroupClearBits(joystickctrlEventGroup, EVT_UPDATE_TIME_2);		//Clear LED ON event flat.
        debug_log("The simulation updates every 2 seconds\n\r");
    }
    // Check if set update time to 2 joystick Event flag is set
    if ((uxBits & EVT_UPDATE_TIME_5) != 0) {
        updateTime = 5;
        uxBits = xEventGroupClearBits(joystickctrlEventGroup, EVT_UPDATE_TIME_5);		//Clear LED ON event flat.
        debug_log("The simulation updates every 5 seconds\n\r");
    }
    // Check if set update time to 10 joystick Event flag is set
    if ((uxBits & EVT_UPDATE_TIME_10) != 0) {
        updateTime = 10;
        uxBits = xEventGroupClearBits(joystickctrlEventGroup, EVT_UPDATE_TIME_10);		//Clear LED ON event flat.
        debug_log("The simulation updates every 10 second\n\r");
    }


    if (drawLifeSemaphore != NULL) {	// Check if semaphore exists
        /* See if we can obtain the PB semaphore. If the semaphore is not available
        wait 10 ticks to see if it becomes free. */
        if( xSemaphoreTake( drawLifeSemaphore, 10 ) == pdTRUE ) {
            read_queue();
            send_grid_to_display();
        }
    }
}

/**
 * @brief cyclic executive for running game of life simulation and sending grid to display
 * 
 */
void s4375116TaskCAGSimulator( void ) {

    portDISABLE_INTERRUPTS();	//Disable interrupts
	
    BRD_debuguart_init();  //Initialise UART for debug log output

	portENABLE_INTERRUPTS();	//Enable interrupts
	
	
	//queue to receive the game of life grid
	CAGSimulatorMessageQueue = xQueueCreate(5, sizeof(rcvdCaMessage));		// Create queue of length 10 Message items 


    // Initialise grid and pattern
	clear_grid();        
	clear_pattern();   


    
    add_life(2,2,(int *)BLOCK);
    add_life(10,2,(int *)BEEHIVE);
    add_life(20,2,(int *)LOAF);
    add_life(30,2,(int *)BLINKER);
    add_life(40,2,(int *)TOAD);
    add_life(50,2,(int *)BEACON);
    add_life(2,7,(int *)GLIDER);

    TickType_t currTick = xTaskGetTickCount();
    TickType_t lastTick = xTaskGetTickCount();
    send_grid_to_display();
    

	for (;;) {

        handle_event_bits();
        


        if ((currTick-lastTick) > ((updateTime) * configTICK_RATE_HZ) && !stop) { // check that a the time passed is greater than update time and we're not in paused mode
            // debug_log("UPDATE\r\n");
            update_pattern();        
            update_GRID(); 
            send_grid_to_display();

            lastTick = currTick;
        }
        currTick = xTaskGetTickCount();
        

		// Wait for 1000ms 
		vTaskDelay(10);
	}
}

/**
 * @brief create the task that runs the game of life simulation
 * 
 */
void s4375116_tsk_cag_simulator_init(void) {

	xTaskCreate( (void *) &s4375116TaskCAGSimulator, (const signed char *) "CAGSIMULATOR", CAGSIMULATORTASK_STACK_SIZE, NULL, CAGSIMULATORTASK_PRIORITY, &xSimulatorCagTaskHandle );

}