#include <stdio.h>
#include <nds.h>
#include "common.h"
#include "menu.h"


int main(int argc, char **argv) {
    Status current_status = MAIN_MENU;
    u16 keys_held, keys_down; // for buttons
    int menu_position = 0;

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


    print_top_screen(&top_screen_console, screens, screen_on_length, screen_off_length, repetition_count, mode);



    while (1)
    {
        swiWaitForVBlank();

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
                if (keys_down & KEY_B) {
                    consoleSetColor(&bottom_screen_console, CONSOLE_WHITE);
                    current_status = MAIN_MENU;
                }
                else if (keys_down & KEY_UP && menu_position > 0)
                    menu_position--;
                else if (keys_down & KEY_DOWN && menu_position < ARRAY_LENGTH(SETTING_ENTRIES) - 1)
                    menu_position++;
                break;
        }


    }

    return 0;
}


