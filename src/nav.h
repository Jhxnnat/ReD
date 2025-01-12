#ifndef NAV_H
#define NAV_H

// #include "raylib.h"
#include "../raylib/include/raylib.h"
#include "ds.h"

void cursor_move_h(Cursor *cursor, Lines *lines, bool left);
void cursor_move_v(Cursor *cursor, Lines *lines, int dir);
void cursor_move_sol(Cursor *cursor, Lines *lines);
void cursor_move_eol(Cursor *cursor, Lines *lines);
void cursor_move_start(Cursor *cursor);
void cursor_move_end(Cursor *cursor, Lines *lines);
void update_cursor_display(Editor *e);

void selection_reset(Cursor *cursor);

#endif // !NAV_H
