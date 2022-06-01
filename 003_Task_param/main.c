#include "GPIO_STM32F10x.h"             // Keil::Device:GPIO
#include "FreeRTOS.h"                   // ARM.FreeRTOS::RTOS:Core
#include "task.h"                       // ARM.FreeRTOS::RTOS:Core

#include "stm32f10x_rcc.h"              // Keil::Device:StdPeriph Drivers:RCC
#include "stm32f10x_gpio.h"             // Keil::Device:StdPeriph Drivers:GPIO

const uint16_t *red = (uint16_t*)GPIO_Pin_0;
const uint16_t *green = (uint16_t*)GPIO_Pin_1;
const uint16_t *blue = (uint16_t*)GPIO_Pin_2;

void ledInit(void){
	GPIO_InitTypeDef GPIO_InitStruct;
	// GPIO Clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	/* Configure the GPIO_LED pin */
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void vLedToggleTask(void *pvParameter){
	while(1){
		GPIO_SetBits(GPIOA, (uint16_t)pvParameter);
		for(int i = 0; i < 500000; i++){}
		GPIO_ResetBits(GPIOA, (uint16_t)pvParameter);
		for(int i = 0; i < 500000; i++){}
	}
}


int main(){
	ledInit();
	// Create tasks
	xTaskCreate(vLedToggleTask, "Red Led Blink", 100, (void *)red, 1,NULL);
	xTaskCreate(vLedToggleTask, "Green Led Blink", 100, (void *)green, 1,NULL);
	xTaskCreate(vLedToggleTask, "Blue Led Blink", 100, (void *)blue, 1,NULL);
	// Start Schedular
	vTaskStartScheduler();
	
	while(1){

	}
}


