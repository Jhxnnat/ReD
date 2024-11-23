#ifndef NAV_H
#define NAV_H

#include "ds.h"

void cursor_move_h(Cursor *cursor, Lines *lines, bool left);
void cursor_move_v(Cursor *cursor, Lines *lines, int dir);
void cursor_move_sol(Cursor *cursor, Lines *lines);
void cursor_move_eol(Cursor *cursor, Lines *lines);
void cursor_move_start(Cursor *cursor, Lines *lines);
void cursor_move_end(Cursor *cursor, Lines *lines);

#endif // !NAV_H
