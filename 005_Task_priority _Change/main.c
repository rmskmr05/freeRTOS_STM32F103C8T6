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

void vRedLedToggleTask(void *pvParameters);
void vGreenLedToggleTask(void *pvParameters);
void vBlueLedToggleTask(void *pvParameters);


TaskHandle_t red_handle, green_handle, blue_handle;

int main(){
	ledInit();
	// Create tasks
	xTaskCreate(vRedLedToggleTask, "Red Led Blink", 100, NULL, 1,&red_handle);
	xTaskCreate(vGreenLedToggleTask, "Green Led Blink", 100, NULL, 1 ,&green_handle);
	xTaskCreate(vBlueLedToggleTask, "Blue Led Blink", 100, NULL , 1,&blue_handle);
	// Start Schedular
	vTaskStartScheduler();
	
	while(1){

	}
}

void vRedLedToggleTask(void *pvParameter){
	while(1){
		GPIO_SetBits(GPIOA, GPIO_Pin_0);
		for(int i = 0; i < 500000; i++){
			// Change the priority & 
			// this will result red led ON since green got the heighest priority
			vTaskPrioritySet(green_handle, 3);
		}
		GPIO_ResetBits(GPIOA, GPIO_Pin_0);
		for(int i = 0; i < 500000; i++){}
	}
}
void vGreenLedToggleTask(void *pvParameter){
	while(1){
		GPIO_SetBits(GPIOA, GPIO_Pin_1);
		for(int i = 0; i < 500000; i++){}
		GPIO_ResetBits(GPIOA, GPIO_Pin_1);
		for(int i = 0; i < 500000; i++){}
	}
}
void vBlueLedToggleTask(void *pvParameter){
	while(1){
		GPIO_SetBits(GPIOA, GPIO_Pin_2);
		for(int i = 0; i < 500000; i++){}
		GPIO_ResetBits(GPIOA, GPIO_Pin_2);
		for(int i = 0; i < 500000; i++){}
	}
}
