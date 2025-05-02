#include "colors.h"
#include "core/platform.h"

void InitializeColors() {
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_BLACK, COLOR_WHITE);
    init_pair(3, COLOR_BLACK, 244);
    init_pair(4, COLOR_GREEN, COLOR_WHITE);
    init_pair(5, 244, COLOR_BLACK);
    init_pair(6, 244, COLOR_WHITE);
    init_pair(7, COLOR_WHITE, COLOR_RED);
    init_pair(8, COLOR_RED, COLOR_WHITE);
}
