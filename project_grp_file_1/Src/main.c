#include <stdint.h>
#include <stm32g0xx.h>
#include <stdio.h>
#include <string.h>

volatile uint32_t uwTick      = 0;
volatile uint32_t pulse_count = 0;   /* ISR increments this */

void SysTick_Handler(void) { uwTick++; }

void EXTI0_1_IRQHandler(void) {
    if (EXTI->RPR1 & EXTI_RPR1_RPIF0) {
        pulse_count++;
        EXTI->RPR1 = EXTI_RPR1_RPIF0;
    }
}

void delay_ms(uint32_t ms) { uint32_t s = uwTick; while ((uwTick-s) < ms); }
void uart_send_byte(uint8_t b) {
    while (!(USART2->ISR & USART_ISR_TXE_TXFNF));
    USART2->TDR = b;
}
void uart_send_string(const char *s) { while (*s) uart_send_byte((uint8_t)*s++); }

int main(void)
{
    /* Clocks */
    RCC->IOPENR  |= RCC_IOPENR_GPIOAEN;
    RCC->APBENR1 |= RCC_APBENR1_USART2EN | RCC_APBENR1_TIM2EN;

    /* PA2=TX, PA3=RX → UART AF1 */
    GPIOA->MODER &= ~((0x3U<<(2*2))|(0x3U<<(3*2)));
    GPIOA->MODER |=   (0x2U<<(2*2))|(0x2U<<(3*2));
    GPIOA->AFR[0] &= ~((0xFU<<(2*4))|(0xFU<<(3*4)));
    GPIOA->AFR[0] |=   (0x1U<<(2*4))|(0x1U<<(3*4));

    /* PA5 → TIM2_CH1 PWM output (TX) */
    GPIOA->MODER &= ~(0x3U<<(5*2));
    GPIOA->MODER |=  (0x2U<<(5*2));
    GPIOA->AFR[0] &= ~(0xFU<<(5*4));
    GPIOA->AFR[0] |=  (0x2U<<(5*4));

    /* PA0 → digital input, pull-down (RX) */
    GPIOA->MODER &= ~(0x3U<<(0*2));
    GPIOA->PUPDR &= ~(0x3U<<(0*2));
    GPIOA->PUPDR |=  (0x2U<<(0*2));

    /* USART2 */
    USART2->BRR  = 139;
    USART2->CR1 |= USART_CR1_TE | USART_CR1_UE;

    /* SysTick 1ms */
    SysTick->LOAD = 15999;
    SysTick->VAL  = 0;
    SysTick->CTRL = 0x7;

    /* TIM2 PWM — 100Hz on PA5 */
    TIM2->PSC   = 15;
    TIM2->ARR   = 9999;
    TIM2->CCMR1 |= (0x6U << TIM_CCMR1_OC1M_Pos);   /* PWM mode 1, no preload */
    TIM2->CCR1  = 4999;                               /* 50% duty — written AFTER mode set */
    TIM2->CCER  |= TIM_CCER_CC1E;
    TIM2->CR1   |= TIM_CR1_CEN;

    /* EXTI on PA0 — rising edge */
    EXTI->RTSR1 |= EXTI_RTSR1_RT0;
    EXTI->IMR1  |= EXTI_IMR1_IM0;
    NVIC_EnableIRQ(EXTI0_1_IRQn);
    NVIC_SetPriority(EXTI0_1_IRQn, 0);

    uart_send_string("L16 loopback ready\r\n");

    char buf[64];
    uint32_t expected_per_sec = 100;   /* 100Hz PWM = 100 pulses/second */
    uint32_t last_count = 0;


    for (;;) {
        delay_ms(1000);


        uint32_t received = pulse_count - last_count;
        uint32_t errors   = (received < expected_per_sec) ?
                            (expected_per_sec - received) : 0;
        uint32_t ber_x1000 = (errors * 1000) / expected_per_sec;

        snprintf(buf, sizeof(buf),
                 "RX:%lu EXP:%lu ERR:%lu BER:0.%03lu\r\n",
                 received, expected_per_sec, errors, ber_x1000);
        uart_send_string(buf);

        last_count = pulse_count;
    }
}
