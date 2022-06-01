#include "GPIO_STM32F10x.h"             // Keil::Device:GPIO
#include "FreeRTOS.h"                   // ARM.FreeRTOS::RTOS:Core
#include "task.h"                       // ARM.FreeRTOS::RTOS:Core

#include "stm32f10x_rcc.h"              // Keil::Device:StdPeriph Drivers:RCC
#include "stm32f10x_gpio.h"             // Keil::Device:StdPeriph Drivers:GPIO

// vTaskDelay blocks the heighest priority task for specified delay, that allows lower priority task to execute

uint32_t suspend_monitor;
uint32_t resume_monitor;
bool _suspended= false;

const TickType_t _50ms = pdMS_TO_TICKS(50);
TaskHandle_t redHandle, greenHandle, blueHandle;
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
		vTaskDelay(_50ms);
		GPIO_ResetBits(GPIOA, GPIO_Pin_0);
		vTaskDelay(_50ms);
	}
}



void vGreenLedToggleTask(void *pvParameter){
	while(1){
		GPIO_SetBits(GPIOA, GPIO_Pin_1);
		vTaskDelay(_50ms);
		GPIO_ResetBits(GPIOA, GPIO_Pin_1);
		vTaskDelay(_50ms);
	}
}



void vBlueLedToggleTask(void *pvParameter){
	while(1){
		GPIO_SetBits(GPIOA, GPIO_Pin_2);
		vTaskDelay(_50ms);
		GPIO_ResetBits(GPIOA, GPIO_Pin_2);
		vTaskDelay(_50ms);
	}
}


int main(){
	ledInit();
	// Create tasks
	xTaskCreate(vRedLedToggleTask, "Red Led Blink", 100, NULL, 1, &redHandle);
	xTaskCreate(vGreenLedToggleTask, "Green Led Blink", 100, NULL, 2, &greenHandle);
	xTaskCreate(vBlueLedToggleTask, "Blue Led Blink", 100, NULL, 3, &blueHandle);
	// Start Schedular
	vTaskStartScheduler();
	
	while(1){
		//GPIOB->ODR = 0b1000;
	}
}


