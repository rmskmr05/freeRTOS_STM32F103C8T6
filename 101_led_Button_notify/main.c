
#include <stm32f10x.h>
#include <String.h>
#include <stdio.h>
#include <FreeRTOS.h>
#include "Task.h"

TaskHandle_t xTask1Handle, xTask2Handle;
void vTask1Handler(void *pvParameters);
void vTask2Handler(void *pvParameters);
static void prvSetupHardware(void);
void printmsg(char *msg);

// MACROS
#define TRUE 1
#define FALSE 0
#define PRESSED 1
#define NOT_PRESSED FALSE
#define AVAILABLE TRUE
#define NOT_AVAILABLE 0
// Global variables
uint8_t UART_ACCESS_KEY = AVAILABLE;
uint8_t buttonStatusFlag = NOT_PRESSED;

char user_msg[250];
int main(){
	
	// 1.  Reset the RCC Clock configuration to default reset state
	RCC_DeInit(); // 8MHz System Clock

	// Update core clock to reset
	SystemCoreClockUpdate();
	
	prvSetupHardware();
	sprintf(user_msg, "This is Task Notification Demo\r\n");  
	printmsg(user_msg);
	//  Create Tasks
	xTaskCreate(vTask1Handler, "TASK-1", (configMINIMAL_STACK_SIZE * 3), NULL, 1, &xTask1Handle);
	xTaskCreate(vTask2Handler, "TASK-2", (configMINIMAL_STACK_SIZE * 3), NULL, 2, &xTask2Handle);

	/* Start the Schedular */
	vTaskStartScheduler();
	
	// This place will be never reached by the code unless error in scheduler
	for(;;);

}

void rtos_delay(uint32_t delay_ms){
	uint32_t current_tick_count = xTaskGetTickCount();
	uint32_t delay_in_ticks = (delay_ms * configTICK_RATE_HZ)/1000;
	while(xTaskGetTickCount() < (current_tick_count + delay_in_ticks));
}

uint8_t state = 0;
void vTask1Handler(void *pvParameters){
	//uint32_t current_notification_count = 0;
	while(1){
		// print the state of Led
		state = GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_0);
		sprintf(user_msg, "Status of Led %d\r\n ", state);
		printmsg(user_msg);
		vTaskDelay(pdMS_TO_TICKS(1000));
		//vTaskDelay(1000); // 1000 ticks
		/*
		//if(xTaskNotifyWait(0, 0, NULL, portMAX_DELAY) == pdTRUE){
		if(xTaskNotifyWait(0, 0, &current_notification_count, portMAX_DELAY) == pdTRUE){
			buttonStatusFlag ^= 1;
			//sprintf(user_msg, "Notification is Received\r\n");
			sprintf(user_msg, "Notification is Received: Press Count = %u \r\n", current_notification_count);
			printmsg(user_msg);
		}
		if (buttonStatusFlag == PRESSED){
			// Turn On the LEd
			GPIO_WriteBit(GPIOB, GPIO_Pin_0, Bit_SET);
		}else{
			GPIO_WriteBit(GPIOB, GPIO_Pin_0, Bit_RESET);
		}
		*/
	}
}

void vTask2Handler(void *pvParameters){
	while(1){
		// Toggle the GPIO Pin
		state ^= 1; 
		GPIO_WriteBit(GPIOB, GPIO_Pin_0, state);
		vTaskDelay(pdMS_TO_TICKS(1000));
		/*
		if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) == 0){
			// Button Debouncing
			rtos_delay(100);
			// Button is pressed, Send a notification to vLedControl task
			//xTaskNotify(xLedHandle, 0, eNoAction);
			xTaskNotify(xLedHandle, 0, eIncrement);
			rtos_delay(100);
		}
		*/
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

void prvSetupLedButton(){
	GPIO_InitTypeDef GPIO_InitStruct;
	// Enable CLock for the GPIO PORT
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	// Led Pin
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	// Button Pin
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
}
 

/*All Hardware Specific Configuration  */
static void prvSetupHardware(void){
	// UART Setup
	prvSetupUart();
	prvSetupLedButton();
}

void printmsg(char *msg){
	 for (uint32_t i = 0; i < strlen(msg); i++){
		 while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) != SET);
		 USART_SendData(USART2, msg[i]);
	 }
}

void NMI_Handler(){
}

