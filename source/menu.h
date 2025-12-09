#ifndef DS_DEYELLOWER_MENU_H
#define DS_DEYELLOWER_MENU_H

#include "common.h"
#include <stdio.h>
#include <nds.h>

typedef struct {
    const Status target;
    const char *text;
} SettingsEntry;

static const SettingsEntry SETTING_ENTRIES[] = {
    {.target = SELECT_SCREENS_MENU, .text = "Select screens"},
    {.target = SCREEN_ON_LENGTH_MENU, .text = "Screen on length"},
    {.target = SCREEN_ON_LENGTH_MENU, .text = "Screen off length"},
    {.target = REPETITION_COUNT_MENU, .text = "Repetition count"},
    {.target = MODE_MENU, .text = "Mode"},
    {.target = BRIGHTNESS_MENU, .text = "Brightness"}
};

void print_top_screen (
    PrintConsole *top_screen_console,
    Screens screens,
    unsigned int screen_on_length,
    unsigned int screen_off_length,
    unsigned int repetition_count,
    Mode mode
);

void print_settings_menu (
    PrintConsole *bottom_screen_console,
    int pos
);

#endif
