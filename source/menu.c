#include "menu.h"

const SettingsEntry SETTING_ENTRIESS[] = {
    {.target = SELECT_SCREENS_MENU, .text = "Select screens"},
    {.target = SCREEN_ON_LENGTH_MENU, .text = "Screen on length"},
    {.target = SCREEN_ON_LENGTH_MENU, .text = "Screen off length"},
    {.target = REPETITION_COUNT_MENU, .text = "Repetition count"},
    {.target = MODE_MENU, .text = "Mode"},
    {.target = BRIGHTNESS_MENU, .text = "Brightness"}
};

void print_top_screen(
    PrintConsole *top_screen_console,
    Screens screens,
    unsigned int screen_on_length,
    unsigned int screen_off_length,
    unsigned int repetition_count,
    Mode mode
) {
    consoleSelect(top_screen_console);
    consoleClear();
    printf("DS_deyellower v2.0\n");
    printf("by derivativeoflog7\n");
    printf("\n");
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
    PrintConsole *bottom_screen_console,
    int pos
) {
    consoleSelect(bottom_screen_console);
    for (int i = 0; i < 6; i++) {
        if (i == pos) {
            consoleSetColor(bottom_screen_console, CONSOLE_WHITE);
            printf("-> ");
        } else {
            consoleSetColor(bottom_screen_console, CONSOLE_LIGHT_GRAY);
            printf("   ");
        }
        printf("%s\n", SETTING_ENTRIESS[i].text);
    }
}
