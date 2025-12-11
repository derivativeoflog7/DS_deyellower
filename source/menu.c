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

typedef struct {
    const Mode target;
    const char *text;
} ModeEntry;

static const SettingsEntry SETTING_ENTRIES[] = {
    {.target = SELECT_SCREENS_MENU, .text = "Select screens"},
    {.target = SCREEN_ON_LENGTH_MENU, .text = "Screen on length"},
    {.target = SCREEN_OFF_LENGTH_MENU, .text = "Screen off length"},
    {.target = REPETITION_COUNT_MENU, .text = "Repetition count"},
    {.target = MODE_MENU, .text = "Mode"},
    {.target = BACKLIGHT_LEVEL_MENU, .text = "Backlight level"}
};

static const ScreenEntry SCREEN_ENTRIES[] = {
    {.target = BOTH, .text = "Both screens"},
    {.target = TOP, .text = "Top screen"},
    {.target = BOTTOM, .text = "Bottom screen"}
};

static const ModeEntry MODE_ENTRIES[] = {
    {.target = WHITE_SCREEN, .text = "White screen"},
    {.target = CYCLING, .text = "Cycling colors"}
};

// Print backlight level and text next to it for max/off
void print_backlight_level (
    int backlight_level,
    ConsoleType console_type
) {
    printf("%d ", backlight_level);
    if (backlight_level == 0)
        printf("(screen off)");
    else if (
        (console_type == DS_WITHOUT_BACKLIGHT_CONTROL && backlight_level == 1) ||
        (console_type == DS_WITH_BACKLIGHT_CONTROL && backlight_level == 4) ||
        (console_type == DSI && backlight_level == 5))
        printf("(max)");
}

void print_top_screen (
    PrintConsole* const top_screen_console,
    const Screens screens,
    const int screen_on_length,
    const int screen_off_length,
    const int repetition_count,
    const int backlight_level,
    const Mode mode,
    const ConsoleType console_type
) {
    consoleSelect(top_screen_console);
    consoleClear();
    printf("DS_deyellower v2.0\n");
    printf("by derivativeoflog7\n");
    printf("\n");
    printf("Detected console type:\n");
    printf(
        "%s\n",
        console_type == DSI ?
        "DSi\n" :
        console_type == DS_WITH_BACKLIGHT_CONTROL ?
        "DS (Lite) with backlight control" : // No \n as it fits a row perfectly
        "DS without backlight control\n"
    );
    printf("Current settings: \n");
    printf("Top screen: %s\n", screens == BOTTOM ? "NO" : "YES");
    printf("Bottom screen: %s\n", screens == TOP ? "NO" : "YES");
    printf("Screen on length: %umin\n", screen_on_length);
    printf("Screen off length: %umin\n", screen_off_length);
    printf("Repetition count: %u\n", repetition_count);
    printf("Mode: %s\n", mode == WHITE_SCREEN ? "white screen" : "cycling colors");
    printf("Backlight level: ");
    print_backlight_level(backlight_level, console_type);
    printf("\n");
}

void print_submenu_bottom_text(
    PrintConsole* const bottom_screen_console,
    const int print_A
) {
    consoleSetCursor(bottom_screen_console, 0, (*bottom_screen_console).consoleHeight - 1);
    printf("Press B to go back");
    if (print_A) {
        consoleSetCursor(bottom_screen_console, 0, (*bottom_screen_console).consoleHeight - 2);
        printf("Press A to confirm");
    }
}

void print_list_line (
    PrintConsole* const bottom_screen_console,
    const int selected,
    const char* const text
) {
    if (selected)
        printf("-> ");
    else {
        consoleSetColor(bottom_screen_console, CONSOLE_LIGHT_GRAY);
        printf("   ");
    }
    printf("%s\n", text);

    // Make sure color is reset to white at the end
    consoleSetColor(bottom_screen_console, CONSOLE_WHITE);
}

void print_settings_menu (
    PrintConsole* const bottom_screen_console,
    const int pos
) {
    for (int i = 0; i < ARRAY_LENGTH(SETTING_ENTRIES); i++) {
        print_list_line(bottom_screen_console, i == pos, SETTING_ENTRIES[i].text);
    }
    consoleSetCursor(bottom_screen_console, 0, (*bottom_screen_console).consoleHeight - 2);
    printf("Press A to enter");
    print_submenu_bottom_text(bottom_screen_console, 0);
}

void print_screens_menu (
    PrintConsole* const bottom_screen_console,
    const int pos
) {
    for (int i = 0; i < ARRAY_LENGTH(SCREEN_ENTRIES); i++) {
        print_list_line(bottom_screen_console, i == pos, SCREEN_ENTRIES[i].text);
    }
    print_submenu_bottom_text(bottom_screen_console, 1);
}

void print_modes_menu (
    PrintConsole* const bottom_screen_console,
    const int pos
) {
    for (int i = 0; i < ARRAY_LENGTH(MODE_ENTRIES); i++) {
        print_list_line(bottom_screen_console, i == pos, MODE_ENTRIES[i].text);
    }
    print_submenu_bottom_text(bottom_screen_console, 1);
}

void print_number_input (
    PrintConsole* const bottom_screen_console,
    const int pos,
    const int* const number_input_buffer
) {
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

void print_backlight_level_menu (
    PrintConsole* const bottom_screen_console,
    const int level,
    const ConsoleType console_type
) {
    printf("Backlight level:\n");
    print_backlight_level(level, console_type);
    printf("\n\n");
    printf("This will not take effect\n");
    printf("until the process is started\n\n");
    consoleSetColor(bottom_screen_console, CONSOLE_LIGHT_YELLOW);
    printf("Even if the console type has\n");
    printf("been misdetected, setting the\n");
    printf("level to max will work as\n");
    printf("expected on all models\n\n");
    consoleSetColor(bottom_screen_console, CONSOLE_LIGHT_RED);
    printf("This is for experimenting only,\n");
    printf("the current consensus is that\n");
    printf("max brightness works best for\n");
    printf("the process");
    consoleSetColor(bottom_screen_console, CONSOLE_WHITE);
    consoleSetCursor(bottom_screen_console, 0, bottom_screen_console->consoleHeight - 3);
    printf("Use dpad UP/DOWN to incr/decr");
    print_submenu_bottom_text(bottom_screen_console, 1);
}

Status get_settings_target(const int pos) {
    assert(pos >= 0);
    assert(pos < ARRAY_LENGTH(SETTING_ENTRIES));
    return SETTING_ENTRIES[pos].target;
}

Screens get_screen_target(const int pos) {
    assert(pos >= 0);
    assert(pos < ARRAY_LENGTH(SCREEN_ENTRIES));
    return SCREEN_ENTRIES[pos].target;
}

Mode get_mode_target(const int pos) {
    assert(pos >= 0);
    assert(pos < ARRAY_LENGTH(MODE_ENTRIES));
    return MODE_ENTRIES[pos].target;
}

int setting_entries_count() {
    return ARRAY_LENGTH(SETTING_ENTRIES);
}

int screen_entries_count() {
    return ARRAY_LENGTH(SCREEN_ENTRIES);
}

int mode_entries_count() {
    return ARRAY_LENGTH(MODE_ENTRIES);
}