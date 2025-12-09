#include "menu.h"
#include "common.h"

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
    printf("Top screen: %s\n", screens == BOTTOM ? "OFF" : "ON");
    printf("Bottom screen: %s\n", screens == TOP ? "OFF" : "ON");
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
    consoleSetColor(bottom_screen_console, CONSOLE_WHITE);
    consoleSetCursor(bottom_screen_console, 0, (*bottom_screen_console).consoleHeight - 1);
    printf("Press B to go back");
}

int setting_entries_count() {
    return ARRAY_LENGTH(SETTING_ENTRIES);
}
