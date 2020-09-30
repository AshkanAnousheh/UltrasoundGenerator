
#include "projector.h"
#include <stdlib.h>

static uint8_t string_compare(char* str1, char* str2);
volatile uint8_t                    uart_com_enter_pressed;
extern volatile Projector_req_t 	projector_request;
extern struct Projector_tag		    projector_data;

void    Parse_Uart_Comm(Ring_t data)
{
    uart_com_enter_pressed = 0;

    projector_request = 0;
    Ring_cell_t process_buffer[PROCESS_BUFFER_SIZE];
    uint16_t buffer_len = 0;
    for(int i = 0; (i < PROCESS_BUFFER_SIZE); i++)
    {
        process_buffer[i] = Ring_Pop(data);
        if(process_buffer[i] == 0)
            { buffer_len = i; break; }
    }

    uint8_t res = string_compare(process_buffer,"id\n");
    if(res)
        { projector_request       |= id_req; return; }

    res = string_compare(process_buffer,"cfg\n");
    if(res)
        { projector_request       |= configuration_req; return; }

    //      cfg: f1 f2 dur pps
    // ex:  cfg: 10000 200000 10 5\n
    Ring_cell_t*   buffer_ptr = process_buffer;
    uint16_t       space_count = 0;
    Ring_cell_t    seperated_data[5][7];
    uint8_t j = 0;
    uint8_t i = 0;
    while ( *buffer_ptr )
    {
        seperated_data[j][i] = *buffer_ptr++;
        if( (j > 4) || (i > 6) ) { projector_request |= undefined_req; break; }
        if( (seperated_data[j][i] == '\n') && (space_count == 4)) { seperated_data[j][i] = 0; break; }
        if(seperated_data[j][i] == ' ') { seperated_data[j][i] = 0; j++; i = 0; space_count++; } 
        else { i++; }
    }
    res = string_compare(seperated_data[0],"cfg:");
    if(res)
    {
        projector_data._freq1               =  atoi(seperated_data[1]);
        projector_data._freq2               =  atoi(seperated_data[2]);
        projector_data._duration            =  atoi(seperated_data[3]);
        projector_data._pulse_per_second    =  atoi(seperated_data[4]);
    
        projector_request |= configuration_set;
    }
}


static uint8_t string_compare(char* str1, char* str2)
{
    while( *str1++ == *str2++ )
    {
        if(*str1 == 0)
            return 1;
    }
    return 0;

}
