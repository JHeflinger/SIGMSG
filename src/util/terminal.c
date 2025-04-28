#include "terminal.h"
#include "core/platform.h"
#include <easylogger.h>

size_t TerminalHeight() {
    #ifdef __WIN32
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
            EZ_FATAL("Not attached to a valid terminal");
            exit(1);
        }
        return csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    #elif __linux__
        struct winsize w;
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
            EZ_FATAL("Not attached to a valid terminal");
            exit(1);
        }
        return w.ws_row;
    #else
        #error "Operating system not supported"
    #endif
}

size_t TerminalWidth() {
    #ifdef __WIN32
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
            EZ_FATAL("Not attached to a valid terminal");
            exit(1);
        }
        return csbi.srWindow.Right - csbi.srWindow.Left + 1;
    #elif __linux__
        struct winsize w;
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
            EZ_FATAL("Not attached to a valid terminal");
            exit(1);
        }
        return w.ws_col;
    #else
        #error "Operating system not supported"
    #endif
}

void Wait(size_t milliseconds) {
    #ifdef __WIN32
        Sleep(milliseconds);
    #elif __linux__
        usleep(milliseconds);
    #else
        #error "Operating system not supported"
    #endif
}

void Clear() {
    #ifdef __WIN32
        system("cls");
    #elif __linux__
        system("clear");
    #else
        #error "Operating system not supported"
    #endif
}