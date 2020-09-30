#ifndef STM32_IO_H
#define STM32_IO_H

#include "stm32f4xx.h"
/* Low Level Macro Defines */
#if 1

#define MODE_IN         0x0
#define MODE_OU         0x1
#define MODE_AF         0x2
#define MODE_AN         0x3
#define MODE_MASK       0x3

#define TYPE_PP         0x0
#define TYPE_OD         0x1
#define TYPE_MASK         0x1

#define PULL_NO         0x0
#define PULL_UP         0x1
#define PULL_DO         0x2
#define PULL_MASK       0x3

#define SPEED_LO        0x0
#define SPEED_ME        0x1
#define SPEED_HI        0x2
#define SPEED_VH        0x3
#define SPEED_MASK      0x3

#define AF_0            0x0
#define AF_1            0x1
#define AF_2            0x2
#define AF_3            0x3
#define AF_4            0x4
#define AF_5            0x5
#define AF_6            0x6
#define AF_7            0x7
#define AF_8            0x8
#define AF_9            0x9
#define AF_10           0xA
#define AF_11           0xB
#define AF_12           0xC
#define AF_13           0xD
#define AF_14           0xE
#define AF_15           0xF
#define AF_MASK         0xF

#define EXTI_MASK       0xF
#endif

/* Structures */
typedef enum 
{
    rising_edge,
    falling_edge
}IO_irq_edge_t;

typedef enum
{
    io_pin_0        = 0,
    io_pin_1,
    io_pin_2,
    io_pin_3,
    io_pin_4,
    io_pin_5,
    io_pin_6,
    io_pin_7,
    io_pin_8,
    io_pin_9,
    io_pin_10,
    io_pin_11,
    io_pin_12,
    io_pin_13,
    io_pin_14,
    io_pin_15
}IO_pin_number_t;

struct IO_configure_tag
{
    uint8_t _mode;
    uint8_t _output_type;
    uint8_t _pull_type;
    uint8_t _speed;
    uint8_t _alternate_function;
};


/* data type decleration */
typedef struct IO_configure_tag         IO_configure_t;
typedef struct IO_pin_tag*              IO_pin_t;

/* function pointer decleration */
typedef uint16_t (*IO_function_t)(IO_pin_t me);
typedef uint16_t (*IO_irq_function_t)(IO_pin_t me,IO_irq_edge_t edge);


/* IO Abstraction Data Type */
typedef struct IO_tag
{
  IO_function_t  _set;
  IO_function_t  _reset;
  IO_function_t  _read;
  IO_function_t  _port_read;
  IO_function_t  _toggle;
  IO_function_t  _irq_disable;
  IO_irq_function_t  _irq_enable;
//   IO_process_t  _pull;
//   IO_process_t  _speed;
//   IO_process_t  _alternate_function;
}IO_t;




/* Interface */
extern IO_t   IO;

IO_pin_t      IO_Create(GPIO_TypeDef* port,
                  IO_pin_number_t pin, IO_configure_t cfg);

// IO_t*   IO_Init(IO_t* io, IO_pin_number_t pin, IO_cfg_t cfg);
void          IO_Destroy(IO_pin_t io);



#endif
