#ifndef DS_H
#define DS_H

#include <stdlib.h>
#include <stdbool.h>

#define NAME "Retro eDitor - [0.0.1]"
#define GW 800
#define GH 600
#define RTEXT_LEFT 72
#define RTEXT_TOP 22
#define RTEXT_LEFT_LINES 10
#define RFONT_SPACING 2
#define RFONT_SIZE 24
#define MAX_LINES 11

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
  size_t offset; //WARNING this can be vert_offset on Editor
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

typedef struct {
  // int margin_top;
  int max_lines;
  int hori_offset;
} Editor;

void init_text(Text *t, size_t size);
void free_text(Text *t);
void init_cursor(Cursor *c);
void init_lines(Lines *lines, size_t initial_capacity);
void free_lines(Lines *lines);

void init_editor(Editor *editor, int window_w, int window_h, int font_height);

#endif // !DS_H

