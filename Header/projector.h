#ifndef PROJECTOR_H
#define PROJECTOR_H
#include "ring_buffer.h"

#define PROCESS_BUFFER_SIZE 30


struct Chirp_data_tag
{
    volatile uint16_t       _size;
    volatile uint16_t*      _freq_array;
    volatile uint16_t*      _duty_array;
    volatile uint16_t       _index;
};
typedef struct Chirp_data_tag*  Chirp_data_t;

struct Projector_tag
{
    uint32_t                _freq1;
    uint32_t                _freq2;
    uint32_t                _duration;
    uint32_t                _pulse_per_second;
    uint32_t                _pulse_period;
    struct Chirp_data_tag   _chirp;
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
void    Projector_Chirp_Create(Projector_t projector);
void    Projector_Data_Validate(Projector_t projector);

#endif