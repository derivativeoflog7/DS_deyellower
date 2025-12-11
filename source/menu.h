#ifndef DS_DEYELLOWER_MENU_H
#define DS_DEYELLOWER_MENU_H

#include "common.h"
#include <stdio.h>
#include <nds.h>

void print_backlight_level (
    int backlight_level,
    ConsoleType console_type
);

void print_top_screen (
    PrintConsole* top_screen_console,
    int screen_on_length,
    int screen_off_length,
    int repetition_count,
    int backlight_level,
    Screens screens,
    Mode mode,
    ConsoleType console_type
);

void print_settings_menu (
    PrintConsole* bottom_screen_console,
    int pos
);

void print_screens_menu (
    PrintConsole* bottom_screen_console,
    int pos
);

void print_modes_menu (
    PrintConsole* bottom_screen_console,
    int pos
);

void print_backlight_level_menu (
    PrintConsole* bottom_screen_console,
    int level,
    ConsoleType console_type
);

void print_number_input (
    PrintConsole* bottom_screen_console,
    int pos,
    const int* number_input_buffer
);

int setting_entries_count();
int screen_entries_count();
int mode_entries_count();
Status get_settings_target(int pos);
Screens get_screen_target(int pos);
Mode get_mode_target(int pos);

#endif
