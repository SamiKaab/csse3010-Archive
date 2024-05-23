/** 
 **************************************************************
 * @file mylib/s4375116_lta1000g.h
 * @author Sami Kaab - s4375116 
 * @date 25022022
 * @brief LTA1000g LED bar display register driver functions
 * REFERENCE: LTA-1000G_LIGHTBAR.pdf, csse3010_mylib_ref_lta1000g.pdf 
 ***************************************************************
 * EXTERNAL FUNCTIONS 
 ***************************************************************
 * s4375116_reg_lta1000g_init() - intialise LED bar GPIO pins ad outputs
 * s4375116_reg_lta1000g_write() - Write the LED Bar segments high or low
 *************************************************************** 
 */

#ifndef S4375116_LTA1000G_H
#define S4375116_LTA1000G_H

void s4375116_reg_lta1000g_init(void);
void s4375116_reg_lta1000g_write(unsigned short value);

#endif