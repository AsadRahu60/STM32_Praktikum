#include <stdint.h>
#include <stm32g0xx.h>
#include <stdio.h>
#include <string.h>

volatile uint32_t uwTick = 0;
void SysTick_Handler(void) { uwTick++; }
void delay_ms(uint32_t ms) { uint32_t s = uwTick; while ((uwTick-s) < ms); }

void uart_send_byte(uint8_t b) {
    while (!(USART2->ISR & USART_ISR_TXE_TXFNF));
    USART2->TDR = b;
}
void uart_send_string(const char *s) { while (*s) uart_send_byte((uint8_t)*s++); }

int main(void)
{
    /* Step 1: clocks — GPIOA, USART2, TIM2 */
    RCC->IOPENR  |= RCC_IOPENR_GPIOAEN;
    RCC->APBENR1 |= RCC_APBENR1_USART2EN;
    RCC->APBENR1 |= RCC_APBENR1_TIM2EN;   /* TIM2 clock ON */

    /* Step 2: PA2=TX, PA3=RX → UART AF1 */
    GPIOA->MODER &= ~((0x3U<<(2*2))|(0x3U<<(3*2)));
    GPIOA->MODER |=   (0x2U<<(2*2))|(0x2U<<(3*2));
    GPIOA->AFR[0] &= ~((0xFU<<(2*4))|(0xFU<<(3*4)));
    GPIOA->AFR[0] |=   (0x1U<<(2*4))|(0x1U<<(3*4));

    /* Step 3: PA5 → TIM2_CH1 (AF1) */
    GPIOA->MODER &= ~(0x3U << (5*2));     /* clear PA5 mode */
    GPIOA->MODER |=  (0x2U << (5*2));     /* set AF mode */
    GPIOA->AFR[0] &= ~(0xFU << (5*4));    /* clear AF selection */
    GPIOA->AFR[0] |=  (0x1U << (5*4));    /* AF1 = TIM2_CH1 */

    /* Step 4: USART2 */
    USART2->BRR  = 139;
    USART2->CR1 |= USART_CR1_TE | USART_CR1_UE;

    /* Step 5: SysTick 1ms */
    SysTick->LOAD = 15999;
    SysTick->VAL  = 0;
    SysTick->CTRL = 0x7;

    TIM2->PSC = 15;      /* 16MHz / 16 = 1MHz */
    TIM2->ARR = 499999;  /* 1MHz / 500000 = 2Hz — visible blinking */
    TIM2->CCR1 = 249999; /* 50% duty */

    /* PWM mode 1: pin HIGH while counter < CCR1, LOW when counter >= CCR1 */
    TIM2->CCMR1 |= (0x6U << TIM_CCMR1_OC1M_Pos);  /* PWM mode 1 */
    TIM2->CCMR1 |= TIM_CCMR1_OC1PE;                /* preload enable */
    TIM2->CCER  |= TIM_CCER_CC1E;                  /* enable CH1 output */
    TIM2->CR1   |= TIM_CR1_ARPE;                   /* ARR preload enable */
    TIM2->CR1   |= TIM_CR1_CEN;                    /* START the timer */

    uart_send_string("L14 PWM ready — 1kHz 50%\r\n");

    char buf[64];
    uint32_t duty_percent = 50;

    for (;;) {
        /* Change duty cycle every 2 seconds */
        duty_percent += 25;
        if (duty_percent > 75) duty_percent = 25;

        /* Update CCR1 — takes effect at next ARR reload */
        TIM2->CCR1 = (TIM2->ARR * duty_percent) / 100;

        snprintf(buf, sizeof(buf),
                 "Duty:%lu%%  CCR1:%lu  ARR:%lu\r\n",
                 duty_percent, TIM2->CCR1, TIM2->ARR);
        uart_send_string(buf);
        delay_ms(2000);
    }
}
