#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>
#include <stm32f10x.h>


TaskHandle_t xTaskHandleM = NULL;
TaskHandle_t xTaskHandleE = NULL;
// Global Space for message
char user_msg[250] = {0};

// function prototypes
static void prvSetupHardware(void);
void printmsg(char *msg);

// Tasks
static void vManagerTask(void *pvParameters);
static void vEmployeeTask(void *pvParameters);

// Binary Semaphore which used to synchronize the manage an dEmployee tasks
xSemaphoreHandle xWork;

// This is the queue which manager uses to put work ticket id
xQueueHandle xWorkQueue;



int main(){
	RCC_DeInit(); // 8MHz System Clock
	// Update core clock to reset
	SystemCoreClockUpdate();
	
	prvSetupHardware();
	sprintf(user_msg, "Binary Semaphore Task Sync\r\n");
	printmsg(user_msg);
	
	xWork = xSemaphoreCreateBinary();
	
	
	xWorkQueue = xQueueCreate(1, sizeof(unsigned int));
	
	if (xWork != NULL && xWorkQueue != NULL){
		// Manager Task
		xTaskCreate(vManagerTask, "Manager", 500, NULL, 3, NULL);
		// Employee Task
		xTaskCreate(vEmployeeTask, "Employee", 500, NULL, 1, NULL);
		vTaskStartScheduler();
	}
	sprintf(user_msg, "Semaphore/Queue creation failed\r\n");
	printmsg(user_msg);
	
}

void rtos_delay(uint32_t delay_ms){
	uint32_t current_tick_count = xTaskGetTickCount();
	uint32_t delay_in_ticks = (delay_ms * configTICK_RATE_HZ)/1000;
	while(xTaskGetTickCount() < (current_tick_count + delay_in_ticks));
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


void EmployeeDoWork(unsigned int TicketId){
	// Implement the work as per ticketID
	sprintf(user_msg, "Employee Task: Working on Ticket id: %d\r\n", TicketId);
	printmsg(user_msg);
	vTaskDelay(TicketId);
}
void vManagerTask(void *pvParameters){
	unsigned int xWorkTicketId;
	portBASE_TYPE xStatus;
	xSemaphoreGive(xWork);
	while(1){
		xWorkTicketId = (rand() & 0x1FF);
		// Send work ticket to the work queue
		xStatus = xQueueSend(xWorkQueue, &xWorkTicketId, portMAX_DELAY);
		
		if(xStatus != pdPASS){
			sprintf(user_msg, "Could not send to queue\r\n");
			printmsg(user_msg);
		} else{
			// Manager notifying the employee by Giving semaphore
			xSemaphoreGive(xWork);
			taskYIELD();
		}
	}
}

void vEmployeeTask(void *pvParameters){
	unsigned int xWorkTicketId;
	portBASE_TYPE xStatus;
	while(1){
		// First Employee Tries to take the semaphore, if it it available 
		xSemaphoreTake(xWork, 0);// semaphore do not block if semaphore not availabe
		//Semaphore tasken successfuly, Get the ticket
		xStatus = xQueueReceive(xWorkQueue, &xWorkTicketId, 0);
		if (xStatus == pdPASS){
			// Employee may decode the xWorkTicketId in this function to do the work
			EmployeeDoWork(xWorkTicketId);
		}else{
			sprintf(user_msg, "Employee Task: Queue is empty, noting to do\r\n");
			printmsg(user_msg);
		}
	}
}

