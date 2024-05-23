 /** 
 **************************************************************
 * @file mylib/s4375116_pantilt.h
 * @author Sami Kaab - s4375116 
 * @date 18032022
 * @brief Pan and Tilt Register Driver Functions
 * REFERENCE: csse3010_mylib_reg_pantilt.pdf,  stm32f429zi_reference.pdf
 ***************************************************************
 * EXTERNAL FUNCTIONS 
 ***************************************************************
 * s4375116_reg_pantilt_init() - Initialize pins D6 and D7 to be pwm output
 * s4375116_pantilt_angle_write() - Move specied motor to specified angle 
 * s4375116_pantilt_angle_read() - Read specified motor angle
 *************************************************************** 
 */

#ifndef S4375116_PANTILT_H
#define S4375116_PANTILT_H

#define S4375116_REG_PANTILT_PAN_90_CAL_OFFSET 30
#define S4375116_REG_PANTILT_TILT_90_CAL_OFFSET -10

void s4375116_reg_pantilt_init(void);
void s4375116_pantilt_angle_write(int type, int angle);
int s4375116_pantilt_angle_read(int type);

#endif