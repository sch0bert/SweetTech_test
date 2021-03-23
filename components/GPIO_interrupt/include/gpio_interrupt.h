#ifndef GPIO_INTERRUPTS_H_
#define GPIO_INTERRUPTS_H_

/* RESET Interrupt pinout */
#define GPIO_INPUT_IO_0     4
#define GPIO_INPUT_PIN_SEL_RESET_PIN    (1ULL<<GPIO_INPUT_IO_0)

/* mini DC motor control GPIO */
#define GPIO_OUTPUT_IO_1    19
#define GPIO_OUTPUT_PIN_SEL_DCMOTOR  (1ULL<<GPIO_OUTPUT_IO_1)

#define ESP_INTR_FLAG_DEFAULT 0

void gpio_interrupts_RESET_PIN(void);
void gpio_DCMotor_init(void);
void timer_DCMotor_init(void);

#endif /* GPIO_INTERRUPTS_H_ */
