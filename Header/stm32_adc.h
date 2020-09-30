#ifndef STM32_ADC_H
#define STM32_ADC_H
#include "stm32f4xx.h"

#define ADC_RESOLUTION_6_BITS       (3)   // takes 9 ADCCLK cycles
#define ADC_RESOLUTION_8_BITS       (2)   // takes 11 ADCCLK cycles
#define ADC_RESOLUTION_10_BITS      (1)   // takes 13 ADCCLK cycles
#define ADC_RESOLUTION_12_BITS      (0)   // takes 15 ADCCLK cycles

typedef uint16_t*                    ADC_buff_t;

typedef enum
{
    adc_channel_0        = 0,
    adc_channel_1,
    adc_channel_2,
    adc_channel_3,
    adc_channel_4,
    adc_channel_5,
    adc_channel_6,
    adc_channel_7,
    adc_channel_8,
    adc_channel_9,
    adc_channel_10,
    adc_channel_11,
    adc_channel_12,
    adc_channel_13,
    adc_channel_14,
    adc_channel_15,
    adc_channel_16,
    adc_channel_17,
    adc_channel_18
}ADC_channel_t;

struct ADC_config_tag
{
    ADC_TypeDef*    _adc_address;
    ADC_channel_t   _channel;
    uint32_t        _size;
    uint8_t         _resolution;
};
typedef struct ADC_config_tag*      ADC_config_t;

struct ADC_tag
{
    ADC_buff_t      _buffer;
    ADC_config_t    _config;
};
typedef struct ADC_tag*             ADC_t;

ADC_t ADC_Create(ADC_config_t config);
uint16_t ADC_Sample(ADC_t);
void ADC_Sample_DMA(ADC_t);
void ADC_Sample_IRQ(ADC_t);
void ADC_Destroy(ADC_t);
#endif