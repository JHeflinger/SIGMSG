#include "login.h"
#include "core/app.h"
#include "util/macros.h"
#include "util/colors.h"
#include <easylogger.h>
#include <string.h>
#include <curses.h>

void LoginState(Event event) {
    int height, width;
    getmaxyx(stdscr, height, width);
    if (event.empty) {
        clear();
        move(height/2 - 7, width/2 - 28);
        attron(COLOR_PAIR(RED_BLACK));
        printw("  _________.___  ________    _____    _________ ________\n");
        amove(0, width/2 - 28);
        printw(" /   _____/|   |/  _____/   /     \\  /   _____//  _____/\n");
        amove(0, width/2 - 28);
        printw(" \\_____  \\ |   /   \\  ___  /  \\ /  \\ \\_____  \\/   \\  ___\n");
        amove(0, width/2 - 28);
        printw(" /        \\|   \\    \\_\\  \\/    Y    \\/        \\    \\_\\  \\\n");
        amove(0, width/2 - 28);
        printw("/_______  /|___|\\______  /\\____|__  /_______  /\\______  /\n");
        amove(0, width/2 - 28);
        printw("        \\/             \\/         \\/        \\/        \\/\n");
        amove(0, width/2 - 28);
        attroff(COLOR_PAIR(RED_BLACK));
        printw("                           V0.1                         \n");
        amove(1, width/2 - 9);
        printw("\xda\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xb7\n");
        amove(0, width/2 - 9);
        printw("\xb3    LOG-IN (l)    \xba\n");
        amove(0, width/2 - 9);
        printw("\xd4\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xbc\n");
        amove(1, width/2 - 9);
        printw("\xda\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xb7\n");
        amove(0, width/2 - 9);
        printw("\xb3     QUIT (q)     \xba\n");
        amove(0, width/2 - 9);
        printw("\xd4\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xbc\n");
        refresh();
    } else if (event.kevent == 'q') {
        clear();
        refresh();
        Stop();
    } else if (event.mevent.type == MOUSE_LEFT_CLICK || event.mevent.type == MOUSE_LEFT_DOWN) {
        if (mcollide(event, 36, 31, 20, 3)) {
            clear();
            refresh();
            Stop();
        }
    }
}