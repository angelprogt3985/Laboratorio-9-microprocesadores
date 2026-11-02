#include "RTE_Components.h"
#include CMSIS_device_header

volatile int potenciometro = 0;
volatile int fotoresistor = 0;
volatile int canal = 0;

void gpio_config(void) {

    RCC->AHB2ENR |= (1 << 0) | (1 << 1);

    GPIOA->MODER |= (3 << 0) | (3 << 2);
    GPIOA->PUPDR &= ~((3 << 0) | (3 << 2));

    GPIOB->MODER &= ~((3 << 0) |
                      (3 << 8) |
                      (3 << 10) |
                      (3 << 12) |
                      (3 << 14));

    GPIOB->MODER |= (2 << 0) |
                    (1 << 8) |
                    (1 << 10) |
                    (1 << 12) |
                    (1 << 14);

    GPIOB->OTYPER &= ~((1 << 0) |
                       (1 << 4) |
                       (1 << 5) |
                       (1 << 6) |
                       (1 << 7));

    GPIOB->PUPDR &= ~((3 << 0) |
                      (3 << 8) |
                      (3 << 10) |
                      (3 << 12) |
                      (3 << 14));

    GPIOB->AFR[0] &= ~(15 << 0);
    GPIOB->AFR[0] |= (2 << 0);

    GPIOB->BSRR = ((1 << 0) |
                   (1 << 4) |
                   (1 << 5) |
                   (1 << 6) |
                   (1 << 7)) << 16;
}

void pwm_config(void) {

    RCC->APB1ENR1 |= (1 << 1);

    TIM3->PSC = 15;
    TIM3->ARR = 999;
    TIM3->CCR3 = 0;

    TIM3->CCMR2 = (6 << 4) | (1 << 3);

    TIM3->CCER = (1 << 8);

    TIM3->EGR = (1 << 0);

    TIM3->CR1 = (1 << 7) | (1 << 0);
}

void adc_config(void) {

    RCC->AHB2ENR |= (1 << 13);

    ADC12_COMMON->CCR &= ~(3 << 16);
    ADC12_COMMON->CCR |= (3 << 16);

    ADC2->IER = 0;
    ADC2->CFGR = 0;
    ADC2->CFGR2 = 0;
    ADC2->DIFSEL = 0;

    ADC2->SMPR1 = (7 << 3) | (7 << 6);

    ADC2->SQR1 = (1 << 6);

    ADC2->CR &= ~(1 << 29);
    ADC2->CR |= (1 << 28);

    for (volatile int i = 0; i < 10000; i++);

    ADC2->CR &= ~(1 << 30);
    ADC2->CR |= 0x80000000;

    while (ADC2->CR & 0x80000000);

    for (volatile int i = 0; i < 100; i++);

    ADC2->ISR = (1 << 0) |
                (1 << 2) |
                (1 << 3) |
                (1 << 4);

    ADC2->IER = (1 << 2);

    NVIC_ClearPendingIRQ(ADC1_2_IRQn);
    NVIC_EnableIRQ(ADC1_2_IRQn);

    ADC2->CR |= (1 << 0);

    while (!(ADC2->ISR & (1 << 0)));

    ADC2->CR |= (1 << 2);
}

void ADC1_2_IRQHandler(void) {

    if (ADC2->ISR & (1 << 2)) {

        int dato = ADC2->DR;

        if (canal == 0) {

            potenciometro = dato;

            ADC2->SQR1 = (2 << 6);
            canal = 1;

        } else {

            fotoresistor = dato;

            ADC2->SQR1 = (1 << 6);
            canal = 0;
        }

        ADC2->ISR = (1 << 3) | (1 << 4);

        ADC2->CR |= (1 << 2);
    }
}

int main(void) {

    gpio_config();
    pwm_config();
    adc_config();

    for (;;) {

        int pot = potenciometro;
        int luz = fotoresistor;
        int leds = 0;

        if (pot >= 820) {
            leds |= (1 << 6);
        }

        if (pot >= 1639) {
            leds |= (1 << 5);
        }

        if (pot >= 2458) {
            leds |= (1 << 4);
        }

        if (pot >= 3277) {
            leds |= (1 << 7);
        }

        GPIOB->BSRR = leds |
            ((((1 << 4) |
               (1 << 5) |
               (1 << 6) |
               (1 << 7)) & ~leds) << 16);

        int brillo = (pot * 1000) / 4095;

        if (luz > 3000) {
            TIM3->CCR3 = brillo;
        } else {
            TIM3->CCR3 = 0;
        }
    }
}