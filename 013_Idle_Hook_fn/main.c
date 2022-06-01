#include "GPIO_STM32F10x.h"             // Keil::Device:GPIO
#include "FreeRTOS.h"                   // ARM.FreeRTOS::RTOS:Core
#include "task.h"                       // ARM.FreeRTOS::RTOS:Core

#include "stm32f10x_rcc.h"              // Keil::Device:StdPeriph Drivers:RCC
#include "stm32f10x_gpio.h"             // Keil::Device:StdPeriph Drivers:GPIO

uint32_t redCounter = 0;
uint32_t idleCounter = 0;

const TickType_t _250ms = pdMS_TO_TICKS(250); // Calculates no of ticks for 250ms
const TickType_t _1ms = pdMS_TO_TICKS(1); // Calculates no of ticks for 250ms

void ledInit(void){
	GPIO_InitTypeDef GPIO_InitStruct;
	// GPIO Clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	/* Configure the GPIO_LED pin */
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_3;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void vRedLedToggleTask(void *pvParameter){
	while(1){
		GPIO_SetBits(GPIOA, GPIO_Pin_0);
		vTaskDelay(_1ms);
		GPIO_ResetBits(GPIOA, GPIO_Pin_0);
		vTaskDelay(_1ms);
		redCounter++;
	}
}

// Idle Application
// This task will be executed in idle time of cpu after enabling  configUSE_IDLE_HOOK in freeRTOSConfig.h
void vApplicationIdleHook(){
	idleCounter++;
}

int main(){
	ledInit();
	// Create tasks
	xTaskCreate(vRedLedToggleTask, "Red Led Blink", 100, NULL, 1,NULL);
	
	// Start Schedular
	vTaskStartScheduler();
	
	while(1){
		
	}
}


