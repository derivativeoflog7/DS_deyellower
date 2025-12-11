#include "common.h"
#include <nds.h>

void int_to_buffer(int val, int* const target) {
    assert(val >= 0);
    for (int i = NUMBER_INPUT_LENGTH - 1; i >= 0; i--) {
        target[i] = val % 10;
        val /= 10;
    }
    assert(val == 0); // ensure that val is not >= 10**NUMBER_INPUT_LENGTH
}

int buffer_to_int(int* const buf) {
    int ret = 0, mult = 1;
    for (int i = NUMBER_INPUT_LENGTH - 1; i >= 0; i--) {
        assert(buf[i] >= 0);
        assert(buf[i] <= 9);
        ret += buf[i] * mult;
        mult *= 10;
    }
    return ret;
}

ConsoleType detect_console_type() {
    /*
    https://blocksds.skylyrac.net/libnds/system_8h.html#a9bd93bee5409c05451447034b250959b
    It's probably possible to do this in a more elegant way by reading some registers directly (which seemingly
    is how the library determines if the DS has backlight control or not), but this will suffice for now
    */
    u32 test = systemSetBacklightLevel(1);
    systemSetBacklightLevel(5);
    if (isDSiMode() || test == 1)
        return DSI;
    if (test == 2)
        return DS_WITH_BACKLIGHT_CONTROL;
    return DS_WITHOUT_BACKLIGHT_CONTROL;
}