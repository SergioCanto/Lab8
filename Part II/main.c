/* Kernel includes. */
#include "sam.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "uart.h"
#include "myprintf.h"

/* Priorities at which the tasks are created. */
#define	myTASK_TASK_PRIORITY		( tskIDLE_PRIORITY + 1 )
//Queue handler that manages both tasks
xQueueHandle Global_Queue_handle = 0;
//Sender Task, function in charge of sending information to the receiver task
void sender_task(void *p){
	while(1){
		static uint32_t i = 0;
		myprintf("Send %d to receiver task\n",i);
		if(! xQueueSend(Global_Queue_handle , &i , 1000)){
			myprintf("Failed to send to  queue\n");
		}
		i++;
		vTaskDelay(3000);
	}
}
//Receiver task, function in charge of receiving the information of the sender task. It receives the number of times sender has sent  information.
void receiver_task(void *p){
	uint32_t rx_int =0;
	while(1){
		if(xQueueReceive(Global_Queue_handle , &rx_int , 1000)){
			myprintf("Received %d\n", rx_int);
		}
		else{
			myprintf("Failed to receive data from the sender task\n");
		}
	}
}
//Main function that has the initiation of system, UART and the Tasks
int main()
{
	SystemInit();
	/* Switch to 8MHz clock (disable prescaler) */
	SYSCTRL->OSC8M.bit.PRESC = 0;
	initUART();
	Global_Queue_handle = xQueueCreate(3,sizeof(int));
	
	xTaskCreate(sender_task , "TX", 512 , NULL , myTASK_TASK_PRIORITY , NULL);
	xTaskCreate(receiver_task , "RX", 512 , NULL , myTASK_TASK_PRIORITY , NULL);
	
	/* Start the tasks and timer running. */
	vTaskStartScheduler();

	for( ;; );
	return(0);
}
