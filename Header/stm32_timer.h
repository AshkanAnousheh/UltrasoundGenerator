#ifndef STM32_TIMER_H
#define STM32_TIMER_H

#include "stm32f4xx.h"

typedef void    (*Alarm_function_t)(void);

#if 1
#define     NUMBER_OF_ALARMS    (10)
#define     PWM_MODE_1          (0x6 << 4)
#define     PWM_CHANNEL_OC1     (0)
#define     PWM_CHANNEL_OC2     (8)

#define     _US                 (1)
#define     _MS                 (1000*(_US))
#define     _S                  (1000*(_MS))

#endif

struct Timer_alarm_tag
{
    uint8_t             _valid;
    uint32_t            _time;
    Alarm_function_t    _action;
};
typedef struct Timer_alarm_tag*     Timer_alarm_t;

struct Timer_config_tag
{
    TIM_TypeDef*    _timer_address;
    uint8_t         _dead_time;
};
typedef struct Timer_config_tag* Timer_config_t;

struct Timer_H_bridge_tag
{
    Timer_config_t  _config;
    uint16_t        _frequency;
    uint8_t         _duty_cycle;
};
typedef struct Timer_H_bridge_tag* Timer_H_bridge_t;

struct Timer_pwm_config_tag
{
    TIM_TypeDef*    _timer_address;
    uint8_t         _channel;
};
typedef struct Timer_pwm_config_tag* Timer_pwm_config_t;

struct Timer_pwm_tag
{
    uint32_t                _frequency;
    uint8_t                 _duty_cycle;
    Timer_pwm_config_t      _config;
};
typedef struct Timer_pwm_tag* Timer_pwm_t;

struct Timer_stopwatch_tag
{
    uint32_t                _begin;
    volatile uint8_t        _overflow_count;
    Timer_config_t          _config;
};
typedef struct Timer_stopwatch_tag* Timer_stopwatch_t;

void Timer_Alarm( Timer_alarm_t alarm );
// *******************************************
Timer_H_bridge_t Timer_H_Bridge_Create(Timer_config_t);
void Timer_H_Bridge_Run(Timer_H_bridge_t);
void Timer_H_Bridge_Stop(Timer_H_bridge_t);
// *******************************************
Timer_pwm_t Timer_PWM_Create( Timer_pwm_config_t );
void Timer_PWM_Start(Timer_pwm_t);
void Timer_PWM_Stop(Timer_pwm_t);
void Timer_PWM_Destroy(Timer_pwm_t);
// *******************************************
Timer_stopwatch_t Timer_Stopwatch_Create(Timer_config_t);
void Timer_Stopwatch_Start(Timer_stopwatch_t);
uint32_t Timer_Stopwatch_Stop(Timer_stopwatch_t);
void Timer_Stopwatch_Destroy(Timer_stopwatch_t);
// *******************************************
#endif
