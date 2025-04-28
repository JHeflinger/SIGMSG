#include "animations.h"
#include "core/platform.h"
#include "util/colors.h"
#include "util/macros.h"
#include <easylogger.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <curses.h>

void Wait(size_t milliseconds) {
    #ifdef __WIN32
        Sleep(milliseconds);
    #elif __linux__
        usleep(milliseconds);
    #else
        #error "Operating system not supported"
    #endif
}

void Type(float speed, const char* format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[4096];
    vsnprintf(buffer, 4096, format, args);
    va_end(args);
    size_t blen = strnlen(buffer, 4096);
    for (size_t i = 0; i < blen; i++) {
        printw("%c", buffer[i]);
        refresh();
        Wait(speed);
    }
}

void BootAnimation() {
    int height, width;
    getmaxyx(stdscr, height, width);
    clear();
    // size_t theight = TerminalHeight();
    // for (size_t i = 0; i < theight + 1; i++) printf("\n");
    move(height - 1, 0);
    Type(50, "Booting up");
    Type(500, "...\n");
    Type(5, "[");
    attron(COLOR_PAIR(RED_BLACK));
    Type(5, "ERROR");
    attroff(COLOR_PAIR(RED_BLACK));
    Type(5, "]");
    Type(20, " - undefined behavior detected.\n");
    Type(20, "it was");
    Type(500, "...\n");
    clear();
    move(height / 2 - 1, width / 2 - 4);
    Type(100, "a ");
    attron(COLOR_PAIR(RED_BLACK));
    Type(100, "SIGMSG");
    attroff(COLOR_PAIR(RED_BLACK));
    Wait(2000);
    clear();
    for (int i = 0; i < 50000; i++) {
        int r = (rand()%96) + 32;
        if (r == 127) r = '\n';
        printw("%c", (char)r);
        refresh();
    }
    int waittime = 50;
    for (int i = 0; i < height; i++) { printw("\n"); refresh(); }
    Wait(500);
    amove(0, width/2 - 37);
    printw("               .__                                  __\n");
    refresh();
    Wait(waittime);
    amove(0, width/2 - 37);
    printw("__  _  __ ____ |  |   ____  ____   _____   ____   _/  |_  ____\n");
    refresh();
    Wait(waittime);
    amove(0, width/2 - 37);
    printw("\\ \\/ \\/ // __ \\|  | _/ ___\\/  _ \\ /     \\_/ __ \\  \\   __\\/  _ \\\n");
    refresh();
    Wait(waittime);
    amove(0, width/2 - 37);
    printw(" \\     /\\  ___/|  |_\\  \\__(  <_> )  Y Y  \\  ___/   |  | (  <_> )\n");
    refresh();
    Wait(waittime);
    amove(0, width/2 - 37);
    printw("  \\/\\_/  \\___  >____/\\___  >____/|__|_|  /\\___  >  |__|  \\____/ /\\ /\\ /\\\n");
    refresh();
    Wait(waittime);
    amove(0, width/2 - 37);
    printw("             \\/          \\/            \\/     \\/                \\/ \\/ \\/\n");
    refresh();
    for (int i = 0; i < height; i++) { Wait(waittime); printw("\n"); refresh(); }
    clear();
    refresh();
    Wait(1000);
}