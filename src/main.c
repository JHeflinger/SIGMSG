#include "core/app.h"
#include <curses.h>

int main(int argc, const char** argv) {
    Run(GetFlagsFromArgs(argc, argv));
    return 0;
}