#include "chat.h"
#include "core/platform.h"
#include "util/colors.h"

void ChatState(Event event) {
    int height, width;
    getmaxyx(stdscr, height, width);
    if (event.resize) {
        clear();
        attron(COLOR_PAIR(BLACK_WHITE));
        for (int i = 0; i < width; i++) {
            mvaddch(0, i, ' ');
        }
        mvprintw(0, 0, "%d - %d", height, width);
        for (int i = 0; i < width; i++) {
            mvaddch(height - 5, i, ' ');
        }
        for (int i = 0; i < width; i++) {
            mvaddch(height - 2, i, ' ');
        }
        attroff(COLOR_PAIR(BLACK_WHITE));
        refresh();
    }
}