#ifndef APP_H
#define APP_H

#include <easybool.h>
#include <easythreads.h>

typedef enum {
    MOUSE_NONE = 0,
    MOUSE_LEFT_DOWN,
    MOUSE_MIDDLE_DOWN,
    MOUSE_RIGHT_DOWN,
    MOUSE_LEFT_CLICK,
    MOUSE_MIDDLE_CLICK,
    MOUSE_RIGHT_CLICK,
    MOUSE_LEFT_UP,
    MOUSE_MIDDLE_UP,
    MOUSE_RIGHT_UP,
    MOUSE_SCROLL,
    MOUSE_MOVE,
    MOUSE_OTHER
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
    BOOL resize;
    BOOL recieve;
} Event;

typedef void (*AppState)(Event);

typedef enum {
    NO_APP_FLAGS = 0,
    BOOT_ANIM = 1 << 0,
} AppFlags;

AppFlags GetFlagsFromArgs(int argc, const char** argv);

void Stop();

void ChangeState(AppState state);

AppState GetState();

EZ_MUTEX* Lock();

void Run(AppFlags flags);

#endif
