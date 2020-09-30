#include "stm32_adc.h"
#include "stm32_io.h"
#include <stdlib.h>
static void ADC_IO_Configure(ADC_config_t config);
static void ADC_Clock_Enable(ADC_config_t config);
static void ADC_DMA_Configure(ADC_t adc);

ADC_t ADC_Create(ADC_config_t config)
{
    ADC_t adc = malloc(sizeof(struct ADC_tag));
    adc->_config = config;

    ADC_buff_t buffer = malloc(sizeof(uint16_t)*config->_size);
    adc->_buffer = buffer;
    for (int i = 0; i < config->_size; i++)
        buffer[i] = 0;
    // configure io pins
    ADC_IO_Configure(config);
    // turn on clock
    ADC_Clock_Enable(config);
    // configure adc peripheral
    ADC->CCR = (ADC_CCR_TSVREFE | ADC_CCR_ADCPRE_0); // enable temp & prescale to 4

    config->_adc_address->CR2   &= ~(ADC_CR2_ADON);
    config->_adc_address->CR1   &= ~(ADC_CR1_RES | ADC_CR1_SCAN);
    config->_adc_address->SQR1  &= ~(ADC_SQR1_L); 

    config->_adc_address->SMPR1 |= (7 << 24); // 480cycle sampling time for channel 18 (temperature)
    config->_adc_address->SMPR2 = (0); // fast sampling time for channel 0 ~ 9 (temperature)
    config->_adc_address->CR1   |= (config->_resolution << 24) /* | (ADC_CR1_SCAN) */;
    config->_adc_address->SQR3  |= (config->_channel << 0);
    config->_adc_address->CR2   |= (ADC_CR2_ADON /* | ADC_CR2_EOCS */);

    // config->_adc_address->SQR2  |= (config->_channel << 10);
    
    return adc;
}
uint16_t ADC_Sample_Poll(ADC_t adc)
{
    adc->_config->_adc_address->CR2 |= (ADC_CR2_SWSTART);

    while( !(adc->_config->_adc_address->SR & ADC_SR_EOC) );
    return (adc->_config->_adc_address->DR);
}

void ADC_Sample_IRQ(ADC_t adc)
{
    adc->_config->_adc_address->CR1   |= (ADC_CR1_EOCIE);

    NVIC_SetPriority(ADC_IRQn,0x2);
    NVIC_EnableIRQ(ADC_IRQn);
    adc->_config->_adc_address->CR2 |= (ADC_CR2_SWSTART);

}

void ADC_Sample_DMA(ADC_t adc)
{
    ADC_DMA_Configure(adc);
    adc->_config->_adc_address->CR2 |= (ADC_CR2_DMA | ADC_CR2_CONT);
    
    NVIC_SetPriority(DMA2_Stream0_IRQn,0x01);
    NVIC_EnableIRQ(DMA2_Stream0_IRQn);

    DMA2_Stream0->CR        |= (DMA_SxCR_EN);

   adc->_config->_adc_address->CR2 |= (ADC_CR2_SWSTART);

}

void ADC_Destroy(ADC_t adc)
{
    free(adc->_buffer);
    free(adc);
}


static void ADC_IO_Configure(ADC_config_t config)
{
    if(config->_channel <= adc_channel_15)
    {
    /* IO_pin_t analog_signal = */ IO_Create(GPIOA,config->_channel,(IO_configure_t)
                                {._mode = MODE_AN, ._pull_type = PULL_DO});
    }
}
static void ADC_Clock_Enable(ADC_config_t config)
{
    if(config->_adc_address == ADC1)
        RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    if(config->_adc_address == ADC2)
        RCC->APB2ENR |= RCC_APB2ENR_ADC2EN;
    if(config->_adc_address == ADC3)
        RCC->APB2ENR |= RCC_APB2ENR_ADC3EN;
}

static void ADC_DMA_Configure(ADC_t adc)
{
    ADC_buff_t buff = adc->_buffer;
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;
    DMA2_Stream0->CR    &= ~(DMA_SxCR_CHSEL);
    // channel 0 select - priority high - mem & peri size is 16bits - memory incremented 
    DMA2_Stream0->CR    |= ((0 << 25) | (2 << 16) | (1 << 13) | (1 << 11) | (1 << 10));
    // circular enable - direction peri to mem - transfer complete interrupt enabled
    DMA2_Stream0->CR    |= ((1 << 8) | (0 << 6) | (1 << 4));
    DMA2_Stream0->PAR   = (uint32_t)&(adc->_config->_adc_address->DR);
    DMA2_Stream0->M0AR  = (uint32_t)(buff);
    DMA2_Stream0->NDTR  = adc->_config->_size;
    DMA2_Stream0->FCR   &= ~(DMA_SxFCR_DMDIS);
}

static void ADC_Temperature()
{
    
    // ADC_t	internal_temperature = ADC_Create(& (struct ADC_config_tag) {._adc_address = ADC1,
    //                                                                 ._channel	 = adc_channel_6,
    //                                                                 ._size 		 = 1,
    //                                                                 ._resolution = ADC_RESOLUTION_12_BITS });
	// Delay_Ms(100);
	// float temperature = 0.0;

    // uint16_t v = ADC_Sample(internal_temperature);
	// v = v*3300/4095;
	// temperature = ((v - 760) / 2.5) + 25;
}
