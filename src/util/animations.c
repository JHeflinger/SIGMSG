#include "animations.h"
#include "util/terminal.h"
#include <easylogger.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

void Type(float speed, const char* format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[4096];
    vsnprintf(buffer, 4096, format, args);
    va_end(args);
    size_t blen = strnlen(buffer, 4096);
    for (size_t i = 0; i < blen; i++) {
        printf("%c", buffer[i]);
        Wait(speed);
    }
}

void BootAnimation() {
    Clear();
    size_t theight = TerminalHeight();
    for (size_t i = 0; i < theight + 1; i++) printf("\n");
    Type(50, "Booting up");
    Type(500, "...\n");
    Type(5, "[%sERROR%s]", EZ_RED, EZ_RESET);
    Type(20, " - undefined behavior detected.\n");
    Type(20, "it was");
    Type(500, "...\n");
    Clear();
    size_t twidth = TerminalWidth();
    for (size_t i = 0; i < theight/2 - 1; i++) printf("\n");
    for (size_t i = 0; i < twidth/2 - 4; i++) printf(" ");
    Type(100, "a %sSIGMSG%s", EZ_RED, EZ_RESET);
    Wait(2000);
    Clear();
    for (int i = 0; i < 50000; i++) {
        int r = (rand()%96) + 32;
        if (r == 127) r = '\n';
        printf("%c", (char)r);
    }
    int waittime = 100;
    for (size_t i = 0; i < theight; i++) Type(1, "\n");
    for (size_t i = 0; i < theight/2; i++) Type(waittime, "\n");
    for (size_t i = 0; i < twidth/2 - 37; i++) printf(" ");
    printf("               .__                                  __\n");
    for (size_t i = 0; i < twidth/2 - 37; i++) printf(" ");
    Wait(waittime);
    printf("__  _  __ ____ |  |   ____  ____   _____   ____   _/  |_  ____\n");
    for (size_t i = 0; i < twidth/2 - 37; i++) printf(" ");
    Wait(waittime);
    printf("\\ \\/ \\/ // __ \\|  | _/ ___\\/  _ \\ /     \\_/ __ \\  \\   __\\/  _ \\\n");
    for (size_t i = 0; i < twidth/2 - 37; i++) printf(" ");
    Wait(waittime);
    printf(" \\     /\\  ___/|  |_\\  \\__(  <_> )  Y Y  \\  ___/   |  | (  <_> )\n");
    for (size_t i = 0; i < twidth/2 - 37; i++) printf(" ");
    Wait(waittime);
    printf("  \\/\\_/  \\___  >____/\\___  >____/|__|_|  /\\___  >  |__|  \\____/ /\\ /\\ /\\\n");
    for (size_t i = 0; i < twidth/2 - 37; i++) printf(" ");
    Wait(waittime);
    printf("             \\/          \\/            \\/     \\/                \\/ \\/ \\/\n");
    for (size_t i = 0; i < twidth/2 - 29; i++) printf(" ");
    Wait(waittime);
    printf("%s  _________.___  ________    _____    _________ ________\n", EZ_RED);
    for (size_t i = 0; i < twidth/2 - 29; i++) printf(" ");
    Wait(waittime);
    printf(" /   _____/|   |/  _____/   /     \\  /   _____//  _____/\n");
    for (size_t i = 0; i < twidth/2 - 29; i++) printf(" ");
    Wait(waittime);
    printf(" \\_____  \\ |   /   \\  ___  /  \\ /  \\ \\_____  \\/   \\  ___\n");
    for (size_t i = 0; i < twidth/2 - 29; i++) printf(" ");
    Wait(waittime);
    printf(" /        \\|   \\    \\_\\  \\/    Y    \\/        \\    \\_\\  \\\n");
    for (size_t i = 0; i < twidth/2 - 29; i++) printf(" ");
    Wait(waittime);
    printf("/_______  /|___|\\______  /\\____|__  /_______  /\\______  /\n");
    for (size_t i = 0; i < twidth/2 - 29; i++) printf(" ");
    Wait(waittime);
    printf("        \\/             \\/         \\/        \\/        \\/\n%s", EZ_RESET);
    for (size_t i = 0; i < theight + 1; i++) Type(waittime, "\n");
    Clear();
}