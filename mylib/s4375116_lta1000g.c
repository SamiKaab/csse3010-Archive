 /** 
 **************************************************************
 * @file mylib/s4375116_lta1000g.c
 * @author Sami Kaab - s4375116 
 * @date 25022022
 * @brief LTA1000g LED bar display register driver functions
 * REFERENCE: LTA-1000G_LIGHTBAR.pdf, csse3010_mylib_ref_lta1000g.pdf 
 ***************************************************************
 * EXTERNAL FUNCTIONS 
 ***************************************************************
 * s4375116_reg_lta1000g_init() - intialise LED bar GPIO pins as outputs
 * s4375116_reg_lta1000g_write() - Write the LED Bar segments high or low
 *************************************************************** 
 */

#include "board.h"
#include "processor_hal.h"
#include "s4375116_lta1000g.h"
/*
 * Initialises GPIO pins D16 to D25 as outputs
 */
void s4375116_reg_lta1000g_init(void) {

    // Enable the GPIO A, B and C Clock
    __GPIOA_CLK_ENABLE();
    __GPIOB_CLK_ENABLE();
    __GPIOC_CLK_ENABLE();

    //Initialise A4 and A15 as an output.
    GPIOA->MODER &= ~((0x03 << (4 * 2)) | (0x03 << (15 * 2)));  //clear bits
    GPIOA->MODER |= ((0x01 << (4 * 2)) | (0x01 << (15 * 2)));   //Set for push pull

    GPIOA->OSPEEDR &= ~((0x03<<(4 * 2)) | (0x03<<(15 * 2)));
    GPIOA->OSPEEDR |=   ((0x02<<(4 * 2)) | (0x02<<(15 * 2)));  // Set for Fast speed

    GPIOA->OTYPER &= ~((0x01 << 4) | (0x01 << 15));       //Clear Bit for Push/Pull utput

    // Activate the Pull-up or Pull down resistor for the current IO
    GPIOA->PUPDR &= ~((0x03 << (4 * 2)) | (0x03 << (15 * 2)));   //Clear Bits
    GPIOA->PUPDR |= (((0x01) << (4 * 2)) | ((0x01) << (15 * 2)));  //Set for Pull down output

    //Initialise B3, B4, B5, B12, B13 and B15 as outputs.
    GPIOB->MODER &= ~((0x03 << (3 * 2)) | 
                        (0x03 << (4 * 2)) | 
                        (0x03 << (5 * 2)) | 
                        (0x03 << (12 * 2)) | 
                        (0x03 << (13 * 2)) | 
                        (0x03 << (15 * 2)));    //clear bits
    GPIOB->MODER |= ((0x01 << (3 * 2)) | 
                        (0x01 << (4 * 2)) | 
                        (0x01 << (5 * 2)) | 
                        (0x01 << (12 * 2)) | 
                        (0x01 << (13 * 2)) | 
                        (0x01 << (15 * 2)));    //Set for push pull

    GPIOB->OSPEEDR &= ~((0x03 << (3 * 2)) | 
                            (0x03 << (4 * 2)) | 
                            (0x03 << (5 * 2)) | 
                            (0x03 << (12 * 2)) | 
                            (0x03 << (13 * 2)) | 
                            (0x03 << (15 * 2)));

    GPIOB->OSPEEDR |= ((0x02 << (3 * 2)) | 
                        (0x02 << (4 * 2)) | 
                        (0x02 << (5 * 2)) | 
                        (0x02 << (12 * 2)) | 
                        (0x02 << (13 * 2)) | 
                        (0x02 << (15 * 2)));    // Set for Fast speed

    GPIOB->OTYPER &= ~((0x01 << 3) | 
                        (0x01 << 4) | 
                        (0x01 << 5) | 
                        (0x01 << 12) | 
                        (0x01 << 13) | 
                        (0x01 << 15));  //Clear Bit for Push/Pull utput

    // Activate the Pull-up or Pull down resistor for the current IO
    GPIOB->PUPDR &= ~((0x03 << (3 * 2)) | 
                        (0x03 << (4 * 2)) | 
                        (0x03 << (5 * 2)) | 
                        (0x03 << (12 * 2)) | 
                        (0x03 << (13 * 2)) | 
                        (0x03 << (15 * 2)));    //Clear Bits

    GPIOB->PUPDR |= ((0x01 << (3 * 2)) | 
                        (0x01 << (4 * 2)) | 
                        (0x01 << (5 * 2)) | 
                        (0x01 << (12 * 2)) | 
                        (0x01 << (13 * 2)) | 
                        (0x01 << (15 * 2)));    //Set for Pull down output
    
    //Initialise C6 and C7 as outputs.
    GPIOC->MODER &= ~((0x03 << (6 * 2)) | (0x03 << (7 * 2)));   //clear bits
    GPIOC->MODER |= ((0x01 << (6 * 2)) | (0x01 << (7 * 2)));    //Set for push pull

    GPIOC->OSPEEDR &= ~((0x03<<(6 * 2)) | (0x03<<(7 * 2)));
    GPIOC->OSPEEDR |=   ((0x02<<(6 * 2)) | (0x02<<(7 * 2)));    // Set for Fast speed

    GPIOC->OTYPER &= ~((0x01 << 6) | (0x01 << 7));              //Clear Bit for Push/Pull utput

    // Activate the Pull-up or Pull down resistor for the current IO
    GPIOA->PUPDR &= ~((0x03 << (4 * 2)) | (0x03 << (7 * 2)));       //Clear Bits
    GPIOA->PUPDR |= (((0x01) << (4 * 2)) | ((0x01) << (7 * 2)));    //Set for Pull down output

}

/*
 * Set the specified segement (0 to 9) on the LED to the specified value (0 or 1)
 */
void lta1000g_seg_set(int segment, unsigned char value) {

    // Check whether to set the LED Bar high or low
    if (value) {

        // Decide which pin to set to high based on the segment value
        switch (segment) {
            case 0:

                GPIOC->ODR |= (0x01 << 6);
                break;

            case 1:

                GPIOB->ODR |= (0x01 << 15);
                break;

            case 2:

                GPIOB->ODR |= (0x01 << 13);
                break;

            case 3:

                GPIOB->ODR |= (0x01 << 12);
                break;

            case 4:

                GPIOA->ODR |= (0x01 << 15);
                break;

            case 5:

                GPIOC->ODR |= (0x01 << 7);
                break;

            case 6:

                GPIOB->ODR |= (0x01 << 5); 
                break;

            case 7:

                GPIOB->ODR |= (0x01 << 3);
                break;

            case 8:

                GPIOA->ODR |= (0x01 << 4);   
                break;

            case 9:

                GPIOB->ODR |= (0x01 << 4);   
                break;        
        }

    } else {
        
        // Decide which pin to set to low based on the segment value
        switch (segment) {
        
            case 0:
        
                GPIOC->ODR &= ~(0x01 << 6);
                break;
        
            case 1:
        
                GPIOB->ODR &= ~(0x01 << 15);
                break;
        
            case 2:
        
                GPIOB->ODR &= ~(0x01 << 13);
                break;
        
            case 3:
        
                GPIOB->ODR &= ~(0x01 << 12);
                break;
        
            case 4:
        
                GPIOA->ODR &= ~(0x01 << 15);
                break;
        
            case 5:
        
                GPIOC->ODR &= ~(0x01 << 7);
                break;
        
            case 6:
        
                GPIOB->ODR &= ~(0x01 << 5); 
                break;
        
            case 7:
        
                GPIOB->ODR &= ~(0x01 << 3);
                break;
        
            case 8:
        
                GPIOA->ODR &= ~(0x01 << 4);   
                break;
        
            case 9:
        
                GPIOB->ODR &= ~(0x01 << 4);   
                break;        
        
        }
    }
}

/*
 * Set the LED bar segments high or low base on the specified value
 */
void s4375116_reg_lta1000g_write(unsigned short value) {
    
    // Iterate through segment values and mask corresponding bit
    for(int segment = 0; segment < 10; segment++) {
        lta1000g_seg_set(segment, (value >> segment) & 0x01);
    }

}