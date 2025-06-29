#include "time_tracker.h"

volatile uint32_t game_timer_minutes;
volatile uint8_t game_timer_seconds;
volatile uint8_t game_timer_active;

HeroTimer hero_timers[HERO_COUNT];

volatile uint8_t all_timers_active;
volatile uint8_t start_up_buybacks;

volatile int user_data;
uint8_t something_happened;

uint8_t indexing;