#include "nav.h"
#include <stdio.h>
void __selection_reset(Cursor *cursor) {
  cursor->selection_begin = cursor->pos;
  cursor->selection_end = cursor->pos;
  cursor->selection_line_begin = cursor->current_line;
  cursor->selection_line_end = cursor->current_line;
}
void __selection_move_left(Cursor *cursor) {
  if (cursor->is_selecting && cursor->selection_begin < cursor->pos) {
    cursor->selection_end = cursor->pos;
    cursor->selection_line_end = cursor->current_line;
  }
  else if (cursor->is_selecting && cursor->selection_begin >= cursor->pos) {
    cursor->selection_begin = cursor->pos;
    cursor->selection_line_begin = cursor->current_line;
  }
}
void __selection_move_right(Cursor *cursor) {
  if (cursor->is_selecting && cursor->selection_end > cursor->pos) {
    cursor->selection_begin = cursor->pos;
    cursor->selection_line_begin = cursor->current_line;
  }
  else if (cursor->is_selecting && cursor->selection_end <= cursor->pos) {
    cursor->selection_end = cursor->pos;
    cursor->selection_line_end = cursor->current_line;
  }
}
void __selection_move_up(Cursor *cursor, Lines *lines, size_t prev_pos) {
  if (cursor->selection_end == prev_pos) {
    cursor->selection_end = cursor->selection_begin;
    cursor->selection_line_end = cursor->selection_line_begin;
    cursor->selection_begin = cursor->pos;
    cursor->selection_line_begin = cursor->current_line;
    if (cursor->selection_end < cursor->selection_begin) {
      size_t temp = cursor->selection_end;
      cursor->selection_end = cursor->selection_begin;
      cursor->selection_begin = temp;
    }
  }
  else if (cursor->selection_end > prev_pos) {
    cursor->selection_begin = cursor->pos;
    cursor->selection_line_begin = cursor->current_line;
  }
}
void __selection_move_down(Cursor *cursor, Lines *lines, size_t prev_pos) {
  if (cursor->selection_begin < prev_pos) { 
    cursor->selection_end = cursor->pos;
    cursor->selection_line_end = cursor->current_line;
  }
  else if (cursor->selection_begin == prev_pos) {
    cursor->selection_begin = cursor->selection_end;
    cursor->selection_line_begin = cursor->selection_line_end;
    cursor->selection_end = cursor->pos;
    cursor->selection_line_end = cursor->current_line;
    if (cursor->selection_end < cursor->selection_begin) {
      size_t temp = cursor->selection_end;
      cursor->selection_end = cursor->selection_begin;
      cursor->selection_begin = temp;
    }
  }
}

void __update_cam_offset(Cursor *cursor, Lines *lines, int dir) {
  size_t relative_line = cursor->current_line - lines->offset;
  if (lines->offset < lines->size && dir > 0 && relative_line > MAX_LINES-1) {
    lines->offset++;
  }
  if (lines->offset > 0 && dir < 0 && relative_line <= 0) {
    lines->offset--;
  }
}

void cursor_move_h(Cursor *cursor, Lines *lines, bool left) {
  if (left) {
    if (cursor->line_pos == 0 && cursor->current_line > 0) {
      cursor->current_line--;
      cursor->line_pos = lines->lines[cursor->current_line].end-lines->lines[cursor->current_line].start-1;
      cursor->pos--;
    } else {
      cursor->pos--;
      cursor->line_pos--;
    }
    
    __update_cam_offset(cursor, lines, -1);
    __selection_move_left(cursor);
  } 
  else {
    if (cursor->pos == lines->lines[cursor->current_line].end-1 
      && cursor->current_line < cursor->line_num) {
      cursor->current_line++;
      cursor->line_pos = 0;
      cursor->pos++;
    } else {
      cursor->pos++;
      cursor->line_pos++;
    }

    __update_cam_offset(cursor, lines, 1);
    __selection_move_right(cursor);
  }

  if (!cursor->is_selecting) {
    __selection_reset(cursor);
  }
}

void cursor_move_v(Cursor *cursor, Lines *lines, int dir) {
  if (!cursor->is_selecting) {
    __selection_reset(cursor);
  }

  if (dir > 0 && cursor->current_line == lines->size-1) return;
  if (dir < 0 && cursor->current_line == 0) return;

  size_t prev_pos = cursor->pos;

  size_t prev_line = cursor->current_line;
  cursor->current_line += dir;
  size_t current_line = cursor->current_line;
  size_t _prev_len = lines->lines[prev_line].end - lines->lines[prev_line].start;
  size_t _cur_len = lines->lines[current_line].end - lines->lines[current_line].start;
  if (cursor->line_pos > _cur_len) {
    int _off = (_cur_len < 1) ? 0 : 1;
    cursor->line_pos = _cur_len - _off;
    cursor->pos = lines->lines[current_line].end - _off;
  }
  else {
    //cursor.line_pos remains the same
    cursor->pos = lines->lines[current_line].start + cursor->line_pos;
  }

  //cam-mov TODO handle all cam-navigation cases 
  __update_cam_offset(cursor, lines, dir);

  if (!cursor->is_selecting) return;
  if (dir > 0) {
    __selection_move_down(cursor, lines, prev_pos);
  }
  else {
    __selection_move_up(cursor, lines, prev_pos);
  }
}

void cursor_move_sol(Cursor *cursor, Lines *lines) {
  cursor->pos = lines->lines[cursor->current_line].start;
  cursor->line_pos = 0;

  if (!cursor->is_selecting) __selection_reset(cursor);
  else __selection_move_left(cursor);
}
void cursor_move_eol(Cursor *cursor, Lines *lines) {
  size_t _len = lines->lines[cursor->current_line].end - lines->lines[cursor->current_line].start;
  if (_len > 0) {
    int _off = 1;
    if (lines->size <= 1 || cursor->current_line == lines->size-1) {
      _off = 0;
    }
    cursor->pos = lines->lines[cursor->current_line].end - _off;
    cursor->line_pos = _len;
  }

  if (!cursor->is_selecting) __selection_reset(cursor);
  else __selection_move_right(cursor);
}
void cursor_move_start(Cursor *cursor, Lines *lines) {
  size_t prev_pos = cursor->pos;
  cursor->pos = 0;
  cursor->line_pos = 0;
  cursor->current_line = 0;

  lines->offset = 0;
  if (!cursor->is_selecting) __selection_reset(cursor);
  else __selection_move_up(cursor, lines, prev_pos); 
}
void cursor_move_end(Cursor *cursor, Lines *lines) {
  size_t prev_pos = cursor->pos;
  cursor->current_line = lines->size-1;
  cursor->pos = lines->lines[cursor->current_line].start;
  cursor->line_pos = 0;

  long _off = (long)lines->size - MAX_LINES - 1;
  if (_off < 0) _off = 0;
  lines->offset = (size_t)_off;

  if (!cursor->is_selecting) __selection_reset(cursor); //TODO could get rid of this if and make __selection_reset return 0 if the selection was reset
  else __selection_move_down(cursor, lines, prev_pos);

  // cursor_move_eol(cursor, lines);
}

