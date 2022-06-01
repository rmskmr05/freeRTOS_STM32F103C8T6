#include "GPIO_STM32F10x.h"             // Keil::Device:GPIO
#include "FreeRTOS.h"                   // ARM.FreeRTOS::RTOS:Core
#include "task.h"                       // ARM.FreeRTOS::RTOS:Core

#include "stm32f10x_rcc.h"              // Keil::Device:StdPeriph Drivers:RCC
#include "stm32f10x_gpio.h"             // Keil::Device:StdPeriph Drivers:GPIO



void ledInit(void){
	GPIO_InitTypeDef GPIO_InitStruct;
	// GPIO Clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	/* Configure the GPIO_LED pin */
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_3;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void vRedLedToggleTask(void *pvParameter){
	while(1){
		GPIO_SetBits(GPIOB, GPIO_Pin_0);
		for(int i = 0; i < 500000; i++){}
		GPIO_ResetBits(GPIOB, GPIO_Pin_0);
		for(int i = 0; i < 500000; i++){}
	}
}

void vGreenLedToggleTask(void *pvParameter){
	while(1){
		GPIO_SetBits(GPIOB, GPIO_Pin_1);
		for(int i = 0; i < 500000; i++){}
		GPIO_ResetBits(GPIOB, GPIO_Pin_1);
		for(int i = 0; i < 500000; i++){}
	}
}

void vBlueLedToggleTask(void *pvParameter){
	while(1){
		GPIO_SetBits(GPIOB, GPIO_Pin_3);
		for(int i = 0; i < 500000; i++){}
		GPIO_ResetBits(GPIOB, GPIO_Pin_3);
		for(int i = 0; i < 500000; i++){}
	}
}


int main(){
	ledInit();
	// Create tasks
	xTaskCreate(vRedLedToggleTask, "Red Led Blink", 100, NULL, 1,NULL);
	xTaskCreate(vGreenLedToggleTask, "Green Led Blink", 100, NULL, 1,NULL);
	xTaskCreate(vBlueLedToggleTask, "Blue Led Blink", 100, NULL, 1,NULL);
	// Start Schedular
	vTaskStartScheduler();
	
	while(1){
		GPIOB->ODR = 0b1000;
	}
}


