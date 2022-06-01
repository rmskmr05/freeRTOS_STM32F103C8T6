
#include <stm32f10x.h>
#include <String.h>
#include <stdio.h>
#include <FreeRTOS.h>
#include <Task.h>
#include <queue.h>
#include <timers.h>
#include <RTC.h>

#define TRURE 1
#define FALSE 0

// Commands
#define LED_ON_CMD 						1
#define LED_OFF_CMD 					2
#define LED_TOGGLE_CMD 				3
#define LED_TOGGLEL_STOP_CMD 	4
#define LED_READ_STATE_CMD 		5
#define RTC_READ_DATETIME 		6

uint8_t cmd_buffer[20];
uint8_t cmd_length = 0;

// Task handles
TaskHandle_t xTask1Handle = NULL, xTask2Handle = NULL, xTask3Handle = NULL, xTask4Handle = NULL;
// Task prototypes
void vTask1_DisplayMenu(void *pvParameters);
void vTask2_CmdHandle(void *pvParameters);
void vTask3_CmdPrecess(void *pvParameters);
void vTask4_UartWrite(void *pvParameters);
// Queue Handle
QueueHandle_t CmdQueue = NULL, uartWriteQueue = NULL;
// Software Timer handle
TimerHandle_t ledTimerHandle = NULL;

static void prvSetupHardware(void);
void printmsg(char *msg);

char menu[] = {"\r\nEXIT_Application --> 0\
\r\nLED_ON --> 1\
\r\nLED_OFF	--> 2\
\r\nLED_TOGGLE --> 3\
\r\nLED_TOGGLE_OFF --> 4\
\r\nLED_READ_STATE --> 5\
\r\nRTC_PRINT_DATETIME --> 6\
\r\nTYPE YOUR CHOICE "};


// Command Structure
typedef  struct AppCmd{
	uint8_t CmdNum;
	uint8_t CmdArg[10];
}AppCmd_t;

char user_msg[250];
int main(){
	
	// 1.  Reset the RCC Clock configuration to default reset state
	RCC_DeInit(); // 8MHz System Clock

	// Update core clock to reset
	SystemCoreClockUpdate();
	
	prvSetupHardware();
	
	
	// Create Queues
	// Command Queue
	CmdQueue = xQueueCreate(10, sizeof(AppCmd_t *)); // instead of whole data & pointer to data type is used to create queue (requres less memory)
	uartWriteQueue = xQueueCreate(10, sizeof(char *));
	  
	if (CmdQueue != NULL){
		//  Create Tasks
		uint8_t t1 = xTaskCreate(vTask1_DisplayMenu, "TASK1-Menu", (configMINIMAL_STACK_SIZE * 3), NULL, 1, &xTask1Handle);
		uint8_t t2 = xTaskCreate(vTask2_CmdHandle, "TASK2-CMD_HANDLE", (configMINIMAL_STACK_SIZE * 3), NULL, 2, &xTask2Handle);
		uint8_t t3 = xTaskCreate(vTask3_CmdPrecess, "TASK3-CMD_PROCESS", (configMINIMAL_STACK_SIZE * 3), NULL, 2, &xTask3Handle);
		uint8_t t4 = xTaskCreate(vTask4_UartWrite, "TASK4-UART_WRITE", (configMINIMAL_STACK_SIZE * 3), NULL, 2, &xTask4Handle);
		
		if ((t1 == pdTRUE) & (t2 == pdTRUE) & (t3 == pdTRUE) & (t4 == pdTRUE)){ 
			sprintf(user_msg, "\r\nQueue & Task Creation Successful \r\n");  
			printmsg(user_msg);
		}else{
			sprintf(user_msg, "\r\nTask Creation Failed \r\n");  
			printmsg(user_msg);
		}
		/* Start the Schedular */
		vTaskStartScheduler();
		
	}else{
		sprintf(user_msg, "Queue Creation Failed\r\n");  
		printmsg(user_msg);
	}
	
	// This place will be never reached by the code unless error in scheduler
	for(;;);

}

void rtos_delay(uint32_t delay_ms){
	uint32_t current_tick_count = xTaskGetTickCount();
	uint32_t delay_in_ticks = (delay_ms * configTICK_RATE_HZ)/1000;
	while(xTaskGetTickCount() < (current_tick_count + delay_in_ticks));
}

uint8_t state = 0;


//Menu -> Queue
void vTask1_DisplayMenu(void *pvParameters){
	sprintf(user_msg, "\r\nT1 \r\n");  
	printmsg(user_msg);
	char *pData = menu; 
	while(1){
		// Post menu to Queue
		xQueueSend(uartWriteQueue, &pData, portMAX_DELAY);
		// Wait for the notification
		xTaskNotifyWait(0, 0, NULL, portMAX_DELAY); 

	}
}

void getArguments(uint8_t *buffer){
}
uint8_t getCmdCode(uint8_t *buffer){
	return (buffer[0] - 48);
}
	

void vTask2_CmdHandle(void *pvParameters){
	uint8_t cmdCode = 0;
	AppCmd_t *newCmd;
	while(1){
		xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);
		// Send Command to Queue
		
		// Not disturb code area (Serializing Shared Resources)
		taskENTER_CRITICAL();
		newCmd = (AppCmd_t*)pvPortMalloc(sizeof(AppCmd_t));
		cmdCode = getCmdCode(cmd_buffer);
		newCmd->CmdNum = cmdCode;
		getArguments(newCmd->CmdArg);
		taskEXIT_CRITICAL();
		
		// Send the Command to command to CmdQueue
		xQueueSend(CmdQueue, &newCmd, portMAX_DELAY);
	}
} 

// Software Timer callback function
void ledToggle(TimerHandle_t xTimer){
	uint8_t state = GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_0);
	state ^= 1;
	GPIO_WriteBit(GPIOB, GPIO_Pin_0, state);
}

void LedToggleOn(uint32_t duration){
	if (ledTimerHandle == NULL){
		// Create software Timers
		ledTimerHandle = xTimerCreate("LED_TIMER", duration, pdTRUE, NULL, ledToggle);
	}else{
		// Start the Software timer
		xTimerStart(ledTimerHandle, portMAX_DELAY);
	}
}

void LedToggleOff(){
	xTimerStop(ledTimerHandle, portMAX_DELAY);
}
void readLedState(char *task_msg){
	sprintf(task_msg, "\r\nLED State is: %d\r\n", GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_0));
	xQueueSend(uartWriteQueue, &task_msg, portMAX_DELAY);
}
void readRTC(char *task_msg){
	RTC_TimeTypeDef RTC_time;
	RTC_DateTypeDef RTC_date;
	// Read time and date from RTC peripheral
	RTC_GetTime(RTC_Format_BIN, &RTC_time);
	RTC_GetDate(RTC_Format_BIN, &RTC_date);
	
	sprintf(task_msg, "\r\nTime: %02d \rDate: %02d-%2d-%2d <%s>\r\n", RTC_time.RTC_Hours, RTC_time,RTC_Minutes, RTC_time.RTC, RTC_time.Seconds,
	RTC_date.RTC_Date, RTC_date.RTC_Month, RTC_date.RTC_Year, getDayofweek(RTC_date.RTC_Weekday));
	
	xQueueSend(uartWriteQueue, &task_msg, portMAX_DELAY)
}
void print_Error_Message(char *task_msg){
	sprintf(task_msg, "\r\nINVALID COMMAND\r\n");
	xQueueSend(uartWriteQueue, &task_msg, portMAX_DELAY);
}

void vTask3_CmdPrecess(void *pvParameters){
	char task_msg[25];
	AppCmd_t *newCmd;
	
 uint32_t toggle_duration = pdMS_TO_TICKS(500);
	while(1){
		// Read new Data from QUEUE
		xQueueReceive(CmdQueue, (void *)&newCmd, portMAX_DELAY);
		if (newCmd->CmdNum == LED_ON_CMD){
			// make Led ON
			GPIO_WriteBit(GPIOB, GPIO_Pin_0, Bit_SET);
		}else if (newCmd->CmdNum == LED_OFF_CMD){
			// Make led OFF
			GPIO_WriteBit(GPIOB, GPIO_Pin_0, Bit_RESET);
		}else if (newCmd->CmdNum == LED_TOGGLE_CMD){
			LedToggleOn(toggle_duration);
		}else if (newCmd->CmdNum == LED_TOGGLEL_STOP_CMD){
			LedToggleOff();
		}else if (newCmd->CmdNum == LED_READ_STATE_CMD){
			readLedState(task_msg);
		}else if (newCmd->CmdNum == RTC_READ_DATETIME){
			readRTC();
		}else{
			print_Error_Message(task_msg);
		}
		// Free the allocated Memory for new command
		vPortFree(newCmd);
	}
}

//Queue -> USER (via USART)
void vTask4_UartWrite(void *pvParameters){
	sprintf(user_msg, "\r\nT2 \r\n");  
	printmsg(user_msg);
	char *pData = NULL;
	while(1){
		xQueueReceive(uartWriteQueue, &pData, portMAX_DELAY);
		printmsg(pData);
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
	
	// Enbale UART Byte Reception Interrupt
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	// set priority in NVIC for USART2 (must follow interrupt priority rule of freeRTOS)
	NVIC_SetPriority(USART2_IRQn, 16);
	// Enable the USART2 IRQ in the NVIC
	NVIC_EnableIRQ(USART2_IRQn);
	
	/* Enable USART2 */  
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
	/*
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	*/
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


void USART2_IRQHandler(void){
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	uint16_t data_byte; // receive buffer
	if(USART_GetFlagStatus(USART2, USART_FLAG_RXNE)){
		// Data byte is received from user
		data_byte = USART_ReceiveData(USART2);
		// Store data in global buffer
		cmd_buffer[cmd_length++] = data_byte & 0xFF;
		if(data_byte == '\r'){
			// User finished entering a command
			cmd_length = 0;
			// Notify the command handling task
			xTaskNotifyFromISR(xTask2Handle, 0, eNoAction, &xHigherPriorityTaskWoken);
			xTaskNotifyFromISR(xTask1Handle, 0, eNoAction, &xHigherPriorityTaskWoken);
		}
	}
	// Yield the CPU to high priority task thant has woken up 
	if(xHigherPriorityTaskWoken){
		taskYIELD();
	}
	
}

