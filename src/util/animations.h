#ifndef ANIMATIONS_H
#define ANIMATIONS_H

#include <stddef.h>

void Wait(size_t milliseconds);

void Type(float speed, const char* format, ...);

void BootAnimation();

#endif