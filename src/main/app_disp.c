#include "driver/rmt.h"
#include "led_strip.h"
#include "esp_log.h"

#define CONFIG_EXAMPLE_STRIP_LED_NUMBER 12
#define CONFIG_EXAMPLE_RMT_TX_GPIO 2
#define RMT_TX_CHANNEL RMT_CHANNEL_0

static const char *TAG = "disp";

led_strip_t *strip = NULL;

void display(uint8_t level, uint8_t brightness)
{
    for (int i = 0; i < CONFIG_EXAMPLE_STRIP_LED_NUMBER; ++i)
    {
        uint32_t red = brightness;
        uint32_t green = 0;
        uint32_t blue = 0;

        if (level < i*100.0/CONFIG_EXAMPLE_STRIP_LED_NUMBER)
            red = 0;

        ESP_ERROR_CHECK(strip->set_pixel(strip, i, red, green, blue));
    }
    ESP_ERROR_CHECK(strip->refresh(strip, 100));
}

void display_init()
{
    rmt_config_t config = RMT_DEFAULT_CONFIG_TX(CONFIG_EXAMPLE_RMT_TX_GPIO, RMT_TX_CHANNEL);
    config.clk_div = 2;

    ESP_ERROR_CHECK(rmt_config(&config));
    ESP_ERROR_CHECK(rmt_driver_install(config.channel, 0, 0));

    led_strip_config_t strip_config = LED_STRIP_DEFAULT_CONFIG(CONFIG_EXAMPLE_STRIP_LED_NUMBER, (led_strip_dev_t)config.channel);
    strip = led_strip_new_rmt_ws2812(&strip_config);
    if (!strip) {
        ESP_LOGE(TAG, "install WS2812 driver failed");
    }

    ESP_ERROR_CHECK(strip->clear(strip, 100));
}
