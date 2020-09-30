/*
 * main.c
 *	[User Manual: RM0090]
 *  Created on: Sep 4, 2020
 *      Author: Ashkan
 */
#include "stm32f4xx.h"
#include "system_stm32f4xx.h"
/* 
Global Variable
*/
uint32_t SystemCoreClock = 8000000;

/* 
Local Variable
*/

/*
	This function called by assembly startup file
	initialize external memory and Vector Table Location
*/
void SystemInit()
{
	// [User Manual Page 98]
	// Clear Prefetch Enable, Data Cache, Instr. cache and Latency bits
	FLASH->ACR &= ~(FLASH_ACR_LATENCY | FLASH_ACR_PRFTEN |
                FLASH_ACR_DCEN | FLASH_ACR_ICEN);
	while((FLASH->ACR & FLASH_ACR_LATENCY) != 0x00);
    // Reset Data cache and Instruction cache
    FLASH->ACR |= FLASH_ACR_DCRST | FLASH_ACR_ICRST;
	// Enable Prefetch and Set Flash Read Latency to 5 wait state
	FLASH->ACR |= FLASH_ACR_LATENCY_5WS |
					FLASH_ACR_PRFTEN;
	while ((FLASH->ACR & FLASH_ACR_LATENCY) != FLASH_ACR_LATENCY_5WS);

//	SystemCoreClockUpdate();
}

/* 
	1. choose proper flash wait state (dependent to system clock)
	2. enable prefetch 
	3. config the PLL 
	4. switch clock into HSE + PLL
	TODO: 
	5. enable instruction cache and data cache
	6. refactor the code to change easily frequency
 */
void SystemCoreClockUpdate()
{
	
    // [User Manual Page 166]
	// Enable External crystall Oscillator (HSE)
	RCC->CR |= RCC_CR_HSEON;
	while(!(RCC->CR & RCC_CR_HSERDY));
	// PLLCLK = ( input frequency / M ) * N / P
	// External crystal oscillation frequency is 8.000MHz
	// M = 4 	-	N = 168		-	P = 2
	uint32_t rcc_pll_cfgr = RCC->PLLCFGR;
	// 			PLLQ	PLLSRC	PLLP	PLLN	PLLM
	rcc_pll_cfgr &= ~((0xF << 24) | (1 << 22) | (0x3 << 16)
					| (0x1FF << 6) | (0x3F << 0));
	
	rcc_pll_cfgr |= ((7 << 24) | (1 << 22) | (0 << 16)
					| (168 << 6) | (4 << 0));
	RCC->PLLCFGR = rcc_pll_cfgr;

	// Enable PLL
	// RCC->CR &= ~(RCC_CR_PLLON);
	RCC->CR |= (RCC_CR_PLLON);
	while ((RCC->CR & RCC_CR_PLLRDY) == 0);
	
	RCC->CFGR &= ~(RCC_CFGR_SW | RCC_CFGR_HPRE | RCC_CFGR_PPRE1
					| RCC_CFGR_PPRE2);
	// Set AHB prescaler to 1
	//RCC->CFGR &= ~(RCC_CFGR_HPRE);
	// Set APB1 ( low speed ) prescaler to 4 (PCLK1 = 42MHz)
	RCC->CFGR |= RCC_CFGR_PPRE1_DIV4;
	// Set APB2 ( high speed ) prescaler to 2 (PCLK2 = 84MHz)
	RCC->CFGR |= RCC_CFGR_PPRE2_DIV2;
	
	// Switch System Clock Source to PLL
	RCC->CFGR |= RCC_CFGR_SW_PLL;
	while ( (RCC->CFGR & RCC_CFGR_SWS_PLL) == 0);


	SystemCoreClock = 168000000;
//	SystemCoreClockUpdate();

}
