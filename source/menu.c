#include "menu.h"

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
