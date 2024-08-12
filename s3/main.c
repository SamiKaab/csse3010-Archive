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
#include "s4375116_hamming.h"
#include "s4375116_irremote.h"

#define S0  0
#define S1  1
#define S2  2
#define S3  3

void hardware_init(void);
int fsm_processing(int currState, char lastChar, char currChar);
uint8_t process_inputs(int lastChar, int currChar);


int main(void)  {
  
	uint16_t codedWord, decodeByte;
  int parity; // parity error from decode
  int error;  // one bit error from decode
  int currState = S0, lastState; 
  char lastChar = '\0', currChar; // last and current char entered via keyboard
  char lastIRChar = '\0', currIRChar; // last and current char entered via ir remote
  int x0,x1; // converted digits from keyboard or ir remote
  uint8_t x, irx; // numbers converted from chars
  int nbCharEntered = 1; // keep track of the number of keys entered
  int nbIRCharEntered = 1; // keep track of the number of keys entered

  // list of possible states to be in
  char* stateList[] = {"S0 (Idle)", "S1 (Encode)", "S2 (Decode)", "S3 (IR Decode)"};

  // Counter to keep track of the number of time the pushbutton has been pressed
	unsigned short press_counter; 

  // Holds the number specifiying which LED to light up
	unsigned short led_val = 0; 

  // Last time the FSM input processed
  uint16_t last_refresh_time = HAL_GetTick();

	HAL_Init();			  //Initialise board.

	hardware_init();	//Initialise hardware modules

  // Print initial state on the console
  debug_log ("\n\rState: %s  -  Commands: FF, EE, DD, IR\n\r", stateList[currState]);

	// Main processing loop
  while (1) {

    if ((HAL_GetTick() - last_refresh_time) >= 50) { // Process FSM input every 50 ms

      // get next char from ir remote
      if (keyPressedFlag && s4375116_reg_irremote_readkey(&keyPressedValue)) {
        // char has been received and is not null
        currIRChar = keyPressedValue;
        
      } else { // no char has been received or char is null
        currIRChar = '\0';
      }    

      // Deal with IR remote recieved numbers 
      if (currIRChar != '\0') {

        nbIRCharEntered = ! nbIRCharEntered; // if first ir char increment if second reset to 0;
        debug_log("%c",currIRChar); // Echo entered character

        if (nbIRCharEntered) {

          debug_log("\n\r"); // pair complete return to next line

          // Check received chars are correct and concatenate them into hex number
          irx = process_inputs(lastIRChar, currIRChar);

          if (currState == S3) {
        
            // Hamming decode received character 
            decodeByte = s4375116_lib_hamming_byte_decode(irx);

            parity = s4375116_lib_hamming_parity_error(irx);

            /* check if the byte had a bit reversal (error) 
               1-bit error -> 0x1, parity check error -> 0x2, 2-bit (parity and error) -> 0x2 
             */
            if (!parity) {
              error = !(((x & 0xF0)>>4) == decodeByte);
            } else {
              error = 0;
            }
            debug_log("ir value 0x%02x decoded is 0x%x\n\r", irx, error | (parity << 1) | (irx << 2));
          }
        }

        lastIRChar = currIRChar;
      }
      
      currChar = BRD_debuguart_getc();// get char from keyboard input
      
      //Check that the key received is not null 
      if (currChar != '\0') {

        nbCharEntered = ! nbCharEntered; // if first char increment if second reset to 0;
        debug_log("%c",currChar); // Echo entered character
        
        if (nbCharEntered) { // two chars have been entered 
          
          debug_log("\n\r"); // Go to next line after a pair has been received

          // check if we should change state
          lastState = currState;
          currState = fsm_processing(currState, lastChar, currChar);

          if (currState != lastState) { // Changed state

            debug_log ("\n\rState: %s  -  Commands: FF, EE, DD, IR\n\r", stateList[currState]);
            
            // Reset values
            decodeByte = 0;
            codedWord = 0;
            error = 0;
            parity = 0;
          } else {

            // Check received chars are correct and concatenate them into hex number
            if (currState != S0) {
              x = process_inputs(lastChar, currChar);
            }          
            if (currState == S1) { //encode state
            
              // Hamming encode received character 
              codedWord = s4375116_lib_hamming_byte_encode(x);
              debug_log("console value 0x%02x encoded is 0x%04x\n\r", x, codedWord);
            } else if (currState ==  S2) { //decode state
            
              // Hamming decode received character 
              decodeByte = s4375116_lib_hamming_byte_decode(x);
              parity = s4375116_lib_hamming_parity_error(x);
              
              /* check if the byte had a bit reversal (error) 
                1-bit error -> 0x1, parity check error -> 0x2, 2-bit (parity and error) -> 0x2 
              */
              if (!parity) {
                error = !(((x & 0xF0)>>4) == decodeByte);
              } else {
                error = 0;
              }
              
              debug_log("console value 0x%02x decoded is 0x%x\n\r", x, decodeByte);
            }
          }
        }

        lastChar = currChar;
      }
      // Format value display based on push buton and state 
      if (currState == S1) { //encode state
      
        // display lower or upper byte depending on the number of time the push button has been pressed
        led_val = (codedWord >> (8 * ((press_counter + 1)%2))) & 0xFF;
      } else if (currState == S2) { // decode state
      
        // display data on LED segment 0 to 3, and error bit and parity check on LED Segment 8 and 9
        led_val = decodeByte | parity << 9 | error << 8;
      } else { // turn of every led bar if not in S1 or S2

        led_val = 0;
      }

      // Display formated pattern on the LED bar
      s4375116_reg_lta1000g_write(led_val);

      last_refresh_time = HAL_GetTick();  
    }

    // Get number of time the Joystick button has been pressed
    press_counter = s4375116_reg_joystick_press_get();
  }

  return 0;
}

/**
 * @brief Convert the last 2 charactered entered to a hexadecimal value after checking 
 * the inputs are valid (0 to F) and return value otherwise return 0 
 * 
 * @param currState 
 * @param lastChar 
 * @param currChar 
 * @return uint8_t 
 */
uint8_t process_inputs(int lastChar, int currChar) {

  int x0,x1;
  uint8_t x = 0;
  
  //convert character to number between 0 and 9 using ASCII value manipulation
  x0 = currChar-48;
  x1 = lastChar-48;
  //check that the character entered is a number from 0 to 9 or A to F
  if (x0 < 0 || x0 > 9) {

    //convert character to hex between A and F using ASCII value manipulation
    x0 = currChar - 55; 
    
    if (x0 < 10 || x0 > 15) { // check if character entered is A to F
    
      x0 = 0; //x0 is not a hex number (0 to F)
    }
  }

  //check if the last character is a number from 0 to 9 or A to F
  if (x1 < 0 || x1 > 9) {
    
    //convert character to hex between A and F using ASCII value manipulation
    x1 = lastChar - 55; 
    
    if (x1 < 10 || x1 > 15) { // check if character entered is A to F
    
      x1 = 0; //x1 is not a hex number (0 to F)
    }
  }

  x = x0 | (x1 << 4); // concatenate two converted inputs
  
  /* check that the user did not enter FF, EE or DD which are valid
     hexadecimal number but are used to change states
  */
  if (x == 0xFF || x == 0xEE || x == 0xDD){

    x = 0;
  }

  return x;
}

/*
 * returns next state base on current state and last two consol iputs
 */
int fsm_processing(int currState, char lastChar, char currChar) {

  int nextState = S0;   //Hold Next state value

  switch(currState) {
    
    case S0:    //Idle state
    
      if (lastChar == 'E' && currChar == 'E') {  
    
        nextState = S1;
      } else if (lastChar == 'D' && currChar == 'D'){
    
        nextState = S2;
      } else if (lastChar == 'I' && currChar == 'R'){
    
        nextState = S3;
      } else {
    
        nextState = S0;
      }
      break;

    case S1: // Encode state

      if (lastChar == 'F' && currChar == 'F') {  
      
        nextState = S0;
      } else if (lastChar == 'D' && currChar == 'D'){
      
        nextState = S2;
      } else if (lastChar == 'I' && currChar == 'R'){
      
        nextState = S3;
      } else {
      
        nextState = S1;
      }
      break;

    case S2: // Decode State

      if (lastChar == 'F' && currChar == 'F') {  
      
        nextState = S0;
      } else if (lastChar == 'E' && currChar == 'E'){
      
        nextState = S1;
      } else if (lastChar == 'I' && currChar == 'R'){
      
        nextState = S3;
      } else {
      
        nextState = S2;
      }
      break;
    
    case S3: // Decode State

      if (lastChar == 'F' && currChar == 'F'){
       
        nextState = S0;
      } else if (lastChar == 'E' && currChar == 'E') {  
      
        nextState = S1;
      } else if (lastChar == 'D' && currChar == 'D') {  
      
        nextState = S2;
      } else {
      
        nextState = S3;
      }
      break;
  }

  return nextState;
}

/*
 * Initialise Hardware
 */
void hardware_init(void) {

  //Initialise UART for debug log output
  BRD_debuguart_init();

  // Initialise LED bar
  s4375116_reg_lta1000g_init();

  // Initialise joystick pushbutton interrupt
	s4375116_reg_joystick_pb_init();

  // Initialise ir remote input capture
  s4375116_reg_irremote_init();
 
}

