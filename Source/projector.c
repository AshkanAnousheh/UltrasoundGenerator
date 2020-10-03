
#include "projector.h"
#include "stm32_io.h"
#include <stdlib.h>

static uint8_t string_compare(char* str1, char* str2);
volatile uint8_t                    console_enter_pressed;
extern volatile Projector_req_t 	projector_request;
extern struct Projector_tag		    projector_data;
extern IO_pin_t        				led_1,led_2,pb_1,pb_2,power_enable;

void    Parse_Uart_Comm(Ring_t data)
{
    console_enter_pressed = 0;

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


void Projector_Chirp_Create(Projector_t projector)
{
    uint32_t slope =( projector->_freq2 - projector->_freq1 ) / (projector->_duration);
    // F = slope * time + F1
    // time step (resolution) is 10 us
    // duration / time step -> array size of F
    // initialize F with above formula
    uint32_t temp = 0;


    uint32_t time_step = 10;
    uint32_t array_size = projector->_duration / time_step;

    projector->_chirp._freq_array = malloc(array_size * sizeof(uint16_t));
    projector->_chirp._duty_array = malloc(array_size * sizeof(uint16_t));
    projector->_chirp._index = 0;
    projector->_chirp._size = array_size;
    

    // Generate Frequencies Array
    for(int i = 0 ; i <= array_size; i++)
    {
        temp = (slope * (time_step * i)) + projector->_freq1;
        projector->_chirp._freq_array[i] = (uint16_t)(168000000 / temp);
        projector->_chirp._duty_array[i] = projector->_chirp._freq_array[i] / 2;
        // projector->_chirp->_duty_array[i] = projector->_chirp->_freq_array[i] / 2;
    }

    // // Convert Frequencies Array to Proper ARR Value
    // for(int i = 0 ; i <= array_size; i++)
    // {
    //     projector->_chirp._freq_array[i] = (uint16_t)(168000000 / projector->_chirp._freq_array[i]);
    //     projector->_chirp._duty_array[i] = projector->_chirp._freq_array[i] / 2;
    // }

}

void Projector_Data_Validate(Projector_t projector)
{
    // swap if data order not valid
    uint32_t temp;
    if(projector->_freq1 > projector->_freq2)
    {
        temp = projector->_freq1;
        projector->_freq1 = projector->_freq2;
        projector->_freq2 = temp;
    }
    // minimum frequency should be 4KHz (becouse of 16 bit ARR limitation)
    if(projector->_freq1 <= 4000)
        projector->_freq1 = 4000;
    // maximum frequency should be 500KHz
    if(projector->_freq2 >= 500000)
        projector->_freq2 = 500000;
    // nemidunam ino chera gozashtam !! :|
    if(projector->_duration <= 100)
        projector->_duration = 100;
    
    if(projector->_duration * projector->_pulse_per_second * 2 > 1000000)
        projector->_pulse_per_second = 1000000 / projector->_duration / 2;
        
    if(projector->_pulse_per_second != 0)
        projector->_pulse_period = 1000 / projector->_pulse_per_second;
    // if(projector->_pulse_period < (2*projector->_duration))
    // {
    //     projector->_pulse_per_second = 1;
    //     projector->_pulse_period = (2*projector->_duration);
    // }
    
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


void Projector_Interface_Configure(void)
{
    led_1 = IO_Create(GPIOB,io_pin_8,(IO_configure_t)   {._mode = MODE_OU,
                                                                    ._output_type = TYPE_PP,
                                                                    ._pull_type = PULL_NO});

    led_2 = IO_Create(GPIOB,io_pin_9,(IO_configure_t)   {._mode = MODE_OU,
                                                                    ._output_type = TYPE_PP,
                                                                    ._pull_type = PULL_NO});
    IO._set(led_1);
    IO._set(led_2);

    pb_1 = IO_Create(GPIOB,io_pin_6,(IO_configure_t)    {._mode = MODE_IN,
                                                                    ._output_type = TYPE_PP,
                                                                    ._pull_type = PULL_NO});

    pb_2 = IO_Create(GPIOB,io_pin_7,(IO_configure_t)   {._mode = MODE_IN,
                                                                    ._output_type = TYPE_PP,
                                                                    ._pull_type = PULL_NO});
    
    power_enable = IO_Create(GPIOA,io_pin_0,(IO_configure_t)   {._mode = MODE_OU,
                                                                        ._output_type = TYPE_PP,
                                                                        ._pull_type = PULL_DO});

    IO._set(power_enable);                                                                    
    IO._irq_enable(pb_1,rising_edge);
    IO._irq_enable(pb_2,rising_edge);
}
