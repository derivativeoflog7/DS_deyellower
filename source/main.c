#include <stdio.h>
#include <nds.h>
#include "common.h"
#include "menu.h"


int main(int argc, char **argv) {
    Status current_status = MAIN_MENU;
    u16 keys_held, keys_down; // for buttons
    int menu_position = 0, flag;
    int number_input_buffer[NUMBER_INPUT_LENGTH];

    const int SETTING_ENTRIES_COUNT = setting_entries_count();
    const int SCREEN_ENTRIES_COUNT = screen_entries_count();

    // Default settings
    unsigned int screen_on_length = DEFAULT_SCREEN_ON_LENGTH;
    unsigned int screen_off_length = DEFAULT_SCREEN_OFF_LENGTH;
    unsigned int repetition_count = DEFAULT_REPETITION_COUNT;
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


    while (1)
    {
        swiWaitForVBlank();

        // TODO maybe there's a better way to do this?
        flag = 0;
        if (current_status >= SELECT_SCREENS_MENU && current_status <= BRIGHTNESS_MENU)
            flag = 1;

        scanKeys();
        keys_down = keysDown();
        keys_held = keysHeld();


        if (keys_held & KEY_START)
            systemShutDown();

        if (current_status < RUNNING_SCREEN_ON) {
            print_top_screen(&top_screen_console, screens, screen_on_length, screen_off_length, repetition_count, mode);
        }

        consoleSelect(&bottom_screen_console);
        consoleClear();



        switch(current_status) {
            case MAIN_MENU:
                printf("Press A to begin\n");
                printf("Press X for settings\n");
                printf("Press B to power down\n");
                printf("Press START to power off\n\t(at any time)\n");
                if (keys_down & KEY_B)
                    systemShutDown();
                else if (keys_down & KEY_X) {
                    menu_position = 0;
                    current_status = SETTINGS_MENU;
                }
                else if (keys_down & KEY_A)
                    current_status = RUNNING_SCREEN_ON;
                break;
            case SETTINGS_MENU:
                print_settings_menu(&bottom_screen_console, menu_position);
                if (keys_down & KEY_B)
                    current_status = MAIN_MENU;
                else if (keys_down & KEY_A) {
                    current_status = get_settings_target(menu_position);
                    menu_position = 0;
                    if (current_status == SCREEN_ON_LENGTH_MENU)
                        int_to_buffer(screen_on_length, number_input_buffer);
                    else if (current_status == SCREEN_OFF_LENGTH_MENU)
                        int_to_buffer(screen_off_length, number_input_buffer);
                    else if (current_status == REPETITION_COUNT_MENU)
                        int_to_buffer(repetition_count, number_input_buffer);
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
        }

        if (current_status == SCREEN_ON_LENGTH_MENU || current_status == SCREEN_OFF_LENGTH_MENU || current_status == REPETITION_COUNT_MENU) {
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


        if (flag && keys_down & (KEY_B | KEY_A)) {
            menu_position = 0;
            current_status = SETTINGS_MENU;
        }

    }
}


