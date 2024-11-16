#ifndef CURSOR_H

#include <stdlib.h>

typedef struct {
  size_t pos; 
  size_t line_num; //TODO change this to: row or line_amount
  size_t line_pos; //TODO change this to: col (column)
  size_t current_line;
  bool   is_selecting;
  size_t selection_begin; 
  size_t selection_end;
  size_t selection_line_begin;
  size_t selection_line_end;
} Cursor;

#endif // !CURSOR_H
