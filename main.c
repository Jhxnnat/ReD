#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "raylib.h"

#define GW 800
#define GH 450
#define RTEXT_LEFT 72
#define RTEXT_TOP 22
#define RTEXT_LEFT_LINES 18
#define RFONT_SPACING 2
#define RFONT_SIZE 24

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
    for (size_t i = cu->current_line+1; i <= cu->line_num; i++) { //Move other lines when inserting before last line
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

void free_lines(Lines *lines) {
  free(lines->lines);
  lines->lines = NULL;
  lines->capacity = lines->size = 0;
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
  if (cursor->current_line < 0) cursor->current_line = 0;
  else if (cursor->current_line > lines->size) cursor->current_line = lines->size;

  size_t current_line = cursor->current_line;
  size_t _len = lines->lines[current_line].end - lines->lines[current_line].start;
  if (_len <= cursor->line_pos) {
    cursor->pos = lines->lines[current_line].end-1;
    if (_len <= 0) cursor->line_pos = 0;
    else cursor->line_pos = _len-1;
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
      // cursor->selection_line_begin = cursor->selection_line_end;
      cursor->selection_end = cursor->pos;
      cursor->selection_line_end = cursor->current_line;
      printf("this case has been\n");
    }
  }
  else {
    if (cursor->selection_end == prev_pos) { //TODO when selection_line_end != selection_line_begin this does not work the same
      cursor->selection_end = cursor->selection_begin;
      // cursor->selection_line_end = cursor->selection_line_begin;
      cursor->selection_begin = cursor->pos;
      cursor->selection_line_begin = cursor->current_line;
    }
    else if (cursor->selection_end > prev_pos) {
      cursor->selection_begin = cursor->pos;
      cursor->selection_line_begin = cursor->current_line;
    }
  }
}

void paste_text(Text *text, Cursor *cursor, Lines *lines) { //TODO check for, \t, \r etc to insert them manualy
  const char *source = GetClipboardText();
  size_t len = strlen(source);
  
  for (size_t i = 0; i < len; ++i) {
    if (source[i] == '\n') new_line(text, lines, cursor);
    else insert_text(text, source[i], cursor, lines);
  }
}

int main(void) 
{
  Text text;
  init_text(&text, 8);

  Lines lines;
  init_lines(&lines, 10);
  add_line(&lines,0, 0, 0);

  Cursor cursor;
  init_cursor(&cursor);

  InitWindow(GW, GH, "Retro eDitor - [0.0.1]");
  
  Font font = LoadFontEx("./assets/iosevka-font.ttf", RFONT_SIZE, 0, 250);
  // int frames = 0;
  float input_delay = 0.1f; //when key is constantly pressed
  float input_lasttime = 0.0f;
  int font_width = MeasureText("W", font.baseSize);
  Vector2 font_measuring = MeasureTextEx(font, "W", font.baseSize, RFONT_SPACING);
  int key_ = 0;
  while (!WindowShouldClose()) {

    //Update
    //---------------------------------------------
    //
    //writing
    int key = GetCharPressed();
    while (key > 0) {
      if (key >= 32 && key <= 125) {
        insert_text(&text, (char)key, &cursor, &lines);
        // cursor.pos++;
        // cursor.line_pos++;
      }
      for (size_t i = 0; i < lines.size; i++) {
        printf("Line %zu: start = %zu, end = %zu\n", i, lines.lines[i].start, lines.lines[i].end);
      }
      
      key = GetCharPressed();
    }

    float current_time = GetTime();
    if (current_time - input_lasttime >= input_delay) {
      if (IsKeyDown(KEY_RIGHT) && cursor.pos < text.capacity) { 
        cursor_move_h(&cursor, &lines, false);
        input_lasttime = current_time;
      } 
      if (IsKeyDown(KEY_LEFT) && cursor.pos > 0) {
        cursor_move_h(&cursor, &lines, true);
        input_lasttime = current_time;
      }
      if (IsKeyDown(KEY_UP)) {
        cursor_move_v(&cursor, &lines, -1);
        input_lasttime = current_time;
      }
      if (IsKeyDown(KEY_DOWN)) {
        cursor_move_v(&cursor, &lines, 1);
        input_lasttime = current_time;
      }

      if (IsKeyDown(KEY_ENTER)) {
        new_line(&text, &lines, &cursor);

        for (size_t i = 0; i < lines.size; i++) {
          printf("Line %zu: start = %zu, end = %zu\n", 
                 i, lines.lines[i].start, lines.lines[i].end);
        }
        input_lasttime = current_time;
      }

      if (IsKeyDown(KEY_BACKSPACE)) {
        delete_text(&text, &cursor, &lines);
        input_lasttime = current_time;
      }

      if (IsKeyDown(KEY_TAB)) { //TODO this could be a loop that iterates n (indent config) times!
        insert_text(&text, ' ', &cursor, &lines);
        insert_text(&text, ' ', &cursor, &lines);
        input_lasttime = current_time;
      }

      if (IsKeyDown(KEY_DELETE) && text.capacity > 0 && cursor.pos < text.capacity) { 
        cursor_move_h(&cursor, &lines, false);
        delete_text(&text, &cursor, &lines);
        input_lasttime = current_time;
      }

      if (IsKeyDown(KEY_LEFT_SHIFT)) {
        cursor.is_selecting = true;
        if (cursor.selection_end - cursor.selection_begin <= 0) {
          cursor.selection_begin = cursor.pos;
          cursor.selection_end = cursor.pos;
        }
      }
      if (IsKeyUp(KEY_LEFT_SHIFT)) cursor.is_selecting = false;

      if (IsKeyDown(KEY_LEFT_CONTROL)) {
        if (GetKeyPressed() == KEY_V) {
          paste_text(&text, &cursor, &lines);
        }
      }
    }
    
    //Drawing
    //---------------------------------------------
    BeginDrawing();
    ClearBackground(BLACK);
    
    DrawTextEx(font, text.text, (Vector2){ RTEXT_LEFT, RTEXT_TOP }, (float)font.baseSize, RFONT_SPACING, WHITE);

    //lines num
    for (size_t i = 1; i < lines.size+1; ++i) {
      int mul = 3;
      if (i > 9) mul = 2;
      if (i > 99) mul = 1;
      if (i > 999) mul = 0;

      const char *t = TextFormat("%d", i);
      Vector2 pos = { RTEXT_LEFT_LINES+(font_measuring.x*mul), RTEXT_TOP+(font_measuring.y*(i-1))+(RFONT_SPACING*(i-1)) };
      DrawTextEx(font, t, pos, (float)font.baseSize, RFONT_SPACING, GRAY);
    }

    DrawText(TextFormat(
      "> %d, %d,%d          %d|%d|%d    select: %d, %d", 
      cursor.pos, cursor.line_pos, cursor.current_line, cursor.line_num, 
      lines.lines[cursor.current_line].start,
      lines.lines[cursor.current_line].end,
      cursor.selection_begin, cursor.selection_end
    ), 160, GH - 30, 20, ORANGE);

    ////Cursor --------------------
    //MeasureText from start of the line to cursor
    size_t _range = cursor.pos - lines.lines[cursor.current_line].start;
    if (_range <= 0) {
      Vector2 _cursor_pos = { 
        RTEXT_LEFT, 
        RTEXT_TOP+(font_measuring.y*cursor.current_line)+(RFONT_SPACING*cursor.current_line) 
      };
      DrawText("|", _cursor_pos.x, _cursor_pos.y, font.baseSize, RED);
    } else {
      char _part[_range];
      strncpy(_part, text.text+lines.lines[cursor.current_line].start, _range);
      _part[_range] = '\0';
      Vector2 _text_measure = MeasureTextEx(font, _part, font.baseSize, RFONT_SPACING);
      Vector2 _cursor_pos = { 
        RTEXT_LEFT+_text_measure.x, 
        RTEXT_TOP+(font_measuring.y*cursor.current_line)+(RFONT_SPACING*cursor.current_line) 
      };
      DrawText("|", _cursor_pos.x, _cursor_pos.y, font.baseSize, RED);
    }

    //selection
    size_t select_range = cursor.selection_end - cursor.selection_begin;
    size_t select_line_range = cursor.selection_line_end - cursor.selection_line_begin;
    Color select_color = {255, 255, 255, 80 };

    int _x;
    int _w;
    int _y; 
    int _h = font_measuring.y;

    //TODO do something like with the cursor to get the selection right
    //one line selected
    if (select_line_range == 0 && select_range > 0) {
      _x = RTEXT_LEFT+(font_measuring.x*(cursor.selection_begin-lines.lines[cursor.current_line].start));
      _w = (font_measuring.x* (select_range));
      _y = RTEXT_TOP+(font_measuring.y*cursor.current_line)+(RFONT_SPACING*cursor.current_line);
      DrawRectangle(_x, _y, _w, _h, select_color);
    }
    else if (select_line_range > 0 && select_range > 0) {
      for (size_t i = cursor.selection_line_begin; i <= cursor.selection_line_end; ++i) {
        _y = RTEXT_TOP+(font_measuring.y * i);

        if (i == cursor.selection_line_begin) {
          //first line
          _x = RTEXT_TOP+(font_width*(cursor.selection_begin-lines.lines[i].start));
          _w = (font_width*(lines.lines[i].end-cursor.selection_begin));
          DrawRectangle(_x, _y, _w, _h, select_color);
        }
        else if (i == cursor.selection_line_end) {
          //last line
          _x = RTEXT_TOP;
          _w = (font_width*(cursor.selection_end-lines.lines[i].start));
          DrawRectangle(_x, _y, _w, _h, select_color);
        }
        else {
          //between lines
          _x = RTEXT_TOP;
          _w = (font_width*(lines.lines[i].end-lines.lines[i].start));
          DrawRectangle(_x, _y, _w, _h, select_color);
        }
      }
    }

    DrawFPS(20, GH-30);

    EndDrawing();
  }
  
  UnloadFont(font);
  CloseWindow();
  free_text(&text);
  free_lines(&lines);
  return 0;
}

