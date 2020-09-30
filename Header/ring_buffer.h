#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>


typedef struct  Ring_tag*       Ring_t;
// determine ring buffer each cell type
typedef char                    Ring_cell_t;
/* Interfaces */

Ring_t Ring_Create( uint32_t size );
void Ring_Push(Ring_t ring,Ring_cell_t data);
Ring_cell_t Ring_Pop();
void Ring_Destroy(Ring_t ring);

#endif