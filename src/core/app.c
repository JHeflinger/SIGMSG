#include "app.h"
#include "util/animations.h"
#include "states/login.h"
#include "core/platform.h"
#include <easynet.h>

AppState g_state;
AppFlags g_flags;
BOOL g_running = TRUE;

void Initialize() {
    EZ_INIT_NETWORK();
    if (!(g_flags & NO_BOOT_ANIM)) BootAnimation();
    ChangeState(LoginState);
}

void Clean() {
    EZ_CLEAN_NETWORK();
}

void Listen() {
    #ifdef __WIN32
        HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
        DWORD mode;
        GetConsoleMode(hStdin, &mode);
        mode = ENABLE_EXTENDED_FLAGS;
        SetConsoleMode(hStdin, mode);
        mode = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT | ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT;
        SetConsoleMode(hStdin, mode);
        INPUT_RECORD record;
        DWORD events;
        while (g_running) {
            ReadConsoleInput(hStdin, &record, 1, &events);
            Event e = { 0 };
            if (record.EventType == KEY_EVENT && record.Event.KeyEvent.bKeyDown) {
                e.kevent = record.Event.KeyEvent.uChar.AsciiChar;
            }
            if (record.EventType == MOUSE_EVENT) {
                MOUSE_EVENT_RECORD me = record.Event.MouseEvent;
                e.mevent.x = me.dwMousePosition.X;
                e.mevent.y = me.dwMousePosition.Y;
                e.mevent.type = MOUSE_MOVE;
                if (me.dwEventFlags == 0) {
                    e.mevent.type = MOUSE_UP;
                    if (me.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) {
                        e.mevent.type = MOUSE_LEFT;
                    } else if (me.dwButtonState & RIGHTMOST_BUTTON_PRESSED) {
                        e.mevent.type = MOUSE_RIGHT;
                    }
                } else if (me.dwEventFlags == MOUSE_WHEELED) {
                    e.mevent.type = MOUSE_SCROLL;
                    e.mevent.delta = (SHORT)HIWORD(record.Event.MouseEvent.dwButtonState);
                }
            }
            if (e.kevent == 27)
                Stop();
            else
                g_state(e);
        }
    #elif __linux__
        #error "Finish this portion"
    #else
        #error "Operating system not supported"
    #endif
}

AppFlags GetFlagsFromArgs(int argc, const char** argv) {
    AppFlags flags = NO_APP_FLAGS;
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-no_boot_animation") == 0)
            flags |= NO_BOOT_ANIM;
    }
    return flags;
}

void Stop() {
    g_running = FALSE;
}

void ChangeState(AppState state) {
    g_state = state;
    Event e = { 0 };
    e.empty = TRUE;
    state(e);
}

void Run(AppFlags flags) {
    g_flags = flags;
    Initialize();
    Listen();
    Clean();
}