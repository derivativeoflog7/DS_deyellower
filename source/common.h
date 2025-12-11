#ifndef DS_DEYELLOWER_COMMON_H
#define DS_DEYELLOWER_COMMON_H

typedef enum {
    MAIN_MENU,
    SETTINGS_MENU,
    SELECT_SCREENS_MENU,
    SCREEN_ON_LENGTH_MENU,
    SCREEN_OFF_LENGTH_MENU,
    REPETITION_COUNT_MENU,
    MODE_MENU,
    BACKLIGHT_LEVEL_MENU,
    RUNNING_SCREEN_ON,
    RUNNING_SCREEN_OFF
} Status;

typedef enum {
    BOTH,
    TOP,
    BOTTOM
} Screens;

typedef enum {
    WHITE_SCREEN,
    CYCLING
} Mode;

typedef enum {
    DS_WITHOUT_BACKLIGHT_CONTROL,
    DS_WITH_BACKLIGHT_CONTROL,
    DSI
} ConsoleType;

#define DEFAULT_SCREEN_ON_LENGTH 240
#define DEFAULT_SCREEN_OFF_LENGTH 10
#define DEFAULT_REPETITION_COUNT 12
#define DEFAULT_MODE WHITE_SCREEN
#define NUMBER_INPUT_LENGTH 5 // pretty sure 99999 is a good max value for everything in this program
#define ARRAY_LENGTH(x) (sizeof(x) / sizeof(x[0]))

void int_to_buffer(int val, int* const target);
int buffer_to_int(int* const buf);
ConsoleType detect_console_type();

#endif
