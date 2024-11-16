#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <raylib.h>
#include "text.h"

void init_text(Text *t, size_t size) {
  t->text = malloc(size * sizeof(char));
  t->size = size;
  t->capacity = 0;
}

void insert_text(Text *t, char c, Cursor *cu, Lines *lines) {
  if (cu->pos < 0) cu->pos = 0;
  if (cu->pos > t->capacity) cu->pos = (int)(t->capacity);

  if (t->capacity >= t->size) {
    t->size *= 2;
    char *temp = realloc(t->text, (t->size) * sizeof(char));
    if (temp == NULL) {
      printf("Error realloc!!!\n");
      exit(0);
    } else { 
      t->text = temp;
    }
  }
  t->capacity++;
  memmove(t->text+cu->pos+1, t->text+cu->pos, t->capacity-cu->pos+1);
  t->text[cu->pos] = c;
  cu->pos++;
  cu->line_pos++;
  lines->lines[cu->current_line].end += 1;
  if (cu->current_line < cu->line_num) {
    printf("updating lines!\n");
    for (size_t i = cu->current_line+1; i <= cu->line_num; i++) { // move other lines when inserting before last line
      lines->lines[i].start++;
      lines->lines[i].end++;
      printf("updating line %zu, start: %zu, end: %zu\n", i, lines->lines[i].start, lines->lines[i].end);
    }
  }

  cu->selection_begin = cu->pos;
  cu->selection_end = cu->pos;
}

void delete_text(Text *t, Cursor *cur, Lines *lines) {
  if ((t->capacity <= 0) || (cur->pos <= 0)) return; 
  
  if (cur->line_pos == 0) {
    cur->line_num--;
    cur->current_line--;
    cur->line_pos = lines->lines[cur->current_line].end-lines->lines[cur->current_line].start-1;
    lines->lines[cur->current_line].end = lines->lines[cur->current_line+1].end-1;
    lines->size--;
    for (size_t i = cur->current_line+1; i < lines->size; i++) {
      lines->lines[i].start = lines->lines[i+1].start;
      lines->lines[i].end = lines->lines[i+1].end;
    }
  } 
  else {
    cur->line_pos--;
    lines->lines[cur->current_line].end--;
    for (size_t i = cur->current_line+1; i <= lines->size; ++i) {
      lines->lines[i].start--;
      lines->lines[i].end--;
    }
  }
  cur->pos--;
  memmove(t->text+cur->pos, t->text+cur->pos+1, t->capacity-cur->pos+1);
  t->text[t->capacity] = '\0';
  t->capacity--;
}

void free_text(Text *t) {
  free(t->text);
  t->text = NULL;
  t->capacity = t->size = 0;
}

