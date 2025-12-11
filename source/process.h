#ifndef DS_DEYELLOWER_PROCESS_H
#define DS_DEYELLOWER_PROCESS_H

#include "common.h"

#define WHITE  0b0111111111111111
#define BLUE   0b0111110000000000
#define YELLOW 0b0000001111111111
#define BLACK  0

#define BLUE_INCREMENT   0b0000010000000000
#define YELLOW_INCREMENT 0b0000000000100001
#define WHITE_INCREMENT (BLUE_INCREMENT | YELLOW_INCREMENT)

typedef enum {
    BLACK_TO_BLUE,
    BLUE_TO_YELLOW,
    YELLOW_TO_WHITE,
    WHITE_TO_BLACK
} CyclingPhase;

void init_screen_on_phase(
    const Mode mode,
    const int minutes,
    int* seconds,
    void* timer_handler
);

void init_screen_off_phase (
    const int minutes,
    int* seconds,
    void* timer_handler
);

int handleBacklight(
    int *current_backlight_top,
    int *current_backlight_bottom,
    int *last_backlight_top,
    int *last_backlight_bottom,
    const Screens screens,
    const int backlight_level,
    const int MAX_BACKLIGHT_LEVEL,
    const int CONSOLE_TYPE,
    const u16 keys_held,
    const int is_phase_screen_on
);

void print_progress_message(
    PrintConsole *console,
    int remaining_seconds,
    int remaining_repetitions,
    int is_screen_on_phase
);

void setBacklight(int backlight_level, ConsoleType console_type);
void setBackdropBoth(u16 col);

#endif