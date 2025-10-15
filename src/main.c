// ****************************************************************
// * TEAM18: L. DELUSSEY and A. JACOBS
// * CPEG222 3Amain 10/14/25 ; untrasound based radar 
// * NucleoF466RE CMSIS STM32F4xx 
// ****************************************************************

#include <stm32f446xx.h>
#include "SSD_Array.h"
#include <stdio.h>
#define TRIG_PORT GPIOA
#define TRIG_PIN  4
#define ECHO_PORT GPIOA
#define ECHO_PIN  0
volatile uint32_t rise = 0; //echo goes low to high
volatile uint32_t fall = 0; //echo goes high to low 
volatile uint32_t done = 0; //timing from rise to fall, captured
void GPIO_ini(void);
void TIM5_ini(void);
void SysTick_Ini(void);
void delay_us(uint32_t us);
void distance_disp(uint32_t distance);
void SSD_init(void);

int main(void){
    GPIO_ini(); //init trig and echo
    TIM5_ini(); // init tim5 input capture 
    SysTick_Ini(); //init systick for 10us trig pulse every .5s
    SSD_init(); //init SSD display
    while (1) {
        if (done) {
            done = 0;
            uint32_t Wpulse = fall - rise; //pulse width in us
            float distance = (Wpulse * 0.0343f) / 2.0f; //cm
            distance_disp((uint32_t)distance);
        }
    }
}
void GPIO_ini(void){
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN; //enable GPIOA&B clocks
    TRIG_PORT->MODER &= ~(3U << (TRIG_PIN * 2)); // PA4(trig pin) output
    TRIG_PORT->MODER |=  (1U << (TRIG_PIN * 2));// ^^
    ECHO_PORT->MODER &= ~(3U << (ECHO_PIN * 2)); // PA0(echo pin) as input
    ECHO_PORT->MODER |=  (2U << (ECHO_PIN * 2)); // AF mode
    ECHO_PORT->AFR[0] &= ~(0xF << (ECHO_PIN * 4)); // clear AFR
    ECHO_PORT->AFR[0] |= (2U << (ECHO_PIN * 4)); // AF2 for TIM5_CH1 (PA0 = TIM5_CH1)
}
void SysTick_Handler(void){ //systick interrupt handler; 10us trig pulse every .5s
    TRIG_PORT->ODR |= (1 << TRIG_PIN);
    uint32_t start = TIM5->CNT;
    while ((TIM5->CNT - start) < 10); //wait 10 us
    TRIG_PORT->ODR &= ~(1 << TRIG_PIN); //set trigger low 
}
void TIM5_ini(void){
    RCC->APB1ENR |= RCC_APB1ENR_TIM5EN; //enable TIM5 clock
    TIM5->PSC = 84 - 1; //prescaler, 1MHz clock; 1us resolution (APB1 timer runs at 84MHz on F446RE)
    TIM5->ARR = 0xFFFFFFFF; //max ARR
    TIM5->CCMR1 &= ~(3U << TIM_CCMR1_CC1S_Pos);
    TIM5->CCMR1 |= (1U << TIM_CCMR1_CC1S_Pos); //CC1 channel input, IC1 mapped on TI1 (ECHO = PA0)
    TIM5->CCER |= TIM_CCER_CC1E; //capture enabled on rising edge
    TIM5->DIER |= TIM_DIER_CC1IE; //enable interrupt
    TIM5->CR1 |= TIM_CR1_CEN; //enable counter
    NVIC_EnableIRQ(TIM5_IRQn); //enable TIM5 interrupt
}
void SysTick_Ini(void){ //interrupt every .5s
    SysTick_Config(SystemCoreClock / 2); 
    SysTick->VAL = 0; //clear value register
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
}
void TIM5_IRQHandler(void){
    if (TIM5->SR & TIM_SR_CC1IF) { // check for capture flag (channel 1)
        if (!(TIM5->CCER & TIM_CCER_CC1P)) { // rising edge
            rise = TIM5->CCR1;
            TIM5->CCER |= TIM_CCER_CC1P; // switch to falling edge
        } else { // falling edge
            fall = TIM5->CCR1;
            TIM5->CCER &= ~TIM_CCER_CC1P; // back to rising
            done = 1; // pulse captured
        }
        TIM5->SR &= ~TIM_SR_CC1IF; // clear interrupt flag
    }
}
void distance_disp(uint32_t distance){ //show distance on ssd
    SSD_update(0, distance, 0); // send value to SSD display
}
