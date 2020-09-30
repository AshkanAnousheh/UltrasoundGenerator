#include <stdlib.h>
#include "stm32_uart.h"
#include "stm32_io.h"


// struct Uart_data_tag
// {
//     USART_TypeDef*          _uart;
//     Uart_configure_t        _config;
//     uint8_t                 _initialized;
// };

static uint8_t transmit_block( Uart_t uart );
static uint8_t transmit_irq( Uart_t uart );
static uint8_t transmit_dma( Uart_t uart );
static uint8_t receive_block( Uart_t uart );
static uint8_t receive_irq( Uart_t uart );
static uint8_t receive_dma( Uart_t uart );
static uint8_t uart_config(Uart_configure_t configure);
static uint8_t transmit_dma_config(Uart_t uart);
static uint8_t receive_dma_config(Uart_t uart);
//  ***********************************************************



Uart_t UART_Create(Uart_configure_t configure)
{
    Uart_t uart = malloc(sizeof(struct Uart_tag));
    // // associate allocated memory address to global variable Uart;
    // Uart._data = uart;
    // associate configuration struct address to allocated variable uart;
    uart->_config = configure;
    // // associate specific peripheral address to local static variable;
    // uart->_config->_uart = configure->_uart;
    
    uart_config(configure);
    switch (configure->_rx_transfer_type)
    {
    case UART_TRANSFER_IRQ:
        uart->_receive = receive_irq;   break;
    case UART_TRANSFER_DMA:
        uart->_receive = receive_dma;
        receive_dma_config(uart);
        break;
    default:   
        uart->_receive = receive_block; break;
    }
    switch (configure->_tx_transfer_type)
    {
    case UART_TRANSFER_IRQ:
        uart->_transmit = transmit_irq;   break;
    case UART_TRANSFER_DMA:
        uart->_transmit = transmit_dma;
        transmit_dma_config(uart);
        break;
    default:   
        uart->_transmit = transmit_block; break;
    }

    return uart;

}
void    UART_Destroy(Uart_t uart)
{
    free(uart);
}

static uint8_t uart_config(Uart_configure_t configure)
{
    // using pb10 and pb11 in uart3 ( change it if other pins selected )
    IO_pin_t pin_b_10 = IO_Create(GPIOB, io_pin_10,(IO_configure_t) 
                                { MODE_AF, TYPE_PP, PULL_NO, SPEED_HI, AF_7 } );
    IO_pin_t pin_b_11 = IO_Create(GPIOB, io_pin_11,(IO_configure_t) 
                                { MODE_AF, TYPE_PP, PULL_NO, SPEED_HI, AF_7 } );
    
    if(configure->_uart_address == USART1)
        RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    if(configure->_uart_address == USART2)
        RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    if(configure->_uart_address == USART3)
        RCC->APB1ENR |= RCC_APB1ENR_USART3EN;
    if(configure->_uart_address == UART4)
        RCC->APB1ENR |= RCC_APB1ENR_UART4EN;
    if(configure->_uart_address == UART5)
        RCC->APB1ENR |= RCC_APB1ENR_UART5EN;
    if(configure->_uart_address == USART6)
        RCC->APB2ENR |= RCC_APB2ENR_USART6EN;

    // uart3 connected to APB1 which is equal to 42MHz (SystemCoreClock = 168MHz)
    uint32_t uartdiv = ( 42000000 / (configure->_baudrate) );
    uartdiv = ( (uartdiv / 16 ) << 4) | ( uartdiv % 16 );
    configure->_uart_address->BRR = ( uartdiv ); 
 
    configure->_uart_address->CR1 &= ~( 0x0000FFFF );
    configure->_uart_address->CR1 |= (USART_CR1_UE | USART_CR1_TE | USART_CR1_RE);
 
    if (configure->_rx_transfer_type == UART_TRANSFER_IRQ)
        configure->_uart_address->CR1 |= USART_CR1_RXNEIE;
    else if (configure->_rx_transfer_type == UART_TRANSFER_DMA)
    {
        configure->_uart_address->CR3 |= USART_CR3_DMAR;
        if((configure->_uart_address == USART1) || (configure->_uart_address == USART6))
            RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;
        else
            RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
    }
    if (configure->_tx_transfer_type == UART_TRANSFER_IRQ)
        configure->_uart_address->CR1 |= USART_CR1_TXEIE;
    else if (configure ->_tx_transfer_type == UART_TRANSFER_DMA)
    {
        configure->_uart_address->CR3 |= USART_CR3_DMAT;
        if((configure->_uart_address == USART1) || (configure->_uart_address == USART6))
            RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;
        else
            RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
    }
    // configure->_baudrate;
}

void UART_Transmit( Uart_t uart, Uart_buff_t buff, uint16_t len )
{
    uart->_tx_buffer = buff;
    uart->_tx_len = len;
    uart->_transmit( uart );
}
void UART_Receive( Uart_t uart, Uart_buff_t buff, uint16_t len )
{
    uart->_rx_buffer = buff;
    uart->_rx_len = len;
    uart->_receive( uart );
}

static uint8_t transmit_block( Uart_t uart )
{
    for ( int cnt = 0; cnt < uart->_tx_len; cnt++ )
    {
        while( !((uart->_config->_uart_address->SR) & USART_SR_TXE) );
        uart->_config->_uart_address->DR = uart->_tx_buffer[cnt];
    }
}
static uint8_t transmit_irq( Uart_t uart )
{
    // commented code should be in the ISR
    // uart->_transmit(uart);
    if(uart->_tx_idx < uart->_tx_len)
        uart->_config->_uart_address->DR = uart->_tx_buffer[uart->_tx_idx++];
    else
        uart->_tx_idx = 0;
}
static uint8_t transmit_dma( Uart_t uart )
{
    Uart_buff_t buff =  uart->_tx_buffer;

    DMA1->LIFCR |= DMA_LIFCR_CTCIF3;
    while(DMA1->LISR & DMA_LISR_TCIF3);

    DMA1->LIFCR |= DMA_LIFCR_CTCIF3;
    while(DMA1->LISR & DMA_LISR_TCIF3);
//  introduce memory address to the dma
    DMA1_Stream3->M0AR  = (uint32_t) (buff);
//  introduce peripheral to the dma
    DMA1_Stream3->PAR   = (uint32_t)& (uart->_config->_uart_address->DR);
//  determine transfer len
    DMA1_Stream3->NDTR  = (uint16_t) (uart->_tx_len);

//  enable dma
    DMA1_Stream3->CR |= (DMA_SxCR_EN);
}
static uint8_t receive_block( Uart_t uart )
{
    for ( int cnt = 0; cnt < uart->_rx_len; cnt++ )
    {
        while( !((uart->_config->_uart_address->SR) & USART_SR_RXNE) );
        uart->_rx_buffer[cnt] = uart->_config->_uart_address->DR;
    }
}
static uint8_t receive_irq( Uart_t uart )
{
    NVIC_SetPriority(USART3_IRQn,0x01);
    NVIC_EnableIRQ(USART3_IRQn);

    // main code should be in the ISR
    // use ring_buffer
}
static uint8_t receive_dma( Uart_t uart )
{
    Uart_buff_t buff =  uart->_tx_buffer;

    DMA1->LIFCR |= DMA_LIFCR_CTCIF3;
    while(DMA1->LISR & DMA_LISR_TCIF3);
//  introduce memory address to the dma
    DMA1_Stream3->M0AR  = (uint32_t) (buff);
//  introduce peripheral to the dma
    DMA1_Stream3->PAR   = (uint32_t)& (uart->_config->_uart_address->DR);
//  determine transfer len
    DMA1_Stream3->NDTR  = (uint16_t) (uart->_tx_len);

//  enable dma
    DMA1_Stream1->CR |= (DMA_SxCR_EN);
}


static uint8_t transmit_dma_config(Uart_t uart)
{
    Uart_buff_t buff =  uart->_tx_buffer;
// This settings should configure for each uart seperately.
    DMA1_Stream3->CR &= ~(DMA_SxCR_EN | DMA_SxCR_CHSEL | DMA_SxCR_CIRC | DMA_SxCR_DIR |
                          DMA_SxCR_MSIZE | DMA_SxCR_PSIZE | DMA_SxCR_PINC | DMA_SxCR_PL);
//  config: channel4 - priority med - memory incremented - direction memory to peripheral
    DMA1_Stream3->CR |= (DMA_SxCR_CHSEL_2 | DMA_SxCR_PL_0 | DMA_SxCR_MINC | DMA_SxCR_DIR_0);
//  be sure dma stream turned off and no any active stream exist.
    while( DMA1_Stream3->CR & DMA_SxCR_EN );
//  introduce memory address to the dma
    DMA1_Stream3->M0AR = (uint32_t) (buff);
//  introduce peripheral to the dma
    DMA1_Stream3->PAR = (uint32_t)& (uart->_config->_uart_address->DR);
//  determine transfer len
    DMA1_Stream3->NDTR = (uint16_t) (uart->_tx_len);
}

static uint8_t receive_dma_config(Uart_t uart)
{
// This settings should configure for each uart seperately.
    DMA1_Stream1->CR &= ~(DMA_SxCR_EN | DMA_SxCR_CHSEL | DMA_SxCR_CIRC | DMA_SxCR_DIR |
                          DMA_SxCR_MSIZE | DMA_SxCR_PSIZE | DMA_SxCR_PINC | DMA_SxCR_PL);
//  config: channel4 - priority med - memory incremented - direction peripheral to memory
    DMA1_Stream1->CR |= (DMA_SxCR_CHSEL_2 | DMA_SxCR_PL_0 | DMA_SxCR_MINC );
//  be sure dma stream turned off and no any active stream exist.
    while( DMA1_Stream1->CR & DMA_SxCR_EN );
//  introduce memory address to the dma
    DMA1_Stream1->M0AR = (uint32_t)& (uart->_rx_buffer);
//  introduce peripheral to the dma
    DMA1_Stream1->PAR = (uint32_t)& (uart->_config->_uart_address->DR);
//  determine transfer len
    DMA1_Stream1->NDTR = (uint16_t) (uart->_rx_len);
}