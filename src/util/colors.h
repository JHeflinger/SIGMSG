#ifndef COLORS_H
#define COLORS_H

typedef enum {
    RED_BLACK = 1,
	BLACK_WHITE = 2,
	BLACK_GRAY = 3,
    GREEN_WHITE = 4,
    GRAY_BLACK = 5,
    GRAY_WHITE = 6,
    WHITE_RED = 7,
    RED_WHITE = 8,
} ColorPair;

void InitializeColors();

#endif
