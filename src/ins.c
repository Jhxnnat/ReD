#include <string.h>
#include <stdio.h>

// #include "raylib.h"
#include "ins.h"
// #include "cam.h"

void insert_text(Text *t, char c, Cursor *cu, Lines *lines) {
  // if (cu->pos < 0) cu->pos = 0;
  // if (cu->pos > t->capacity) cu->pos = t->capacity;

  // inserting in a selection:
  // - delete selected text 
  // - proceed with the insertion 
  const size_t selection_range = cu->selection_end - cu->selection_begin;
  if (selection_range > 0) {
    delete_text(t, cu, lines);
  }

  if (t->capacity >= t->size) { //TODO this should be a separate function!
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
  if (cu->current_line < lines->size-1) {
    for (size_t i = cu->current_line+1; i < lines->size; ++i) { // move other lines when inserting before last line
      lines->lines[i].start++;
      lines->lines[i].end++;
    }
  }

  cu->selection_begin = cu->pos;
  cu->selection_end = cu->pos;
}

char *_delete_text(Text *t, Cursor *cur, Lines *lines) {
  
  /// Deleting a selection: 
  // easy way: put cursor in selection_end, delete char per char in a loop
  // not so easy way: 
  //   if there one of two line selected just do the easy way
  //   if there are multiple lines selected we:
  //      - easy way on final line 
  //      - delete all betwen lines, update selection lines.lines etc 
  //      - easy way on first line 
  //      - memmove 

  //lets go easy way for now 
  size_t repeat = 1;
  const size_t selection_range = cur->selection_end - cur->selection_begin;
  if (selection_range > 0) {
    repeat = selection_range;
    cur->current_line = cur->selection_line_end;
    cur->pos = cur->selection_end;
    cur->line_pos = cur->selection_end - lines->lines[cur->current_line].start;
  }

  char* deleted = (char*)malloc(selection_range+1);

  if ((t->capacity <= 0) || (cur->pos <= 0)) {
    return deleted;
  }

  deleted[selection_range] = '\0';
  if (selection_range > 0) {
    strncpy(deleted, t->text + cur->selection_begin, selection_range);
  }

  while (repeat > 0) {
    repeat--;
    if (cur->line_pos == 0) {
      // cur->line_num--;
      cur->current_line--;
      lines->size--;

      int diff = lines->lines[cur->current_line].end-lines->lines[cur->current_line].start-1;
      cur->line_pos = diff;

      lines->lines[cur->current_line].end = lines->lines[cur->current_line+1].end-1;
      for (size_t i = cur->current_line+1; i < lines->size; ++i) {
        lines->lines[i].start = lines->lines[i+1].start-1;
        lines->lines[i].end = lines->lines[i+1].end-1;
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

  if (selection_range > 0) {
    cur->selection_begin = cur->pos;
    cur->selection_end = cur->pos;
    cur->selection_line_begin = cur->current_line;
    cur->selection_line_end = cur->current_line;
  }

  //NOTE I think this isn't necessary
  // //Move cam up if necessary
  // update_cam_offset_up(cur, lines);

  return deleted;
}

void delete_text(Text *t, Cursor *c, Lines *l) {
  char *_d = _delete_text(t,c,l);
  free(_d);
}

void cut_text(Text *text, Cursor *cursor, Lines *lines) {
  size_t selection_range = cursor->selection_end - cursor->selection_begin;
  if (selection_range > 0) {
    char *_d = _delete_text(text, cursor, lines);
    SetClipboardText(_d);
    free(_d);
  }
}

void resize_lines(Lines *lines) {
    const size_t new_capacity = lines->capacity * 2;
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

void update_lines(Lines *lines, size_t line_num, size_t start, size_t end) { 
  //line num is the position of the line we just inserted
  if (lines->size >= lines->capacity) {
      resize_lines(lines);
  }
  
  lines->lines[line_num].start = start;
  lines->lines[line_num].end = end;

  //update rest of lines
  const size_t right_amount = end - start + 1;
  for (size_t i = line_num+1; i < lines->size; ++i) {
    lines->lines[i].start += right_amount;
    lines->lines[i].end += right_amount;
  }
  lines->size++;
}

void new_line(Text *text, Lines *lines, Cursor *c) {
  insert_text(text, '\n', c, lines); 

  const size_t start = c->pos;
  const size_t end = lines->lines[c->current_line].end;

  lines->lines[c->current_line].end = c->pos;

  c->line_pos = 0;
  c->current_line++;
  selection_reset(c);
  update_lines(lines, c->current_line, start, end);
}


void paste_text(Text *text, Cursor *cursor, Lines *lines) { //TODO check for, \t, \r etc to insert them manualy
  const char *source = GetClipboardText();
  const size_t len = strlen(source);
  
  for (size_t i = 0; i < len; ++i) {
    if (source[i] == '\n') new_line(text, lines, cursor);
    else if (source[i] == '\0') continue;
    else insert_text(text, source[i], cursor, lines);
  }
}

void copy_text(Text *text, Cursor *cursor) { //TODO add copy line feature if nothing is selected
  const size_t range = cursor->selection_end - cursor->selection_begin;
  if (range <= 0) return;
  char copied_text[range];
  strncpy(copied_text, text->text + cursor->selection_begin, range);
  copied_text[range] = '\0';
  printf("copied_text: %s", copied_text);
  SetClipboardText(copied_text);
  cursor->selection_begin = cursor->pos;
  cursor->selection_end = cursor->pos;
  cursor->selection_line_begin = cursor->current_line;
  cursor->selection_line_end = cursor->current_line;
}
