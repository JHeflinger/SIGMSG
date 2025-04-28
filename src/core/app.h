#ifndef APP_H
#define APP_H

#include <easybool.h>

typedef enum {
    MOUSE_NONE = 0,
    MOUSE_LEFT,
    MOUSE_RIGHT,
    MOUSE_UP,
    MOUSE_MOVE,
    MOUSE_SCROLL,
} MouseEventType;

typedef struct {
    MouseEventType type;
    int x;
    int y;
    int delta;
} MouseEvent;

typedef struct {
    MouseEvent mevent;
    char kevent;
    BOOL empty;
} Event;

typedef void (*AppState)(Event);

typedef enum {
    NO_APP_FLAGS = 0,
    NO_BOOT_ANIM = 1 << 0,
} AppFlags;

AppFlags GetFlagsFromArgs(int argc, const char** argv);

void Stop();

void ChangeState(AppState state);

void Run(AppFlags flags);

#endif