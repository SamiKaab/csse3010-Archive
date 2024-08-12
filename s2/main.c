/**
  ******************************************************************************
  * @file    s2/main.c
  * @author  Sami Kaab s43751167
  * @date    16032022
  * @brief    
  ******************************************************************************
  */

#include "board.h"
#include "processor_hal.h"
#include "debug_log.h"
#include "s4375116_lta1000g.h"
#include "s4375116_joystick.h"
#include "s4375116_pantilt.h"

#define MAX_ABS_ANGLE   90    // Pan and Tilt servo motor angle absolute range
#define ADC_MAX         4095  // ADC Max value

void hardware_init(void);

/**
 * Linearly convert the adc value into an angle after 
 * checking the adc value is not out off bound due to offset correction
 */
int adc_to_angle(int adc_val) {

  // Lineraly convert adc value to angle within defined range
  int angle = -1 * (adc_val * (2 * MAX_ABS_ANGLE)/ADC_MAX - MAX_ABS_ANGLE);

  // The angle value maybe out of bound due to applied offset
  if (angle > MAX_ABS_ANGLE) {

    angle = MAX_ABS_ANGLE;
  } else if (adc_val < - MAX_ABS_ANGLE) {

    angle = -MAX_ABS_ANGLE;
  }
  return angle;
}

int main(void)  {
  
  // Holds the number specifiying which LED to light up
	unsigned short led_val = 0; 

  unsigned int adc_value_x; // Holds the adc value for the x axis
	unsigned int adc_value_y; // Holds the adc value for the y axis

  // Variable used to specify which motor to write to or read from
  int pan = 0;
  int tilt = 1;

  // Initialise pan and tilt angles
  int pan_angle = 0;
  int tilt_angle = 0;

  // Last time the LED bar was refresh
  uint16_t last_refresh_time = HAL_GetTick();

  int dutycycle = 7.25;

	HAL_Init();			  //Initialise board.

	hardware_init();	//Initialise hardware modules

	// Main processing loop
  while (1) {

    // Get x and y adc value from the joystick
    adc_value_x = S4375116_REG_JOYSTICK_X_READ();
    adc_value_y = S4375116_REG_JOYSTICK_Y_READ();
  
    // Convert ADC value to corresponding angle
    pan_angle = adc_to_angle(adc_value_x);
    tilt_angle = adc_to_angle(adc_value_y);
    // debug_log("x=%d, y=%d, pan=%d, tilt=%d\r\n", adc_value_x, adc_value_y, pan_angle, tilt_angle);
    
    // Move specified motor to specified angle
    s4375116_pantilt_angle_write(pan, pan_angle); 
    s4375116_pantilt_angle_write(tilt, tilt_angle);

    if (HAL_GetTick() - last_refresh_time >= 10) { // Refresh LED bar every 10 ms

      last_refresh_time = HAL_GetTick();

      // Select which led bar lights up by converting read angle from -90 to 90, to 0 to 180 degrees
      led_val = (0x01 << ((90-s4375116_pantilt_angle_read(pan))/20));

      // debug_log("angle = %d\t%d\n\r",pan_angle, ((90+pan_angle)/20));

      // Display binary value on LED Bar
      s4375116_reg_lta1000g_write(led_val); 
    }
  
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

  // Initialise joystick x and y adc pins
  s4375116_reg_joystick_init();

  // Initialise pan and tilt PWM pins
  s4375116_reg_pantilt_init();
}