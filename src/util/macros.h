#ifndef MACROS_H
#define MACROS_H

#define amove(y, x) {int amove_x, amove_y; getyx(stdscr, amove_y, amove_x); move(amove_y + y, amove_x + x); }
#define mcollide(e, xp, yp, wp, hp) (e.mevent.x >= (xp) && e.mevent.x <= (xp + wp) && e.mevent.y >= (yp) && e.mevent.y <= (yp + hp))

#endif