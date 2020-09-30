#include "stm32f4xx.h"
#include "stm32_io.h"
#include "stm32_timer.h"
#include "ring_buffer.h"
extern volatile uint32_t        systick;
extern volatile uint32_t        timer_alarm_tick_us;
extern struct Timer_alarm_tag   alarms[NUMBER_OF_ALARMS];
extern Timer_stopwatch_t        duration;
extern Ring_t                   uart_com_ring;
extern volatile uint8_t         uart_com_enter_pressed;

void SysTick_Handler(void)
{
    ++systick;
}

void EXTI0_IRQHandler(void)
{
    EXTI->PR |= EXTI_PR_PR0;
    
    asm("NOP");
}

void USART3_IRQHandler(void)
{
    if(USART3->SR & USART_SR_RXNE)
    {
        uint8_t data = USART3->DR;    
        if(data == '\n')
            uart_com_enter_pressed = 1;

        Ring_Push(uart_com_ring,data);
        //USART3->DR;
    }
}

void TIM7_IRQHandler(void)
{
    // alarm interrupt service
    if(TIM7->SR & TIM_SR_UIF)
    {
        TIM7->SR &= ~(TIM_SR_UIF);
        for( int idx = 0; idx < NUMBER_OF_ALARMS; idx++)
        {
            if(alarms[idx]._valid == 0)
                continue;
            
            alarms[idx]._time--;
            
            if(alarms[idx]._time == 0)
            {
                alarms[idx]._action();
                alarms[idx]._valid = 0;
            }
        }
    }


}

void TIM8_BRK_TIM12_IRQHandler(void)
{
    if(TIM12->SR & TIM_SR_UIF)
    {
        TIM12->SR &= ~(TIM_SR_UIF);
        duration->_overflow_count++;
//        duration->_config->_timer_address->CR1  |= (TIM_CR1_CEN);
    }

}

void ADC_IRQHandler(void)
{
    uint16_t data;
    if(ADC1->SR & ADC_SR_EOC)
    {
        data = ADC1->DR;
    }
}

void DMA2_Stream0_IRQHandler(void)
{
	if ((DMA2->LISR & DMA_LISR_TCIF0) && (DMA2_Stream0->CR & DMA_SxCR_TCIE))
	{
		DMA2->LIFCR = DMA_LIFCR_CTCIF0;  // acknowledge interrupt
	}
}