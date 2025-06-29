idf_component_register(SRCS "time_tracker.c" "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES keyboard gpio_setup display lvgl)
