#include "nav.h"

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

    if (cursor->is_selecting && cursor->selection_begin < cursor->pos) {
      cursor->selection_end = cursor->pos;
      cursor->selection_line_end = cursor->current_line;
    }
    else if (cursor->is_selecting && cursor->selection_begin >= cursor->pos) {
      cursor->selection_begin = cursor->pos;
      cursor->selection_line_begin = cursor->current_line;
    }
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

    if (cursor->is_selecting && cursor->selection_end > cursor->pos) {
      cursor->selection_begin = cursor->pos;
      cursor->selection_line_begin = cursor->current_line;
    }
    else if (cursor->is_selecting && cursor->selection_end <= cursor->pos) {
      cursor->selection_end = cursor->pos;
      cursor->selection_line_end = cursor->current_line;
    }

  }

  if (!cursor->is_selecting) {
    cursor->selection_begin = cursor->pos;
    cursor->selection_end = cursor->pos;
    cursor->selection_line_begin = cursor->current_line;
    cursor->selection_line_end = cursor->current_line;
  }
}

void cursor_move_v(Cursor *cursor, Lines *lines, int dir) {
  if (!cursor->is_selecting) {
    cursor->selection_begin = cursor->pos;
    cursor->selection_end = cursor->pos;
    cursor->selection_line_begin = cursor->current_line;
    cursor->selection_line_end = cursor->current_line;
  }

  if (dir > 0 && cursor->current_line == lines->size-1) return;
  if (dir < 0 && cursor->current_line == 0) return;

  size_t prev_pos = cursor->pos;

  size_t prev_line = cursor->current_line;
  cursor->current_line += dir;
  size_t current_line = cursor->current_line;
  size_t _prev_len = lines->lines[prev_line].end - lines->lines[prev_line].start;
  size_t _cur_len = lines->lines[current_line].end - lines->lines[current_line].start;
  if (_cur_len < _prev_len) {
    if (_cur_len <= 0) { 
      cursor->line_pos = 0; 
      cursor->pos = lines->lines[current_line].start;
    }
    else {
      cursor->line_pos = _cur_len-1;
      cursor->pos = lines->lines[current_line].end-1;
    }
  }
  else {
    //cursor.line_pos remains the same
    cursor->pos = lines->lines[current_line].start + cursor->line_pos;
  }

  if (!cursor->is_selecting) return;
  if (dir > 0) {
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
  else {
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
}

