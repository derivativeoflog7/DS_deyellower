#include <nds.h>
#include <stdio.h>
#include "process.h"

#define DEBUG_BUFFER_SIZE 50

void setBackdropBoth(u16 col) {
    setBackdropColor(col);
    setBackdropColorSub(col);
}

void handleBacklight(
    int *current_backlight_top,
    int *current_backlight_bottom,
    int *last_backlight_top,
    int *last_backlight_bottom,
    const int backlight_level,
    const int MAX_BACKLIGHT_LEVEL,
    const int CONSOLE_TYPE,
    const int is_phase_screen_on,
    const u16 keys_held,
    const Screens screens
) {
    /*
     * This monster handles changing backlight levels on the screens and turning them on or off
     * It exists because you definitely don't want to rapidly toggle the LEDs on and off
     * or change their light level; it also avoids setting the backlight level and/or
     * turning the screens off constantly, which is probably for the better even when
     * the backlight level hasn't actually changed
     */
    int has_backlight_changed = 0; //flag
    char debug_buffer[DEBUG_BUFFER_SIZE]; //for the nocash console

    // Copy current backlights to last
    *last_backlight_bottom = *current_backlight_bottom;
    *last_backlight_top = *current_backlight_top;

    // If in the screen on phase, set both current levels to the one set by the user
    if (is_phase_screen_on) {
        *current_backlight_bottom = backlight_level;
        *current_backlight_top = backlight_level;
    // Otherwise, set both current levels to off (-1)
    } else {
        *current_backlight_bottom = -1;
        *current_backlight_top = -1;
    }

    // If dpad is being held, set both screens to max backlight
    if (keys_held & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT)) {
        *current_backlight_bottom = MAX_BACKLIGHT_LEVEL;
        *current_backlight_top = MAX_BACKLIGHT_LEVEL;
    }
    // If X is being held (and not dpad) and backlight or LCD is off, set both screens to min
    else if (keys_held & KEY_X && (!is_phase_screen_on || backlight_level == 0)) {
        *current_backlight_bottom = PM_BACKLIGHT_MIN;
        *current_backlight_top = PM_BACKLIGHT_MIN;
    }

    // Set to zero if the corresponding backlight is off, but only if the screen and backlight are on
    if (screens == TOP && *current_backlight_top > PM_BACKLIGHT_OFF)
        *current_backlight_bottom = PM_BACKLIGHT_OFF;
    else if (screens == BOTTOM && *current_backlight_bottom > PM_BACKLIGHT_OFF)
        *current_backlight_top = PM_BACKLIGHT_OFF;

    // Turn on screens if they were off...
    if (*last_backlight_bottom == -1 && *current_backlight_bottom != -1) {
        assert(*last_backlight_top == -1);
        assert(*current_backlight_bottom != -1);

        powerOn(POWER_LCD);
        nocashMessage("Turned ON the screens");
        has_backlight_changed = 1;
    }
    // ...and vice versa
    else if (*last_backlight_bottom != -1 && *current_backlight_bottom == -1) {
        assert(*last_backlight_top != -1);
        assert(*current_backlight_bottom == -1);

        powerOff(POWER_LCD);
        nocashMessage("Turned OFF the screens");
        has_backlight_changed = 1;
    }

    // If both screens have backlight turned on...
    if (*current_backlight_bottom > PM_BACKLIGHT_OFF && *current_backlight_top > PM_BACKLIGHT_OFF) {
        // Assert that levels are equal
        assert(*current_backlight_bottom == *current_backlight_top);

        // If either screen had another level last time, change backlight level
        if (*last_backlight_bottom != *current_backlight_bottom || *last_backlight_top != *current_backlight_top) {
            setBacklightAdjusted(*current_backlight_bottom, CONSOLE_TYPE);
            snprintf(debug_buffer, DEBUG_BUFFER_SIZE, "Set backlight to %d", *current_backlight_bottom);
            nocashMessage(debug_buffer);
            has_backlight_changed = 1;
        }
    // ...if both screens have backlight off, turn them off if they weren't last time...
    } else if (
        (*current_backlight_bottom == PM_BACKLIGHT_OFF && *current_backlight_top == PM_BACKLIGHT_OFF) &&
        (*last_backlight_bottom != PM_BACKLIGHT_OFF || *last_backlight_top != PM_BACKLIGHT_OFF)
    ) {
        systemSetBacklightLevel(PM_BACKLIGHT_OFF);
        nocashMessage("Set backlight to 0");
        has_backlight_changed = 1;
    // ...if bottom screen is on and top is off, and if their level is different from last time, set backlight level then turn top off...
    } else if (
        (*current_backlight_bottom > PM_BACKLIGHT_OFF && *current_backlight_top == PM_BACKLIGHT_OFF) &&
        (*current_backlight_bottom != *last_backlight_bottom || *current_backlight_top != *last_backlight_top)
    ) {
        setBacklightAdjusted(*current_backlight_bottom, CONSOLE_TYPE);
        powerOff(PM_BACKLIGHT_TOP);
        snprintf(debug_buffer, DEBUG_BUFFER_SIZE, "Set backlight to %d, then turned OFF top screen", *current_backlight_bottom);
        nocashMessage(debug_buffer);
        has_backlight_changed = 1;
    // ...vice versa of the above
    } else if (
        (*current_backlight_top > PM_BACKLIGHT_OFF && *current_backlight_bottom == PM_BACKLIGHT_OFF) &&
        (*current_backlight_bottom != *last_backlight_bottom || *current_backlight_top != *last_backlight_top)
    ) {
        setBacklightAdjusted(*current_backlight_top, CONSOLE_TYPE);
        powerOff(PM_BACKLIGHT_BOTTOM);
        snprintf(debug_buffer, DEBUG_BUFFER_SIZE, "Set backlight to %d, then turned OFF bottom screen", *current_backlight_top);
        nocashMessage(debug_buffer);
        has_backlight_changed = 1;
    }

    if (has_backlight_changed) {
        snprintf(
            debug_buffer, DEBUG_BUFFER_SIZE, "Last top %d bot %d; curr top %d bot %d",
            *last_backlight_top, *last_backlight_bottom, *current_backlight_top, *current_backlight_bottom
        );
        nocashMessage(debug_buffer);
    }
}

static void set_seconds(const int minutes, int* seconds) {
    *seconds = minutes * 60;
}

// Init screen on phase by changing variables, and setting and starting the timer
void init_screen_on_phase (
    const Mode mode,
    const int minutes,
    int* seconds,
    u16* backdrop_color,
    CyclingPhase *current_cycling_phase
) {
    *backdrop_color = mode == WHITE_SCREEN ? WHITE : BLACK;
    *current_cycling_phase = BLACK_TO_BLUE;
    set_seconds(minutes, seconds);
}

// Init screen off phase by setting the timer
void init_screen_off_phase (
    const int minutes,
    int* seconds
) {
    set_seconds(minutes, seconds);
}

static void _print_warning_message (
    PrintConsole *console
) {
    consoleSetCursor(console, 0, console->consoleHeight - 3);
    consoleSetColor(console, CONSOLE_LIGHT_RED);
    printf("Avoid turning the screen on\n");
    printf("repeatedly and/or rapidly while\n");
    printf("in the screen off phase");
}

static void _print_progress_message (
    PrintConsole *console,
    const int remaining_seconds,
    const int remaining_repetitions,
    const int is_screen_on_phase
) {
    consoleSetColor(console, CONSOLE_LIGHT_MAGENTA);
    printf("Screen %s phase\n\n", is_screen_on_phase ? "ON" : "OFF");
    printf("Remaining time for phase:\n");
    printf("%d:%02d:%02d\n\n", remaining_seconds / 60 / 60, remaining_seconds / 60 % 60, remaining_seconds % 60);
    printf("Remaining repetitions: %d\n", remaining_repetitions);
}

void print_progress_message(
    PrintConsole *top_screen_console,
    PrintConsole *bottom_screen_console,
    const int remaining_seconds,
    const int remaining_repetitions,
    const int is_screen_on_phase
) {
    consoleSelect(bottom_screen_console);
    _print_progress_message(bottom_screen_console, remaining_seconds, remaining_repetitions, is_screen_on_phase);
    consoleSelect(top_screen_console);
    _print_progress_message(top_screen_console, remaining_seconds, remaining_repetitions, is_screen_on_phase);
}

void print_warning_message(
    PrintConsole *top_screen_console,
    PrintConsole *bottom_screen_console
) {
    consoleSelect(bottom_screen_console);
    _print_warning_message(bottom_screen_console);
    consoleSelect(top_screen_console);
    _print_warning_message(top_screen_console);
}
