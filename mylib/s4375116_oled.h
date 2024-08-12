 /**
 **************************************************************
 * @file mylib/s4375116_oled.h
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

#ifndef S4375116_OLED_H
#define s4375116_OLED_H

#define I2C_DEV_SDA_PIN		9
#define I2C_DEV_SCL_PIN		8
#define I2C_DEV_GPIO		GPIOB
#define I2C_DEV_GPIO_AF 	GPIO_AF4_I2C1
#define I2C_DEV_GPIO_CLK()	__GPIOB_CLK_ENABLE()

#define I2C_DEV				I2C1
#define I2C_DEV_CLOCKSPEED 	100000

// Task Priorities (Idle Priority is the lowest priority)
#define OLEDTASK_PRIORITY					( tskIDLE_PRIORITY + 2 )

// Task Stack Allocations (must be a multiple of the minimal stack size)
#define OLEDTASK_STACK_SIZE		( configMINIMAL_STACK_SIZE * 2 )

struct oledTextMsg {
	int startX;
	int startY;
	char displayText[20];
};
typedef struct oledTextMsg OledTextMsg;

QueueHandle_t OledMessageQueue;	// Queue used

void s4375116_reg_oled_init(void);
void s4375116_tsk_oled_init(void);

#endif