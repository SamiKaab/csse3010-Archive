/**
  ******************************************************************************
  * @file    s1/main.c
  * @author  Sami Kaab s43751167
  * @date    28022022
  * @brief   Displays the number of times the Joystick pushbutton has been
  *          pressed as a binary number on the LED Bar 
  ******************************************************************************
  */

#include "board.h"
#include "processor_hal.h"
#include "s4375116_lta1000g.h"
#include "s4375116_joystick.h"
#include "debug_log.h"


void hardware_init(void);

int main(void)  {
  
  // Counter to keep track of the number of time the pushbutton has been pressed
	unsigned short press_counter; 

  // Last time the LED bar was refresh
  uint32_t last_refresh_time = HAL_GetTick();

	HAL_Init();			  //Initialise board.

	hardware_init();	//Initialise hardware modules

	// Main processing loop
  while (1) {

    // Get number of time the Joystick button has been pressed
    press_counter = s4375116_reg_joystick_press_get();


    if (last_refresh_time - HAL_GetTick() >= 10) {

      // Display binary value on LED Bar
      s4375116_reg_lta1000g_write(press_counter); 
      debug_log("%d\n\r", press_counter);

      
      last_refresh_time = HAL_GetTick();
    }

    // HAL_Delay(10); //Wait for 10ms
  }

  return 0;
}

/*
 * Initialise Hardware
 */
void hardware_init(void) {

  BRD_debuguart_init();  //Initialise UART for debug log output

  // Initialise LED bar
  s4375116_reg_lta1000g_init();

  // Initialise joystick pushbutton interrupt
	s4375116_reg_joystick_pb_init();

  // Set joystick pushbutton pressed number to 0
  s4375116_reg_joystick_press_reset();

}