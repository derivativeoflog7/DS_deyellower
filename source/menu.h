#ifndef DS_DEYELLOWER_MENU_H
#define DS_DEYELLOWER_MENU_H

#include "common.h"
#include <stdio.h>
#include <nds.h>

void print_top_screen (
    PrintConsole* top_screen_console,
    Status current_status,
    int screen_on_length,
    int screen_off_length,
    int repetition_count,
    int backlight_level,
    int do_print_progress,
    int remaining_seconds,
    int remaining_repetitions,
    Screens screens,
    Mode mode,
    ConsoleType console_type
);

void print_bottom_screen(
    PrintConsole* bottom_screen_console,
    Status current_status,
    int settings_menu_position,
    int submenu_position,
    int do_print_progress,
    int remaining_seconds,
    int remaining_repetitions,
    int* number_input_buffer,
    int backlight_level_buffer,
    ConsoleType CONSOLE_TYPE
);

int setting_entries_count();
int screen_entries_count();
int mode_entries_count();
Status get_settings_target(int pos);
Screens get_screen_target(int pos);
Mode get_mode_target(int pos);

#endif
