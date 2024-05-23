 /**
 **************************************************************
 * @file mylib/s4375116_hamming.h
 * @author Sami Kaab - s4375116 
 * @date 03042022
 * @brief hamming encoding and decoding lib functions
 * REFERENCE: csse3010_mylib_lib_hamming.pdf, csse3010_stage3.pdf,
 * sourcelib/examples/getting-started/hamming/nucleo-f429/main.c
 ***************************************************************
 * EXTERNAL FUNCTIONS
 ***************************************************************
 * s4375116_lib_hamming_byte_encode() - Hamming encode byte
 * s4375116_lib_hamming_byte_decode() - Hamming decode byte
 * s4375116_lib_hamming_parity_error() - Checks for parity error occured on hamming encoded byte
 ***************************************************************
 */

#ifndef S4375116_HAMMING_H
#define s4375116_HAMMING_H

unsigned short s4375116_lib_hamming_byte_encode(unsigned char value);
unsigned char s4375116_lib_hamming_byte_decode(unsigned char value);
int s4375116_lib_hamming_parity_error(unsigned char value);
        
#endif