#include "app.h"
#include "util/colors.h"
#include "util/animations.h"
#include "states/login.h"
#include "states/chat.h"
#include "core/platform.h"
#include "core/network.h"
#include <string.h>
#include <easylogger.h>
#include <easymemory.h>

AppState g_state;
AppFlags g_flags;
BOOL g_running = TRUE;
size_t g_memcheck = 0;
EZ_MUTEX g_lock;

void Initialize() {
    g_memcheck = EZ_ALLOCATED();
    EZ_CREATE_MUTEX(g_lock);
    InitializeNetwork();
    initscr();
    keypad(stdscr, TRUE);
	start_color();
    raw();
    cbreak();
    noecho();
    mousemask(ALL_MOUSE_EVENTS, NULL);
    timeout(-1);
    InitializeColors();
    if (g_flags & BOOT_ANIM) BootAnimation();
    if (NetworkRef()->id.first == 0 && NetworkRef()->id.second == 0) {
        ChangeState(LoginState);
    } else {
        ChangeState(ChatState);
    }
}

void Clean() {
    endwin();
    CleanNetwork();
    EZ_ASSERT(g_memcheck == EZ_ALLOCATED(), "Memory leak detected of %d bytes", (int)(EZ_ALLOCATED() - g_memcheck));
    EZ_INFO("Successfully shut down!");
}

void Listen() {
    while (g_running) {
        int ch = getch();
        Event e = { 0 };
		if (ch == KEY_RESIZE) {
            e.resize = TRUE;
            resize_term(0, 0);
        } else if (ch == KEY_MOUSE) {
            MEVENT me;
			if (getmouse(&me) == OK) {
                e.mevent.x = me.x;
                e.mevent.y = me.y;
                e.mevent.type = MOUSE_MOVE;
                if (me.bstate & BUTTON1_PRESSED) {
                    e.mevent.type = MOUSE_LEFT_DOWN;
                }
                if (me.bstate & BUTTON2_PRESSED) {
                    e.mevent.type = MOUSE_MIDDLE_DOWN;
                }
                if (me.bstate & BUTTON3_PRESSED) {
                    e.mevent.type = MOUSE_RIGHT_DOWN;
                }
                if (me.bstate & BUTTON1_RELEASED) {
                    e.mevent.type = MOUSE_LEFT_UP;
                }
                if (me.bstate & BUTTON2_RELEASED) {
                    e.mevent.type = MOUSE_MIDDLE_UP;
                }
                if (me.bstate & BUTTON3_RELEASED) {
                    e.mevent.type = MOUSE_RIGHT_UP;
                }
                if (me.bstate & BUTTON1_CLICKED) {
                    e.mevent.type = MOUSE_LEFT_CLICK;
                }
                if (me.bstate & BUTTON2_CLICKED) {
                    e.mevent.type = MOUSE_MIDDLE_CLICK;
                }
                if (me.bstate & BUTTON3_CLICKED) {
                    e.mevent.type = MOUSE_RIGHT_CLICK;
                }
                if (me.bstate & BUTTON4_PRESSED) {
                    e.mevent.type = MOUSE_SCROLL;
                    e.mevent.delta = 1;
                }
                if (me.bstate & BUTTON5_PRESSED) {
                    e.mevent.type = MOUSE_SCROLL;
                    e.mevent.delta = -1;
                }
            }
        } else if (ch != ERR) {
			if (ch == 263) ch = 8;
            e.kevent = ch;
        }
        if (e.kevent == 27) {
            Stop();
        } else {
            EZ_LOCK_MUTEX(g_lock);
            g_state(e);
            EZ_RELEASE_MUTEX(g_lock);
        }
    }
}

AppFlags GetFlagsFromArgs(int argc, const char** argv) {
    AppFlags flags = NO_APP_FLAGS;
    for (int i = 1; i < argc; i++) {
        if (strlen(argv[i]) > 3 && argv[i][0] == '-' && argv[i][1] == 'i' && argv[i][2] == 'd') {
            for (size_t j = 3; j < strlen(argv[i]); j++) {
                if (!((argv[i][j] >= '0' && argv[i][j] <= '9') || (argv[i][j] >= 'a' && argv[i][j] <= 'f'))) {
                    printf("Invalid ID# given - please use only lowercase hexidecimal digits.");
                    exit(1);
                }
            }
            if (strlen(argv[i]) <= 19) {
                UUID id = { 0 };
                id.second = strtoull(argv[i] + 3, NULL, 16);
                NetworkRef()->id = id;
                continue;
            } else if (strlen(argv[i]) <= 35) {
                UUID id = { 0 };
                id.second = strtoull(argv[i] + strlen(argv[i]) - 16, NULL, 16);
                char buf[64] = { 0 };
                strcpy(buf, argv[i]);
                buf[strlen(argv[i]) - 16] = '\0';
                id.first = strtoull(buf + 3, NULL, 16);
                NetworkRef()->id = id;
                continue;
            } else {
                printf("Given ID# is too long - IDs are 32 hexidecimal characters long\n");
                exit(1);
            }
        }
        if (strcmp(argv[i], "-boot_anim") == 0) {
            flags |= BOOT_ANIM;
            continue;
        }
        printf("Unknown argument detected \"%s\"\n", argv[i]);
        exit(1);
    }
    return flags;
}

void Stop() {
    g_running = FALSE;
}

void ChangeState(AppState state) {
    g_state = state;
    Event e = { 0 };
    e.resize = TRUE;
    state(e);
}

AppState GetState() {
    return g_state;
}

EZ_MUTEX* Lock() {
    return &g_lock;
}

void Run(AppFlags flags) {
    g_flags = flags;
    Initialize();
    Listen();
    Clean();
}
