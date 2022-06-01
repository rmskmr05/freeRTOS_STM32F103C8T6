#include <stm32f10x.h>
#include <String.h>
#include <stdio.h>
#include <FreeRTOS.h>
#include "Task.h"

TaskHandle_t xTaskHandle1, xTaskHandler2;
void vTaskFunction1(void *pvParameters);
void vTaskFunction2(void *pvParameters);
static void prvSetupHardware(void);
void printmsg(char *msg);

// MACROS
#define TRUE 1
#define FALSE 0
#define AVAILABLE 1
#define NOT_AVAILABLE 0
// Global variables
uint8_t UART_ACCESS_KEY = AVAILABLE;

char user_msg[250];
int main(){
	
	// 1.  Reset the RCC Clock configuration to default reset state
	RCC_DeInit(); // 8MHz System Clock

	// Update core clock to reset
	SystemCoreClockUpdate();
	
	prvSetupHardware();
	sprintf(user_msg, "This is Hello World Application Starting\r\n");  
	printmsg(user_msg);
	//  Create Tasks
	xTaskCreate(vTaskFunction1, "Task-1", configMINIMAL_STACK_SIZE, NULL, 2, &xTaskHandle1);
	xTaskCreate(vTaskFunction2, "Task-1", configMINIMAL_STACK_SIZE, NULL, 2, &xTaskHandler2);

	/* Start the Schedular */
	vTaskStartScheduler();
	
	// This place will be never reached by the code unless error in scheduler
	for(;;);

}

void vTaskFunction1(void *pvParameters){
	while(1){
		if (UART_ACCESS_KEY == AVAILABLE){
			UART_ACCESS_KEY = NOT_AVAILABLE;
			printmsg("FIRST\r\n");
			UART_ACCESS_KEY = AVAILABLE;
			taskYIELD();
		}
	}
}

void vTaskFunction2(void *pvParameters){
	while(1){
		if (UART_ACCESS_KEY == AVAILABLE){
			UART_ACCESS_KEY = NOT_AVAILABLE;
			printmsg("SECOND\r\n");
			UART_ACCESS_KEY = AVAILABLE;
			taskYIELD();
		}
	}
}


static void prvSetupUart(void){
	GPIO_InitTypeDef GPIO_InitStruct;
	memset(&GPIO_InitStruct, 0, sizeof(GPIO_InitStruct));
	// Enable clock for UART2 & GPIOA Clock
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	/* Configure USART1 Tx (PA.09) as alternate function push-pull */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	/* Configure USART1 Rx (PA.10) as input floating */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	
	// AF Mode setting for the pins
	// Configure UART2 Peripheral Configuration
	USART_InitTypeDef USART_InitStruct;
	memset(&USART_InitStruct, 0, sizeof(USART_InitStruct));
	USART_InitStruct.USART_BaudRate = 115200;
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;
	USART_InitStruct.USART_StopBits = USART_StopBits_1;
	USART_InitStruct.USART_Parity = USART_Parity_No;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(USART2, &USART_InitStruct);

	/* Enable USART1 */
	USART_Cmd(USART2, ENABLE);
}
 

/*All Hardware Specific Configuration  */
static void prvSetupHardware(void){
	// UART Setup
	prvSetupUart();
	
}

void printmsg(char *msg){
	 for (uint32_t i = 0; i < strlen(msg); i++){
		 while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) != SET);
		 USART_SendData(USART2, msg[i]);
	 }
}

void NMI_Handler(){
}

