#include "chat.h"
#include "core/platform.h"
#include "util/colors.h"

Network* g_nref = NULL;
size_t g_selected_friend = 0;
int g_height, g_width = 0;
int g_chat_cursor = 0;
char g_chat_buffer[MAX_MESSAGE_SIZE] = { 0 };

void draw_contacts() {
    for (size_t i = 0; i < g_nref->friends.size; i++) {
        if (i == g_selected_friend) {
            attron(COLOR_PAIR(BLACK_WHITE));
            for (size_t j = 0; j < MAX_USERNAME_SIZE + 3; j++) mvaddch(2 + i, 1 + j, ' ');
        }
        mvprintw(2 + i, 1, g_nref->friends.data[i].name);
        if (g_nref->friends.data[i].unread) {
            attron(COLOR_PAIR(RED_BLACK));
            mvprintw(2 + i, MAX_USERNAME_SIZE + 2, "!!");
            attroff(COLOR_PAIR(RED_BLACK));
        }
        if (i == g_selected_friend) attroff(COLOR_PAIR(BLACK_WHITE));
    }
}

void draw_divider() {
    for (int i = 1; i < g_height - 5; i++) {
        mvaddch(i, MAX_USERNAME_SIZE + 5, ACS_VLINE);
    }
}

void draw_headers() {
    attron(COLOR_PAIR(BLACK_WHITE));
    for (int i = 0; i < g_width; i++) {
        mvaddch(0, i, ' ');
        mvaddch(g_height - 5, i, ' ');
    }
    mvprintw(0, 1, "SIGMSG %s", SM_VERSION);
    mvprintw(0, g_width - 15, "3 new messages"); // TODO:
    mvprintw(0, g_width/2 - 3, "Online"); // TODO:
    mvprintw(g_height - 5, 1, "%s", g_nref->friends.data[g_selected_friend].name); 
    attroff(COLOR_PAIR(BLACK_WHITE));
    attron(COLOR_PAIR(GRAY_WHITE));
    mvprintw(g_height - 5, 2 + strlen(g_nref->friends.data[g_selected_friend].name), "#7ab882bd728a10"); //TODO:
    attroff(COLOR_PAIR(GRAY_WHITE));
}

void draw_options() {
    attron(COLOR_PAIR(BLACK_GRAY));
    for (int i = 0; i < g_width; i++) {
        mvaddch(g_height - 1, i, ' ');
    }
    mvprintw(g_height - 1, 1, "(ENTR) Send | (ESC) Quit |");
    attroff(COLOR_PAIR(BLACK_GRAY));
    refresh();
}

void draw_chat() {
    for (int x = MAX_USERNAME_SIZE + 7; x < g_width; x++)
        for (int y = 1; y < g_height - 5; y++)
            mvaddch(y, x, ' ');
    User* user = &(g_nref->friends.data[g_selected_friend]);
    int cursor = g_height - 5;
    for (size_t i = user->history.size; i > 0; i--) {
        Message* msg = &(user->history.data[i - 1]);
        int linewidth = g_width - MAX_USERNAME_SIZE - 8;
        int lines = msg->size / linewidth;
        if (lines * linewidth < msg->size) lines++;
        cursor -= lines + 2;
        if (cursor < 1) break;
        attron(A_BOLD);
        mvprintw(cursor, MAX_USERNAME_SIZE + 7, "%s", (msg->user ? user->name : "Me"));
        attroff(A_BOLD);
        attron(COLOR_PAIR(GRAY_BLACK));
        mvprintw(cursor, MAX_USERNAME_SIZE + 8 + (msg->user ? strlen(user->name) : 2), "(%d-%d-%d %d:%d:%d)",
            (int)msg->time.month,
            (int)msg->time.day,
            (int)msg->time.year,
            (int)msg->time.hour,
            (int)msg->time.minute,
            (int)msg->time.second);
        attroff(COLOR_PAIR(GRAY_BLACK));
        cursor++;
        char buffer[MAX_MESSAGE_SIZE * 2] = { 0 };
        for (int j = 0; j < lines; j++) {
            strcpy(buffer, msg->text + (j * linewidth));
            buffer[linewidth] = '\0';
            mvprintw(cursor, MAX_USERNAME_SIZE + 7, buffer);
            cursor++;
        }
        cursor -= lines + 1;
    }
        // attron(A_BOLD);
        // mvprintw(2, 22, "Cameron (4-12-25 12:25:13)");
        // attroff(A_BOLD);
        // mvprintw(3, 22, "Bro... come over.... :eggplant: :wet:");
}

void ChatState(Event event) {
	curs_set(1);
    getmaxyx(stdscr, g_height, g_width);
    g_nref = NetworkRef();
    if (event.resize) {
        clear();

        // header bars
        draw_headers();

        // chat text
        mvprintw(g_height - 4, 0, ">");

        // contacts
        draw_contacts();

        // divider
        draw_divider();

        // chat
        draw_chat();

        // bottom bar
        draw_options();
    }
    if (event.recieve) {
        draw_chat();
    }
    if (event.kevent >= 32 && event.kevent <= 126) {
        if (g_chat_cursor >= MAX_MESSAGE_SIZE - 1) {
            attron(COLOR_PAIR(WHITE_RED));
            for (int i = 0; i < g_width; i++) {
                mvaddch(g_height - 5, i, ' ');
            }
            mvprintw(g_height - 5, g_width/2 - 16, "Maximum character limit reached!");
            attroff(COLOR_PAIR(WHITE_RED));
        } else {
            g_chat_buffer[g_chat_cursor] = (char)event.kevent;
            g_chat_cursor++;
        }
    } else if (event.kevent == 8 && g_chat_cursor > 0) {
        g_chat_cursor--;
        g_chat_buffer[g_chat_cursor] = '\0';
    } else if ((event.kevent == 13 || event.kevent == 10) && g_chat_cursor > 0) {
        SendChat(&(g_nref->friends.data[g_selected_friend]), g_chat_buffer);
        draw_chat();
        g_chat_cursor = 0;
        memset(g_chat_buffer, '\0', MAX_MESSAGE_SIZE);
    }
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < g_width; j++) {
            mvaddch(g_height - 4 + i, j, ' ');
        }
    }
    mvprintw(g_height - 4, 0, "> ");
    int linewidth = g_width - 3;
    int lines = g_chat_cursor / linewidth;
    if (lines * linewidth < g_chat_cursor) lines++;
    int third_to_last_line = lines - 3 < 0 ? 0 : lines - 3;
    char buffer[MAX_MESSAGE_SIZE] = { 0 };
    for (int i = 0; i < 3 && i < lines; i++) {
        strcpy(buffer, g_chat_buffer + (third_to_last_line * linewidth) + (i * linewidth));
        buffer[linewidth] = '\0';
        mvprintw(g_height - 4 + i, 2, buffer);
    }
    refresh();
}
