/* GPIOs for keyboard [11 keys total]
 * 2x5 matrix + 1 standalone key
 * 
 * Row #1: D7 [GPIO 10]
 * Row #2: D8 [GPIO 17]
 * 
 * Column #1: D2 [GPIO 5]
 * Column #2: D3 [GPIO 6]
 * Column #3: D4 [GPIO 7]
 * Column #4: D5 [GPIO 8]
 * Column #5: D6 [GPIO 9]
 * 
 * Standalone Key: D9 [GPIO 18]
 * 
 */
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "lvgl.h"
#include "../../components/display/display.h"
#include "esp_rom_sys.h"

#include "../../main/time_tracker.h"

#define TAG "KEYSCAN"

#define ROW1 GPIO_NUM_10  // D7
#define ROW2 GPIO_NUM_17  // D8

#define COL1 GPIO_NUM_5   // D2
#define COL2 GPIO_NUM_6   // D3
#define COL3 GPIO_NUM_7   // D4
#define COL4 GPIO_NUM_8   // D5
#define COL5 GPIO_NUM_9   // D6

#define STANDALONE_KEY GPIO_NUM_18  // D9

#define DEBOUNCE_TIME_MS 100

static int64_t last_key_press_time[2][5] = {0};

static bool key_states[2][5] = { false };
static bool standalone_state = false;

static const gpio_num_t row_pins[2] = {ROW1, ROW2};
static const gpio_num_t col_pins[5] = {COL1, COL2, COL3, COL4, COL5};

static const char *key_labels[2][5] = {
    {"K1", "K2", "K3", "K4", "K5"},
    {"K6", "K7", "K8", "K9", "K10"}
};

static const char *standalone_label = "K11";

void init_keys(void)
{
    // Initialize row pins as input with internal pulldown
    for (int i = 0; i < 2; i++) {
        gpio_reset_pin(row_pins[i]);
        gpio_set_direction(row_pins[i], GPIO_MODE_INPUT);
        gpio_pulldown_en(row_pins[i]);
        gpio_pullup_dis(row_pins[i]);
    }

    // Initialize column pins as input (Hi-Z)
    for (int i = 0; i < 5; i++) {
        gpio_reset_pin(col_pins[i]);
        gpio_set_direction(col_pins[i], GPIO_MODE_INPUT);
        gpio_pullup_dis(col_pins[i]);
        gpio_pulldown_dis(col_pins[i]);
    }

    // Standalone key as input with pull-up
    gpio_reset_pin(STANDALONE_KEY);
    gpio_set_direction(STANDALONE_KEY, GPIO_MODE_INPUT);
    gpio_pullup_en(STANDALONE_KEY);
}

static gpio_mode_t col_pin_modes[5] = {
    GPIO_MODE_INPUT, GPIO_MODE_INPUT, GPIO_MODE_INPUT, GPIO_MODE_INPUT, GPIO_MODE_INPUT
};

void set_col_pin_mode(int col, gpio_mode_t mode) {
    if (col_pin_modes[col] != mode) {
        gpio_set_direction(col_pins[col], mode);
        col_pin_modes[col] = mode;
    }
}

void switch_tab_cb(void *arg) {
    lv_tabview_set_act(tabview, (uint8_t)(uintptr_t)arg, LV_ANIM_OFF);
}

void start_hero_timer(int index) {
    hero_timers[index].minutes = HERO_START_MIN;
    hero_timers[index].seconds = HERO_START_SEC;
    hero_timers[index].active = true;
}

void end_hero_timer(int index) {
    hero_timers[index].minutes = 0;
    hero_timers[index].seconds = 0;
    hero_timers[index].active = false;
}

void process_key(uint8_t row, uint8_t col) {        
    /*
    {"K1", "K2", "K3", "K4", "K5"},
    {"K6", "K7", "K8", "K9", "K10"}
                "k11" - standalone key
    */

    // K1 - Hero #1 timer 
    if (row == 0 && col == 0) {
        if (!hero_timers[0].active && all_timers_active) {
            start_hero_timer(0);
        } else {
            end_hero_timer(0);
        }  
    }
    // K2 - Hero #2 timer 
    if (row == 0 && col == 1) {
        if (!hero_timers[1].active && all_timers_active) {
            start_hero_timer(1);
        } else {
            end_hero_timer(1);
        }  
    }
    // K3 - Hero #3 timer 
    if (row == 0 && col == 2) {
        if (!hero_timers[2].active && all_timers_active) {
            start_hero_timer(2);
        } else {
            end_hero_timer(2);
        }  
    }
    // K4 - Hero #4 timer 
    if (row == 0 && col == 3) {
        if (!hero_timers[3].active && all_timers_active) {
            start_hero_timer(3);
        } else {
            end_hero_timer(3);
        }  
    }
    // K5 - Hero #5 timer 
    if (row == 0 && col == 4) {
        if (!hero_timers[4].active && all_timers_active) {
            start_hero_timer(4);
        } else {
            end_hero_timer(4);
        }  
    }
    // K6 - Remove/Close in-game timer
    if (row == 1 && col == 0) {
        all_timers_active = 0;
        game_timer_active = 0;
        game_timer_minutes = 0;
        game_timer_seconds = 0;
    }
    // K7 - Decrease in-game timer (if active)
    if (row == 1 && col == 1) {
        if (all_timers_active && !game_timer_active) {
            if (game_timer_minutes > 0 || game_timer_seconds > 0) {
                if (game_timer_seconds == 0) {
                    game_timer_minutes--;
                    game_timer_seconds = 59;
                } else {
                    game_timer_seconds--;
                }
            }
            // Increase all active hero timers (but cap at 8:00)
            for (int i = 0; i < HERO_COUNT; i++) {
                if (hero_timers[i].active) {
                    uint16_t total_secs = hero_timers[i].minutes * 60 + hero_timers[i].seconds;
                    if (total_secs < 8 * 60) {
                        hero_timers[i].seconds++;
                        if (hero_timers[i].seconds >= 60) {
                            hero_timers[i].seconds = 0;
                            hero_timers[i].minutes++;
                        }
                    }
                }
            }
        }
    }
    // K8 - Pause/Resume in-game timer (if active)
    if (row == 1 && col == 2) {
        if (all_timers_active) {
            game_timer_active = !game_timer_active;
        }
    }
    // K9 - Increase in-game timer (if active)
    if (row == 1 && col == 3) {
        if (all_timers_active && !game_timer_active) {
            game_timer_seconds++;
            if (game_timer_seconds >= 60) {
                game_timer_seconds = 0;
                game_timer_minutes++;
            }
            // Decrease all active hero timers
            for (int i = 0; i < HERO_COUNT; i++) {
                if (hero_timers[i].active) {
                    if (hero_timers[i].seconds == 0) {
                        if (hero_timers[i].minutes == 0) {
                            hero_timers[i].active = false; // timer expires
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
    // K10 - Starts in-game timer (if in-active) 
    if (row == 1 && col == 4) {
        if (!all_timers_active) {
            all_timers_active = 1;
            game_timer_minutes = 0;
            game_timer_seconds = 0;
            game_timer_active = 1;
        }
    }
}

void scan_keys(void)
{
    int64_t now = esp_timer_get_time() / 1000; // Microseconds -> milliseconds

    for (int col = 0; col < 5; col++) {
        gpio_set_direction(col_pins[col], GPIO_MODE_OUTPUT);
        gpio_set_level(col_pins[col], 1);
        vTaskDelay(1 / portTICK_PERIOD_MS);

        for (int row = 0; row < 2; row++) {
            int level = gpio_get_level(row_pins[row]);
            bool is_pressed = (level == 1);
            if (is_pressed && !key_states[row][col]) {
                // Check debounce
                if (now - last_key_press_time[row][col] > DEBOUNCE_TIME_MS) {
                    last_key_press_time[row][col] = now;
                    key_states[row][col] = true;
                    process_key(row, col);
                }
            } else if (!is_pressed && key_states[row][col]) {
                key_states[row][col] = false;
            }
        }
        gpio_set_direction(col_pins[col], GPIO_MODE_INPUT);
    }

    // Debounce for standalone key
    static int64_t last_standalone_time = 0;
    int standalone_level = gpio_get_level(STANDALONE_KEY);
    bool standalone_pressed = (standalone_level == 0);
    if (standalone_pressed && !standalone_state) {
        if (now - last_standalone_time > DEBOUNCE_TIME_MS) {
            last_standalone_time = now;
            standalone_state = true;
            indexing = (indexing + 1) % 3;
            //lv_tabview_set_act(tabview, indexing, LV_ANIM_OFF);
        }
    } else if (!standalone_pressed && standalone_state) {
        standalone_state = false;
    }
}