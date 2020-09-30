/*
 * main.c
 *
 *  Created on: Sep 6, 2020
 *      Author: Ashkan
 */
#include <stdlib.h>
#include "stm32f4xx.h"
#include "system_stm32f4xx.h"
#include "logger.h"
#include "stm32_io.h"
#include "delay.h"
#include "stm32_timer.h"
#include "stm32_adc.h"
#include "stm32_uart.h"
#include "ring_buffer.h"
#include "projector.h"
#include "printf.h"

static void uart_output_stream(const Logger_buff_t* data);
static void itm_output_stream(const Logger_buff_t* data);


volatile uint32_t 			systick = 0;
static Stream_function_t 	logger_function_list[2] = {uart_output_stream,itm_output_stream};
Timer_stopwatch_t 			duration;
ADC_t 						internal_temperature;
Ring_t						uart_com_ring;
volatile uint8_t  			uart_com_enter_pressed = 0;
volatile Projector_req_t	projector_request;
struct Projector_tag		projector_data = {0};

int main()
{
	SystemCoreClockUpdate();
	// initialize log system 
	Logger_Init( logger_function_list , itm_output_stream );
	
	LOGGER_PRINT("Hello UART! \r\n");

	// enable delay function
	SysTick_Config( SystemCoreClock / 1000 );
	
	uart_com_ring = Ring_Create(50);


	Uart_t uart_com_port = UART_Create( & (struct Uart_configure_tag) 
								{._baudrate			=	460800,
								._rx_transfer_type 	= 	UART_TRANSFER_IRQ,
								._tx_transfer_type	=	UART_TRANSFER_DMA,
								._uart_address		=	USART3}	);

	char id_buff[] = "\r\nDevice ID Number: AE17D1A7\r\n";
	char done_buff[] = "\r\nDone\r\n";
//	char test_buff1[10];
	char temp_buff[100];
	uint16_t temp_len;
	UART_Receive(uart_com_port,NULL,10);
	while(1)
	{
		// ADC_Sample_IRQ(internal_temperature);
		// UART_Transmit(uart_com_port,test_buff,6);
		Delay_Ms(10);
		if(uart_com_enter_pressed)
		{
			Parse_Uart_Comm(uart_com_ring);
		}
		switch (projector_request)
		{
		case id_req:
			UART_Transmit(uart_com_port,id_buff,sizeof(id_buff));
			projector_request &= ~(id_req);
			break;
		case configuration_req:
			temp_len = snprintf(temp_buff,100,
				"\r\nFreq1: %d\r\nFreq2: %d\r\nDuration: %d\r\nPulse Per Sec: %d\r\n",
				projector_data._freq1, projector_data._freq2, 
				projector_data._duration, projector_data._pulse_per_second );
			UART_Transmit(uart_com_port,temp_buff,temp_len);
			projector_request &= ~(configuration_req);
			break;
		case configuration_set:
			UART_Transmit(uart_com_port,done_buff,sizeof(done_buff));
			projector_request &= ~(configuration_set);
			break;	
		default:
			break;
		}
	}

}

static void uart_output_stream(const Logger_buff_t* data)
{
	asm("NOP");
}


static void itm_output_stream(const Logger_buff_t* data)
{
	asm("NOP");
	while(*data)
		ITM_SendChar(*data++);
}
