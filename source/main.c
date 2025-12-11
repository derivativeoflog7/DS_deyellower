#include <stdio.h>
#include <nds.h>
#include "common.h"
#include "menu.h"


int main(int argc, char **argv) {
    // Working variables
    Status current_status = MAIN_MENU;
    u16 keys_held, keys_down; // for buttons
    int menu_position = 0, in_setting_submenu;
    int number_input_buffer[NUMBER_INPUT_LENGTH];
    int backlight_level_buffer;

    // Constants
    const int SETTING_ENTRIES_COUNT = setting_entries_count();
    const int SCREEN_ENTRIES_COUNT = screen_entries_count();
    const int MODE_ENTRIES_COUNT = mode_entries_count();
    const ConsoleType CONSOLE_TYPE = detect_console_type();
    const int MAX_BACKLIGHT_LEVEL = CONSOLE_TYPE == DSI ? 5 :
        CONSOLE_TYPE == DS_WITH_BACKLIGHT_CONTROL ? 4 : 1;

    // Settings loaded with their defaults
    int screen_on_length = DEFAULT_SCREEN_ON_LENGTH;
    int screen_off_length = DEFAULT_SCREEN_OFF_LENGTH;
    int repetition_count = DEFAULT_REPETITION_COUNT;
    int backlight_level = MAX_BACKLIGHT_LEVEL;
    Mode mode = DEFAULT_MODE;
    Screens screens = BOTH;

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

        if (keys_held & KEY_START)
            systemShutDown();

        // Print info on top screen if process is not running
        if (current_status < RUNNING_SCREEN_ON) {
            print_top_screen(&top_screen_console, screens, screen_on_length, screen_off_length, repetition_count, backlight_level, mode, CONSOLE_TYPE);
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
                    menu_position = 0;
                    current_status = SETTINGS_MENU;
                }
                else if (keys_down & KEY_A) {
                    // Start the process
                    // TODO incomplete
                    
                    // Turn off screen if needed
                    if (screens == TOP)
                        powerOff(PM_BACKLIGHT_BOTTOM);
                    else if (screens == BOTTOM)
                        powerOff(PM_BACKLIGHT_TOP);
                    disableSleep();
                    systemSetBacklightLevel(backlight_level);
                    current_status = RUNNING_SCREEN_ON;
                }
                break;

            // ===Setting menus with a list===
            case SETTINGS_MENU:
                print_settings_menu(&bottom_screen_console, menu_position);
                if (keys_down & KEY_B)
                    current_status = MAIN_MENU;
                else if (keys_down & KEY_A) {
                    // Handle entering a settings submenu

                    current_status = get_settings_target(menu_position);
                    menu_position = 0;
                    // Copy corresponding value to number input buffer
                    if (current_status == SCREEN_ON_LENGTH_MENU)
                        int_to_buffer(screen_on_length, number_input_buffer);
                    else if (current_status == SCREEN_OFF_LENGTH_MENU)
                        int_to_buffer(screen_off_length, number_input_buffer);
                    else if (current_status == REPETITION_COUNT_MENU)
                        int_to_buffer(repetition_count, number_input_buffer);
                    else if (current_status == BACKLIGHT_LEVEL_MENU)
                        backlight_level_buffer = backlight_level;
                }
                else if (keys_down & KEY_UP && menu_position > 0)
                    menu_position--;
                else if (keys_down & KEY_DOWN && menu_position < SETTING_ENTRIES_COUNT - 1)
                    menu_position++;
                break;

            case SELECT_SCREENS_MENU:
                print_screens_menu(&bottom_screen_console, menu_position);
                if (keys_down & KEY_A)
                    screens = get_screen_target(menu_position);
                else if (keys_down & KEY_UP && menu_position > 0)
                    menu_position--;
                else if (keys_down & KEY_DOWN && menu_position < SCREEN_ENTRIES_COUNT - 1)
                    menu_position++;
                break;

            case MODE_MENU:
                print_modes_menu(&bottom_screen_console, menu_position);
                if (keys_down & KEY_A)
                    mode = get_mode_target(menu_position);
                else if (keys_down & KEY_UP && menu_position > 0)
                    menu_position--;
                else if (keys_down & KEY_DOWN && menu_position < MODE_ENTRIES_COUNT - 1)
                    menu_position++;
                break;

            // ===Setting menus with numerical input===
            // Number input is printed further down below
            case SCREEN_ON_LENGTH_MENU:
                printf("Screen on length (minutes):\n\n");
                if (keys_down & KEY_A)
                    screen_on_length = buffer_to_int(number_input_buffer);
                break;

            case SCREEN_OFF_LENGTH_MENU:
                printf("Screen off length (minutes):\n\n");
                if (keys_down & KEY_A)
                    screen_off_length = buffer_to_int(number_input_buffer);
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

            case RUNNING_SCREEN_ON:
                // TODO this is just for testing
                setBackdropColor(0xFFFF);
                setBackdropColorSub(0xFFFF);
        }

        // Print number input and text at the bottom if needed
        if (
            in_setting_submenu && // Avoids having the numbers printed in the wrong place for a frame
            (current_status == SCREEN_ON_LENGTH_MENU ||
            current_status == SCREEN_OFF_LENGTH_MENU ||
            current_status == REPETITION_COUNT_MENU)
        ) {
            print_number_input(&bottom_screen_console, menu_position, number_input_buffer);
            if (keys_down & KEY_LEFT && menu_position > 0)
                menu_position--;
            else if (keys_down & KEY_RIGHT && menu_position < NUMBER_INPUT_LENGTH - 1)
                menu_position++;
            else if (keys_down & KEY_UP && number_input_buffer[menu_position] < 9)
                number_input_buffer[menu_position]++;
            else if (keys_down & KEY_DOWN && number_input_buffer[menu_position] > 0)
                number_input_buffer[menu_position]--;
        }

        // Handle leaving a settings submenu when pressing A or B
        if (in_setting_submenu && keys_down & (KEY_B | KEY_A)) {
            menu_position = 0;
            current_status = SETTINGS_MENU;
        }
    }
}