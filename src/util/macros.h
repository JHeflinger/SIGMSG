#ifndef MACROS_H
#define MACROS_H

#define amove(y, x) {int amove_x, amove_y; getyx(stdscr, amove_y, amove_x); move(amove_y + y, amove_x + x); }
#define mcollide(e, xp, yp, wp, hp) (e.mevent.x >= (xp) && e.mevent.x < (xp + wp) && e.mevent.y >= (yp) && e.mevent.y < (yp + hp))
#define uuideq(u1, u2) (u1.first == u2.first && u1.second == u2.second)
#define printdest(dest) EZ_INFO("%d:%d:%d:%d %d", dest.address.address[0], dest.address.address[1], dest.address.address[2], dest.address.address[3], dest.port)

#endif
