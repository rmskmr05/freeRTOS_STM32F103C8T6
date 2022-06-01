
#include <stm32f10x.h>
#include <String.h>
#include <stdio.h>
#include <FreeRTOS.h>
#include "Task.h"

TaskHandle_t xLedHandle, xButtonHandle;
void vLedControl(void *pvParameters);
void vButtonRead(void *pvParameters);
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
	sprintf(user_msg, "This is Hello World Application Starting\r\n");  
	printmsg(user_msg);
	//  Create Tasks
	xTaskCreate(vLedControl, "LED_TASK", configMINIMAL_STACK_SIZE, NULL, 2, &xLedHandle);

	/* Start the Schedular */
	vTaskStartScheduler();
	
	// This place will be never reached by the code unless error in scheduler
	for(;;);

}

void vLedControl(void *pvParameters){
	while(1){
		if (buttonStatusFlag == PRESSED){
			// Turn On the LEd
			GPIO_WriteBit(GPIOB, GPIO_Pin_0, Bit_SET);
		}else{
			GPIO_WriteBit(GPIOB, GPIO_Pin_0, Bit_RESET);
		}
		/*
		if (UART_ACCESS_KEY == AVAILABLE){
			UART_ACCESS_KEY = NOT_AVAILABLE;
			printmsg("FIRST\r\n");
			UART_ACCESS_KEY = AVAILABLE;
			taskYIELD();
		}
		*/
	}
}

void vButtonHandle(void){ 
	buttonStatusFlag ^= 1;
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
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	// Button Pin
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	// Button Interruppt configuration (PB1)
	//1. Clock Enbale for AFIO Block
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource1);
	//2. Configure EXTI BLOCK (Rising Edge, Interrupt  mode)
	EXTI_InitTypeDef EXTI_InitStruct;
	EXTI_InitStruct.EXTI_Line = EXTI_Line1;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStruct);

	//3. NVIC Settings for Selected EXTI Lines (1)
	NVIC_SetPriority(EXTI1_IRQn, 5); // Set the interrupt priority (0(hihest)-15)
	NVIC_EnableIRQ(EXTI1_IRQn);
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

void EXTI1_IRQHandler(){
	//1. Clear the interrupt pending bit of EXTI line
	EXTI_ClearITPendingBit(EXTI_Line1);
	vButtonHandle();
	
}

void NMI_Handler(){
}

