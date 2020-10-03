#include <stdlib.h>
#include "stm32_io.h"
#include "stm32_timer.h"

static      uint8_t                     alarm_initialized = 0;
volatile    uint32_t                    timer_alarm_tick_us;
struct      Timer_alarm_tag             alarms[NUMBER_OF_ALARMS] = {0};
extern      Timer_config_t  		    chirp_update;

static void Timer_H_Bridge_Configure(Timer_H_bridge_t timer);
static void Timer_Clock_Enable(TIM_TypeDef* timer);
static void Timer_IO_Configure(void);

void Timer_Alarm( Timer_alarm_t alarm )
{
    TIM_TypeDef* timer_address = TIM7;
    static uint16_t alarm_idx = 0;
    if( ! alarm_initialized )
    {
        // timer enable
        RCC->APB1ENR |= RCC_APB1ENR_TIM7EN;
        NVIC_SetPriority( TIM7_IRQn, 0x2 );
        NVIC_EnableIRQ( TIM7_IRQn );
        // timer configuration to generate 1us tick;
        timer_address->CR1  &= ~(TIM_CR1_CEN);
        // timer 7 uses APB1 * 2 (42MHz * 2 = 84MHz) so we divide freq to 84 
        timer_address->PSC  = ( 84 - 1 );           // 1MHz timer frequency
        timer_address->ARR  = ( 1000 - 1 );         // 1ms interrupt generation
        timer_address->EGR  |= TIM_EGR_UG;
        timer_address->DIER |= TIM_DIER_UIE;
        timer_address->CR1  |= TIM_CR1_CEN;
        
        alarm_initialized   = 1;
    }

    for( alarm_idx = 0; alarm_idx < NUMBER_OF_ALARMS; alarm_idx++)
    {
        if(alarms[alarm_idx]._valid == 0)
        {
            alarms[alarm_idx]._time = alarm->_time;
            alarms[alarm_idx]._action = alarm->_action;
            alarms[alarm_idx]._valid = 1;
            break;
        }
    }
    
}


Timer_H_bridge_t Timer_H_Bridge_Create(Timer_config_t config)
{
    Timer_H_bridge_t h_bridge = malloc(sizeof(struct Timer_H_bridge_tag));
    h_bridge->_config = config;
    
    if(config->_timer_address == TIM1)
        RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;

    Timer_H_Bridge_Configure(h_bridge);
    config->_timer_address->CR1 |= TIM_CR1_CEN;

    return h_bridge;
}

static void Timer_H_Bridge_Configure(Timer_H_bridge_t timer)
{
    // config timer OC into alternate function 
    /* IO_pin_t a7_ch1N =  */IO_Create(GPIOA,io_pin_7,(IO_configure_t) 
                            { MODE_AF, TYPE_PP, PULL_NO, SPEED_HI, AF_1 });
    /* IO_pin_t a8_ch1  =  */IO_Create(GPIOA,io_pin_8,(IO_configure_t) 
                            { MODE_AF, TYPE_PP, PULL_NO, SPEED_HI, AF_1 });
    /* IO_pin_t a10_ch3 = */ IO_Create(GPIOA,io_pin_10,(IO_configure_t) 
                            { MODE_AF, TYPE_PP, PULL_NO, SPEED_HI, AF_1 });
    /* IO_pin_t b1_ch3N = */ IO_Create(GPIOB,io_pin_1,(IO_configure_t) 
                            { MODE_AF, TYPE_PP, PULL_NO, SPEED_HI, AF_1 });

    timer->_frequency = 4000;
    uint32_t f_pres = ( 168000000.0 / timer->_frequency );       
    timer->_config->_timer_address->CR1     &= ~(TIM_CR1_CEN);
    timer->_config->_timer_address->CR2     &= ~(TIM_CR2_OIS1| TIM_CR2_OIS1N| TIM_CR2_OIS3| TIM_CR2_OIS3N);
    timer->_config->_timer_address->CCER    &= ~(TIM_CCER_CC1E| TIM_CCER_CC1NE| TIM_CCER_CC3E| TIM_CCER_CC3NE );
    timer->_config->_timer_address->BDTR    &= ~(0xFFFF);
    timer->_config->_timer_address->CCMR1   &= ~(TIM_CCMR1_CC1S | TIM_CCMR1_OC1M);
    timer->_config->_timer_address->CCMR2   &= ~(TIM_CCMR2_CC3S | TIM_CCMR2_OC3M);
    // config idle state of OC pins: set oc1 and oc3
    timer->_config->_timer_address->CR2     |= (TIM_CR2_OIS1N| TIM_CR2_OIS3);
    // config mode pwm1 for oc1 and oc3
    timer->_config->_timer_address->CCMR1   |= (PWM_MODE_1);
    timer->_config->_timer_address->CCMR2   |= (PWM_MODE_1);
    // OC1 and OC3 and their complementaries enable with **proper Polarity !**
    timer->_config->_timer_address->CCER    |= (TIM_CCER_CC1E| (TIM_CCER_CC1NE| TIM_CCER_CC1NP)|
                                            (TIM_CCER_CC3E| TIM_CCER_CC3P)| TIM_CCER_CC3NE );
    timer->_config->_timer_address->PSC     = ( 1 - 1 );
    timer->_config->_timer_address->ARR     = (f_pres);
    timer->_config->_timer_address->CCR1    = (f_pres/2);
    timer->_config->_timer_address->CCR3    = (f_pres/2);
    timer->_config->_timer_address->CNT     = (0);
    // Turn Main Output on - inactive state: idle
    timer->_config->_timer_address->BDTR    |= (/* TIM_BDTR_MOE | */ TIM_BDTR_OSSI | timer->_config->_dead_time );
    timer->_config->_timer_address->EGR     |= (TIM_EGR_UG);
}
void Timer_H_Bridge_Run(Timer_H_bridge_t timer)
{
//    timer->_frequency = 100;
    // uint32_t fr = (uint32_t)( 168000000.0 / timer->_frequency );
    // // uint32_t dc = (uint32_t)(fr*timer->_duty_cycle/100.0);
    // timer->_config->_timer_address->ARR     = fr;
    // timer->_config->_timer_address->CCR1    = fr/2;
    // timer->_config->_timer_address->CCR3    = fr/2;
    timer->_config->_timer_address->CNT     = (0);
    chirp_update->_timer_address->CNT = (0);
    chirp_update->_timer_address->CR1 |= (TIM_CR1_CEN);
    timer->_config->_timer_address->BDTR    |= (TIM_BDTR_MOE);
}
void Timer_H_Bridge_Stop(Timer_H_bridge_t timer)
{
    timer->_config->_timer_address->BDTR    &= ~(TIM_BDTR_MOE);
    chirp_update->_timer_address->CR1 &= ~(TIM_CR1_CEN);

}

Timer_pwm_t Timer_PWM_Create( Timer_pwm_config_t config )
{
    Timer_pwm_t timer = malloc(sizeof(struct Timer_pwm_tag));
    timer->_config = config;
    // pwm io configuration
    Timer_IO_Configure();
    // turn on clock
    Timer_Clock_Enable(config->_timer_address);

    config->_timer_address->CR1             &= ~(TIM_CR1_CEN);
    config->_timer_address->CCMR1           &= ~( (TIM_CCMR1_CC1S | TIM_CCMR1_OC1M) << config->_channel );
    config->_timer_address->CCER            = (0);

    config->_timer_address->CCMR1           |= (PWM_MODE_1 << config->_channel );

    config->_timer_address->CCER            |= (1 << (config->_channel/2));
    config->_timer_address->PSC             = (84 - 1);
    // timer APB1 clock is 84MHz, so with 84 psc frequency would be 1MHz
    uint32_t timer_frequency                = 1000000/50000;        // for 50KHz 
    timer->_frequency                       = timer_frequency;
    timer->_duty_cycle                      = timer_frequency/2;
//    config->_timer_address->ARR             = (timer_frequency);
//    config->_timer_address->CCR1            = (timer_frequency/2);  // for 50% dutycycle
    config->_timer_address->EGR             |= TIM_EGR_UG;
    return timer;
}

void Timer_PWM_Start(Timer_pwm_t timer)
{
    // timer APB1 clock is 84MHz, so with 84 psc frequency would be 1MHz
    uint32_t timer_frequency                        = 1000000/timer->_frequency; 
    timer->_config->_timer_address->ARR             = (timer_frequency);
    timer->_config->_timer_address->CCR1            = (timer_frequency*(timer->_duty_cycle))/100;  // for 50% dutycycle
    timer->_config->_timer_address->CCER            |= (1 << (timer->_config->_channel/2));
    timer->_config->_timer_address->CR1             |= TIM_CR1_CEN;
}

void Timer_PWM_Stop(Timer_pwm_t timer)
{
    timer->_config->_timer_address->CCER            &= ~(1 << (timer->_config->_channel/2));
    timer->_config->_timer_address->CR1             &= ~(TIM_CR1_CEN);
}

void Timer_PWM_Destroy(Timer_pwm_t timer)
{
    free(timer->_config);
    free(timer);
}



Timer_stopwatch_t Timer_Stopwatch_Create(Timer_config_t config)
{
    Timer_stopwatch_t timer = malloc(sizeof(struct Timer_stopwatch_tag));
    timer->_config = config;

    // Turn on clock
    Timer_Clock_Enable(config->_timer_address);

    config->_timer_address->CR1             &= ~(TIM_CR1_CEN);
    config->_timer_address->CCER            = (0);
    config->_timer_address->DIER            |= (TIM_DIER_UIE);
    config->_timer_address->PSC             = (84 - 1);
    config->_timer_address->ARR             = (0xFFFF);
    config->_timer_address->CNT             = (0);
    config->_timer_address->EGR             |= (TIM_EGR_UG);

    timer->_begin                           = (0);
    timer->_overflow_count                  = (0);
//  put proper parameter to irq enable functions 
   NVIC_SetPriority(TIM8_BRK_TIM12_IRQn,0x3);
   NVIC_EnableIRQ(TIM8_BRK_TIM12_IRQn);


    return timer;

}
void Timer_Stopwatch_Start(Timer_stopwatch_t timer)
{
    timer->_config->_timer_address->CR1     |= (TIM_CR1_CEN);
    // timer->_begin = timer->_config->_timer_address->CNT;
    timer->_overflow_count = 0;
    timer->_config->_timer_address->CNT = 0;
}
uint32_t Timer_Stopwatch_Stop(Timer_stopwatch_t timer)
{
    uint32_t temp       = timer->_config->_timer_address->CNT;
    timer->_config->_timer_address->CR1     &= ~(TIM_CR1_CEN);

    // temp = (timer->_overflow_count)*(0xFFFF) + temp - timer->_begin;
    temp += (timer->_overflow_count)*(0xFFFF);
    timer->_overflow_count = 0;

    return temp;    
}
void Timer_Stopwatch_Destroy(Timer_stopwatch_t timer)
{
    free(timer->_config);
    free(timer);
}


static void Timer_IO_Configure(void)
{
    /* IO_pin_t f9_tim14_ch1 =  */IO_Create(GPIOF,io_pin_9,(IO_configure_t) 
                             { MODE_AF, TYPE_PP, PULL_DO, SPEED_HI, AF_9 });
    /* IO_pin_t f8_tim13_ch1 =  */IO_Create(GPIOF,io_pin_8,(IO_configure_t) 
                             { MODE_AF, TYPE_PP, PULL_NO, SPEED_HI, AF_9 });
    
}

static void Timer_Clock_Enable(TIM_TypeDef* timer)
{
    // APB2 (fast timers) Enable
    if(timer == TIM1)
        RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
    if(timer == TIM8)
        RCC->APB2ENR |= RCC_APB2ENR_TIM8EN;
    
    if(timer == TIM9)
        RCC->APB2ENR |= RCC_APB2ENR_TIM9EN;
    if(timer == TIM10)
        RCC->APB2ENR |= RCC_APB2ENR_TIM10EN;
    if(timer == TIM11)
        RCC->APB2ENR |= RCC_APB2ENR_TIM11EN;
    // APB1 (slow timers) Enable
    if(timer == TIM2)
        RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    if(timer == TIM3)
        RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    if(timer == TIM4)
        RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
    if(timer == TIM5)
        RCC->APB1ENR |= RCC_APB1ENR_TIM5EN;
    if(timer == TIM6)
        RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;
    if(timer == TIM7)
        RCC->APB1ENR |= RCC_APB1ENR_TIM7EN;

    if(timer == TIM12)
        RCC->APB1ENR |= RCC_APB1ENR_TIM12EN;
    if(timer == TIM13)
        RCC->APB1ENR |= RCC_APB1ENR_TIM13EN;
    if(timer == TIM14)
        RCC->APB1ENR |= RCC_APB1ENR_TIM14EN;
    
}


void Timer_Create(Timer_config_t timer)
{
    Timer_Clock_Enable( timer->_timer_address );
    // TIM5 connected to APB1 so Fck = 168/4 = 42MHz
    timer->_timer_address->CR2 = 0 ;
    // timer->_timer_address->CR2 |= TIM_CR2_CCDS;
    timer->_timer_address->DIER |=/*  TIM_DIER_CC1DE |  */TIM_DIER_UIE;
    // timer->_timer_address->CCMR1 |= (PWM_MODE_1);
    // timer->_timer_address->CCMR1 &= ~(TIM_CCMR1_CC1S);
    // timer->_timer_address->CCER |= (TIM_CCER_CC1E);
    timer->_timer_address->PSC = (84 - 1);
    timer->_timer_address->ARR = 10;      // interrupt at each 10 us
    timer->_timer_address->EGR |= TIM_EGR_UG;

    NVIC_SetPriority(TIM5_IRQn,0x1);
    NVIC_EnableIRQ(TIM5_IRQn);
    // connect this timer to DMA
    // //  config: channel4 - priority med - memory incremented - direction memory to peripheral
    // DMA1_Stream6->CR = (DMA_SxCR_CHSEL_2| DMA_SxCR_CHSEL_1  | DMA_SxCR_PL_1 | DMA_SxCR_PSIZE_0 |
    //                     DMA_SxCR_PL_0   | DMA_SxCR_MINC     | DMA_SxCR_DIR_0| DMA_SxCR_MSIZE_0);
    // DMA1_Stream6->M0AR = 
}