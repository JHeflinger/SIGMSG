#include "colors.h"
#include <curses.h>

void InitializeColors() {
    init_pair(1, COLOR_RED, COLOR_BLACK);
}