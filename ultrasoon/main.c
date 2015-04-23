//--------------------------------------------------------------
// File		: main.c
// Datum	: 22.12.2013
// Version	: 1.0
// Author	: UB
// Email	: mc-4u(@)t-online.de
// Web		: www.mikrocontroller-4u.de
// CPU		: STM32F4
// IDE		: CooCox CoIDE 1.7.4
// GCC		: 4.7 2012q4
// Module	: CMSIS_BOOT, M4_CMSIS_CORE
// Function	: Demo of the HC-SR04-Library
// Note		: These two files must stay at 8MHz
//							"cmsis_boot/stm32f4xx.h"
//							"cmsis_boot/system_stm32f4xx.c"
//--------------------------------------------------------------

#include "main.h"
#include "stm32_ub_led.h"
#include "stm32_ub_hcsr04.h"
#include <stdio.h>

int multiplier;

void TM_Delay_Init(void) {
	RCC_ClocksTypeDef RCC_Clocks;

	//Get system clocks
	RCC_GetClocksFreq(&RCC_Clocks);

	//While loop takes 4 cycles
	//For 1 us delay, we need to divide with 4M
	multiplier = RCC_Clocks.HCLK_Frequency / 4000000;
}

void TM_DelayMicros(uint32_t micros) {
	micros = micros * multiplier - 10;

	//4 cycles for one loop
	while (micros--);
}

int main(void) {
	float distance;

	SystemInit(); //Initialize quartz settings
	UB_Led_Init();
	UB_HCSR04_Init();
	TM_Delay_Init();

	int i;

	uint32_t pins_e[4] = {
		GPIO_Pin_0, GPIO_Pin_1
	};

	uint32_t pins_t[4] = {
		GPIO_Pin_3, GPIO_Pin_4
	};

	uint8_t pinsources[4] = {
		GPIO_PinSource0, GPIO_PinSource1
	};

	while(1) {
		for(i = 0; i < 2; i++) {
			changeEchoPin(pins_e[i], pinsources[i]);
			changeTriggerPin(pins_t[i]);

			//Measure the distance
			distance = UB_HCSR04_Distance_cm(pins_t[i]);

			if(distance > 0) {
				//Switch LEDs based on distance
				UB_Led_Off(LED_GREEN);
				UB_Led_Off(LED_ORANGE);
				UB_Led_Off(LED_RED);
				UB_Led_On(LED_BLUE);
			} else {
				//Outside of the measuring range
				UB_Led_Off(LED_GREEN);
				UB_Led_Off(LED_BLUE );
				UB_Led_Off(LED_ORANGE);
				UB_Led_On(LED_RED);
			}

			printf("[%d] %d\r\n", i, (int)distance);
			TM_DelayMicros(300000);
		}
	}
}

//hoi

