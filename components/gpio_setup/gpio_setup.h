#ifndef GPIO_SETUP_H
#define GPIO_SETUP_H

#include "driver/spi_master.h"

/* GPIOs for Display
 * 
 * Header:
 * 1 - VCC
 * 2 - GND
 * 3 - SPI_MISO     [GPIO 2]    SPI DRIVER
 * 4 - SPI_MOSI     [GPIO 3]    SPI DRIVER
 * 5 - SPI_SCLK     [GPIO 4]    SPI DRIVER
 * 6 - LCD_CS       [GPIO 5]    OUTPUT
 * 7 - LCD_DC       [GPIO 21]    OUTPUT 
 * 8 - LCD_RESET    [GPIO 7]    OUTPUT
 * 9 - LCD_BL       [GPIO 20]    OUTPUT
 * 10 - I2C_SDA     [GPIO 8]   I2C DRIVER
 * 11 - I2C_SCL     [GPIO 6]   I2C DRIVER
 * 12 - I2C_INT     [GPIO 9]   INPUT w/ pull-up enabled
 * 13 - TP_RESET    [GPIO 12]   OUTPUT
 * 14 - GND         
 * 15 - GND
 * 
 */

 #if(1)

#define A0      1
#define A1      2
#define A2      3
#define A3      4
#define A4      11
#define A5      12
#define A6      13
#define A7      14
#define D2      5
#define D3      6
#define D4      7
#define D5      8
#define D6      9
#define D7      10
#define D8      17
#define D9      18
#define D10     21
#define D11     38
#define D12     47
#define D13     48

/* Warning 

Pins being used by keyboard.c below: 
D2, D3, D4, D5, D6, D7, D8, D9

*/

#define SPI_MISO    D12        
#define SPI_MOSI    D11  
#define SPI_SCL     D13    
#define LCD_CS      A7   
#define LCD_DC      A6   
#define LCD_RESET   A5    
#define LCD_BL      A4  
#define I2C_SDA     A3   
#define I2C_SCL     A2    
#define TP_INT      A1    
#define TP_RESET    A0
#endif

#if(0)

#define SPI_MISO        2 // 
#define SPI_MOSI        3 // 
#define SPI_SCL         4 // 
#define LCD_CS          5 // 
#define LCD_DC          21  // 
#define LCD_RESET       7  // 
#define LCD_BL          20  // 

#endif

#define ST7796_COLMOD 0x3A
#define ST7796_SLPOUT 0x11
#define ST7796_DISPON 0x29
#define ST7796_CASET  0x2A
#define ST7796_RASET  0x2B
#define ST7796_RAMWR  0x2C


// LEDC (PWM) configuration for controlling the LCD backlight
#define LEDC_CHANNEL        LEDC_CHANNEL_0
#define LEDC_TIMER          LEDC_TIMER_0
#define LEDC_GPIO           LCD_BL  // GPIO where the backlight is connected
#define LEDC_FREQUENCY      5000  // PWM frequency (Hz)
#define LEDC_RESOLUTION      LEDC_TIMER_13_BIT  // 13-bit resolution (8192 steps)

extern spi_device_handle_t spi;

extern QueueHandle_t gpio_evt_queue;

void gpio_setup(void);
void pwm_setup(void);

#endif