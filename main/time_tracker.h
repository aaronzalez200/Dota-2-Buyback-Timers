#ifndef TIME_TRACKER_H
#define TIME_TRACKER_H

#include <stdint.h>
#include <stdbool.h>

#define HERO_COUNT 5
#define HERO_START_MIN 8
#define HERO_START_SEC 0

typedef struct {
    uint8_t minutes;
    uint8_t seconds;
    bool active;
} HeroTimer;

extern HeroTimer hero_timers[HERO_COUNT];

extern volatile uint32_t game_timer_minutes;
extern volatile uint8_t game_timer_seconds;

extern volatile uint8_t game_timer_active;  // game-timer counts up if set to 1, stops if set to 0 (paused)
                                            // game-timer should only be modifiable if a timer is created/present
extern volatile uint8_t all_timers_active;  // set to 1 if a timer has been created, 0 if timer is deleted
extern volatile uint8_t start_up_buybacks;  // not used yet....
extern volatile int user_data;
extern uint8_t something_happened;

extern uint8_t indexing;

#endif // TIME_TRACKER_H