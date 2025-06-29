#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_attr.h"
#include "esp_timer.h"
#include "lvgl.h"
#include "esp_heap_caps.h"

#include "../gpio_setup/gpio_setup.h"
#include "display.h"

#include "../../main/time_tracker.h"

uint8_t *buf1 = NULL;
lv_obj_t * tabview = NULL;
lv_obj_t * footer = NULL;
lv_obj_t * label1 = NULL;
lv_obj_t * label2 = NULL;
lv_obj_t * hero_labels[HERO_COUNT];


#define BUFFER_LINES 20  // Number of lines to buffer (adjust as needed)
#define MAX_SPI_TRANSFER_SIZE 1024

/* Display driver things; Sending data/reset/initialization */
void lcd_reset(void);
void lcd_write_data_byte(spi_device_handle_t spi, const uint8_t data);
void lcd_write_data_word(spi_device_handle_t spi, const uint16_t data);
void lcd_write_register(spi_device_handle_t spi, const uint8_t data);
void lcd_init(spi_device_handle_t spi);

/* Display driver: Sending pixels, clearing screen, etc. */
void lcd_set_cursor(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t  yEnd);
void lcd_clear(uint16_t color);
void lcd_clear_window(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd, uint16_t color);
void lcd_set_window_color(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd, uint16_t color);
void lcd_set_pixel(uint16_t x, uint16_t y, uint16_t color);

/* @brief Resets LCD
 * @param N/A
 */
void lcd_reset(void) {
    gpio_set_level(LCD_CS, 0);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    gpio_set_level(LCD_RESET, 0);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    gpio_set_level(LCD_RESET, 1);   
}

/* @brief Writes byte of data to LCD via SPI
 * @param SPI device (spi_device_handle_t)
 * @param Data: 1 byte
 * @note CS starts LOW and ends HIGH
 * @note DC is HIGH
 */
void lcd_write_data_byte(spi_device_handle_t spi, uint8_t data) {
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = 8;
    t.tx_buffer = &data;
    gpio_set_level(LCD_CS, 0);
    gpio_set_level(LCD_DC, 1);

    ESP_ERROR_CHECK(spi_device_polling_transmit(spi, &t));
    gpio_set_level(LCD_CS, 1);
}

/* @brief Writes byte of data to LCD via SPI
 * @param SPI device (spi_device_handle_t)
 * @param Data: 2 bytes OR 1 word
 * @note CS starts LOW and ends HIGH
 * @note DC is HIGH
 */
void lcd_write_data_word(spi_device_handle_t spi, uint16_t data) {
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = 16;
    t.tx_buffer = &data;
    gpio_set_level(LCD_CS, 0);
    gpio_set_level(LCD_DC, 1);

    ESP_ERROR_CHECK(spi_device_polling_transmit(spi, &t));
    gpio_set_level(LCD_CS, 1);
}

void lcd_fast_word(spi_device_handle_t spi, uint16_t data) {
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = 16;
    t.tx_buffer = &data;
    ESP_ERROR_CHECK(spi_device_polling_transmit(spi, &t));
}

/* @brief Writes byte of data to LCD via SPI
 * @param SPI device (spi_device_handle_t)
 * @param Data: 1 byte
 * @note CS starts LOW and ends HIGH
 * @note DC is LOW
 */
void lcd_write_register(spi_device_handle_t spi, uint8_t data) {
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = 8;
    t.tx_buffer = &data;
    gpio_set_level(LCD_CS, 0);
    gpio_set_level(LCD_DC, 0);

    ESP_ERROR_CHECK(spi_device_polling_transmit(spi, &t));
    gpio_set_level(LCD_CS, 1);
}

/* @brief LCD init sequence, taken from Arduino example
 *
 */
void lcd_init(spi_device_handle_t spi) {
    lcd_reset();
    /* Start Initial Sequence */
    lcd_write_register(spi, 0xEF);
	lcd_write_register(spi, 0xEB);
	lcd_write_data_byte(spi, 0x14); 
	
  	lcd_write_register(spi, 0xFE);			 
	lcd_write_register(spi, 0xEF); 

	lcd_write_register(spi, 0xEB);	
	lcd_write_data_byte(spi, 0x14); 

	lcd_write_register(spi, 0x84);			
	lcd_write_data_byte(spi, 0x40); 

	lcd_write_register(spi, 0x85);			
	lcd_write_data_byte(spi, 0xFF); 

	lcd_write_register(spi, 0x86);			
	lcd_write_data_byte(spi, 0xFF); 

	lcd_write_register(spi, 0x87);			
	lcd_write_data_byte(spi, 0xFF);

	lcd_write_register(spi, 0x88);			
	lcd_write_data_byte(spi, 0x0A);

	lcd_write_register(spi, 0x89);			
	lcd_write_data_byte(spi, 0x21); 

	lcd_write_register(spi, 0x8A);			
	lcd_write_data_byte(spi, 0x00); 

	lcd_write_register(spi, 0x8B);			
	lcd_write_data_byte(spi, 0x80); 

	lcd_write_register(spi, 0x8C);			
	lcd_write_data_byte(spi, 0x01); 

	lcd_write_register(spi, 0x8D);			
	lcd_write_data_byte(spi, 0x01); 

	lcd_write_register(spi, 0x8E);			
	lcd_write_data_byte(spi, 0xFF); 

	lcd_write_register(spi, 0x8F);			
	lcd_write_data_byte(spi, 0xFF); 


	lcd_write_register(spi, 0xB6);
	lcd_write_data_byte(spi, 0x00);
	lcd_write_data_byte(spi, 0x20);

	lcd_write_register(spi, 0x36);	// Memory Access Control (36h)
	//lcd_write_data_byte(spi, 0x08);	OLD
	lcd_write_data_byte(spi, 0x28);
	lcd_write_register(spi, 0x3A);			
	lcd_write_data_byte(spi, 0x05); 


	lcd_write_register(spi, 0x90);			
	lcd_write_data_byte(spi, 0x08);
	lcd_write_data_byte(spi, 0x08);
	lcd_write_data_byte(spi, 0x08);
	lcd_write_data_byte(spi, 0x08); 

	lcd_write_register(spi, 0xBD);			
	lcd_write_data_byte(spi, 0x06);
	
	lcd_write_register(spi, 0xBC);			
	lcd_write_data_byte(spi, 0x00);	

	lcd_write_register(spi, 0xFF);			
	lcd_write_data_byte(spi, 0x60);
	lcd_write_data_byte(spi, 0x01);
	lcd_write_data_byte(spi, 0x04);

	lcd_write_register(spi, 0xC3);			
	lcd_write_data_byte(spi, 0x13);
	lcd_write_register(spi, 0xC4);			
	lcd_write_data_byte(spi, 0x13);

	lcd_write_register(spi, 0xC9);			
	lcd_write_data_byte(spi, 0x22);

	lcd_write_register(spi, 0xBE);			
	lcd_write_data_byte(spi, 0x11); 

	lcd_write_register(spi, 0xE1);			
	lcd_write_data_byte(spi, 0x10);
	lcd_write_data_byte(spi, 0x0E);

	lcd_write_register(spi, 0xDF);			
	lcd_write_data_byte(spi, 0x21);
	lcd_write_data_byte(spi, 0x0c);
	lcd_write_data_byte(spi, 0x02);

	lcd_write_register(spi, 0xF0);   
	lcd_write_data_byte(spi, 0x45);
	lcd_write_data_byte(spi, 0x09);
	lcd_write_data_byte(spi, 0x08);
	lcd_write_data_byte(spi, 0x08);
	lcd_write_data_byte(spi, 0x26);
 	lcd_write_data_byte(spi, 0x2A);

 	lcd_write_register(spi, 0xF1);    
 	lcd_write_data_byte(spi, 0x43);
 	lcd_write_data_byte(spi, 0x70);
 	lcd_write_data_byte(spi, 0x72);
 	lcd_write_data_byte(spi, 0x36);
 	lcd_write_data_byte(spi, 0x37);  
 	lcd_write_data_byte(spi, 0x6F);


 	lcd_write_register(spi, 0xF2);   
 	lcd_write_data_byte(spi, 0x45);
 	lcd_write_data_byte(spi, 0x09);
 	lcd_write_data_byte(spi, 0x08);
 	lcd_write_data_byte(spi, 0x08);
 	lcd_write_data_byte(spi, 0x26);
 	lcd_write_data_byte(spi, 0x2A);

 	lcd_write_register(spi, 0xF3);   
 	lcd_write_data_byte(spi, 0x43);
 	lcd_write_data_byte(spi, 0x70);
 	lcd_write_data_byte(spi, 0x72);
 	lcd_write_data_byte(spi, 0x36);
 	lcd_write_data_byte(spi, 0x37); 
 	lcd_write_data_byte(spi, 0x6F);

	lcd_write_register(spi, 0xED);	
	lcd_write_data_byte(spi, 0x1B); 
	lcd_write_data_byte(spi, 0x0B); 

	lcd_write_register(spi, 0xAE);			
	lcd_write_data_byte(spi, 0x77);
	
	lcd_write_register(spi, 0xCD);			
	lcd_write_data_byte(spi, 0x63);		


	lcd_write_register(spi, 0x70);			
	lcd_write_data_byte(spi, 0x07);
	lcd_write_data_byte(spi, 0x07);
	lcd_write_data_byte(spi, 0x04);
	lcd_write_data_byte(spi, 0x0E); 
	lcd_write_data_byte(spi, 0x0F); 
	lcd_write_data_byte(spi, 0x09);
	lcd_write_data_byte(spi, 0x07);
	lcd_write_data_byte(spi, 0x08);
	lcd_write_data_byte(spi, 0x03);

	lcd_write_register(spi, 0xE8);			
	lcd_write_data_byte(spi, 0x34);

	lcd_write_register(spi, 0x62);			
	lcd_write_data_byte(spi, 0x18);
	lcd_write_data_byte(spi, 0x0D);
	lcd_write_data_byte(spi, 0x71);
	lcd_write_data_byte(spi, 0xED);
	lcd_write_data_byte(spi, 0x70); 
	lcd_write_data_byte(spi, 0x70);
	lcd_write_data_byte(spi, 0x18);
	lcd_write_data_byte(spi, 0x0F);
	lcd_write_data_byte(spi, 0x71);
	lcd_write_data_byte(spi, 0xEF);
	lcd_write_data_byte(spi, 0x70); 
	lcd_write_data_byte(spi, 0x70);

	lcd_write_register(spi, 0x63);			
	lcd_write_data_byte(spi, 0x18);
	lcd_write_data_byte(spi, 0x11);
	lcd_write_data_byte(spi, 0x71);
	lcd_write_data_byte(spi, 0xF1);
	lcd_write_data_byte(spi, 0x70); 
	lcd_write_data_byte(spi, 0x70);
	lcd_write_data_byte(spi, 0x18);
    lcd_write_data_byte(spi, 0x13);
	lcd_write_data_byte(spi, 0x71);
	lcd_write_data_byte(spi, 0xF3);
	lcd_write_data_byte(spi, 0x70); 
	lcd_write_data_byte(spi, 0x70);

	lcd_write_register(spi, 0x64);			
	lcd_write_data_byte(spi, 0x28);
	lcd_write_data_byte(spi, 0x29);
	lcd_write_data_byte(spi, 0xF1);
	lcd_write_data_byte(spi, 0x01);
	lcd_write_data_byte(spi, 0xF1);
	lcd_write_data_byte(spi, 0x00);
	lcd_write_data_byte(spi, 0x07);

	lcd_write_register(spi, 0x66);			
	lcd_write_data_byte(spi, 0x3C);
	lcd_write_data_byte(spi, 0x00);
	lcd_write_data_byte(spi, 0xCD);
	lcd_write_data_byte(spi, 0x67);
	lcd_write_data_byte(spi, 0x45);
	lcd_write_data_byte(spi, 0x45);
	lcd_write_data_byte(spi, 0x10);
	lcd_write_data_byte(spi, 0x00);
	lcd_write_data_byte(spi, 0x00);
	lcd_write_data_byte(spi, 0x00);

	lcd_write_register(spi, 0x67);			
	lcd_write_data_byte(spi, 0x00);
	lcd_write_data_byte(spi, 0x3C);
	lcd_write_data_byte(spi, 0x00);
	lcd_write_data_byte(spi, 0x00);
	lcd_write_data_byte(spi, 0x00);
	lcd_write_data_byte(spi, 0x01);
	lcd_write_data_byte(spi, 0x54);
	lcd_write_data_byte(spi, 0x10);
	lcd_write_data_byte(spi, 0x32);
	lcd_write_data_byte(spi, 0x98);

	lcd_write_register(spi, 0x74);			
	lcd_write_data_byte(spi, 0x10);	
	lcd_write_data_byte(spi, 0x85);	
	lcd_write_data_byte(spi, 0x80);
	lcd_write_data_byte(spi, 0x00); 
	lcd_write_data_byte(spi, 0x00); 
	lcd_write_data_byte(spi, 0x4E);
	lcd_write_data_byte(spi, 0x00);					
	
    lcd_write_register(spi, 0x98);			
	lcd_write_data_byte(spi, 0x3e);
	lcd_write_data_byte(spi, 0x07);

	lcd_write_register(spi, 0x35);	
	lcd_write_register(spi, 0x21);

	lcd_write_register(spi, 0x11);
	vTaskDelay(120 / portTICK_PERIOD_MS);
	lcd_write_register(spi, 0x29);
	vTaskDelay(20 / portTICK_PERIOD_MS);
}

/* @brief Set the cursor position
 * @param xStart Start word x coordinate
 * @param xStart:   Start word x coordinate
 * @param yStart:   Start word y coordinate
 * @param xEnd  :   End word coordinates
 */
void lcd_set_cursor(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd) {
  // Set Column Address (X-axis)
  lcd_write_register(spi, 0x2A); // Column address set
  lcd_write_data_byte(spi, (xStart >> 8) & 0xFF); // Start high byte
  lcd_write_data_byte(spi, xStart & 0xFF);        // Start low byte
  lcd_write_data_byte(spi, (xEnd >> 8) & 0xFF);   // End high byte
  lcd_write_data_byte(spi, xEnd & 0xFF);          // End low byte

  // Set Page Address (Y-axis)
  lcd_write_register(spi, 0x2B); // Page address set
  lcd_write_data_byte(spi, (yStart >> 8) & 0xFF); // Start high byte
  lcd_write_data_byte(spi, yStart & 0xFF);        // Start low byte
  lcd_write_data_byte(spi, (yEnd >> 8) & 0xFF);   // End high byte
  lcd_write_data_byte(spi, yEnd & 0xFF);          // End low byte

  // Memory Write (ready for pixel data)
  lcd_write_register(spi, 0x2C);
}



/* @brief Clear screen function, refresh the screen to a certain color
 * @param color The color you want to clear all the screen
 */
void lcd_clear(uint16_t color) {
  uint16_t i,j;    
  lcd_set_cursor(0,0,MY_DISP_HOR_RES-1,MY_DISP_VER_RES-1);
  for(i = 0; i < MY_DISP_HOR_RES; i++){
    for(j = 0; j < MY_DISP_VER_RES; j++){
      lcd_write_data_word(spi, color);
    }
  }
}


/* @brief Refresh a certain area to the same color
 * @param xStart Start word x coordinate
 * @param yStart Start word y coordinate
 * @param xEnd End word coordinates
 * @param yEnd End word coordinates
 * @param color Set the color
 */
void lcd_clear_window(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd, uint16_t color) {          
  uint16_t i,j; 
  lcd_set_cursor(xStart, yStart, xEnd-1,yEnd-1);
  for(i = yStart; i <= yEnd-1; i++){                                
    for(j = xStart; j <= xEnd-1; j++){
      lcd_write_data_word(spi, color);
    }
  }                   
}

/* @brief Set the color of an area
 * @param xStart Start word x coordinate
 * @param yStart Start word y coordinate
 * @param xEnd End word coordinates
 * @param yEnd End word coordinates
 * @param color Set the color
 */
void lcd_set_window_color(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd, uint16_t color) {
  lcd_set_cursor( xStart,yStart,xEnd,yEnd);
  lcd_write_data_word(spi, color);      
}

/////////////////////////
#define MAX_SPI_TRANSFER_SIZE 4096
// Define the color buffer in DRAW

void lcd_clear2(uint16_t color) {
    esp_err_t ret;
    int chunk_size = MAX_SPI_TRANSFER_SIZE / 2; // Each pixel is 2 bytes

    // Set the cursor to the full screen
    lcd_set_cursor(0, 0, MY_DISP_HOR_RES - 1, MY_DISP_VER_RES - 1);

    gpio_set_level(LCD_CS, 0);
    gpio_set_level(LCD_DC, 1);

    // Small buffer that fits within MAX_SPI_TRANSFER_SIZE
    uint16_t COLOR_BUFFER[chunk_size];

    // Fill the screen in chunks
    int total_pixels = MY_DISP_HOR_RES * MY_DISP_VER_RES;
    for (int i = 0; i < total_pixels; i += chunk_size) {
        int current_chunk_size = (total_pixels - i < chunk_size) ? (total_pixels - i) : chunk_size;

        // Fill current buffer chunk with the desired color
        for (int j = 0; j < current_chunk_size; j++) {
            COLOR_BUFFER[j] = color;
        }

        // Prepare and queue SPI transaction
        spi_transaction_t trans = {
            .length = current_chunk_size * 2 * 8, // Length in bits
            .tx_buffer = COLOR_BUFFER,
            .flags = 0,
            .user = (void*)1
        };

        ret = spi_device_queue_trans(spi, &trans, portMAX_DELAY);
        assert(ret == ESP_OK);

        // Wait for transaction to complete
        spi_transaction_t *ret_trans;
        ret = spi_device_get_trans_result(spi, &ret_trans, portMAX_DELAY);
        assert(ret == ESP_OK);
    }
}

/////////////////////////
/* @brief Write pixel to coordinate with specified color
 * @param X Set the X coordinate
 * @param Y Set the Y coordinate
 * @param color Set the color
 */
void lcd_set_pixel(uint16_t x, uint16_t y, uint16_t color)
{
  lcd_set_cursor(x,y,x,y);
  lcd_write_data_word(spi, color);       
} 

/* LVGL Section
 *
 */
uint32_t my_tick_get_cb(void) {
    return esp_timer_get_time() / 1000;
}

void my_flush_cb(lv_display_t * display, const lv_area_t * area, uint8_t * px_map)
{
    esp_err_t ret;
    int chunk_size = MAX_SPI_TRANSFER_SIZE / 2; // Each pixel is 2 bytes (RGB565)

    // Set the cursor to the area being flushed
    lcd_set_cursor(area->x1, area->y1, area->x2, area->y2);

    gpio_set_level(LCD_CS, 0);  // Select LCD
    gpio_set_level(LCD_DC, 1);  // Set to data mode

    // Compute number of pixels in flush area
    int width = area->x2 - area->x1 + 1;
    int height = area->y2 - area->y1 + 1;
    int num_pixels = width * height;

    // Cast source buffer to uint16_t (LVGL gives RGB565 data)
    uint16_t *buf16 = (uint16_t *)px_map;

    // Allocate small working buffer for one SPI chunk
    uint16_t COLOR_BUFFER[chunk_size];

    // Send buffer in chunks
    for (int i = 0; i < num_pixels; i += chunk_size) {
        int current_chunk_size = (num_pixels - i < chunk_size) ? (num_pixels - i) : chunk_size;

        // Swap bytes and fill COLOR_BUFFER
        for (int j = 0; j < current_chunk_size; j++) {
            uint16_t color = buf16[i + j];
            COLOR_BUFFER[j] = (color >> 8) | (color << 8); // RGB565 byte swap
        }

        spi_transaction_t trans = {
            .length = current_chunk_size * 2 * 8, // bits
            .tx_buffer = COLOR_BUFFER,
            .flags = 0,
            .user = (void*)1
        };

        ret = spi_device_queue_trans(spi, &trans, portMAX_DELAY);
        assert(ret == ESP_OK);

        spi_transaction_t *ret_trans;
        ret = spi_device_get_trans_result(spi, &ret_trans, portMAX_DELAY);
        assert(ret == ESP_OK);
    }
    // Notify LVGL that the flush is complete
    lv_display_flush_ready(display);
}

void hero_timer(lv_timer_t * hero_1_t)
{
	for (int i = 0; i < HERO_COUNT; i++) {
        if (!all_timers_active) {
            lv_label_set_text_fmt(hero_labels[i], "Hero #%d: -------", i + 1);
        }
        else if (!hero_timers[i].active) {
            lv_label_set_text_fmt(hero_labels[i], "Hero #%d: Available", i + 1);
        }
        else {
            lv_label_set_text_fmt(hero_labels[i], "Hero #%d: %02d:%02d", i + 1,
                                  hero_timers[i].minutes, hero_timers[i].seconds);
        }
    }
}




void my_timer(lv_timer_t * timer)
{
	if (!all_timers_active) {
        lv_label_set_text(footer, "In-game Timer: --:--");
        return;
    }

    if (!game_timer_active) {
        lv_label_set_text_fmt(footer, "In-game Timer: %02ld:%02d\nTimer is paused.", game_timer_minutes, game_timer_seconds);
    } else {
        lv_label_set_text_fmt(footer, "In-game Timer: %02ld:%02d", game_timer_minutes, game_timer_seconds);
    }
}

void index_timer(lv_timer_t * timer)
{
	lv_tabview_set_act(tabview, indexing, LV_ANIM_OFF);
}

void lv_example_tabview_1(void)
{
    // Create a parent container with vertical flex layout
    lv_obj_t * parent = lv_obj_create(lv_screen_active());
    lv_obj_set_size(parent, 480, 320);
    lv_obj_set_layout(parent, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);

    // REMOVE default styling (important!)
    lv_obj_set_style_pad_all(parent, 0, 0);
    lv_obj_set_style_border_width(parent, 0, 0);
    lv_obj_set_style_bg_opa(parent, LV_OPA_TRANSP, 0);  // Optional: transparent background

    // Create the tabview
    tabview = lv_tabview_create(parent);
    lv_obj_set_width(tabview, LV_PCT(100));
    lv_obj_set_height(tabview, 270); // Manually leave space for footer
    lv_obj_set_flex_grow(tabview, 1);

    // Add tabs
    lv_obj_t * tab1 = lv_tabview_add_tab(tabview, "Ult Cooldowns");
    lv_obj_t * tab2 = lv_tabview_add_tab(tabview, "Tormentor");
    lv_obj_t * tab3 = lv_tabview_add_tab(tabview, "Buybacks");

    // Add content to tabs
    label1 = lv_label_create(tab1);
	label2 = lv_label_create(tab2);

	/* Tab #1 */
	lv_label_set_text(label1, "Filler Text\nReserved for later use.");
	lv_obj_set_style_text_font(label1, &lv_font_montserrat_38, 0);

	/* Tab #2 */
    lv_label_set_text(label2, "Filler Text\nReserved for later use.");
	lv_obj_set_style_text_font(label2, &lv_font_montserrat_38, 0);

	/* Tab #3 */
	lv_timer_t * heroes_t = lv_timer_create(hero_timer, 250, NULL);

	// Create a container for the hero labels
	lv_obj_t *hero_container = lv_obj_create(tab3);
	lv_obj_set_size(hero_container, LV_PCT(100), LV_SIZE_CONTENT);
	lv_obj_set_layout(hero_container, LV_LAYOUT_FLEX);
	lv_obj_set_flex_flow(hero_container, LV_FLEX_FLOW_COLUMN); // Stack vertically
	lv_obj_set_style_pad_column(hero_container, 8, 0); // Optional spacing between labels
	lv_obj_set_style_pad_all(hero_container, 0, 0); // No padding around the container
	lv_obj_set_scrollbar_mode(hero_container, LV_SCROLLBAR_MODE_OFF); // Optional
	lv_obj_set_style_border_width(hero_container, 0, 0);   // Remove border
	lv_obj_set_style_bg_opa(hero_container, LV_OPA_TRANSP, 0); // Make background transparent

	// Now create and add labels into the container
	for (int i = 0; i < HERO_COUNT; i++) {
		hero_labels[i] = lv_label_create(hero_container); // Add to container, not directly to tab
		lv_label_set_text_fmt(hero_labels[i], "Hero #%d: -------", i + 1); // No newline needed!
		lv_obj_set_style_text_font(hero_labels[i], &lv_font_montserrat_24, 0);
	}

    /* FOOTER */
    footer = lv_label_create(parent);
	lv_label_set_text(footer, "In-game Timer: --:--");  // Prevent default "Text"
    lv_timer_t * timer = lv_timer_create(my_timer, 100, NULL);
	lv_obj_set_width(footer, LV_PCT(100));
    lv_obj_set_flex_grow(footer, 0); // Ensure it doesn't expand
    lv_obj_set_style_text_align(footer, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_pad_all(footer, 2, 0);
    lv_obj_set_style_bg_color(footer, lv_color_white(), 0);
    lv_obj_set_style_text_color(footer, lv_color_black(), 0);
	lv_obj_set_style_text_font(footer, &lv_font_montserrat_24, 0);

	lv_tabview_set_act(tabview, indexing, LV_ANIM_OFF);
	lv_timer_t * timer2 = lv_timer_create(index_timer, 50, NULL);
}

void lvgl_setup(void) {
	lv_init(); // Initialize LVGL

	/* Create the display */
    lv_display_t * display1 = lv_display_create(MY_DISP_HOR_RES, MY_DISP_VER_RES);

	/* Define the display resolution */
	#define BUFFER_LINES 2  // Only buffer 10 lines at a time

	#define BYTES_PER_PIXEL (LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB565))

	/* Create a display buffer */
	static uint16_t buf1[MY_DISP_HOR_RES * MY_DISP_VER_RES / 10 * BYTES_PER_PIXEL];

    /* Assign buffer to LVGL */
    lv_display_set_buffers(display1, buf1, NULL, sizeof(buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);

    /* Set flush callback */
    lv_display_set_flush_cb(display1, my_flush_cb);

    /* Set tick callback (required for animations, delays, etc.) */
    lv_tick_set_cb(my_tick_get_cb); 

	lv_example_tabview_1();
}