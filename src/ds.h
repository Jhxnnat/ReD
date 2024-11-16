#ifndef DS_H

#include <stdlib.h>
#include <stdbool.h>

typedef struct {
  size_t capacity;
  size_t size;
  char   *text;
} Text;

typedef struct {
  size_t start;
  size_t end;
} Line;

typedef struct {
  size_t size;
  size_t capacity;
  Line   *lines;
} Lines;

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

void init_text(Text *t, size_t size);
void free_text(Text *t);
void init_cursor(Cursor *c);
void init_lines(Lines *lines, size_t initial_capacity);
void free_lines(Lines *lines);

#endif // !DS_H
