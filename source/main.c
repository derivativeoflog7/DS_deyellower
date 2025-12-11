#include <stdio.h>
#include <nds.h>
#include "common.h"
#include "menu.h"
#include "process.h"

#define NOCASH_BUFFER_SIZE 50

int remaining_seconds;

void backlight_debug(
    const int current_backlight_top,
    const int current_backlight_bottom,
    const int last_backlight_top,
    const int last_backlight_bottom,
    char buf[],
    const size_t size
) {
    snprintf(
        buf, size, "Last top %d bot %d; curr top %d bot %d",
        last_backlight_top, last_backlight_bottom, current_backlight_top, current_backlight_bottom
    );
    nocashMessage(buf);
}

void time_handler() {
    remaining_seconds--;
}

int main(int argc, char **argv) {

    // Working variables
    char nocash_buffer[NOCASH_BUFFER_SIZE];
    Status current_status = MAIN_MENU;
    u16 keys_held, keys_down; // for buttons
    int submenu_position = 0, settings_menu_position = 0, in_setting_submenu;
    int number_input_buffer[NUMBER_INPUT_LENGTH];
    int backlight_level_buffer;
    int remaining_repetitions = 0;
    int current_backlight_top, current_backlight_bottom, last_backlight_top, last_backlight_bottom;

    // Constants
    const int SETTING_ENTRIES_COUNT = setting_entries_count();
    const int SCREEN_ENTRIES_COUNT = screen_entries_count();
    const int MODE_ENTRIES_COUNT = mode_entries_count();
    const ConsoleType CONSOLE_TYPE = detect_console_type();
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

        in_setting_submenu = 0;
        if (current_status >= SELECT_SCREENS_MENU && current_status <= BACKLIGHT_LEVEL_MENU)
            in_setting_submenu = 1;

        scanKeys();
        keys_down = keysDown();
        keys_held = keysHeld();

        // Don't shutdown if select is held so that you can set backlight level on DSi (mostly for testing)
        if (!(keys_held & KEY_SELECT) && (keys_down & KEY_START))
            systemShutDown();

        // Print info on top screen if process is not running
        if (current_status < RUNNING_SCREEN_ON) {
            print_top_screen(&top_screen_console, screens, screen_on_length_minutes, screen_off_length_minutes, repetition_count, backlight_level, mode, CONSOLE_TYPE);
        }

        consoleSelect(&bottom_screen_console);
        consoleClear();

        // Handle input and printing in various menus
        switch(current_status) {
            case MAIN_MENU:
                printf("Press A to begin\n");
                printf("Press X for settings\n");
                printf("Press START to power off\n\t(at any time)\n");
                if (keys_down & KEY_X) {
                    settings_menu_position = 0;
                    current_status = SETTINGS_MENU;
                }
                else if (keys_down & KEY_A) {
                    // Start the process
                    consoleSelect(&top_screen_console);
                    consoleClear();
                    init_screen_on_phase(mode, screen_off_length_minutes, &remaining_seconds, &time_handler);
                    disableSleep();
                    remaining_repetitions = repetition_count - 1;
                    current_status = RUNNING_SCREEN_ON;
                }
                break;

            // ===Setting menus with a list===
            case SETTINGS_MENU:
                print_settings_menu(&bottom_screen_console, settings_menu_position);
                if (keys_down & KEY_B)
                    current_status = MAIN_MENU;
                else if (keys_down & KEY_A) {
                    // Handle entering a settings submenu

                    current_status = get_settings_target(settings_menu_position);
                    submenu_position = 0;
                    // Copy corresponding value to number input buffer
                    if (current_status == SCREEN_ON_LENGTH_MENU)
                        int_to_buffer(screen_on_length_minutes, number_input_buffer);
                    else if (current_status == SCREEN_OFF_LENGTH_MENU)
                        int_to_buffer(screen_off_length_minutes, number_input_buffer);
                    else if (current_status == REPETITION_COUNT_MENU)
                        int_to_buffer(repetition_count, number_input_buffer);
                    else if (current_status == BACKLIGHT_LEVEL_MENU)
                        backlight_level_buffer = backlight_level;
                }
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
            // Number input is printed further down below
            case SCREEN_ON_LENGTH_MENU:
                printf("Screen on length (minutes):\n\n");
                if (keys_down & KEY_A)
                    screen_on_length_minutes = buffer_to_int(number_input_buffer);
                break;

            case SCREEN_OFF_LENGTH_MENU:
                printf("Screen off length (minutes):\n\n");
                if (keys_down & KEY_A)
                    screen_off_length_minutes = buffer_to_int(number_input_buffer);
                break;

            case REPETITION_COUNT_MENU:
                printf("Repetition count:\n\n");
                if (keys_down & KEY_A)
                    repetition_count = buffer_to_int(number_input_buffer);
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
                // TODO
                setBackdropBoth(WHITE);
                if (remaining_seconds <= 0) {
                    if (remaining_repetitions <= 0)
                        systemShutDown();
                    init_screen_off_phase(screen_off_length_minutes, &remaining_seconds, &time_handler);
                    current_status = RUNNING_SCREEN_OFF;
                }
                break;
            case RUNNING_SCREEN_OFF:
                if (remaining_seconds <= 0) {
                    init_screen_on_phase(mode, screen_off_length_minutes, &remaining_seconds, &time_handler);
                    remaining_repetitions--;
                    current_status = RUNNING_SCREEN_ON;
                }
                break;
        }

        // Print number input and text at the bottom if needed
        if (
            in_setting_submenu && // Avoids having the numbers printed in the wrong place for a frame
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
        if (in_setting_submenu && keys_down & (KEY_B | KEY_A)) {
            submenu_position = 0;
            current_status = SETTINGS_MENU;
        }

        if (current_status == RUNNING_SCREEN_ON || current_status == RUNNING_SCREEN_OFF) {
            // Handle setting the backlights
            if (
                handleBacklight(
                    &current_backlight_top,
                    &current_backlight_bottom,
                    &last_backlight_top,
                    &last_backlight_bottom,
                    screens,
                    backlight_level,
                    MAX_BACKLIGHT_LEVEL,
                    CONSOLE_TYPE,
                    keys_held,
                    current_status == RUNNING_SCREEN_ON
                )
            )
                backlight_debug(
                    current_backlight_top,
                    current_backlight_bottom,
                    last_backlight_top,
                    last_backlight_bottom,
                    nocash_buffer,
                    NOCASH_BUFFER_SIZE
                );

            // FIXME
            consoleSelect(&top_screen_console);
            consoleClear();
            consoleSetColor(&top_screen_console, CONSOLE_LIGHT_MAGENTA);
            consoleSelect(&bottom_screen_console);
            consoleClear();
            consoleSetColor(&bottom_screen_console, CONSOLE_LIGHT_MAGENTA);

            if (keys_held & KEY_UP)
                setBackdropBoth(BLUE);
            else if (keys_held & KEY_RIGHT)
                setBackdropBoth(YELLOW);
            else if (keys_held & KEY_DOWN)
                setBackdropBoth(WHITE);
            else if (keys_held & KEY_LEFT)
                setBackdropBoth(BLACK);
            if (keys_held & KEY_X) {
                consoleSelect(&top_screen_console);
                print_progress_message(&top_screen_console, remaining_seconds, remaining_repetitions, current_status == RUNNING_SCREEN_ON);
                consoleSelect(&bottom_screen_console);
                print_progress_message(&top_screen_console, remaining_seconds, remaining_repetitions, current_status == RUNNING_SCREEN_ON);
            }

            //FIXME this is just for quick testing and will be removed
            if (keys_down & KEY_Y) {
                current_status = MAIN_MENU;
            }
        }
    }
}