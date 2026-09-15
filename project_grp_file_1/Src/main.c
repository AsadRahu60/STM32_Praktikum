#include <stdint.h>
#include <stm32g0xx.h>
#include <stdio.h>
#include <string.h>

volatile uint32_t uwTick   = 0;
volatile uint32_t pulse_count = 0;   /* incremented in ISR — must be volatile */

void SysTick_Handler(void) { uwTick++; }

/* EXTI interrupt handler — name must match exactly (from startup file) */
void EXTI0_1_IRQHandler(void) {
    if (EXTI->RPR1 & EXTI_RPR1_RPIF0) {   /* check: was it PA0 rising edge? */
        pulse_count++;                      /* count the pulse */
        EXTI->RPR1 = EXTI_RPR1_RPIF0;     /* CLEAR the flag — critical, else ISR fires forever */
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
    /* Step 1: clocks */
    RCC->IOPENR  |= RCC_IOPENR_GPIOAEN;
    RCC->APBENR1 |= RCC_APBENR1_USART2EN;

    /* Step 2: PA2=TX, PA3=RX → UART */
    GPIOA->MODER &= ~((0x3U<<(2*2))|(0x3U<<(3*2)));
    GPIOA->MODER |=   (0x2U<<(2*2))|(0x2U<<(3*2));
    GPIOA->AFR[0] &= ~((0xFU<<(2*4))|(0xFU<<(3*4)));
    GPIOA->AFR[0] |=   (0x1U<<(2*4))|(0x1U<<(3*4));

    /* Step 3: PA0 = digital input (pull-down — stays LOW when no pulse) */
    GPIOA->MODER  &= ~(0x3U << (0*2));   /* 00 = input mode */
    GPIOA->PUPDR  &= ~(0x3U << (0*2));   /* clear pull */
    GPIOA->PUPDR  |=  (0x2U << (0*2));   /* 10 = pull-down */

    /* Step 4: USART2 */
    USART2->BRR  = 139;
    USART2->CR1 |= USART_CR1_TE | USART_CR1_UE;

    /* Step 5: SysTick */
    SysTick->LOAD = 15999;
    SysTick->VAL  = 0;
    SysTick->CTRL = 0x7;

    /* Step 6: EXTI — connect PA0 to EXTI line 0 */
    EXTI->EXTICR[0] &= ~EXTI_EXTICR1_EXTI0;   /* clear source selection */
    /* default is 0x00 = GPIOA, so PA0 is already selected */

    EXTI->RTSR1 |= EXTI_RTSR1_RT0;    /* rising edge trigger on line 0 */
    EXTI->IMR1  |= EXTI_IMR1_IM0;     /* unmask line 0 — enable interrupt */

    /* Step 7: NVIC — enable EXTI0_1 interrupt in the CPU */
    NVIC_EnableIRQ(EXTI0_1_IRQn);     /* tell CPU to accept EXTI0_1 interrupts */
    NVIC_SetPriority(EXTI0_1_IRQn, 1); /* priority 1 (0=highest, 3=lowest on G071) */

    uart_send_string("L15 EXTI ready — touch PA0 to 3.3V\r\n");

    char buf[64];
    for (;;) {
        snprintf(buf, sizeof(buf), "Pulses:%lu\r\n", pulse_count);
        uart_send_string(buf);
        delay_ms(1000);
    }
}
