#ifndef PROJECTOR_H
#define PROJECTOR_H
#include "ring_buffer.h"

#define PROCESS_BUFFER_SIZE 30

struct Projector_tag
{
    uint32_t    _freq1;
    uint32_t    _freq2;
    uint32_t    _duration;
    uint32_t    _pulse_per_second;
};
typedef struct Projector_tag*       Projector_t;

typedef enum
{
    id_req              =   0x01,
    configuration_req   =   0x02,
    configuration_set   =   0x04,
    undefined_req       =   0x08
}Projector_req_t;

void    Parse_Uart_Comm(Ring_t data);

#endif