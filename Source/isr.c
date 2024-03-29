#include "stm32f4xx.h"
#include "stm32_io.h"
#include "stm32_timer.h"
#include "ring_buffer.h"
#include "projector.h"
extern volatile uint32_t        systick;
extern volatile uint32_t        timer_alarm_tick_us;
extern struct Timer_alarm_tag   alarms[NUMBER_OF_ALARMS];
extern Timer_stopwatch_t        duration;
extern Ring_t                   uart_com_ring;
extern volatile uint8_t         console_enter_pressed;
extern struct Projector_tag		projector_data;
extern IO_pin_t        			led_1,led_2,pb_1,pb_2,power_enable;
extern Timer_H_bridge_t 		power_stage;
extern uint8_t                  button1_lock;
extern Timer_config_t       	chirp_update;

void SysTick_Handler(void)
{
    ++systick;
}

void EXTI0_IRQHandler(void)
{
    EXTI->PR |= EXTI_PR_PR0;
    
    asm("NOP");
}

void EXTI9_5_IRQHandler(void)
{
    if((EXTI->PR & EXTI_PR_PR6))
    {
        if(button1_lock == 0 )
        {
            IO._set(power_enable);
            Timer_H_Bridge_Run(power_stage);
            IO._reset(led_1);
            button1_lock = 1;
        }
        EXTI->PR |= EXTI_PR_PR6;
    }
    if(EXTI->PR & EXTI_PR_PR7)
    {
        // IO._reset(led_2);
        IO._reset(power_enable);
        EXTI->PR |= EXTI_PR_PR7;
    }
}

void USART3_IRQHandler(void)
{
    if(USART3->SR & USART_SR_RXNE)
    {
        uint8_t data = USART3->DR;    
        if(data == '\n')
            console_enter_pressed = 1;

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
            if((alarms[idx]._valid == 0) || (alarms[idx]._time == 0))
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

void TIM5_IRQHandler(void)
{

    chirp_update->_timer_address->SR &= ~(TIM_SR_UIF);
    power_stage->_config->_timer_address->ARR  = projector_data._chirp._freq_array[projector_data._chirp._index];
    power_stage->_config->_timer_address->CCR1 = projector_data._chirp._duty_array[projector_data._chirp._index];
    power_stage->_config->_timer_address->CCR3 = projector_data._chirp._duty_array[projector_data._chirp._index];

    projector_data._chirp._index++;
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
