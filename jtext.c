#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "raylib.h"

#define GW 800
#define GH 450

typedef struct {
  size_t capacity;
  size_t size;
  char *text;
} Text;

typedef struct {
  size_t start;
  size_t end;
} Line;

typedef struct {
  size_t size;
  size_t capacity;
  Line *lines;
} Lines;

typedef struct {
  size_t pos; 
  size_t line_num;
  size_t line_pos;
  size_t current_line;
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
  lines->lines[cu->current_line].end += 1;
  if (cu->current_line < cu->line_num) {
    printf("updating lines!\n");
    for (size_t i = cu->current_line+1; i <= cu->line_num; i++) { //Move other lines when inserting before last line
      lines->lines[i].start++;
      lines->lines[i].end++;
      printf("updating line %zu, start: %zu, end: %zu\n", i, lines->lines[i].start, lines->lines[i].end);
    }
  }
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

void new_line(Lines *lines, Cursor *c) {
  ssize_t start = c->pos;
  size_t end = lines->lines[c->current_line].end;

  lines->lines[c->current_line].end = c->pos;

  c->line_pos = 0;
  c->line_num++;
  c->current_line++;
  add_line(lines, c->current_line, start, end);
}

void free_lines(Lines *lines) {
  free(lines->lines);
  lines->lines = NULL;
  lines->capacity = lines->size = 0;
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

  InitWindow(GW, GH, "JText - Core [0.0.1]");
  SetTargetFPS(60);
  
  Font font = LoadFontEx("assets/cascadia-font.ttf", 20, 0, 250);
  int frames = 0;
  float input_delay = 0.1f; //when key is constantly pressed
  float input_lasttime = 0.0f;
  int key_ = 0;
  while (!WindowShouldClose()) {

    //Update
    //---------------------------------------------
    frames++;
    if (frames > 60) frames = 0;

    //writing
    int key = GetCharPressed();
    while (key > 0) {
      if (key >= 32 && key <= 125) {
        insert_text(&text, (char)key, &cursor, &lines);
        cursor.pos++;
        cursor.line_pos++;
      }
      for (size_t i = 0; i < lines.size; i++) {
        printf("Line %zu: start = %zu, end = %zu\n", i, lines.lines[i].start, lines.lines[i].end);
      }
      
      key = GetCharPressed();
    }

    float current_time = GetTime();
    if (current_time - input_lasttime >= input_delay) {
      if (IsKeyDown(KEY_RIGHT) && cursor.pos < text.capacity) { 
        if (cursor.pos == lines.lines[cursor.current_line].end-1 
          && cursor.current_line < cursor.line_num) {
          cursor.current_line++;
          cursor.line_pos = 0;
          cursor.pos++;
          for (size_t i = 0; i < lines.size; i++) {
            printf("Line %zu: start = %zu, end = %zu\n", i, lines.lines[i].start, lines.lines[i].end);
          }

        } else {
          cursor.pos++;
          cursor.line_pos++;
        }

        input_lasttime = current_time;
      } 
      
      if (IsKeyDown(KEY_LEFT) && cursor.pos > 0) {
        if (cursor.line_pos == 0 && cursor.current_line > 0) {
          cursor.current_line--;
          cursor.line_pos = lines.lines[cursor.current_line].end-lines.lines[cursor.current_line].start-1;
          cursor.pos--;
          for (size_t i = 0; i < lines.size; i++) {
            printf("Line %zu: start = %zu, end = %zu\n", i, lines.lines[i].start, lines.lines[i].end);
          }
        } else {
          cursor.pos--;
          cursor.line_pos--;
        }

        input_lasttime = current_time;
      }
    }

    key_ = GetKeyPressed();
    switch (key_) {
      case KEY_ENTER:
        insert_text(&text, '\n', &cursor, &lines);
        cursor.pos++;
        new_line(&lines, &cursor);

        for (size_t i = 0; i < lines.size; i++) {
          printf("Line %zu: start = %zu, end = %zu\n", 
                 i, lines.lines[i].start, lines.lines[i].end);
        }
      break;
        
      case KEY_BACKSPACE:
        delete_text(&text, &cursor, &lines);
      break;
    }
    key_ = GetKeyPressed();
    
    //Drawing
    //---------------------------------------------
    BeginDrawing();
      ClearBackground(BLACK);
      
      DrawTextEx(font, text.text, (Vector2){ 20.0f, 20.0f, }, (float)font.baseSize, 2, WHITE);

      DrawText(TextFormat(
        "abs: %d, col: %d, Line %d/%d - %d|%d", 
        cursor.pos, cursor.line_pos, cursor.current_line, cursor.line_num, 
        lines.lines[cursor.current_line].start,
        lines.lines[cursor.current_line].end
      ), 160, GH - 30, 20, ORANGE);

      //cursor
      if (frames < 20 || frames > 40) {
        char subtext[cursor.line_pos+1];
        memcpy(subtext, text.text+lines.lines[cursor.current_line].start, cursor.line_pos);
        DrawText("|", 18+(MeasureText("W", 18)*cursor.line_pos), 20+(22*cursor.current_line), 20, RED);
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
