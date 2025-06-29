#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "driver/ledc.h"
#include "../display/display.h"
#include "gpio_setup.h"

/* GPIOs for Display
 * 
 * Header:
 * 1 - VCC
 * 2 - GND
 * 3 - SPI_MISO     [GPIO 2]    SPI DRIVER
 * 4 - SPI_MOSI     [GPIO 3]    SPI DRIVER
 * 5 - SPI_SCL      [GPIO 4]    SPI DRIVER
 * 6 - LCD_CS       [GPIO 5]    OUTPUT
 * 7 - LCD_DC       [GPIO 21]    OUTPUT 
 * 8 - LCD_RESET    [GPIO 7]    OUTPUT
 * 9 - LCD_BL       [GPIO 20]    OUTPUT
 * 10 - I2C_SDA     [GPIO 8]   I2C DRIVER #1
 * 11 - I2C_SCL     [GPIO 6]   I2C DRIVER #2
 * 12 - TP_INT      [GPIO 9]   INPUT w/ pull-up enabled
 * 13 - TP_RESET    [GPIO 12]   OUTPUT
 * 14 - GND         
 * 15 - GND
 * 
 * GPIOs for Step Counter
 * 
 * 4-pin connector:
 * 1 - SCL          [GPIO]
 * 2 - SDA          [GPIO]
 * 3 - VCC          
 * 4 - GND
 * 
 */


/* Defines for configuring GPIOs */
#define GPIO_OUTPUT_PIN_SEL  (  (1ULL<<LCD_CS) | \ 
                                (1ULL<<LCD_DC) | \
                                (1ULL<<LCD_RESET))
                                // (1ULL<<TP_RESET))

#define GPIO_INPUT_PIN_SEL  1ULL<<TP_INT

#define ESP_INTR_FLAG_DEFAULT 0

#define LEDC_DUTY_MAX   8191        // Max duty cycle for 13-bit resolution

static const char *TAG = "GPIO_SETUP";

/* Initialize global variables for SPI */
spi_device_handle_t spi = NULL;

/* Initialize global variable for isr handler queue*/
QueueHandle_t gpio_evt_queue = NULL;

void IRAM_ATTR gpio_isr_handler(void* arg);
void output_setup(void);
void gpio_setup(void);

/* @brief Handler for ISR
 * @param arg GPIO9 TP_INT. Detects user gesture/touch 
 */
void IRAM_ATTR gpio_isr_handler(void* arg) {
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

/* @brief Sets up Output GPIOs
 * @param N/A 
 */
void output_setup() {
    //zero-initialize the config structure.
    gpio_config_t io_conf = {};
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    //gpio_set_level(LCD_BL, 1);  // turn on LCD's backlight
    ESP_LOGI(TAG, "Output_setup completed");
}

/* @brief Configures PWM for BL (backlight) screen brightness
 * @param N/A 
 */
void pwm_setup() {
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_RESOLUTION,  // Set resolution to 13-bit
        .freq_hz = LEDC_FREQUENCY,          // Set frequency to 5kHz
        .speed_mode = LEDC_LOW_SPEED_MODE,  // High-speed mode
        .timer_num = LEDC_TIMER,             // Timer index
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    ledc_channel_config_t ledc_channel = {
        .channel    = LEDC_CHANNEL,
        .duty       = 0,                      // Start with 0 duty cycle (off)
        .gpio_num   = LEDC_GPIO,              // GPIO connected to the backlight
        .speed_mode = LEDC_LOW_SPEED_MODE,   // High-speed mode
        .timer_sel  = LEDC_TIMER,             // Timer index
        .hpoint     = 0,                      // Initial high point
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
    ESP_LOGI(TAG, "PWM_setup completed");
}

void set_backlight_brightness(float brightness) {
    uint32_t duty = (uint32_t)(brightness * LEDC_DUTY_MAX);
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL, duty));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL));
}

/* @brief Call back for SPI transmission, does nothing here.
 * @param N/A 
 */
void pre_transfer_callback(spi_transaction_t *t)
{
    // do nothing, required I guess
}

/* @brief Configures SPI driver
 * @param N/A 
 */
void spi_setup() {
    spi_bus_config_t buscfg = {
        .miso_io_num = SPI_MISO,
        .mosi_io_num = SPI_MOSI,
        .sclk_io_num = SPI_SCL,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096,
    };
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 80 * 1000 * 1000,     //Clock out at 10 MHz
        .mode = 0,                              //SPI mode 0
        .spics_io_num = LCD_CS,                 //CS pin
        .queue_size = 7,                        //We want to be able to queue 7 transactions at a time
        .pre_cb = pre_transfer_callback, //Specify pre-transfer callback to handle D/C line
        .flags = SPI_DEVICE_HALFDUPLEX,
    };
    //Initialize the SPI bus
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));

    //Attach the LCD to the SPI bus
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &spi));

    ESP_LOGI(TAG, "spi_setup completed");
}

void test_display(void) {
    lcd_init(spi);
    printf("lcd_init\n");
    vTaskDelay(700 / portTICK_PERIOD_MS);
    lcd_clear2(0xFF04);
    vTaskDelay(700 / portTICK_PERIOD_MS);
    lcd_clear2(0x4504);
    lvgl_setup();
    printf("lvgl_setup();");
}

/* @brief Sets up all configurations: SPI/I2C/PWM
 * @param N/A 
 */
void gpio_setup() {
    output_setup();
    printf("spi_setup();\n");
    spi_setup();
    printf("output_setup\n");
    pwm_setup();
    printf("pwm_setup();\n");
    set_backlight_brightness(0.5);
    printf(" set_backlight_brightness(0.1);\n");
    test_display();
    printf("test_display();");
}