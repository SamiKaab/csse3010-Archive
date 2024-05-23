 /**
 **************************************************************
 * @file mylib/s4375116_oled.c
 * @author Sami Kaab - s4375116 
 * @date 14042022
 * @brief OLED register and task function
 * REFERENCE: csse3010_mylib_reg_oled.pdf, csse3010_stage4.pdf
 ***************************************************************
 * EXTERNAL FUNCTIONS
 ***************************************************************
 * s4375116_reg_oled_init() - Initialise the OLED SSD1306 
 * s4375116_tsk_oled_init() - Create OLED Task
 ***************************************************************
 */

#include "board.h"
#include "processor_hal.h"
#include "debug_log.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "oled_pixel.h"
#include "oled_string.h"
#include "fonts.h"
#include "s4375116_oled.h"



/*
 * Initialise the OLED SSD1306 device with the I2C interface.
 */
void s4375116_reg_oled_init(void) {

	uint32_t pclk1;
	uint32_t freqrange;


	// Enable GPIO clock
	I2C_DEV_GPIO_CLK();

	//******************************************************
	// IMPORTANT NOTE: SCL Must be Initialised BEFORE SDA
	//******************************************************

	//Clear and Set Alternate Function for pin (lower ARF register) 
	MODIFY_REG(I2C_DEV_GPIO->AFR[1], ((0x0F) << ((I2C_DEV_SCL_PIN-8) * 4)) | ((0x0F) << ((I2C_DEV_SDA_PIN-8)* 4)), ((I2C_DEV_GPIO_AF << ((I2C_DEV_SCL_PIN-8) * 4)) | (I2C_DEV_GPIO_AF << ((I2C_DEV_SDA_PIN-8)) * 4)));
	
	//Clear and Set Alternate Function Push Pull Mode
	MODIFY_REG(I2C_DEV_GPIO->MODER, ((0x03 << (I2C_DEV_SCL_PIN * 2)) | (0x03 << (I2C_DEV_SDA_PIN * 2))), ((GPIO_MODE_AF_OD << (I2C_DEV_SCL_PIN * 2)) | (GPIO_MODE_AF_OD << (I2C_DEV_SDA_PIN * 2))));
	
	//Set low speed.
	SET_BIT(I2C_DEV_GPIO->OSPEEDR, (GPIO_SPEED_LOW << I2C_DEV_SCL_PIN) | (GPIO_SPEED_LOW << I2C_DEV_SDA_PIN));

	//Set Bit for Push/Pull output
	SET_BIT(I2C_DEV_GPIO->OTYPER, ((0x01 << I2C_DEV_SCL_PIN) | (0x01 << I2C_DEV_SDA_PIN)));

	//Clear and set bits for no push/pull
	MODIFY_REG(I2C_DEV_GPIO->PUPDR, (0x03 << (I2C_DEV_SCL_PIN * 2)) | (0x03 << (I2C_DEV_SDA_PIN * 2)), (GPIO_PULLUP << (I2C_DEV_SCL_PIN * 2)) | (GPIO_PULLUP << (I2C_DEV_SDA_PIN * 2)));

	// Configure the I2C peripheral
	// Enable I2C peripheral clock
	__I2C1_CLK_ENABLE();

	// Disable the selected I2C peripheral
	CLEAR_BIT(I2C_DEV->CR1, I2C_CR1_PE);

  	pclk1 = HAL_RCC_GetPCLK1Freq();			// Get PCLK1 frequency
  	freqrange = I2C_FREQRANGE(pclk1);		// Calculate frequency range 

  	//I2Cx CR2 Configuration - Configure I2Cx: Frequency range
  	MODIFY_REG(I2C_DEV->CR2, I2C_CR2_FREQ, freqrange);

	// I2Cx TRISE Configuration - Configure I2Cx: Rise Time
  	MODIFY_REG(I2C_DEV->TRISE, I2C_TRISE_TRISE, I2C_RISE_TIME(freqrange, I2C_DEV_CLOCKSPEED));

   	// I2Cx CCR Configuration - Configure I2Cx: Speed
  	MODIFY_REG(I2C_DEV->CCR, (I2C_CCR_FS | I2C_CCR_DUTY | I2C_CCR_CCR), I2C_SPEED(pclk1, I2C_DEV_CLOCKSPEED, I2C_DUTYCYCLE_2));

   	// I2Cx CR1 Configuration - Configure I2Cx: Generalcall and NoStretch mode
  	MODIFY_REG(I2C_DEV->CR1, (I2C_CR1_ENGC | I2C_CR1_NOSTRETCH), (I2C_GENERALCALL_DISABLE| I2C_NOSTRETCH_DISABLE));

   	// I2Cx OAR1 Configuration - Configure I2Cx: Own Address1 and addressing mode
  	MODIFY_REG(I2C_DEV->OAR1, (I2C_OAR1_ADDMODE | I2C_OAR1_ADD8_9 | I2C_OAR1_ADD1_7 | I2C_OAR1_ADD0), I2C_ADDRESSINGMODE_7BIT);

   	// I2Cx OAR2 Configuration - Configure I2Cx: Dual mode and Own Address2
  	MODIFY_REG(I2C_DEV->OAR2, (I2C_OAR2_ENDUAL | I2C_OAR2_ADD2), I2C_DUALADDRESS_DISABLE);

  	// Enable the selected I2C peripheral
	SET_BIT(I2C_DEV->CR1, I2C_CR1_PE);

	ssd1306_Init();	//Initialise SSD1306 OLED.
}

/**
 * @brief Writes message as specified in the oledTextMsg struct to the oled
 * 
 * @param RcvdMsg 
 */
void write_to_oled(OledTextMsg RcvdMsg) {
	//Clear Screen
	ssd1306_Fill(Black);
	//Show text and udpate screen
	ssd1306_SetCursor(RcvdMsg.startX,RcvdMsg.startY);
	ssd1306_WriteString(RcvdMsg.displayText, Font_7x10, SSD1306_WHITE);
	ssd1306_UpdateScreen();
}

/**
 * @brief Initialises OLED hardware pins
 * and creates the cyclic executive for the oled
 * 
 */
void s4375116TaskOled(void) {

	portDISABLE_INTERRUPTS();	//Disable interrupts

	s4375116_reg_oled_init();

	portENABLE_INTERRUPTS();	//Enable interrupts


	OledTextMsg RcvdMsg;
	

	OledMessageQueue = xQueueCreate(10, sizeof(RcvdMsg));		// Create queue of length 10 Message items 


	for(;;) {

		if (OledMessageQueue != NULL) {	// Check if queue exists

			// Check for item received - block atmost for 10 ticks
			if (xQueueReceive( OledMessageQueue, &RcvdMsg, 10 )) {

				write_to_oled(RcvdMsg);
        	}
		}

		// Delay the task for 1000ms.
		vTaskDelay(1);
	}
}

/**
 * @brief Creates the task that runs the OLED cylic executive.
 * 
 */
void s4375116_tsk_oled_init(void) {

	xTaskCreate( (void *) &s4375116TaskOled, (const signed char *) "OLED", OLEDTASK_STACK_SIZE, NULL, OLEDTASK_PRIORITY, NULL );

}