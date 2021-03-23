/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"

#include "gpio_interrupt.h"

static const char* MAIN_TAG = "MAIN";

void app_main(void)
{

    ESP_LOGI(MAIN_TAG, "Configuring GPIO interrupt...");
    gpio_interrupts_RESET_PIN();
    gpio_DCMotor_init();
    timer_DCMotor_init();
    ESP_LOGI(MAIN_TAG, "GPIO interrupts configured!");

    while (1)
    {

    }
}
