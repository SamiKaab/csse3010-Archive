 /**
 **************************************************************
 * @file mylib/s4375116_irremote.h
 * @author Sami Kaab - s4375116 
 * @date 08042022
 * @brief IR receiver HAL Driver functions
 * REFERENCE: csse3010_mylib_reg_irremote.pdf, csse3010_stage3.pdf,
 * sourcelib/examples/timer/inputcapture/nucleo-f429/main.c
 ***************************************************************
 * EXTERNAL FUNCTIONS
 ***************************************************************
 * s4375116_reg_irremote_init() - Initialise input capture for ir receiver
 * s4375116_reg_irremote_recv() - Convert received pulses to a bit pattern 
 * s4375116_reg_irremote_readkey() - Convert bit pattern to key
 ***************************************************************
 */

#ifndef S4375116_IRREMOTE_H
#define s4375116_IRREMOTE_H

#define TIMER_COUNTER_FREQ      50000       //Frequency (in Hz)
#define TIMER_1SECOND_PERIOD    50000       //Period for 1s (in count ticks)

char keyPressedValue;
int keyPressedFlag;

void s4375116_reg_irremote_init(void);
void s4375116_reg_irremote_recv(void);
int s4375116_reg_irremote_readkey(char *value);
        
#endif