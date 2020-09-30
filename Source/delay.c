#include "delay.h"
extern volatile uint32_t systick;

void Delay_Ms(uint16_t ms)
{
    uint32_t tick = systick + ms;
    if( tick > systick )
        while( systick < tick );
    else
    {
        systick = 0;
        while( systick < ms);
    }

}