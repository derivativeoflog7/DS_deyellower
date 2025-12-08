// SPDX-License-Identifier: CC0-1.0
//
// SPDX-FileContributor: Antonio Niño Díaz, 2024-2025

#include <nds.h>
// These are 5 bits per colour, with the MSB being unused(?)
#define BLUE 0b0111110000000000
#define YELLOW 0b0000001111111111
#define WHITE 0b0111111111111111
#define BLACK 0

typedef enum {
    BLACK_TO_BLUE,
    BLUE_TO_YELLOW,
    YELLOW_TO_WHITE,
    WHITE_TO_BLACK
} ephase;

int main(int argc, char *argv[])
{
    u16 held, down, bgcol = 0; // start from black screen
    ephase phase = BLACK_TO_BLUE;
    unsigned int delay = 0, count = 0;


    disableSleep();
    videoSetMode(MODE_0_2D);
    videoSetModeSub(MODE_0_2D);

    vramSetPrimaryBanks(VRAM_A_MAIN_BG, VRAM_B_LCD,
                        VRAM_C_SUB_BG, VRAM_D_LCD);

    // I don't even know what these modes do lol
    bgInit(0, BgType_Text8bpp, BgSize_T_256x256, 0,1);
    bgInitSub(0, BgType_Text8bpp, BgSize_T_256x256, 0,1);

    while (1) {
        swiWaitForVBlank();

        if (!delay || !count) { // if count isn't zero, skip fading the colour
            switch (phase) {
                case BLACK_TO_BLUE:
                    bgcol += 0b0000010000000000;
                    if (bgcol == BLUE)
                        phase = BLUE_TO_YELLOW;
                    break;
                case BLUE_TO_YELLOW:
                    bgcol += 0b0000000000100001;
                    bgcol -= 0b0000010000000000;
                    if (bgcol == YELLOW)
                        phase = YELLOW_TO_WHITE;
                    break;
                case YELLOW_TO_WHITE:
                    bgcol += 0b0000010000000000;
                    if (bgcol == WHITE)
                        phase = WHITE_TO_BLACK;
                    break;
                case WHITE_TO_BLACK:
                    bgcol -= 0b0000010000100001;
                    if (!bgcol)
                        phase = BLACK_TO_BLUE;
                    break;
            }

            setBackdropColor(bgcol);
            setBackdropColorSub(bgcol);
        }

        scanKeys();
        held = keysHeld();
        down = keysDown();
        if (held & KEY_X) {
            if (down & KEY_UP)
                delay++;
            if ((down & KEY_DOWN) && (delay > 0))
                delay--;
            if (down & KEY_LEFT) {
                delay=0;
            }
        } else if (held & KEY_Y) {
            if (down & KEY_UP)
                systemSetBacklightLevel(0);
            else if (down & KEY_RIGHT)
                systemSetBacklightLevel(1);
            else if (down & KEY_DOWN)
                systemSetBacklightLevel(2);
            else if (down & KEY_LEFT)
                systemSetBacklightLevel(3);
            else if (down & KEY_A)
                systemSetBacklightLevel(4);
            else if (down & KEY_B)
                systemSetBacklightLevel(5);
        } else {
            if (keysHeld() & KEY_UP) {
                setBackdropColor(BLUE);
                setBackdropColorSub(BLUE);
            }
            else if (keysHeld() & KEY_RIGHT) {
                setBackdropColor(YELLOW);
                setBackdropColorSub(YELLOW);
            }
            else if (keysHeld() & KEY_DOWN) {
                setBackdropColor(WHITE);
                setBackdropColorSub(WHITE);
            }
            else if (keysHeld() & KEY_LEFT) {
                setBackdropColor(BLACK);
                setBackdropColorSub(BLACK);
            }
        }

        if (down & KEY_START)
            return 0;

        // increment count, and reset to 0 if bigger than delay
        if (delay > 0) {
            count++;
            count%=delay;
        }
    }
}
