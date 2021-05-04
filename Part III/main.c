#include "sam.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "uart.h"
#include "myprintf.h"

/* Priorities at which the tasks are created. */
#define	myTASK_TASK_PRIORITY ( tskIDLE_PRIORITY + 1 )
#define	NoID 0x0
#define	IDRight 0x1
#define	IDLeft 0x2
#define	IDUp 0x3
#define	IDDown 0x4
#define IDReturn 0x5
#define Wait	500
#define	Clear	0X00000000
#define GroupReg	0
#define Freq	512
xQueueHandle Global_Queue_handle = 0;
//Task in charge of detecting the button press and sending the information to the second task.
void first_task(void *p){
	static uint32_t i = NoID;
	while(1){
		if((PORT->Group[GroupReg].IN.reg & PORT_IN_IN( PORT_PA15)) == _U_(Clear))
		i=IDRight;
		else if((PORT->Group[GroupReg].IN.reg & PORT_IN_IN( PORT_PA14)) == _U_(Clear))
		i=IDUp;
		else if((PORT->Group[GroupReg].IN.reg & PORT_IN_IN( PORT_PA09)) == _U_(Clear))
		i=IDDown;
		else if((PORT->Group[GroupReg].IN.reg & PORT_IN_IN( PORT_PA08)) == _U_(Clear))
		i=IDLeft;
		else{
			myprintf("Failed to detect input\n");
			i=NoID;
		}
		if(i!=NoID){
			if(! xQueueSend(Global_Queue_handle , &i , Wait)){
				myprintf("Failed to send to  queue\n");
			}
			else{
				myprintf("Message sent\n");
				while (xQueueReceive(Global_Queue_handle, &i, Wait)){
					myprintf("Waiting Ack\n");
				}
				if (i==IDReturn)
				myprintf("Messsage acknowledged\n");
				i=NoID;
			}
		}
		vTaskDelay(Wait);
	}
}
//Second task, it receves the messages of the first task and prints the corresponding messages.
void second_task(void *p){
	static uint32_t rx_int = NoID;
	while(1){
		if(xQueueReceive(Global_Queue_handle , &rx_int , Wait)){
			if(rx_int==IDRight){
				myprintf("Right \n");
			}
			else if(rx_int==IDDown){
				myprintf("Down \n");
			}
			else if(rx_int==IDUp){
				myprintf("Up \n");
			}
			else if(rx_int==IDLeft){
				myprintf("Left \n");
			}
			rx_int= IDReturn;
			if(! xQueueSend(Global_Queue_handle , &rx_int , Wait)){
				myprintf("Failed to send to  queue\n");
			}
		}
		vTaskDelay(Wait);
	}
}

int main()
{
	SystemInit();
	/* Switch to 8MHz clock (disable prescaler) */
	SYSCTRL->OSC8M.bit.PRESC = GroupReg;
	initUART();
	//Queue declaration
	Global_Queue_handle = xQueueCreate(3,sizeof(int));
	//Pin declaration for input and GPIO mode
	PORT->Group[GroupReg].PINCFG[PIN_PA15].reg =0X2;
	PORT->Group[GroupReg].PINCFG[PIN_PA14].reg =0X2;
	PORT->Group[GroupReg].PINCFG[PIN_PA09].reg =0X2;
	PORT->Group[GroupReg].PINCFG[PIN_PA08].reg =0X2;
	PORT->Group[GroupReg].PINCFG[PIN_PA27].reg =0X0;
	
	PORT->Group[GroupReg].DIRCLR.reg = PORT_PA15;// PIN D2 AS INPUT
	PORT->Group[GroupReg].DIRCLR.reg = PORT_PA14;// PIN D3 AS INPUT
	PORT->Group[GroupReg].DIRCLR.reg = PORT_PA09;// PIN D4 AS INPUT
	PORT->Group[GroupReg].DIRCLR.reg = PORT_PA08;// PIN D5 AS INPUT
	//Tasks declaration
	xTaskCreate(first_task , "TX", Freq , NULL , myTASK_TASK_PRIORITY , NULL);
	xTaskCreate(second_task , "RX", Freq , NULL , myTASK_TASK_PRIORITY , NULL);
	
	/* Start the tasks and timer running. */
	vTaskStartScheduler();

	for( ;; );
	return(0);
}
