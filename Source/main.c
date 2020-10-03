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
void h_bridge_run(void);
void h_bridge_stop(void);

volatile uint32_t 			systick = 0;
static Stream_function_t 	logger_function_list[2] = {uart_output_stream,itm_output_stream};
Timer_stopwatch_t 			duration;
ADC_t 						internal_temperature;
Ring_t						uart_com_ring;
volatile uint8_t  			console_enter_pressed = 0;
volatile Projector_req_t	projector_request;
struct Projector_tag		projector_data = {0};
IO_pin_t    				led_1,led_2,pb_1,pb_2,power_enable;
Timer_H_bridge_t 			power_stage;
struct Timer_config_tag		chirp_update = {TIM5,80};
const char console_help[] = "Ultrasound Generator Ver 1.0\r\n\r\n"
                            "Command \t\t\t Action\r\n\r\n"
                            "> id[Enter] \t\t\t Get Device ID (AE17D1A7)\r\n"
                            "> cfg[Enter] \t\t\t Last Device Configuration\r\n\r\n"
                            "> cfg: freq1(Hz) freq2(Hz) duration(us) pps[Enter]\r\n\r\n"
							"** [Example] cfg: 90000 160000 10000 10[Enter]\r\n\r\n"
                            "** pps: Pulse Per Second\r\n"
                            "** [Enter] should be \\n[LF]\r\n";
int main()
{
	SystemCoreClockUpdate();
	// initialize log system 
	Logger_Init( logger_function_list , itm_output_stream );
	
	LOGGER_PRINT("Hello UART! \r\n");

	// enable delay function
	SysTick_Config( SystemCoreClock / 1000 );
	Projector_Interface_Configure();
	uart_com_ring = Ring_Create(50);

	projector_data._freq1 = 90000;
	projector_data._freq2 = 110000;
	projector_data._duration = 1000;
	projector_data._pulse_per_second = 5;
	projector_data._pulse_period = 1000 / projector_data._pulse_per_second;
	Projector_Data_Validate(&projector_data);
	Uart_t uart_com_port = UART_Create( & (struct Uart_configure_tag) 
								{._baudrate			=	460800,
								._rx_transfer_type 	= 	UART_TRANSFER_IRQ,
								._tx_transfer_type	=	UART_TRANSFER_DMA,
								._uart_address		=	USART3}	);

	power_stage = Timer_H_Bridge_Create(&(struct Timer_config_tag) { TIM1 , 80 });
	power_stage->_frequency = projector_data._freq1;
	power_stage->_duty_cycle = 50;
	Projector_Chirp_Create(&projector_data);
	Timer_Create(&chirp_update);

	// Timer_H_Bridge_Run(power_stage);
	char id_buff[] = "\r\nDevice ID Number: AE17D1A7\r\n";
	char done_buff[] = "\r\nDone\r\n";
//	char test_buff1[10];
	char temp_buff[100];
	uint16_t temp_len;
	UART_Transmit(uart_com_port,console_help,sizeof(console_help));
	UART_Receive(uart_com_port,NULL,10);
	while(1)
	{
		// ADC_Sample_IRQ(internal_temperature);
		// UART_Transmit(uart_com_port,test_buff,6);
		Delay_Ms(10);
		if(console_enter_pressed)
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
			Projector_Data_Validate(&projector_data);
			Projector_Chirp_Create(&projector_data);
			UART_Transmit(uart_com_port,done_buff,sizeof(done_buff));
			projector_request &= ~(configuration_set);
			break;	
		default:
			break;
		}
	}

}

void h_bridge_run(void)
{
	Timer_H_Bridge_Run(power_stage);
}
void h_bridge_stop(void)
{
	Timer_H_Bridge_Stop(power_stage);
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

