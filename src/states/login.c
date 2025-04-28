#include "login.h"
#include "core/app.h"
#include "util/macros.h"
#include "util/colors.h"
#include "core/platform.h"
#include <easylogger.h>
#include <string.h>

void LoginState(Event event) {
    int height, width;
    getmaxyx(stdscr, height, width);
    if (event.resize) {
		curs_set(0);
        clear();
        move(height/2 - 7, width/2 - 29);
        attron(COLOR_PAIR(RED_BLACK));
        printw("  _________.___  ________    _____    _________ ________\n");
        amove(0, width/2 - 29);
        printw(" /   _____/|   |/  _____/   /     \\  /   _____//  _____/\n");
        amove(0, width/2 - 29);
        printw(" \\_____  \\ |   /   \\  ___  /  \\ /  \\ \\_____  \\/   \\  ___\n");
        amove(0, width/2 - 29);
        printw(" /        \\|   \\    \\_\\  \\/    Y    \\/        \\    \\_\\  \\\n");
        amove(0, width/2 - 29);
        printw("/_______  /|___|\\______  /\\____|__  /_______  /\\______  /\n");
        amove(0, width/2 - 29);
        printw("        \\/             \\/         \\/        \\/        \\/\n");
        amove(0, width/2 - 29);
        attroff(COLOR_PAIR(RED_BLACK));
        printw("                           V0.1                         \n");
        amove(1, width/2 - 9);
        attron(COLOR_PAIR(BLACK_WHITE));
        printw("                  \n");
        amove(0, width/2 - 9);
        printw("    LOG-IN (l)    \n");
        amove(0, width/2 - 9);
        printw("                  \n");
        amove(1, width/2 - 9);
        printw("                  \n");
        amove(0, width/2 - 9);
        printw("     QUIT (q)     \n");
        amove(0, width/2 - 9);
        printw("                  \n"); 
        attroff(COLOR_PAIR(BLACK_WHITE));
		refresh();
    } else if (event.kevent == 'q') {
        clear();
        refresh();
        Stop();
    } else if (event.mevent.type == MOUSE_LEFT_CLICK || event.mevent.type == MOUSE_LEFT_DOWN) {
		if (mcollide(event, width/2 - 9, height/2 + 5, 18, 3)) {
            clear();
            refresh();
            Stop();
        }
    }
}
