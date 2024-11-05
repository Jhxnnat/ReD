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
  Line line;
  size_t line_num;
  size_t line_pos;
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
  lines->lines[cu->line_num].end++;
}

void delete_text(Text *t, Cursor *cur) {
  if ((t->capacity <= 0) || (cur->pos <= 0)) return;
 
  cur->pos--;
  cur->line_pos--; //TODO track deleting a line (backspace on pos 0)
  memmove(t->text+cur->pos, t->text+cur->pos+1, t->capacity-cur->pos+1);
  t->text[t->capacity] = '\0';
  t->capacity--;
}

void free_text(Text *t) {
  free(t->text);
  t->text = NULL;
  t->capacity = t->size = 0;
}

void init_cursor(Cursor *c, Line line) {
  c->pos = 0;
  c->line = line;
  c->line_num = 0;
  c->line_pos = 0;
}

void init_lines(Lines *lines, size_t size) {
  lines->size = size;
  lines->capacity = 1;
  lines->lines = (Line *)malloc(size * sizeof(Line));
  // lines->lines[0].start = 0;
  // lines->lines[0].end = 0;
}

void new_line(Lines *lines, Cursor *c) {
  if (lines->capacity == lines->size) {
    size_t new_size = lines->size * 2;
    Line *new_lines = realloc(lines->lines, new_size*sizeof(Line));
    if (new_lines == NULL) {
      fprintf(stderr, "Memory reallocation failed\n");
      free(lines->lines);
      exit(1);
    }
    lines->lines = new_lines;
    lines->size = new_size;
  }

  c->pos++;
  c->line_pos = 0;
  c->line_num++;
  lines->capacity++;
  lines->lines[lines->capacity].start = c->pos;
  lines->lines[lines->capacity].end = c->pos;
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
  init_lines(&lines, 2);

  Cursor cursor;
  init_cursor(&cursor, lines.lines[0]);

  InitWindow(GW, GH, "JText - Core [0.0.1]");
  
  int frames = 0;
  SetTargetFPS(60);

  while (!WindowShouldClose()) {

    //Update
    //---------------------------------------------
    frames++;
    if (frames > 60) frames = 0;

    //writing
    int key = GetCharPressed();
    while (key > 0) {
      //[32...125]
      if ((key >= 32) && key <= 125) {
        insert_text(&text, (char)key, &cursor, &lines);
        cursor.pos++;
        cursor.line_pos++;
      }

            
      key = GetCharPressed();
    }

    //deleting
    if (IsKeyPressed(KEY_BACKSPACE)) {
      delete_text(&text, &cursor);
    }
    
    //enter [\n]
    if (IsKeyPressed(KEY_ENTER)) {
      insert_text(&text, '\n', &cursor, &lines);
      new_line(&lines, &cursor);
      printf("new line");
    }

    //navigation
    if ((IsKeyPressed(KEY_RIGHT)) && (cursor.pos < text.capacity)) {
      cursor.pos++;
    } else if ((IsKeyPressed(KEY_LEFT)) && (cursor.pos > 0)) {
      cursor.pos--;
    }
    
    //Drawing
    //---------------------------------------------
    BeginDrawing();
      ClearBackground(BLACK);
      
      DrawText(text.text, 20, 20, 20, WHITE);

      DrawText(TextFormat("cursor.pos: %d txt.capacity: %d, Line %d", cursor.pos, text.capacity, (int)sizeof(&lines.lines)), 160, GH - 30, 20, ORANGE);

      //cursor
      if (frames < 20 || frames > 40) {
        DrawText("|", 20+(MeasureText(text.text+text.capacity-cursor.line_pos, 20)), 20+(20*cursor.line_num), 20, WHITE);
      }
     
      DrawFPS(20, GH-30);

    EndDrawing();
  }

  CloseWindow();
  free_text(&text);
  free_lines(&lines);

  return 0;
}
