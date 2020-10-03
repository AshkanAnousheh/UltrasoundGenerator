#ifndef PROJECTOR_MAIN_H
#define PROJECTOR_MAIN_H

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

extern void H_Bridge_Run(void);
extern void H_Bridge_Stop(void);

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
Timer_config_t      		chirp_update;
uint8_t 					button1_lock = 0;
static uint16_t             pulse_count = 0;
char                        temp_buff[100];
uint16_t                    temp_len;

const char id_buff[] = "\r\nDevice ID Number: AE17D1A7\r\n";
const char done_buff[] = "\r\nDone\r\n";
const char console_help[] = "Ultrasound Generator Ver 1.0\r\n\r\n"
                            "Command \t\t\t Action\r\n\r\n"
                            "> id[Enter] \t\t\t Get Device ID (AE17D1A7)\r\n"
                            "> cfg[Enter] \t\t\t Last Device Configuration\r\n\r\n"
                            "> cfg: freq1(Hz) freq2(Hz) duration(us) pps[Enter]\r\n\r\n"
							"** [Example] cfg: 90000 160000 10000 10[Enter]\r\n\r\n"
                            "** pps: Pulse Per Second\r\n"
                            "** [Enter] should be \\n[LF]\r\n";

#endif