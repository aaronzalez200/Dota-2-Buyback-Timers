#ifndef DISPLAY_H
#define DISPLAY_H

#include "driver/spi_master.h"
#include "lvgl.h"

#define MY_DISP_VER_RES 320 // 320
#define MY_DISP_HOR_RES 480 // 480

extern lv_obj_t * tabview;
extern lv_obj_t * footer;

void lcd_reset(void);
void lcd_write_data_byte(spi_device_handle_t spi, const uint8_t data);
void lcd_write_data_word(spi_device_handle_t spi, const uint16_t data);
void lcd_write_register(spi_device_handle_t spi, const uint8_t data);
void lcd_init(spi_device_handle_t spi);
void lcd_set_cursor(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t  yEnd);
void lcd_clear(uint16_t color);
void lcd_clear2(uint16_t color);
void lcd_clear_window(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd, uint16_t color);
void lcd_set_window_color(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd, uint16_t color);
void lcd_set_pixel(uint16_t x, uint16_t y, uint16_t color);
void lvgl_setup(void);

#endif