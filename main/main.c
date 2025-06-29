#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "keyboard.h"
#include "display.h"
#include "gpio_setup.h"
#include "lvgl.h"
#include "esp_heap_caps.h"

#include "time_tracker.h"
#include "display.h"

void key_scan_task(void *pvParameters) {
    while (1) {
        scan_keys();
        vTaskDelay(pdMS_TO_TICKS(50));  // Adjust scan rate as needed
    }
}

void lvgl_task(void *pvParameters) {
    while (1) {
        lv_task_handler();              // Handle LVGL events
        vTaskDelay(pdMS_TO_TICKS(10));   // ~200 Hz refresh
    }
}

void time_tracker_task(void *pvParameters) {
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));  // Delay *after* ticking
        if(all_timers_active && game_timer_active) {
            game_timer_seconds++;
            if (game_timer_seconds >= 60) {
                game_timer_seconds = 0;
                game_timer_minutes++;
            }
            // Update all active hero timers
            for (int i = 0; i < HERO_COUNT; i++) {
                if (hero_timers[i].active) {
                    if (hero_timers[i].seconds == 0) {
                        if (hero_timers[i].minutes == 0) {
                            hero_timers[i].active = false; // Timer done
                        } else {
                            hero_timers[i].minutes--;
                            hero_timers[i].seconds = 59;
                        }
                    } else {
                        hero_timers[i].seconds--;
                    }
                }
            }
        }
    }
}


void app_main(void) {
    init_keys();
    gpio_setup();  // Your own custom GPIO init, presumably for display

    // Create key scan task on core 0
    xTaskCreatePinnedToCore(key_scan_task, "key_scan_task", 2048, NULL, 5, NULL, 0);

    // Create LVGL task on core 1
    xTaskCreatePinnedToCore(lvgl_task, "lvgl_task", 8192, NULL, 6, NULL, 1);

    // Create time tracker task (on core 1, or change as needed)
    xTaskCreatePinnedToCore(time_tracker_task, "time_tracker_task", 2048, NULL, 4, NULL, 1);
}