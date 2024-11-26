#include <stdio.h>
#include "ds.h"

void init_text(Text *t, size_t size) {
  t->text = malloc(size * sizeof(char));
  t->text[size] = '\0';
  t->size = size;
  t->capacity = 0;
}

void free_text(Text *t) {
  free(t->text);
  t->text = NULL;
  t->capacity = t->size = 0;
}

void init_cursor(Cursor *c) {
  c->pos = 0;
  c->line_num = 0;
  c->line_pos = 0;
  c->current_line = 0;
  c->is_selecting = false;
  c->selection_begin = 0;
  c->selection_end = 0;
  c->selection_line_begin = 0;
  c->selection_line_end = 0;
}

void init_lines(Lines *lines, size_t initial_capacity) {
  lines->size = 1;
  lines->capacity = initial_capacity;
  lines->offset = 0;
  lines->lines = (Line *)malloc(sizeof(Line) * initial_capacity);
  if (lines->lines == NULL) {
      // Handle malloc failure
      fprintf(stderr, "Memory allocation failed\n");
      exit(1);
  }
}

void free_lines(Lines *lines) {
  free(lines->lines);
  lines->lines = NULL;
  lines->capacity = lines->size = 0;
}

void init_editor(Editor *editor, int window_w, int window_h, int font_height) {
  if (window_h < 0 || window_w < 0) {
    printf("window size must be positive\n");
    exit(44);
  }
  editor->max_lines = window_h / font_height;
  editor->max_lines -= 4;

  editor->hori_offset = 0;
}
