#include <stdio.h>
#include <nds.h>
#include "common.h"
#include "menu.h"
#include "process.h"

#define DEBUG_BUFFER_SIZE 50

int remaining_seconds;
void timer_handler() {
    remaining_seconds--;
}

int main(int argc, char **argv) {
    // Working variables
    CyclingPhase current_cycling_phase = BLACK_TO_BLUE;
    Status current_status = MAIN_MENU;
    u16 backdrop_color, keys_held, keys_down;
    int submenu_position = 0, settings_menu_position = 0, is_in_setting_submenu;
    int number_input_buffer[NUMBER_INPUT_LENGTH];
    int backlight_level_buffer; // yes, I could reuse the above, but I don't want to deal with that
    int remaining_repetitions = 0; // init to avoid the compiler complaining about it
    int current_backlight_top, current_backlight_bottom, last_backlight_top, last_backlight_bottom;
    int temp;

    // Constants
    const ConsoleType CONSOLE_TYPE = detect_console_type();
    const int SETTING_ENTRIES_COUNT = setting_entries_count();
    const int SCREEN_ENTRIES_COUNT = screen_entries_count();
    const int MODE_ENTRIES_COUNT = mode_entries_count();
    const int MAX_BACKLIGHT_LEVEL = CONSOLE_TYPE == DSI ? 5 :
        CONSOLE_TYPE == DS_WITH_BACKLIGHT_CONTROL ? 4 : 1;

    // Settings loaded with their defaults
    int screen_on_length_minutes = DEFAULT_SCREEN_ON_LENGTH;
    int screen_off_length_minutes = DEFAULT_SCREEN_OFF_LENGTH;
    int repetition_count = DEFAULT_REPETITION_COUNT;
    int backlight_level;
    Mode mode = DEFAULT_MODE;
    Screens screens = BOTH;
    // Init backlight variables and set backlight
    current_backlight_top = current_backlight_bottom = last_backlight_top = last_backlight_bottom = backlight_level = MAX_BACKLIGHT_LEVEL;
    systemSetBacklightLevel(MAX_BACKLIGHT_LEVEL);

    // Init consoles
    PrintConsole top_screen_console;
    PrintConsole bottom_screen_console;
    videoSetMode(MODE_0_2D);
    videoSetModeSub(MODE_0_2D);
    vramSetBankA(VRAM_A_MAIN_BG);
    vramSetBankC(VRAM_C_SUB_BG);
    consoleInit(&top_screen_console, 3, BgType_Text4bpp, BgSize_T_256x256, 31, 0, true, true);
    consoleInit(&bottom_screen_console, 3, BgType_Text4bpp, BgSize_T_256x256, 31, 0, false, true);

    // Main loop
    while (1) {
        swiWaitForVBlank();
        scanKeys();
        keys_down = keysDown();
        keys_held = keysHeld();

        if (keys_down & KEY_START)
            systemShutDown();

        is_in_setting_submenu = 0;
        if (current_status >= SELECT_SCREENS_MENU && current_status <= BACKLIGHT_LEVEL_MENU)
            is_in_setting_submenu = 1;

        // Print info on top screen if process is not running
        if (current_status < RUNNING_SCREEN_ON) {
            print_top_screen(
                &top_screen_console,
                screen_on_length_minutes,
                screen_off_length_minutes,
                repetition_count,
                backlight_level,
                screens,
                mode,
                CONSOLE_TYPE
            );
        }

        // Clear bottom console
        consoleSelect(&bottom_screen_console);
        consoleClear();

        // Handle input and printing in various menus
        switch(current_status) {
            case MAIN_MENU:
                printf("Press A to begin\n");
                printf("Press X for settings\n");
                printf("Press START to power off\n\t(at any time)\n");
                // Enter settings menu
                if (keys_down & KEY_X) {
                    settings_menu_position = 0;
                    current_status = SETTINGS_MENU;
                }
                else if (keys_down & KEY_A) {
                    // Start the process
                    init_screen_on_phase(
                        mode,
                        screen_on_length_minutes,
                        &remaining_seconds,
                        &backdrop_color,
                        &current_cycling_phase
                    );
                    disableSleep();
                    remaining_repetitions = repetition_count - 1;
                    current_status = RUNNING_SCREEN_ON;
                    timerStart(0, ClockDivider_1024, timerFreqToTicks_1024(1), timer_handler);
                }
                break;

            // ===Setting menus with a list===
            case SETTINGS_MENU:
                print_settings_menu(&bottom_screen_console, settings_menu_position);
                if (keys_down & KEY_B)
                    current_status = MAIN_MENU;
                // Handle entering a settings submenu
                else if (keys_down & KEY_A) {
                    current_status = get_settings_target(settings_menu_position);
                    submenu_position = 0;
                    // Copy corresponding value to the input buffer
                    if (current_status == SCREEN_ON_LENGTH_MENU)
                        int_to_buffer(screen_on_length_minutes, number_input_buffer);
                    else if (current_status == SCREEN_OFF_LENGTH_MENU)
                        int_to_buffer(screen_off_length_minutes, number_input_buffer);
                    else if (current_status == REPETITION_COUNT_MENU)
                        int_to_buffer(repetition_count, number_input_buffer);
                    else if (current_status == BACKLIGHT_LEVEL_MENU)
                        backlight_level_buffer = backlight_level;
                }
                // Handle moving in the menu
                else if (keys_down & KEY_UP) {
                    if (settings_menu_position > 0)
                        settings_menu_position--;
                    else
                        settings_menu_position = SETTING_ENTRIES_COUNT - 1;
                }
                else if (keys_down & KEY_DOWN) {
                    if (settings_menu_position < SETTING_ENTRIES_COUNT - 1)
                        settings_menu_position++;
                    else
                        settings_menu_position = 0;
                }
                break;

            case SELECT_SCREENS_MENU:
                print_screens_menu(&bottom_screen_console, submenu_position);
                if (keys_down & KEY_A)
                    screens = get_screen_target(submenu_position);
                else if (keys_down & KEY_UP && submenu_position > 0)
                    submenu_position--;
                else if (keys_down & KEY_DOWN && submenu_position < SCREEN_ENTRIES_COUNT - 1)
                    submenu_position++;
                break;

            case MODE_MENU:
                print_modes_menu(&bottom_screen_console, submenu_position);
                if (keys_down & KEY_A)
                    mode = get_mode_target(submenu_position);
                else if (keys_down & KEY_UP && submenu_position > 0)
                    submenu_position--;
                else if (keys_down & KEY_DOWN && submenu_position < MODE_ENTRIES_COUNT - 1)
                    submenu_position++;
                break;

            // ===Setting menus with numerical input===
            // Number input is printed and handled further down below
            // TODO: handle input of 0 in screen on time or repetition count more gracefully
            case SCREEN_ON_LENGTH_MENU:
                printf("Screen on length (minutes):\n\n");
                if (keys_down & KEY_A) {
                    temp = buffer_to_int(number_input_buffer);
                    if (temp > 0)
                        screen_on_length_minutes = temp;
                }
                break;

            case SCREEN_OFF_LENGTH_MENU:
                printf("Screen off length (minutes):\n\n");
                if (keys_down & KEY_A)
                    screen_off_length_minutes = buffer_to_int(number_input_buffer);
                break;

            case REPETITION_COUNT_MENU:
                printf("Repetition count:\n\n");
                if (keys_down & KEY_A) {
                    temp = buffer_to_int(number_input_buffer);
                    if (temp > 0)
                        repetition_count = temp;
                }
                break;

            // ===Backlight level menu===
            case BACKLIGHT_LEVEL_MENU:
                print_backlight_level_menu(&bottom_screen_console, backlight_level_buffer, CONSOLE_TYPE);
                if (keys_down & KEY_A) {
                    assert(backlight_level >= 0);
                    assert(backlight_level <= MAX_BACKLIGHT_LEVEL);
                    backlight_level = backlight_level_buffer;
                }
                if (keys_down & KEY_UP && backlight_level_buffer < MAX_BACKLIGHT_LEVEL)
                    backlight_level_buffer++;
                else if (keys_down & KEY_DOWN && backlight_level_buffer > 0)
                    backlight_level_buffer--;
                break;

            // ===Running===
            case RUNNING_SCREEN_ON:
                setBackdropBoth(backdrop_color);
                // Cycle colors black→blue→yellow→white
                if (mode == CYCLING_COLORS) {
                    switch (current_cycling_phase) {
                        case BLACK_TO_BLUE:
                            backdrop_color += BLUE_INCREMENT;
                            if (backdrop_color == BLUE)
                                current_cycling_phase = BLUE_TO_YELLOW;
                            break;
                        case BLUE_TO_YELLOW:
                            backdrop_color += YELLOW_INCREMENT;
                            backdrop_color -= BLUE_INCREMENT;
                            if (backdrop_color == YELLOW)
                                current_cycling_phase = YELLOW_TO_WHITE;
                            break;
                        case YELLOW_TO_WHITE:
                            backdrop_color += BLUE_INCREMENT;
                            if (backdrop_color == WHITE)
                                current_cycling_phase = WHITE_TO_BLACK;
                            break;
                        case WHITE_TO_BLACK:
                            backdrop_color -= WHITE_INCREMENT;
                            if (backdrop_color == BLACK)
                                current_cycling_phase = BLACK_TO_BLUE;
                            break;
                    }
                }
                // Change phase or power off when the time has elapsed
                if (remaining_seconds <= 0) {
                    if (remaining_repetitions <= 0)
                        systemShutDown();
                    // If screen off length is greater than 0, switch to screen off phase...
                    if (screen_off_length_minutes > 0) {
                        init_screen_off_phase(screen_off_length_minutes, &remaining_seconds);
                        current_status = RUNNING_SCREEN_OFF;
                    }
                    // ...otherwise, just reset timer and decrease remaining repetitions
                    else {
                        remaining_repetitions--;
                        remaining_seconds = screen_on_length_minutes * 60;
                    }
                }
                break;
            case RUNNING_SCREEN_OFF:
                // Change phase when the time has elapsed, and decrease the number of remaining repetitions
                if (remaining_seconds <= 0) {
                    init_screen_on_phase(
                        mode,
                        screen_off_length_minutes,
                        &remaining_seconds,
                        &backdrop_color,
                        &current_cycling_phase
                    );
                    remaining_repetitions--;
                    current_status = RUNNING_SCREEN_ON;
                }
                break;
        }

        // Print number input and text at the bottom and handle dpad
        if (
            is_in_setting_submenu && // Avoids having the numbers printed in the wrong place for a frame
            (current_status == SCREEN_ON_LENGTH_MENU ||
            current_status == SCREEN_OFF_LENGTH_MENU ||
            current_status == REPETITION_COUNT_MENU)
        ) {
            print_number_input(&bottom_screen_console, submenu_position, number_input_buffer);
            if (keys_down & KEY_LEFT && submenu_position > 0)
                submenu_position--;
            else if (keys_down & KEY_RIGHT && submenu_position < NUMBER_INPUT_LENGTH - 1)
                submenu_position++;
            else if (keys_down & KEY_UP && number_input_buffer[submenu_position] < 9)
                number_input_buffer[submenu_position]++;
            else if (keys_down & KEY_DOWN && number_input_buffer[submenu_position] > 0)
                number_input_buffer[submenu_position]--;
        }

        // Handle leaving a settings submenu when pressing A or B
        if (is_in_setting_submenu && keys_down & (KEY_B | KEY_A)) {
            submenu_position = 0;
            current_status = SETTINGS_MENU;
        }

        // Handle setting the backlights and screens when the process is running
        if (current_status == RUNNING_SCREEN_ON || current_status == RUNNING_SCREEN_OFF) {
            handleBacklight(
                &current_backlight_top,
                &current_backlight_bottom,
                &last_backlight_top,
                &last_backlight_bottom,
                backlight_level,
                MAX_BACKLIGHT_LEVEL,
                CONSOLE_TYPE,
                current_status == RUNNING_SCREEN_ON,
                keys_held,
                screens
            );

            // Clear consoles
            consoleSelect(&top_screen_console);
            consoleClear();
            consoleSelect(&bottom_screen_console);
            consoleClear();

            if (keys_held & KEY_UP)
                setBackdropBoth(BLUE);
            else if (keys_held & KEY_RIGHT)
                setBackdropBoth(YELLOW);
            else if (keys_held & KEY_DOWN)
                setBackdropBoth(WHITE);
            else if (keys_held & KEY_LEFT)
                setBackdropBoth(BLACK);
            if (keys_held & KEY_X) {
                print_progress_message(
                    &top_screen_console,
                    &bottom_screen_console,
                    remaining_seconds,
                    remaining_repetitions,
                    current_status == RUNNING_SCREEN_ON
                );
            }

            // Print warning message if in screen off phase and the user has turned the screen on
            if (
                current_status == RUNNING_SCREEN_OFF &&
                keys_held & (KEY_UP | KEY_LEFT| KEY_DOWN | KEY_RIGHT | KEY_X)
            ) {
                print_warning_message(
                    &top_screen_console,
                    &bottom_screen_console
                );
            }
        }
    }
}
