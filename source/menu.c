#include "menu.h"
#include "common.h"

typedef struct {
    const Status target;
    const char *text;
} SettingsEntry;

typedef struct {
    const Screens target;
    const char *text;
} ScreenEntry;

static const SettingsEntry SETTING_ENTRIES[] = {
    {.target = SELECT_SCREENS_MENU, .text = "Select screens"},
    {.target = SCREEN_ON_LENGTH_MENU, .text = "Screen on length"},
    {.target = SCREEN_OFF_LENGTH_MENU, .text = "Screen off length"},
    {.target = REPETITION_COUNT_MENU, .text = "Repetition count"},
    {.target = MODE_MENU, .text = "Mode"},
    {.target = BRIGHTNESS_MENU, .text = "Brightness"}
};

static const ScreenEntry SCREEN_ENTRIES[] = {
    {.target = BOTH, .text = "Both screens"},
    {.target = TOP, .text = "Top screen"},
    {.target = BOTTOM, .text = "Bottom screen"}
};

void print_top_screen(
    PrintConsole* const top_screen_console,
    const Screens screens,
    const unsigned int screen_on_length,
    const unsigned int screen_off_length,
    const unsigned int repetition_count,
    const Mode mode
) {
    consoleSelect(top_screen_console);
    consoleClear();
    printf("DS_deyellower v2.0\n");
    printf("by derivativeoflog7\n");
    printf("\n");
    /*printf("Detected console type:\n");
    printf(
        "%s\n",
        console_type == DSI ?
        "DSi" :
        console_type == DS_WITH_BACKLIGHT_CONTROL ?
        "DS (Lite) with backlight control" :
        "DS without backlight control"
    );
    printf("\n");*/
    printf("Current settings: \n");
    printf("Top screen: %s\n", screens == BOTTOM ? "NO" : "YES");
    printf("Bottom screen: %s\n", screens == TOP ? "NO" : "YES");
    printf("Screen on length: %umin\n", screen_on_length);
    printf("Screen off length: %umin\n", screen_off_length);
    printf("Repetition count: %u\n", repetition_count);
    printf("Mode: ");
    switch (mode) {
        case WHITE_SCREEN:
            printf("white screen");
            break;
        case CYCLING:
            printf("cycling colors");
            break;
    }
    printf("\n");
}

void print_submenu_bottom_text(
    PrintConsole* const bottom_screen_console,
    const int print_A
) {
    consoleSelect(bottom_screen_console);
    consoleSetColor(bottom_screen_console, CONSOLE_WHITE);
    consoleSetCursor(bottom_screen_console, 0, (*bottom_screen_console).consoleHeight - 1);
    printf("Press B to go back");
    if (print_A) {
        consoleSetCursor(bottom_screen_console, 0, (*bottom_screen_console).consoleHeight - 2);
        printf("Press A to confirm");
    }
}

void print_settings_menu (
    PrintConsole* const bottom_screen_console,
    const int pos
) {
    consoleSelect(bottom_screen_console);
    for (int i = 0; i < ARRAY_LENGTH(SETTING_ENTRIES); i++) {
        if (i == pos) {
            consoleSetColor(bottom_screen_console, CONSOLE_WHITE);
            printf("-> ");
        } else {
            consoleSetColor(bottom_screen_console, CONSOLE_LIGHT_GRAY);
            printf("   ");
        }
        printf("%s\n", SETTING_ENTRIES[i].text);
    }
    print_submenu_bottom_text(bottom_screen_console, 0);
}

void print_screens_menu (
    PrintConsole* const bottom_screen_console,
    const int pos
) {
    consoleSelect(bottom_screen_console);
    for (int i = 0; i < ARRAY_LENGTH(SCREEN_ENTRIES); i++) {
        if (i == pos) {
            consoleSetColor(bottom_screen_console, CONSOLE_WHITE);
            printf("-> ");
        } else {
            consoleSetColor(bottom_screen_console, CONSOLE_LIGHT_GRAY);
            printf("   ");
        }
        printf("%s\n", SCREEN_ENTRIES[i].text);
    }
    print_submenu_bottom_text(bottom_screen_console, 1);
}

void print_number_input (
    PrintConsole* const bottom_screen_console,
    const int pos,
    const int* const number_input_buffer
) {
    consoleSelect(bottom_screen_console);
    for (int i = 0; i < NUMBER_INPUT_LENGTH; i++) {
        printf("%d", number_input_buffer[i]);
        if (pos == i) {
            consoleAddToCursor(bottom_screen_console, -1, 1);
            printf("^");
            consoleAddToCursor(bottom_screen_console, 0, -1);
        }
    }

    consoleSetCursor(bottom_screen_console, 0, (*bottom_screen_console).consoleHeight - 4);
    printf("Use dpad LEFT/RIGHT to move\n");
    printf("Use dpad UP/DOWN to incr/decr");
    print_submenu_bottom_text(bottom_screen_console, 1);
}

Status get_settings_target(int pos) {
    assert(pos >= 0);
    assert(pos < ARRAY_LENGTH(SETTING_ENTRIES));
    return SETTING_ENTRIES[pos].target;
}

Screens get_screen_target(int pos) {
    assert (pos >= 0);
    assert(pos < ARRAY_LENGTH(SCREEN_ENTRIES));
    return SCREEN_ENTRIES[pos].target;
}

int setting_entries_count() {
    return ARRAY_LENGTH(SETTING_ENTRIES);
}

int screen_entries_count() {
    return ARRAY_LENGTH(SCREEN_ENTRIES);
}
