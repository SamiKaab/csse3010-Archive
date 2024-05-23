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


#ifndef S4375116_CAG_SIMULATOR_H
#define S4375116_CAG_SIMULATOR_H

#include "event_groups.h"


// Task Priorities 
#define CAGSIMULATORTASK_PRIORITY			( tskIDLE_PRIORITY + 2 )

// Task Stack Allocations 
#define CAGSIMULATORTASK_STACK_SIZE		( configMINIMAL_STACK_SIZE * 5 )

#define LIFE_DIM 4 

int GRID[GRID_HEIGHT][GRID_WIDTH]; //holds cell values                                                        
int PATTERN[GRID_HEIGHT][GRID_WIDTH]; //each value is the number of live neighbours to the corresponding cell in GRID

static int BLOCK[LIFE_DIM][LIFE_DIM] = {                                                         
    {1,1,0,0},                                                                  
    {1,1,0,0},
    {0,0,0,0},
    {0,0,0,0}                                     
};  

static int BEEHIVE[LIFE_DIM][LIFE_DIM] = {                                                         
    {0,1,1,0},
    {1,0,0,1},                                                                  
    {0,1,1,0},
    {0,0,0,0}                                                            
};    

static int LOAF[LIFE_DIM][LIFE_DIM] = {                                                         
    {0,1,1,0},
    {1,0,0,1},
    {0,1,0,1},
    {0,0,1,0}                                                                   
};                                                                    

static int BLINKER[LIFE_DIM][LIFE_DIM] = {
    {0,1,0,0},
    {0,1,0,0},
    {0,1,0,0},
    {0,0,0,0},
};

static int TOAD[LIFE_DIM][LIFE_DIM] = {
    {0,1,1,1},
    {1,1,1,0},
    {0,0,0,0},
    {0,0,0,0},
};
static int BEACON[LIFE_DIM][LIFE_DIM] = {
    {1,1,0,0},
    {1,1,0,0},
    {0,0,1,1},
    {0,0,1,1},
};

static int GLIDER[LIFE_DIM][LIFE_DIM] = {                                                        
    {0,0,1,0},                                                                
    {1,0,1,0},                                                                
    {0,1,1,0},
    {0,0,0,0}
};    


static int glider_gun[9][36] = {                                                   
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1},
    {0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1},
    {1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {1,1,0,0,0,0,0,0,0,0,1,0,0,0,1,0,1,1,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
}; 


struct caMessage {
	int type;
	int x;
	int y;
};
typedef struct caMessage caMessage_t;

QueueHandle_t CAGSimulatorMessageQueue;	// Queue used
TaskHandle_t xSimulatorCagTaskHandle;

CagDisplayTextMsg sendGridMsg; //message struct sent to CAG_display
caMessage_t rcvdCaMessage; // message struct which holds the cell/life form to create
EventBits_t uxBits;


void s4375116_tsk_cag_simulator_init(void);

#endif