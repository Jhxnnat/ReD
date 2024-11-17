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
    lines->size = 0;
    lines->capacity = initial_capacity;
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
