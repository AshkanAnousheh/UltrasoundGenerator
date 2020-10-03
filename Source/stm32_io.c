#include <stdlib.h>
#include "stm32_io.h"
#include "logger.h"

/* Local Functions */
static uint16_t set(IO_pin_t me);
static uint16_t reset(IO_pin_t me);
static uint16_t read(IO_pin_t me);
static uint16_t toggle(IO_pin_t me);
static uint16_t port_read(IO_pin_t me);
static uint16_t irq_disable(IO_pin_t me);
static uint16_t irq_enable(IO_pin_t me,IO_irq_edge_t edge);
// static uint16_t pull(IO_t* me);
// static uint16_t speed(IO_t* me);
// static uint16_t alternate_function(IO_t* me);

static void     io_clock_enable(IO_pin_t me);



/* Structures */

// ********************** Client can't modify *********************
IO_t        IO = {  set,
                    reset,
                    read,
                    port_read,
                    toggle,
					irq_disable,
					irq_enable
                    };

struct IO_pin_tag
{
    GPIO_TypeDef*       _port;
    IO_pin_number_t     _pin;
};
// ****************************************************************

/* Local Variables */



/* Implementation */



IO_pin_t   IO_Create(GPIO_TypeDef* port,
                  IO_pin_number_t pin, IO_configure_t cfg)
{
    if(port == NULL) return NULL;

    IO_pin_t io = malloc(sizeof(struct IO_pin_tag));
    if(io == NULL)
        { LOGGER_PRINT("Pin: %d Creation Failed\r\n",pin);return NULL; }

    io->_port = port;
    io->_pin = pin;

    // Enable Port 
    io_clock_enable(io);
    // Configure Pin
    port->MODER &= ~(MODE_MASK << (pin*2));
    port->MODER |= (cfg._mode << (pin*2));

    port->OTYPER &= ~(TYPE_MASK << (pin));
    port->OTYPER |= (cfg._output_type << (pin));

    port->OSPEEDR &= ~(SPEED_MASK << (pin*2));
    port->OSPEEDR |= (cfg._speed << (pin*2));

    port->PUPDR &= ~(PULL_MASK << (pin*2));
    port->PUPDR |= (cfg._pull_type << (pin*2));

    if( (pin / 8) )
    {
        port->AFR[ 1 ] &= ~(0xF << ((pin - 8) * 4)); 
        port->AFR[ 1 ] |= (cfg._alternate_function << ((pin - 8) * 4)); 
    }
    else
    {
        port->AFR[ 0 ] &= ~(0xF << (pin * 4));
        port->AFR[ 0 ] |= (cfg._alternate_function << (pin * 4));
    }
    
    return io;

}

void    IO_Destroy(IO_pin_t io)
{
    free(io);
}

static uint16_t set(IO_pin_t me)
{
    me->_port->BSRRL = (1 << me->_pin);
}

static uint16_t reset(IO_pin_t me)
{
    me->_port->BSRRH = (1 << me->_pin);
}

static uint16_t read(IO_pin_t me)
{
    return (me->_port->IDR &= me->_pin)>>me->_pin;
}

static uint16_t toggle(IO_pin_t me)
{
    me->_port->ODR = ( (me->_port->IDR & me->_pin) ^ (1<< me->_pin) );
}

static uint16_t irq_enable(IO_pin_t me,IO_irq_edge_t edge)
{
    
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    // SYSCFG->EXTICR[(me->_pin/4)] &= ~(EXTI_MASK<< (me->_pin * 4));
    // uint32_t temp = (((uint32_t)(me->_port - GPIOA)/25) << (me->_pin % 4));
    // SYSCFG->EXTICR[(me->_pin/4)] |= temp;

    SYSCFG->EXTICR[(me->_pin/4)] &= ~(EXTI_MASK<< (me->_pin * 4));
    uint32_t temp = (1 << (me->_pin % 4)*4);
    SYSCFG->EXTICR[(me->_pin/4)] |= temp;

    EXTI->IMR |= (1 << me->_pin);
    if (edge == rising_edge)
    {
        EXTI->RTSR |= (1 << me->_pin);
        EXTI->FTSR &= ~(1 << me->_pin);
    }
    if (edge == falling_edge)
    {
        EXTI->RTSR &= ~(1 << me->_pin);
        EXTI->FTSR |= (1 << me->_pin);
    }
    // change EXTIx_IRQn to proper exti line
    NVIC_SetPriority(EXTI9_5_IRQn,0x03);
    NVIC_EnableIRQ(EXTI9_5_IRQn);
}

static uint16_t irq_disable(IO_pin_t me)
{
    SYSCFG->EXTICR[(me->_pin/4)] &= ~(EXTI_MASK<< (me->_pin * 4));
    EXTI->IMR &= ~(1 << me->_pin);
    NVIC_DisableIRQ(EXTI0_IRQn);
}

static uint16_t port_read(IO_pin_t me)
{
    return me->_port->IDR;
}

// uint16_t pull(IO_t* me)
// {
//     asm("NOP");
// }
// uint16_t speed(IO_t* me)
// {
//     asm("NOP");
// }
// uint16_t alternate_function(IO_t* me)
// {
//     asm("NOP");
// }

static void     io_clock_enable(IO_pin_t me)
{
    GPIO_TypeDef* port = me->_port;
    if (port == GPIOA)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    else if (port == GPIOB)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    else if(port == GPIOC)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
    else if(port == GPIOD)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
    else if(port == GPIOE)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;
    else if(port == GPIOF)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOFEN;
    else if(port == GPIOG)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN;
    else if(port == GPIOH)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOHEN;
    else if(port == GPIOI)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOIEN;
    
}
