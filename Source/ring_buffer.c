#include <stdlib.h>
#include "ring_buffer.h"
#include "logger.h"


struct Ring_tag
{
    Ring_cell_t*        _buffer;
    volatile uint32_t   _head;
    volatile uint32_t   _tail;
    uint32_t            _maximum;
};


Ring_t Ring_Create( uint32_t size )
{
    Ring_t  ring = malloc(sizeof(struct Ring_tag));
    if(ring == NULL) return;

    Ring_cell_t* ring_buffer = malloc((size * sizeof(Ring_cell_t))+1); 
    if(ring_buffer == NULL) return;

    ring->_buffer = ring_buffer;
    ring->_maximum = size;
    ring->_head = ring->_tail = 0;

    return ring;
}
void Ring_Push(Ring_t ring, Ring_cell_t data)
{
    ring->_buffer[ring->_head] = data;

    if( (ring->_head + 1) >= ring->_maximum)
    {
        LOGGER_PRINT("ring buffer max reached\r\n");
        ring->_head = 0;
    }
    else
    {
        ring->_head = ring->_head + 1;
    }
    
    if( (ring->_head + 1) == ring->_tail)
    {
        LOGGER_PRINT("ring buffer full\r\n");
    }
    
}
Ring_cell_t Ring_Pop(Ring_t ring)
{
    if(ring->_tail == ring->_head)
        return 0;
    Ring_cell_t data = ring->_buffer[ring->_tail];
    ring->_tail = (ring->_tail < (ring->_maximum - 1)) ? (ring->_tail + 1) : 0;
    return data;        
}

void Ring_Destroy(Ring_t ring)
{
    free(ring->_buffer);
}
