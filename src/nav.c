#include <stdio.h>
#include <string.h>

#include "nav.h"

void swap_values(size_t *x, size_t *y) {
  size_t temp = *x;
  *x = *y;
  *y = temp;
}

void selection_reset(Cursor *cursor) {
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
void __selection_move_up(Cursor *cursor, size_t prev_pos) {
  if (cursor->selection_end == prev_pos) {
    cursor->selection_end = cursor->pos;
    cursor->selection_line_end = cursor->current_line;
    if (cursor->selection_end < cursor->selection_begin) {
      swap_values(&cursor->selection_end, &cursor->selection_begin);
      swap_values(&cursor->selection_line_end, &cursor->selection_line_begin);
    }
  }
  else if (cursor->selection_begin == prev_pos) {
    cursor->selection_begin = cursor->pos;
    cursor->selection_line_begin = cursor->current_line;
  }
}
void __selection_move_down(Cursor *cursor, size_t prev_pos) {
  if (cursor->selection_end == prev_pos) { 
    cursor->selection_end = cursor->pos;
    cursor->selection_line_end = cursor->current_line;
  }
  else if (cursor->selection_begin == prev_pos) {
    cursor->selection_begin = cursor->pos;
    cursor->selection_line_begin = cursor->current_line;
    if (cursor->selection_begin > cursor->selection_end) {
      swap_values(&cursor->selection_end, &cursor->selection_begin);
      swap_values(&cursor->selection_line_end, &cursor->selection_line_begin);
    }
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
    
    __selection_move_left(cursor);
  } 
  else {
    if (cursor->pos == lines->lines[cursor->current_line].end-1 && cursor->current_line < lines->size-1) {
      cursor->current_line++;
      cursor->line_pos = 0;
      cursor->pos++;
    } else {
      cursor->pos++;
      cursor->line_pos++;
    }

    __selection_move_right(cursor);
  }

  if (!cursor->is_selecting) {
    selection_reset(cursor);
  }
}

void cursor_move_v(Cursor *cursor, Lines *lines, int dir) {
  if (dir > 0 && cursor->current_line == lines->size-1) return;
  if (dir < 0 && cursor->current_line == 0) return;

  size_t prev_pos = cursor->pos;
  cursor->current_line += dir;
  size_t current_line = cursor->current_line;
  size_t next_line_len = lines->lines[current_line].end - lines->lines[current_line].start;

  if (cursor->line_pos > next_line_len) {
    int _off = (next_line_len < 1) ? 0 : 1;
    cursor->line_pos = next_line_len - _off;
    cursor->pos = lines->lines[current_line].end - _off;
  }
  else {
    cursor->pos = lines->lines[current_line].start + cursor->line_pos;
  }

  if (!cursor->is_selecting) {
    selection_reset(cursor);
    return;
  }
  if (dir > 0) {
    __selection_move_down(cursor, prev_pos);
  }
  else {
    __selection_move_up(cursor, prev_pos);
  }
}

void cursor_move_sol(Cursor *cursor, Lines *lines) {
  cursor->pos = lines->lines[cursor->current_line].start;
  cursor->line_pos = 0;

  if (!cursor->is_selecting) selection_reset(cursor);
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

  if (!cursor->is_selecting) selection_reset(cursor);
  else __selection_move_right(cursor);
}
void cursor_move_start(Cursor *cursor) {
  size_t prev_pos = cursor->pos;
  cursor->pos = 0;
  cursor->line_pos = 0;
  cursor->current_line = 0;

  // lines->offset = 0;
  if (!cursor->is_selecting) selection_reset(cursor);
  else __selection_move_up(cursor, prev_pos); 
}
void cursor_move_end(Cursor *cursor, Lines *lines) {
  size_t prev_pos = cursor->pos;
  cursor->current_line = lines->size-1;
  cursor->pos = lines->lines[cursor->current_line].start;
  cursor->line_pos = 0;

  if (!cursor->is_selecting) selection_reset(cursor);
  else __selection_move_down(cursor, prev_pos);
}

void update_cursor_display(Vector2 *cursor_display, Text *text, Cursor *cursor, Lines *lines, Font font, Vector2 font_measuring) {
  size_t _range = cursor->pos - lines->lines[cursor->current_line].start;
  if (_range <= 0) {
    cursor_display->x = RTEXT_LEFT;
    cursor_display->y = RTEXT_TOP+(font_measuring.y*(cursor->current_line))+(RFONT_SPACING*(cursor->current_line));
  } else {
    char _part[_range];
    strncpy(_part, text->text+lines->lines[cursor->current_line].start, _range);
    _part[_range] = '\0';
    Vector2 _text_measure = MeasureTextEx(font, _part, font.baseSize, RFONT_SPACING);
    cursor_display->x = RTEXT_LEFT+_text_measure.x;
    cursor_display->y = RTEXT_TOP+(font_measuring.y*(cursor->current_line))+(RFONT_SPACING*cursor->current_line);
  }
}
