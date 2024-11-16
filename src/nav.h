#ifndef NAV_H

#include "ds.h"

void cursor_move_h(Cursor *cursor, Lines *lines, bool left);
void cursor_move_v(Cursor *cursor, Lines *lines, int dir);
#endif // !NAV_H
