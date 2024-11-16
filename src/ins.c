#include <string.h>
#include <stdio.h>
#include "ins.h"

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


void resize_lines(Lines *lines) {
    size_t new_capacity = lines->capacity * 2;
    Line *new_lines = realloc(lines->lines, sizeof(Line) * new_capacity);
    if (new_lines == NULL) {
        // Handle realloc failure
        fprintf(stderr, "Memory reallocation failed\n");
        free(lines->lines); // free original allocation if realloc fails
        exit(1);
    }

    lines->lines = new_lines;
    lines->capacity = new_capacity;
}

void add_line(Lines *lines, size_t line_num, size_t start, size_t end) { //line num is where we want to add
    if (lines->size == lines->capacity) {
        resize_lines(lines);
    }
    
    //update rest of lines
    size_t prev_start;
    size_t prev_end;
    for (size_t i = line_num; i < lines->size; ++i) { 
      prev_start = lines->lines[i].start;
      prev_end = lines->lines[i].end;
      lines->lines[i+1].start = prev_start;
      lines->lines[i+1].end = prev_end;
    }  

    lines->lines[line_num].start = start;
    lines->lines[line_num].end = end;
    lines->size++;
}

void new_line(Text *text, Lines *lines, Cursor *c) {
  insert_text(text, '\n', c, lines); 

  size_t start = c->pos;
  size_t end = lines->lines[c->current_line].end;

  lines->lines[c->current_line].end = c->pos;

  c->line_pos = 0;
  c->line_num++;
  c->current_line++;
  c->selection_line_begin = c->current_line;
  c->selection_line_end = c->current_line;
  add_line(lines, c->current_line, start, end);
}
