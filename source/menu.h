#ifndef DS_DEYELLOWER_MENU_H
#define DS_DEYELLOWER_MENU_H

#include "common.h"
#include <stdio.h>
#include <nds.h>

void print_top_screen (
    PrintConsole* const top_screen_console,
    const Screens screens,
    const unsigned int screen_on_length,
    const unsigned int screen_off_length,
    const unsigned int repetition_count,
    const Mode mode
);

void print_settings_menu (
    PrintConsole* const bottom_screen_console,
    const int pos
);

int setting_entries_count();

#endif
