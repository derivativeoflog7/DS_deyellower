#include <stdio.h>
#include <nds.h>
#include "common.h"
#include "menu.h"
#include "process.h"

#define DEBUG_BUFFER_SIZE 50

int remaining_seconds;
int do_reprint_top_screen = 1, do_reprint_bottom_screen = 1, do_print_progress = 0;

void timer_handler() {
    remaining_seconds--;
    // Reprint if X is being held when a second elapses (also handles changing phases)
    if (do_print_progress) {
        do_reprint_bottom_screen = 1;
        do_reprint_top_screen = 1;
    }
}

int main(int argc, char **argv) {
    // Working variables
    CyclingPhase current_cycling_phase = BLACK_TO_BLUE;
    Status current_status = MAIN_MENU;
    u16 backdrop_color, keys_held, keys_down;
    int submenu_position = 0, settings_menu_position = 0, is_in_setting_submenu;
    int number_input_buffer[NUMBER_INPUT_LENGTH];
    int remaining_repetitions = 0; // init to avoid the compiler complaining about it
    int backlight_level_buffer = 0; // same as above
    int current_backlight_top, current_backlight_bottom, last_backlight_top, last_backlight_bottom;
    int conv_result;

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

        // Reprint screens if needed, all printing is done here
        if (do_reprint_top_screen) {
            print_top_screen(
                &top_screen_console,
                current_status,
                screen_on_length_minutes,
                screen_off_length_minutes,
                repetition_count,
                backlight_level,
                do_print_progress,
                remaining_seconds,
                remaining_repetitions,
                screens,
                mode,
                CONSOLE_TYPE
            );

            do_reprint_top_screen = 0;
        }
        if (do_reprint_bottom_screen) {
            print_bottom_screen(
                &bottom_screen_console,
                current_status,
                settings_menu_position,
                submenu_position,
                do_print_progress,
                remaining_seconds,
                remaining_repetitions,
                number_input_buffer,
                backlight_level_buffer,
                CONSOLE_TYPE
            );
            do_reprint_bottom_screen = 0;
        }

        // Set reprint of bottom screen if in settings menu or submenu, and A, B, up or down were pressed...
        if (current_status >= SETTINGS_MENU && current_status <= BACKLIGHT_LEVEL_MENU
            && keys_down & (KEY_B | KEY_A | KEY_UP | KEY_DOWN))
            do_reprint_bottom_screen = 1;
        // ...and additionally if left or right were pressed on a number input
        if (current_status >= SCREEN_ON_LENGTH_MENU && current_status <= REPETITION_COUNT_MENU
            && keys_down & (KEY_LEFT | KEY_RIGHT))
            do_reprint_bottom_screen = 1;
        // Set reprint of top screen if in settings submenu and A was pressed
        if (current_status >= SELECT_SCREENS_MENU && current_status <= BACKLIGHT_LEVEL_MENU
            && keys_down & KEY_A)
            do_reprint_top_screen = 1;


        // Handle input and printing in various menus
        switch(current_status) {
            case MAIN_MENU:
                /*
                 *All of these reprint the bottom screen,
                and all except entering settings also reprint the top
                */

                // Enter settings menu
                if (keys_down & KEY_X) {
                    do_reprint_bottom_screen = 1;
                    settings_menu_position = 0;
                    current_status = SETTINGS_MENU;
                }
                else if (keys_down & KEY_A) {
                    do_reprint_bottom_screen = 1;
                    do_reprint_top_screen = 1;
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
                else if (!(keys_held ^ TEST_MODE_COMBO)) {
                    do_reprint_bottom_screen = 1;
                    do_reprint_top_screen = 1;
                    current_status = TEST_MODE_WARNING;
                }
                break;

            // ===Setting menus with a list===
            case SETTINGS_MENU:
                if (keys_down & KEY_B) {
                    current_status = MAIN_MENU;
                }
                // Handle entering a settings submenu
                else if (keys_down & KEY_A) {
                    current_status = get_settings_target(settings_menu_position);
                    submenu_position = 0;
                    // Copy corresponding value to the input buffer if necessary
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
                if (keys_down & KEY_A)
                    screens = get_screen_target(submenu_position);
                else if (keys_down & KEY_UP && submenu_position > 0)
                    submenu_position--;
                else if (keys_down & KEY_DOWN && submenu_position < SCREEN_ENTRIES_COUNT - 1)
                    submenu_position++;
                break;

            case MODE_MENU:
                if (keys_down & KEY_A)
                    mode = get_mode_target(submenu_position);
                else if (keys_down & KEY_UP && submenu_position > 0)
                    submenu_position--;
                else if (keys_down & KEY_DOWN && submenu_position < MODE_ENTRIES_COUNT - 1)
                    submenu_position++;
                break;

            // ===Setting menus with numerical input===
            // Number input is handled further down below
            // TODO: handle input of 0 in screen on time or repetition count more gracefully
            case SCREEN_ON_LENGTH_MENU:
                if (keys_down & KEY_A) {
                    conv_result = buffer_to_int(number_input_buffer);
                    if (conv_result > 0)
                        screen_on_length_minutes = conv_result;
                }
                break;

            case SCREEN_OFF_LENGTH_MENU:
                if (keys_down & KEY_A)
                    screen_off_length_minutes = buffer_to_int(number_input_buffer);
                break;

            case REPETITION_COUNT_MENU:
                if (keys_down & KEY_A) {
                    conv_result = buffer_to_int(number_input_buffer);
                    if (conv_result > 0)
                        repetition_count = conv_result;
                }
                break;

            // ===Backlight level menu===
            case BACKLIGHT_LEVEL_MENU:
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
                // Note that chaning the backdrop does not require a reprint
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
                        screen_on_length_minutes,
                        &remaining_seconds,
                        &backdrop_color,
                        &current_cycling_phase
                    );
                    remaining_repetitions--;
                    current_status = RUNNING_SCREEN_ON;
                }
                break;

            // ===Test mode===
            case TEST_MODE_WARNING:
                if (keys_down & KEY_X) {
                    current_status = TEST_MODE;
                    // Console color is changed by print_top_screen/print_bottom_screen
                    do_reprint_bottom_screen = 1;
                    do_reprint_top_screen = 1;
                    setBackdropBoth(WHITE);
                }
                else if (keys_down) {
                    do_reprint_bottom_screen = 1;
                    do_reprint_top_screen = 1;
                    current_status = MAIN_MENU;
                }
                break;

            case TEST_MODE:
                // Reset colors and backlight and return to main menu
                if (keys_down & KEY_SELECT) {
                    setBackdropBoth(BLACK);
                    consoleSelect(&bottom_screen_console);
                    consoleSetColor(&bottom_screen_console, CONSOLE_WHITE);
                    consoleSelect(&top_screen_console);
                    consoleSetColor(&top_screen_console, CONSOLE_WHITE);
                    //FIXME? if consoles are not selected, colors aren't applied properly
                    do_reprint_bottom_screen = 1;
                    do_reprint_top_screen = 1;
                    systemSetBacklightLevel(MAX_BACKLIGHT_LEVEL);
                    powerOn(POWER_LCD);
                    current_status = MAIN_MENU;
                }
                else if (keys_held & KEY_Y) {
                    if (keys_down & KEY_UP)
                        powerOn(POWER_LCD);
                    else if (keys_down & KEY_DOWN)
                        powerOff(POWER_LCD);
                }
                else if (keys_held & KEY_L) {
                    if (keys_down & KEY_UP)
                        powerOn(PM_BACKLIGHT_TOP);
                    else if (keys_down & KEY_DOWN)
                        powerOff(PM_BACKLIGHT_TOP);
                }
                else if (keys_held & KEY_R) {
                    if (keys_down & KEY_UP)
                        powerOn(PM_BACKLIGHT_BOTTOM);
                    else if (keys_down & KEY_DOWN)
                        powerOff(PM_BACKLIGHT_BOTTOM);
                }
                else if (keys_held & KEY_X) {
                    if (keys_down & KEY_UP)
                        systemSetBacklightLevel(0);
                    else if (keys_down & KEY_RIGHT)
                        systemSetBacklightLevel(1);
                    else if (keys_down & KEY_DOWN)
                        systemSetBacklightLevel(2);
                    else if (keys_down & KEY_LEFT)
                        systemSetBacklightLevel(3);
                    else if (keys_down & KEY_A)
                        systemSetBacklightLevel(4);
                    else if (keys_down & KEY_B)
                        systemSetBacklightLevel(5);
                }
        }

        // Handle dpad for number input
        if (current_status >= SCREEN_ON_LENGTH_MENU && current_status <= REPETITION_COUNT_MENU) {
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

            if (keys_held & KEY_UP)
                setBackdropBoth(BLUE);
            else if (keys_held & KEY_RIGHT)
                setBackdropBoth(YELLOW);
            else if (keys_held & KEY_DOWN)
                setBackdropBoth(WHITE);
            else if (keys_held & KEY_LEFT)
                setBackdropBoth(BLACK);
            // Reprint if X was released or just pressed
            if (keys_held & KEY_X) {
                if (!do_print_progress) {
                    do_reprint_bottom_screen = 1;
                    do_reprint_top_screen = 1;
                }
                do_print_progress = 1;
            } else {
                if (do_print_progress) {
                    do_reprint_bottom_screen = 1;
                    do_reprint_top_screen = 1;
                }
                do_print_progress = 0;
            }
        }
    }
}