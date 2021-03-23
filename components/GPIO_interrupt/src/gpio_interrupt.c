#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/timers.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/rtc_io.h"

#include "gpio_interrupt.h"

static const char* GPIO_TAG = "GPIO_INTERRUPTS";

static uint8_t value = 0;

xQueueHandle xQueue_RESET_PIN;
TaskHandle_t xDCMotor_Handle;
xTimerHandle timerHndl3Sec;

// Interrupt handler for Reset GPIO
static void IRAM_ATTR RESET_PIN_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t)arg;

    xQueueSendFromISR(xQueue_RESET_PIN, &gpio_num, NULL);
}

// Task that will change the status of the DC Motor
void DC_Motor_task(void* pvParameter)
{
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = 3000 / portTICK_PERIOD_MS;

    // Initialise the xLastWakeTime variable with the current time.
    xLastWakeTime = xTaskGetTickCount();

    for (;;)
    {
        // Wait for the next cycle.
        vTaskDelayUntil(&xLastWakeTime, xFrequency);

        ESP_LOGI(GPIO_TAG, "3 seconds passed, change DC motor status");

        value = value ^ 0x01;
        ESP_LOGI(GPIO_TAG, "Value = %d", value);
        gpio_set_level(GPIO_OUTPUT_IO_1, value);
    }
}

// Task that will react to RESET pin
void RESET_PIN_InterruptRead_task(void* pvParameter)
{
    uint32_t io_num;

    for (;;) {
        // Wait for the notification from the ISR
        if (xQueueReceive(xQueue_RESET_PIN, &io_num, 0)) {

            if (value == 1 && (gpio_get_level(GPIO_INPUT_IO_0) == 1)) {
                ESP_LOGI(GPIO_TAG, "DC motor is running, we should stop it!");
                // Stop the DC motor
                gpio_set_level(GPIO_OUTPUT_IO_1, 0);
                value = 0;
                // Stop the DC Motor Task
                vTaskSuspend(xDCMotor_Handle);
                if(xTimerStart(timerHndl3Sec, 0) != pdPASS)
                {
                    ESP_LOGE(GPIO_TAG, "Lack of resources to launch timer hander 3 sec");
                }
            }
            else {
                //ESP_LOGI(GPIO_TAG, "DC motor is stopped, don't do anything");
            }
        }
    }
}

// Timer callback when the 3 seconds expired
void vTimerCallback3SecExpired(xTimerHandle pxTimer)
{
    // We resume the DC Motor Task
    vTaskResume(xDCMotor_Handle);
    // We stop the timer
    xTimerStop(timerHndl3Sec, 0);
}

// GPIO DC Motor controller initialization
void gpio_DCMotor_init()
{
    gpio_config_t io_conf;

    //disable interrupt
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO19
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL_DCMOTOR;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    xTaskCreate(DC_Motor_task, "DC_Motor_task", 2048, NULL, 10, &xDCMotor_Handle);
}

// Timer for suspend DC Motor task
void timer_DCMotor_init()
{
    timerHndl3Sec = xTimerCreate(
        "timer3Sec", /* name */
        pdMS_TO_TICKS(3000), /* period/time */
        pdTRUE, /* auto-reload */
        (void*)0, /* timer ID */
        vTimerCallback3SecExpired); /* callback */
}

// GPIO Reset pin controller initialization
void gpio_interrupts_RESET_PIN()
{
    gpio_config_t io_conf;

    //interrupt of rising edge
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    //bit mask of the pins,
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL_RESET_PIN;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //disable pull-down mode
    io_conf.pull_down_en = 1;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    xQueue_RESET_PIN = xQueueCreate(1, sizeof(uint32_t));
    xTaskCreate(RESET_PIN_InterruptRead_task, "RESET_PIN_InterruptRead_task", 2048, NULL, 10, NULL);

    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_INPUT_IO_0, RESET_PIN_isr_handler, (void*)GPIO_INPUT_IO_0);
}
