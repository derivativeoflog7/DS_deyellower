#include "common.h"
#include <nds.h>
#include <nds/system.h>

void int_to_buffer(int val, int* const target) {
    assert(val > 0);
    for (int i = NUMBER_INPUT_LENGTH - 1; i >= 0; i--) {
        target[i] = val % 10;
        val /= 10;
    }
    assert(val == 0); // ensure that val is not >= 10^NUMBER_INPUT_LENGTH
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

/*ConsoleType detect_console_type() {
    if (isDSiMode())
        return DSI;
    u8 backlight_level =
    if (backlight_level & 0xF0)
        return DS_WITH_BACKLIGHT_CONTROL;
    return DS_WITHOUT_BACKLIGHT_CONTROL;

}*/