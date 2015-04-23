//--------------------------------------------------------------
// File		: stm32_ub_hcsr04.c
// Datum	: 22.12.2013
// Version	: 1.0
// Author	: UB
// Email	: mc-4u(@)t-online.de
// Web		: www.mikrocontroller-4u.de
// CPU		: STM32F4
// IDE		: CooCox CoIDE 1.7.4
// GCC		: 4.7 2012q4
// Module	: GPIO, TIM, MISC
// Function : Ultrasonic proximity sensor (HC-SR04)
//				Measuring range (according to datasheet) 2cm to 400cm
// Pinout	: Trigger = PD3
//				Echo		 = PA0
// Note		: The echo-pin must be an input-capture-pin of the used timer
//				(TIM2 in this case)
//--------------------------------------------------------------

#include "stm32_ub_hcsr04.h"
#include <stdio.h>

void P_HCSR04_Trigger(uint32_t pin);

GPIO_InitTypeDef GPIO_InitStructure;

void UB_HCSR04_Init(void) {
	HCSR04.t2_akt_time = 0;
	HCSR04.t7_akt_time = 0;
	HCSR04.delay_us = 0;
}

//--------------------------------------------------------------
// Distance measurement via HC-SR04 Sensor
// Note : After the measurement a 60ms delay is added
//
// return_val
//	 > 0	=	Measured distance in centimeters
//	 -1	=	 Measurement failure (probably time-out)
//--------------------------------------------------------------

float UB_HCSR04_Distance_cm(uint32_t pin) {
	float return_val = 0.0;
	uint32_t ok = 0;

	HCSR04.t2_akt_time = 0;
	TIM_SetCounter(TIM2, 0);
	TIM_Cmd(TIM2, ENABLE);

	P_HCSR04_Trigger(pin);

	//Wait until measurement is done or timer 7 gives a time-out
	ok = 0;
	HCSR04.t7_akt_time = 0;

	TIM_Cmd(TIM7, ENABLE);

	do {
		if(HCSR04.t2_akt_time != 0)
			ok = 1; //Measurement done
		if(HCSR04.t7_akt_time >= HCSR04_TIMEOUT)
			ok = 2; //Time-out
	} while(ok == 0);

	TIM_Cmd(TIM7, DISABLE);
	TIM_Cmd(TIM2, DISABLE);

	HCSR04.t7_akt_time = 0;

	TIM_Cmd(TIM7, ENABLE);

	while(HCSR04.t7_akt_time < HCSR04_DELAY);

	TIM_Cmd(TIM7, DISABLE);

	if(ok == 1) {
		//Calculate distance based on time passed
		return_val = (float)(HCSR04.delay_us) * (float)(HCSR04_FAKTOR_US_2_CM);
	} else {
		//Time-out
		return_val = -1.0;
	}

	HCSR04.t7_akt_time = 0;

	return return_val;
}

//Internal function, give a 10us trigger-signal via TIM7
void P_HCSR04_Trigger(uint32_t pin) {
	HCSR04.t7_akt_time = 0;
	HCSR04_TRIGGER_PORT->BSRRL = pin;

	TIM_Cmd(TIM7, ENABLE);

	while(HCSR04.t7_akt_time < 10);

	TIM_Cmd(TIM7, DISABLE);

	HCSR04_TRIGGER_PORT->BSRRH = pin;
}

void changeEchoPin(uint32_t pin, uint8_t pinSource) {
	RCC_AHB1PeriphClockCmd(HCSR04_ECHO_CLK, ENABLE);

	//Configure pins as alternating function (AF)
	GPIO_InitStructure.GPIO_Pin = pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(HCSR04_ECHO_PORT, &GPIO_InitStructure);

	//RCC_AHB1PeriphClockCmd(HCSR04_ECHO_CLK, ENABLE);

	GPIO_PinAFConfig(HCSR04_ECHO_PORT, pinSource, GPIO_AF_TIM2);
}

void changeTriggerPin(uint32_t pin) {
	//RCC_AHB1PeriphClockCmd(HCSR04_TRIGGER_CLK, DISABLE);

	RCC_AHB1PeriphClockCmd(HCSR04_TRIGGER_CLK, ENABLE);

	//Configure as digital output
	GPIO_InitStructure.GPIO_Pin = pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(HCSR04_TRIGGER_PORT, &GPIO_InitStructure);

	HCSR04_TRIGGER_PORT->BSRRH = pin;
}

void changeTimerChannel(uint16_t channel) {
	P_HCSR04_InitTIM2(channel);
}

void P_HCSR04_InitIO(uint32_t triggerPin, uint32_t echoPin, uint8_t echoSource) {
	/** Trigger PIN **/

	RCC_AHB1PeriphClockCmd(HCSR04_TRIGGER_CLK, ENABLE);

	//Configure as digital output
	GPIO_InitStructure.GPIO_Pin = triggerPin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(HCSR04_TRIGGER_PORT, &GPIO_InitStructure);

	HCSR04_TRIGGER_PORT->BSRRH = triggerPin;

	/** Echo PIN **/

	RCC_AHB1PeriphClockCmd(HCSR04_ECHO_CLK, ENABLE);

	//Configure pins as alternating function (AF)
	GPIO_InitStructure.GPIO_Pin = echoPin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(HCSR04_ECHO_PORT, &GPIO_InitStructure);

	//Connect alternative-function with the IO-pins);
	GPIO_PinAFConfig(HCSR04_ECHO_PORT, echoSource, GPIO_AF_TIM2);
}

void P_HCSR04_InitTIM(void) {
	TIM_TimeBaseInitTypeDef	TIM_TimeBaseStructure;

	//TIM7 for delay, TIM2 for input-capture mode

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);

	TIM_Cmd(TIM7, DISABLE);

	//Set timer to 10us
	TIM_TimeBaseStructure.TIM_Period = HCSR04_TIM7_PERIOD;
	TIM_TimeBaseStructure.TIM_Prescaler = HCSR04_TIM7_PRESCALE;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure);

	TIM_ARRPreloadConfig(TIM7, ENABLE);
}

void P_HCSR04_InitTIM2(uint16_t channel) {
	TIM_ICInitTypeDef TIM_ICInitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	//Configure the prescaler
	TIM_PrescalerConfig(TIM2, HCSR04_TIM2_PRESCALE, TIM_PSCReloadMode_Immediate);

	TIM_ICInitStructure.TIM_Channel = channel;
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICFilter = 0x0;
	TIM_PWMIConfig(TIM2, &TIM_ICInitStructure);

	//Input trigger
	TIM_SelectInputTrigger(TIM2, TIM_TS_TI1FP1);

	//Slave-mode (reset)
	TIM_SelectSlaveMode(TIM2, TIM_SlaveMode_Reset);
	TIM_SelectMasterSlaveMode(TIM2, TIM_MasterSlaveMode_Enable);
}

void P_HCSR04_InitNVIC(void) {
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_ITConfig(TIM2, TIM_IT_CC1, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	//Update interrupt enable
	TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);
}

// ISR of TIM2
// Gets called on low flank of echo-signal
void TIM2_IRQHandler(void) {
	uint32_t start, stop;

	if(TIM_GetITStatus(TIM2, TIM_IT_CC1) == SET) {
		//Clear interrupt flags
		TIM_ClearITPendingBit(TIM2, TIM_IT_CC1);

		//Read measured value
		start = TIM_GetCapture1(TIM2);
		stop = TIM_GetCapture2(TIM2);
		HCSR04.delay_us = start - stop;
		HCSR04.t2_akt_time++;
	}
}

// Timer-7 ISR
// Gets called every 10us
void TIM7_IRQHandler(void) {
	if (TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET) {
		//When interrupt occurred
		TIM_ClearITPendingBit(TIM7, TIM_IT_Update);

		HCSR04.t7_akt_time += 10;
	}
}

