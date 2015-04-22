#ifndef __STM32F4_UB_LED_H
	#define __STM32F4_UB_LED_H

#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"

typedef enum {
	LED_GREEN = 0,	//LED4 on the discovery board
	LED_ORANGE = 1, //LED3
	LED_RED = 2,	//LED5
	LED_BLUE = 3	//LED6
} LED_NAME_t;

#define	LED_ANZ	 4

typedef enum {
	LED_OFF = 0,
	LED_ON
} LED_STATUS_t;

typedef struct {
	LED_NAME_t LED_NAME;	// Name
	GPIO_TypeDef* LED_PORT; // Port
	const uint16_t LED_PIN; // Pin
	const uint32_t LED_CLK; // Clock
	LED_STATUS_t LED_INIT;	// Init
} LED_t;

void UB_Led_Init(void);
void UB_Led_Off(LED_NAME_t led_name);
void UB_Led_On(LED_NAME_t led_name);
void UB_Led_Toggle(LED_NAME_t led_name);
void UB_Led_Switch(LED_NAME_t led_name, LED_STATUS_t wert);

#endif // __STM32F4_UB_LED_H
