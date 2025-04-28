#include "login.h"
#include "core/app.h"
#include "util/terminal.h"
#include <easylogger.h>
#include <string.h>

void LoginState(Event event) {
    size_t twidth = TerminalWidth();
    size_t theight = TerminalHeight();
    if (event.kevent == 0) {
        Clear();
        char front_buffer[4096] = { 0 };
        EZ_ASSERT(twidth < 4096, "Terminal width is unusually long");
        memset(front_buffer, '\n', theight/2 - 7);
        printf(front_buffer);
        memset(front_buffer, '\0', theight/2 - 7);
        memset(front_buffer, 32, twidth/2 - 28);
        printf("%s%s  _________.___  ________    _____    _________ ________\n", front_buffer, EZ_RED);
        printf("%s /   _____/|   |/  _____/   /     \\  /   _____//  _____/\n", front_buffer);
        printf("%s \\_____  \\ |   /   \\  ___  /  \\ /  \\ \\_____  \\/   \\  ___\n", front_buffer);
        printf("%s /        \\|   \\    \\_\\  \\/    Y    \\/        \\    \\_\\  \\\n", front_buffer);
        printf("%s/_______  /|___|\\______  /\\____|__  /_______  /\\______  /\n", front_buffer);
        printf("%s        \\/             \\/         \\/        \\/        \\/\n%s", front_buffer, EZ_RESET);
        memset(front_buffer, 0, twidth/2 - 28);
        memset(front_buffer, 32, twidth/2 - 10);
        printf("\n%s\xda\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xb7\n", front_buffer);
        printf("%s\xb3    LOG-IN (l)    \xba\n", front_buffer);
        printf("%s\xd4\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xbc\n\n", front_buffer);
        printf("%s\xda\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xb7\n", front_buffer);
        printf("%s\xb3     QUIT (q)     \xba\n", front_buffer);
        printf("%s\xd4\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xbc\n", front_buffer);
    } else if (event.kevent == 'q') {
        Clear();
        Stop();
    }
}