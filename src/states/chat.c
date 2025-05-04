#include "chat.h"
#include "core/platform.h"
#include "util/colors.h"
#include "util/macros.h"
#include <inttypes.h>

Network* g_nref = NULL;
size_t g_selected_friend = 0;
int g_height, g_width = 0;
int g_chat_cursor = 0;
int g_chat_state = 0;
char g_chat_buffer[MAX_MESSAGE_SIZE] = { 0 };
char g_uuid_buffer[64] = { 0 };
int g_uuid_cursor = 0;

void draw_contacts() {
    for (size_t i = 0; i < g_nref->friends.size; i++) {
        if (i == g_selected_friend) {
            attron(COLOR_PAIR(BLACK_WHITE));
            for (size_t j = 0; j < MAX_USERNAME_SIZE + 3; j++) mvaddch(2 + i, 1 + j, ' ');
            if (g_nref->friends.data[i].unread) g_nref->friends.data[i].unread = FALSE;
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
    int unread = 0;
    for (size_t i = 0; i < g_nref->friends.size; i++) {
        if (g_nref->friends.data[i].unread) unread++;
    }
    char msgbuf[128] = { 0 };
    if (unread > 0)
        sprintf(msgbuf, "%d unread chats", unread);
    else
        sprintf(msgbuf, "No new messages");
    mvprintw(0, g_width - strlen(msgbuf) - 1, "%s", msgbuf);
    char idbuff[64] = { 0 };
    char padbuff[64] = { 0 };
    sprintf(idbuff, "%" PRIx64 "%" PRIx64, g_nref->id.first, g_nref->id.second);
    for (size_t i = 0; i < 32; i++) {
        if (i < 32 - strlen(idbuff)) {
            padbuff[i] = '0';
        } else {
            padbuff[i] = idbuff[i - (32 - strlen(idbuff))];
        }
    }
    if (g_nref->online) {
        mvprintw(0, g_width/2 - 18, "ID#%s", padbuff);
    } else {
        attroff(COLOR_PAIR(BLACK_WHITE));
        attron(COLOR_PAIR(RED_WHITE));
        mvprintw(0, g_width/2 - 4, "OFFLINE");
        attroff(COLOR_PAIR(RED_WHITE));
        attron(COLOR_PAIR(BLACK_WHITE));
    }
    mvprintw(g_height - 5, 1, "%s", g_nref->friends.data[g_selected_friend].name); 
    attroff(COLOR_PAIR(BLACK_WHITE));
    attron(COLOR_PAIR(GRAY_WHITE));
    sprintf(idbuff, "%" PRIx64 "%" PRIx64,
        g_nref->friends.data[g_selected_friend].id.first,
        g_nref->friends.data[g_selected_friend].id.second);
    for (size_t i = 0; i < 32; i++) {
        if (i < 32 - strlen(idbuff)) {
            padbuff[i] = '0';
        } else {
            padbuff[i] = idbuff[i - (32 - strlen(idbuff))];
        }
    }
    mvprintw(g_height - 5, 1 + strlen(g_nref->friends.data[g_selected_friend].name),
        "#%s",
        padbuff);
    attroff(COLOR_PAIR(GRAY_WHITE));
}

void draw_options() {
    attron(COLOR_PAIR(BLACK_GRAY));
    for (int i = 0; i < g_width; i++) {
        mvaddch(g_height - 1, i, ' ');
    }
    if (g_chat_state == 0) {
        mvprintw(g_height - 1, 1, "(ENTR) Send | (ESC) Quit | (TAB) Cycle | ^A Add Friend");
    } else if (g_chat_state == 1) {
        mvprintw(g_height - 1, 1, "(ENTR) Submit | (ESC) Quit | ^A Cancel");
    }
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
        mvprintw(cursor, MAX_USERNAME_SIZE + 7, "%s", (uuideq(msg->to, g_nref->id) ? user->name : "Me"));
        attroff(A_BOLD);
        attron(COLOR_PAIR(GRAY_BLACK));
        mvprintw(cursor, MAX_USERNAME_SIZE + 8 + (uuideq(msg->to, g_nref->id) ? strlen(user->name) : 2), "(%d-%d-%d %d:%d:%d)",
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
}

void draw_add_friend() {
    draw_options();
    attron(COLOR_PAIR(BLACK_WHITE));
    for (int x = 0; x < 34; x++) {
        for (int y = 0; y < 5; y++) {
            if (y == 3 && x > 0 && x < 33) {
                attroff(COLOR_PAIR(BLACK_WHITE));
            }
            mvaddch(g_height/2 - 3 + y, g_width/2 - 17 + x, ' ');
            if (y == 3 && x > 0 && x < 33) {
                attron(COLOR_PAIR(BLACK_WHITE));
            }
        }
    }
    mvprintw(g_height/2 - 2, g_width/2 - 7, "Add New Friend");
    attroff(COLOR_PAIR(BLACK_WHITE));
    if (g_uuid_cursor != 32) {
        attron(COLOR_PAIR(RED_BLACK));
    }
    mvprintw(g_height/2, g_width/2 - 16, "%s", g_uuid_buffer);
    if (g_uuid_cursor != 32) {
        attroff(COLOR_PAIR(RED_BLACK));
    }
}

void draw_all() {
    clear();
    draw_contacts();
    draw_headers();
    mvprintw(g_height - 4, 0, ">");
    draw_divider();
    draw_chat();
    draw_options();
}

void draw_current_text() {
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
}

void ChatState(Event event) {
	curs_set(1);
    getmaxyx(stdscr, g_height, g_width);
    g_nref = NetworkRef();
    if (event.resize) {
        draw_all();
    }
    if (event.recieve) {
        draw_headers();
        draw_chat();
        draw_contacts();
    }
    if (event.kevent >= 32 && event.kevent <= 126) {
        if (g_chat_state == 0) {
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
        } else if (g_chat_state == 1) {
            char mych = (char)event.kevent;
            if (g_uuid_cursor < 32 && ((mych >= '0' && mych <= '9') || (mych >= 'a' && mych <= 'f'))) {
                g_uuid_buffer[g_uuid_cursor] = mych;
                g_uuid_cursor++;
            }
        }
    } else if (event.kevent == 8) {
        if (g_chat_state == 0 && g_chat_cursor > 0) {
            if (g_chat_cursor >= MAX_MESSAGE_SIZE - 1) 
                draw_headers();
            g_chat_cursor--;
            g_chat_buffer[g_chat_cursor] = '\0';
        } else if (g_chat_state == 1 && g_uuid_cursor > 0) {
            g_uuid_cursor--;
            g_uuid_buffer[g_uuid_cursor] = '\0';
        }
    } else if ((event.kevent == 13 || event.kevent == 10)) {
        if (g_chat_state == 0 && g_chat_cursor > 0 && g_nref->online) {
            SendChat(&(g_nref->friends.data[g_selected_friend]), g_chat_buffer);
            draw_chat();
            g_chat_cursor = 0;
            memset(g_chat_buffer, '\0', MAX_MESSAGE_SIZE);
        } else if (g_chat_state == 1 && g_uuid_cursor == 32) {
            User newuser = { 0 };
            strcpy(newuser.name, "Unknown");
            newuser.id.second = strtoull(g_uuid_buffer + 16, NULL, 16);
            g_uuid_buffer[16] = '\0';
            newuser.id.first = strtoull(g_uuid_buffer, NULL, 16);
            BOOL found = FALSE;
            for (size_t i = 0; i < g_nref->friends.size; i++) {
                if (uuideq(g_nref->friends.data[i].id, newuser.id)) {
                    found = TRUE;
                    break;
                }
            }
            if (!found) ARRLIST_User_add(&(g_nref->friends), newuser);
            memset(g_uuid_buffer, 0, 64);
            g_uuid_cursor = 0;
            g_chat_state = 0;
            draw_all();
        }
    } else if (event.kevent == 1) {
        if (g_chat_state == 1) {
            memset(g_uuid_buffer, 0, 64);
            g_uuid_cursor = 0;
            g_chat_state = 0;
            draw_all();
        } else {
            g_chat_state = 1;
        }
    } else if (event.kevent == 9 && g_chat_state == 0) {
        g_chat_cursor = 0;
        memset(g_chat_buffer, '\0', MAX_MESSAGE_SIZE);
        g_selected_friend++;
        if (g_selected_friend >= g_nref->friends.size) g_selected_friend = 0;
        draw_all();
    }
    if (g_chat_state == 1) {
        draw_add_friend();
    } else {
        draw_current_text();
    }
    refresh();
}
