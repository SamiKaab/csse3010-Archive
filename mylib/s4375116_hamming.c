 /**
 **************************************************************
 * @file mylib/s4375116_hamming.c
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

#include "board.h"
#include "processor_hal.h"
#include "debug_log.h"
#include "s4375116_hamming.h"

/**
  * Implement Hamming Code + parity checking
  * Hamming code is based on the following generator and parity check matrices
  * G = [ 0 1 1 | 1 0 0 0 ;
  *       1 0 1 | 0 1 0 0 ;
  *       1 1 0 | 0 0 1 0 ;
  *       1 1 1 | 0 0 0 1 ;
  *
  * y = x * G
  *
  *
  * NOTE: !! is used to get 1 out of non zeros
  * 
  * Modified from getting-started/hamming example
  */
unsigned char hamming_hbyte_encode(unsigned char value) {

	uint8_t d0, d1, d2, d3;
	uint8_t p0 = 0, h0, h1, h2;
	uint8_t z;
	unsigned char out;

	/* extract bits */
	d0 = !!(value & 0x1);
	d1 = !!(value & 0x2);
	d2 = !!(value & 0x4);
	d3 = !!(value & 0x8);

	/* calculate hamming parity bits */
	h0 = d1 ^ d2 ^ d3;
	h1 = d0 ^ d2 ^ d3;
  h2 = d0 ^ d1 ^ d3;

	/* generate out byte without parity bit P0 */
	out = (h0 << 1) | (h1 << 2) | (h2 << 3) |
		(d0 << 4) | (d1 << 5) | (d2 << 6) | (d3 << 7);

	/* calculate even parity bit */
	for (z = 1; z<8; z++)
		p0 = p0 ^ !!(out & (1 << z));

	out |= p0;

	return(out);
}

/*
 * Implement Hamming Code on a full byte of input
 * This means that 16-bits out output is needed
 * 
 * Modified from getting-started/hamming example
 */
unsigned short s4375116_lib_hamming_byte_encode(unsigned char value) {

	unsigned short out;

	/* first encode D0..D3 (first 4 bits),
	 * then D4..D7 (second 4 bits).
	 */
	out = hamming_hbyte_encode(value & 0xF) |
		(hamming_hbyte_encode(value >> 4) << 8);

	return(out);

}


/**
 * @brief Returns the location of the error in the hamming
 * encoded message base on its syndrom
 * 
 * @param syn 
 * @return unsigned char 
 */
unsigned char check_error_location(unsigned char syn) {

    switch (syn) {

      case 0b110:
      
          return 1;
          break;
      
      case 0b101:
      
          return 2;
          break;
      
      case 0b011:
      
          return 3;
          break;
      
      case 0b111:
      
          return 4;
          break;
      
      default:
      
          return 0;
          break;
    }
}

/**
  * Implement Hamming Code + parity checking
  * Hamming code is based on the following generator and parity check matrices
  *
  * hence H =
  * [ 1 0 0 | 0 1 1 1 ;
  *   0 1 0 | 1 0 1 1 ;
  *   0 0 1 | 1 1 0 1 ];
  *
  * syn = H * y'
  *
  * NOTE: !! is used to get 1 out of non zeros
  */
unsigned char s4375116_lib_hamming_byte_decode(unsigned char value) {

  uint8_t d0, d1, d2, d3;
	uint8_t h0, h1, h2;
  uint8_t s0, s1, s2;
	unsigned char syn, out, errLoc;

	/* extract data bits */
	d0 = !!(value & 0x10);
	d1 = !!(value & 0x20);
	d2 = !!(value & 0x40);
	d3 = !!(value & 0x80);

  /* extract hamming bits */
	h0 = !!(value & 0x2);
	h1 = !!(value & 0x4);
	h2 = !!(value & 0x8);

	/* calculate syndrom */
	s0 = h0 ^ d1 ^ d2 ^ d3;
	s1 = h1 ^ d0 ^ d2 ^ d3;
	s2 = h2 ^ d0 ^ d1 ^ d3;

  out = (d0 << 0) | (d1 << 1) | (d2 << 2) | (d3 << 3);
	
	syn = (s0 << 0) | (s1 << 1) | (s2 << 2);

  // debug_log("syn:\t");
  // print_binary(syn, 3);
  // get position of error from computed syn
  errLoc = check_error_location(syn);
  // debug_log("error position:\t%d\n\r", errLoc-1);
  if (errLoc) {// error was detected
    // correct error by flipping appropriate bit
    out = (out ^ (0x1 << (errLoc-1)));
  }

	return(out);

}

/**
 * @brief Takes in the hamming coded byte, perfoms a parity check and returns 1 if it fails.
 * 
 * @param value the hamming coded 8bit value 
 * @return int 0 if there is an even number of ones and 1 if there is an odd number of ones
 */
int s4375116_lib_hamming_parity_error(unsigned char value) {

  uint8_t p0 = 0, z;
  /* calculate even parity bit */
	for (z = 0; z<8; z++) {
		p0 = p0 ^ !!(value & (1 << z));
  }
  
  return p0;
}



