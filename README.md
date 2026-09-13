# stm32-ll-praktikum-prep

STM32 LL library preparation for Praktikum at Leuze Electronic GmbH, Germering.
Start date: 01.10.2026. Task: BER/SNR optical measurement pipeline.

**Board:** NUCLEO-G071RB (STM32G071RBT6, Cortex-M0+, 64 MHz)
**Library:** LL only — no HAL
**Tools:** STM32CubeIDE, arm-none-eabi-gcc, Python 3

---

## Curriculum

| # | Lesson | Status |
|---|--------|--------|
| L01 | GPIO Output — LED blink | 🔄 in progress |
| L02 | GPIO Input + SysTick timing | ⬜ pending |
| L03 | Timers and PWM | ⬜ pending |
| L04 | Interrupts and NVIC | ⬜ pending |
| L05 | UART transmit | ⬜ pending |
| L06 | UART receive + ring buffer | ⬜ pending |
| L07 | ADC | ⬜ pending |
| L08 | ADC + Timer + DMA | ⬜ pending |
| L09 | Linux and git | ⬜ pending |
| L10 | pyserial | ⬜ pending |
| L11 | Binary framing protocol | ⬜ pending |
| L12 | PRBS and LFSR | ⬜ pending |
| L13 | BER measurement | ⬜ pending |
| L14 | SNR, Q-factor, eye diagrams | ⬜ pending |
| L15 | pandas and matplotlib | ⬜ pending |
| L16 | Making it a tool | ⬜ pending |
| — | Integration capstone | ⬜ pending |

---

## Repo structure

```
stm32-ll-praktikum-prep/
├── PROGRESS.md          ← one-line-per-session log
├── README.md            ← this file
├── 01_gpio_blink/
│   ├── main_raw.c       ← raw register version
│   ├── main.c           ← LL version
│   └── README.md
├── 02_button_systick/
├── 03_timer_pwm/
...
└── tools/               ← Python host-side scripts (Phase 2)
```
