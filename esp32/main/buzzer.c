#include "buzzer.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define BUZZER_TIMER_MS 200

static const char *TAG = "link.buzzer";
static int32_t s_buzzer_pin = -1;
static esp_timer_handle_t s_buzzer_timer = NULL;

static void buzzer_off_timer_cb(void *arg)
{
    if (s_buzzer_pin >= 0)
    {
        gpio_set_level((gpio_num_t)s_buzzer_pin, 0); // Turn OFF
    }
    ESP_LOGI(TAG, "Buzzer deactivated");
}

void init_buzzer(int32_t pin)
{
    s_buzzer_pin = pin;

    // Configure the pin as a standard push-pull output
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << pin),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};
    gpio_config(&io_conf);

    // Force it LOW immediately on boot so it stays quiet
    gpio_set_level((gpio_num_t)s_buzzer_pin, 0);

    // Create the non-blocking one-shot timer for turning it off
    const esp_timer_create_args_t buzzer_timer_args = {
        .callback = &buzzer_off_timer_cb,
        .name = "buzzer_off_timer"};
    esp_timer_create(&buzzer_timer_args, &s_buzzer_timer);
}

void activate_buzzer()
{
    if (s_buzzer_pin < 0)
    {
        ESP_LOGE(TAG, "Buzzer pin not initialized!");
        return;
    }

    ESP_LOGI(TAG, "Buzzer activated (active buzzer mode)");

    // Turn ON instantly via DC voltage
    gpio_set_level((gpio_num_t)s_buzzer_pin, 1);

    // Start or restart the 200ms background timer to shut it off non-blockingly
    esp_timer_stop(s_buzzer_timer);
    esp_timer_start_once(s_buzzer_timer, BUZZER_TIMER_MS * 1000); // microseconds
}