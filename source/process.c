#include <nds.h>
#include <stdio.h>
#include "process.h"

#include "../../../../../opt/blocksds/core/libs/libnds/include/nds/arm9/console.h"

void setBackdropBoth(u16 col) {
    setBackdropColor(col);
    setBackdropColorSub(col);
}

void setBacklight(int backlight_level, ConsoleType console_type) {
    /*
     * https://blocksds.skylyrac.net/libnds/system_8h.html#a9bd93bee5409c05451447034b250959b
     * On DSi, just set the level as is
     * On DS (lite) with backlight control, add 1 to the backlight level, as they are shifted by one (except 0)
     * On DS without backlight control, libnds internally maps all non-zero values to backlight on
     */
    systemSetBacklightLevel(
        console_type == DS_WITH_BACKLIGHT_CONTROL && backlight_level > 0 ?
        backlight_level + 1 : backlight_level
    );
}

int handleBacklight(
    int *current_backlight_top,
    int *current_backlight_bottom,
    int *last_backlight_top,
    int *last_backlight_bottom,
    const Screens screens,
    const int backlight_level,
    const int MAX_BACKLIGHT_LEVEL,
    const int CONSOLE_TYPE,
    const u16 keys_held,
    const int is_phase_screen_on
) {
    /*
     * This monster handles changing backlight levels on the screens and turning them on or off
     * It exists because you definitely don't want to rapidly toggle the LEDs on and off
     * or change their light level; it also avoids setting the backlight level and/or
     * turning the screens off constantly, which is probably for the better even when
     * the backlight level hasn't actually changed
     */
    int has_backlight_changed = 0; //flag

    // Copy current backlights to last
    *last_backlight_bottom = *current_backlight_bottom;
    *last_backlight_top = *current_backlight_top;

    // Only if the current phase has the screens on...
    if (is_phase_screen_on) {
        // Start by setting both current levels to the one set by the user
        *current_backlight_bottom = backlight_level;
        *current_backlight_top = backlight_level;

        // Set to zero if the corresponding screen is off
        if (screens == TOP)
            *current_backlight_bottom = PM_BACKLIGHT_OFF;
        else if (screens == BOTTOM)
            *current_backlight_top = PM_BACKLIGHT_OFF;
    // ..otherwise
    } else {
        *current_backlight_bottom = -1;
        *current_backlight_top = -1; // Level -1 represents screens off
    }

    // If dpad is being held, set both screens to max backlight
    if (keys_held & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT)) {
        *current_backlight_bottom = MAX_BACKLIGHT_LEVEL;
        *current_backlight_top = MAX_BACKLIGHT_LEVEL;
    }
    // If X is being held (and not dpad) and user last backlight is off or LCD off, set both screens to min
    else if (keys_held & KEY_X && (!is_phase_screen_on || backlight_level == 0)) {
        *current_backlight_bottom = PM_BACKLIGHT_MIN;
        *current_backlight_top = PM_BACKLIGHT_MIN;
    }

    // Turn on screens if they were off...
    if (*last_backlight_bottom == -1 && *current_backlight_bottom != -1) {
        assert(*last_backlight_top == -1);
        assert(*current_backlight_bottom != -1);

        powerOn(POWER_LCD);
        has_backlight_changed = 1;
    }
    // ...and vice versa
    else if (*last_backlight_bottom != -1 && *current_backlight_bottom == -1) {
        assert(*last_backlight_top != -1);
        assert(*current_backlight_bottom == -1);

        powerOff(POWER_LCD);
        has_backlight_changed = 1;
    }

    // If both screens have backlight turned on...
    if (*current_backlight_bottom > PM_BACKLIGHT_OFF && *current_backlight_top > PM_BACKLIGHT_OFF) {
        // Assert that levels are equal
        assert(*current_backlight_bottom == *current_backlight_top);

        // If either screen had another level last time, change backlight level
        if (*last_backlight_bottom != *current_backlight_bottom || *last_backlight_top != *current_backlight_top) {
            setBacklight(*current_backlight_bottom, CONSOLE_TYPE);
            has_backlight_changed = 1;
        }
    // ...if both screens have backlight off, turn them off if they weren't last time...
    } else if (
        (*current_backlight_bottom == PM_BACKLIGHT_OFF && *current_backlight_top == PM_BACKLIGHT_OFF) &&
        (*last_backlight_bottom > PM_BACKLIGHT_OFF || *last_backlight_top > PM_BACKLIGHT_OFF)
    ) {
        systemSetBacklightLevel(PM_BACKLIGHT_OFF);
        has_backlight_changed = 1;
    // ...if bottom screen is on and top is off, and if their level is different from last time, set backlight level then turn top off...
    } else if (
        (*current_backlight_bottom > PM_BACKLIGHT_OFF && *current_backlight_top == PM_BACKLIGHT_OFF) &&
        (*current_backlight_bottom != *last_backlight_bottom || *current_backlight_top != *last_backlight_top)
    ) {
        setBacklight(*current_backlight_bottom, CONSOLE_TYPE);
        powerOff(PM_BACKLIGHT_TOP);
        has_backlight_changed = 1;
    // ...vice versa of the above
    } else if (
        (*current_backlight_top > PM_BACKLIGHT_OFF && *current_backlight_bottom == PM_BACKLIGHT_OFF) &&
        (*current_backlight_bottom != *last_backlight_bottom || *current_backlight_top != *last_backlight_top)
    ) {
        setBacklight(*current_backlight_top, CONSOLE_TYPE);
        powerOff(PM_BACKLIGHT_BOTTOM);
        has_backlight_changed = 1;
    }

    return has_backlight_changed;
}

static void set_seconds(int minutes, int* seconds) {
    *seconds = 10;// minutes * 60;
}

void init_screen_on_phase (
    const Mode mode,
    const int minutes,
    int* seconds,
    void* timer_handler
) {
    setBackdropBoth(mode == WHITE_SCREEN ? WHITE : BLACK);
    set_seconds(minutes, seconds);
    timerStart(0, ClockDivider_1024, timerFreqToTicks_1024(1), timer_handler);
}

void init_screen_off_phase (
    const int minutes,
    int* seconds,
    void* timer_handler
) {
    set_seconds(minutes, seconds);
    timerStart(0, ClockDivider_1024, timerFreqToTicks_1024(1), timer_handler);
}

void print_progress_message(
    PrintConsole *console,
    const int remaining_seconds,
    const int remaining_repetitions,
    const int is_screen_on_phase
) {
    printf("Screen %s phase\n\n", is_screen_on_phase ? "ON" : "OFF");
    printf("Remaining time for phase:\n");
    printf("%d:%02d:%02d\n\n", remaining_seconds / 60 / 60, remaining_seconds / 60 % 60, remaining_seconds % 60);
    printf("Remaning repetitions: %d\n", remaining_repetitions);
    if (!is_screen_on_phase) {
        consoleSetCursor(console, 0, console->consoleHeight - 3);
        consoleSetColor(console, CONSOLE_LIGHT_RED);
        printf("Avoid turning the screen on\n");
        printf("repeatedly while in the screen\n");
        printf("off phase");
        consoleSetColor(console, CONSOLE_WHITE);
    }
}
