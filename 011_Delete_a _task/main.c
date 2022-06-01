#include "GPIO_STM32F10x.h"             // Keil::Device:GPIO
#include "FreeRTOS.h"                   // ARM.FreeRTOS::RTOS:Core
#include "task.h"                       // ARM.FreeRTOS::RTOS:Core

#include "stm32f10x_rcc.h"              // Keil::Device:StdPeriph Drivers:RCC
#include "stm32f10x_gpio.h"             // Keil::Device:StdPeriph Drivers:GPIO

uint32_t suspend_monitor;
uint32_t blueTask_execution_monitor;
bool _killed= false;
TaskHandle_t redHandle, greenHandle, blueHandle;

// When a task deleted, it cannot be resumed again
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

void vRedLedToggleTask(void *pvParameter){
	while(1){
		GPIO_SetBits(GPIOA, GPIO_Pin_0);
		for(int i = 0; i < 500000; i++){}
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
		
		if (_killed){
			blueTask_execution_monitor++;
			if (blueTask_execution_monitor > 10){
				blueTask_execution_monitor = 0;
				_killed = false; 
				vTaskResume(redHandle);
			}
		}
	}
}



void vBlueLedToggleTask(void *pvParameter){
	while(1){
		GPIO_SetBits(GPIOA, GPIO_Pin_2);
		for(int i = 0; i < 500000; i++){}
		GPIO_ResetBits(GPIOA, GPIO_Pin_2);
		for(int i = 0; i < 500000; i++){}
		suspend_monitor++;
		if (suspend_monitor >= 20){ 
			_killed = true;
			suspend_monitor = 0;
			vTaskDelete(NULL);
		}
	}
}


int main(){
	ledInit();
	// Create tasks
	xTaskCreate(vRedLedToggleTask, "Red Led Blink", 100, NULL, 1, &redHandle);
	xTaskCreate(vGreenLedToggleTask, "Green Led Blink", 100, NULL, 1, &greenHandle);
	xTaskCreate(vBlueLedToggleTask, "Blue Led Blink", 100, NULL, 1, &blueHandle);
	// Start Schedular
	vTaskStartScheduler();
	
	while(1){
		//GPIOB->ODR = 0b1000;
	}
}


