#include "colors.h"
#include "core/platform.h"

void InitializeColors() {
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_BLACK, COLOR_WHITE);
}
