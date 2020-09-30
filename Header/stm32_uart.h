#ifndef STM32_UART_H
#define STM32_UART_H

#include "stm32f4xx.h"

/* Low Level Macro Defines */
#if 1
#define UART_TRANSFER_BLOCK     0x0
#define UART_TRANSFER_IRQ       0x1
#define UART_TRANSFER_DMA       0x2

#define UART_PARITY_EVEN        0x0 << 9
#define UART_PARITY_ODD         0x1 << 9
#define UART_PARITY_NO          0x0 << 10

#define UART_STOP_0             // 0.5 bit stop
#define UART_STOP_1             // 1 bit stop
#define UART_STOP_2             // 1.5 bit stop
#endif

// typedef struct Uart_data_tag*   Uart_data_t;
typedef char*   Uart_buff_t;
typedef struct  Uart_tag*           Uart_t;
typedef uint8_t (*Transfer_function_t) ( Uart_t );

struct Uart_configure_tag
{
    USART_TypeDef*      _uart_address;
    uint32_t            _baudrate;
    uint8_t             _tx_transfer_type;
    uint8_t             _rx_transfer_type;
    uint8_t             _parity_type;
    uint8_t             _stop_bit;

};
typedef struct Uart_configure_tag*  Uart_configure_t;

// typedef uint8_t (*Uart_function_t) ( Uart_data_t, Uart_buff_t, uint16_t );

struct Uart_tag
{
    Uart_buff_t             _tx_buffer;
    Uart_buff_t             _rx_buffer;
    uint16_t                _tx_len;
    uint16_t                _rx_len;
    volatile uint16_t       _tx_idx;
    volatile uint16_t       _rx_idx;
    Uart_configure_t        _config;
    Transfer_function_t     _transmit;
    Transfer_function_t     _receive;
};

/* Interfaces */

Uart_t  UART_Create(Uart_configure_t configure);
void    UART_Transmit(Uart_t uart, Uart_buff_t buff, uint16_t len);
void    UART_Receive(Uart_t uart, Uart_buff_t buff, uint16_t len);
void    UART_Destroy(Uart_t uart);
#endif