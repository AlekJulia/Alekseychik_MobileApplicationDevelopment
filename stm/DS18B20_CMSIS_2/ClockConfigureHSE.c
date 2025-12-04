#include "ClockConfigureHSE.h"
#include "stm32f10x.h"

// настройка тактировния (внешний кварц)
void SystemCoreClockConfigure(void) {
    RCC->CR |= ((uint32_t)RCC_CR_HSEON);               
    while ((RCC->CR & RCC_CR_HSERDY) == 0);                  

    RCC->CFGR = RCC_CFGR_SW_HSE;                             
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSE);  
    
    RCC->CFGR = RCC_CFGR_HPRE_DIV1;                          
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV1;                        
    RCC->CFGR |= RCC_CFGR_PPRE2_DIV1;                        

    RCC->CR &= ~RCC_CR_PLLON;                                

    RCC->CFGR &= ~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL);
    RCC->CFGR |= (RCC_CFGR_PLLSRC_HSE | RCC_CFGR_PLLMULL9);

    RCC->CR |= RCC_CR_PLLON;                                
    while((RCC->CR & RCC_CR_PLLRDY) == 0) __NOP();           

    RCC->CFGR &= ~RCC_CFGR_SW;                               
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);  
}