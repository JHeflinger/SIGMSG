#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdlib.h>

size_t TerminalHeight();

size_t TerminalWidth();

void Wait(size_t milliseconds);

void Clear();

#endif