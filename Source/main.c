/*
 * main.c
 *
 *  Created on: Sep 6, 2020
 *      Author: Ashkan
 */
#include "main.h"

int main()
{
	SystemCoreClockUpdate();
	// initialize log system 
	Logger_Init( logger_function_list , itm_output_stream );
	
	LOGGER_PRINT("Hello UART! \r\n");

	// Enable delay function
	SysTick_Config( SystemCoreClock / 1000 );
	// Leds And Push button configuration
	Projector_Interface_Configure();
	// Create ring buffer for uart console communication
	uart_com_ring = Ring_Create(50);
	// Initialize projector_data -> this structure define main task of this project
	projector_data._freq1 = 90000;
	projector_data._freq2 = 110000;
	projector_data._duration = 1000;
	projector_data._pulse_per_second = 5;
	projector_data._pulse_period = 1000 / projector_data._pulse_per_second;
	// Validate above Initialization
	Projector_Data_Validate(&projector_data);
	// Initialize a com port to console
	Uart_t uart_com_port = UART_Create( & (struct Uart_configure_tag) 
								{._baudrate			=	460800,
								._rx_transfer_type 	= 	UART_TRANSFER_IRQ,
								._tx_transfer_type	=	UART_TRANSFER_DMA,
								._uart_address		=	USART3}	);
	// Initialize a timer for H Bridge application ( Power Stage Driver )
	power_stage = Timer_H_Bridge_Create(&(struct Timer_config_tag) { TIM1 , 80 });
	power_stage->_frequency = projector_data._freq1;
	power_stage->_duty_cycle = 50;
	// Initialize "Frequency" And "Duty cycle" Buffers with LFM data (chirp data)
	Projector_Chirp_Create(&projector_data);
	// Create timer to update H Bridge output with desired frequency sweep (10us rate)
	Timer_Create(& (struct Timer_config_tag) {._timer_address = TIM5});
	// Transmit Console help data to uart terminal
	UART_Transmit(uart_com_port,console_help,sizeof(console_help));
	// activate uart receiving by interrupt
	UART_Receive(uart_com_port,NULL,10);
	// start main loop
	while(1)
	{
// *********** ********** ********* ********** ********** **********
		if(projector_data._chirp._index >= projector_data._chirp._size)
		{
			projector_data._chirp._index = 0;
			Timer_H_Bridge_Stop(power_stage);

			pulse_count++;
			if(pulse_count < projector_data._pulse_per_second)
			{
				Timer_Alarm(& (struct Timer_alarm_tag) {._time = projector_data._pulse_period,
														._action = H_Bridge_Run});
			}
			else 
			{
				pulse_count = 0;
				button1_lock = 0;
			}

			IO._set(led_1);
		}
// *********** ********** ********* ********** ********** **********
		if(console_enter_pressed)
		{
			Projector_Parse_Console(uart_com_ring);
		}
// *********** ********** ********* ********** ********** **********
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
// *********** ********** ********* ********** ********** **********
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

