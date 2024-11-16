#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "raylib.h"
#include "ins.h"

#define GW 800
#define GH 450
#define RTEXT_LEFT 72
#define RTEXT_TOP 22
#define RTEXT_LEFT_LINES 18
#define RFONT_SPACING 2
#define RFONT_SIZE 24

void paste_text(Text *text, Cursor *cursor, Lines *lines) { //TODO check for, \t, \r etc to insert them manualy
  const char *source = GetClipboardText();
  size_t len = strlen(source);
  
  for (size_t i = 0; i < len; ++i) {
    if (source[i] == '\n') new_line(text, lines, cursor);
    else if (source[i] == '\0') continue;
    else insert_text(text, source[i], cursor, lines);
  }
}

void copy_text(Text *text, Cursor *cursor, Lines *lines) { //TODO add copy line feature if nothing is selected
  size_t range = cursor->selection_end - cursor->selection_begin;
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

Vector2 measure_text_part(Text *text, Font font, size_t start, size_t range) {
  if (range <= 0) {
    Vector2 measurement_ = { 0, 0 };
    return measurement_;
  }
  char part[range];
  strncpy(part, text->text+start, range);
  part[range] = '\0';
  Vector2 measurement = MeasureTextEx(font, part, font.baseSize, RFONT_SPACING);
  return measurement;
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
  if (!IsFontValid(font)) {
    font = GetFontDefault();
  }
  float input_delay = 0.1f; //when key is constantly pressed
  float input_lasttime = 0.0f;
  // int font_width = MeasureText("W", font.baseSize);
  Vector2 font_measuring = MeasureTextEx(font, "W", font.baseSize, RFONT_SPACING);
  // int key_ = 0;
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
        insert_text(&text, '\t', &cursor, &lines);
        // insert_text(&text, ' ', &cursor, &lines);
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
        if (IsKeyPressed(KEY_V)) {
          paste_text(&text, &cursor, &lines);
        }
        if (IsKeyPressed(KEY_C)) {
          copy_text(&text, &cursor, &lines);
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
      "%d --- col %d, row %d --- [%d, %d] --- select: %d, %d", 
      cursor.pos, cursor.line_pos, cursor.current_line,
      lines.lines[cursor.current_line].start, lines.lines[cursor.current_line].end,
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
    Color select_color = { 255, 255, 255, 80 };

    int _x, _w, _y, _h = font_measuring.y;

    //one line selected
    if (select_line_range == 0 && select_range > 0) {
      size_t _plsize = cursor.selection_begin-lines.lines[cursor.current_line].start;
      Vector2 _selection_l_measure = measure_text_part(&text, font, lines.lines[cursor.current_line].start, _plsize);
      Vector2 _selection_measure = measure_text_part(&text, font, cursor.selection_begin, select_range);
      _x = RTEXT_LEFT+(_selection_l_measure.x);
      _w = _selection_measure.x;
      _y = RTEXT_TOP+(font_measuring.y*cursor.current_line)+(RFONT_SPACING*cursor.current_line);
      DrawRectangle(_x, _y, _w, _h, select_color);
    }
    else if (select_line_range > 0 && select_range > 0) {
      for (size_t i = cursor.selection_line_begin; i <= cursor.selection_line_end; ++i) {
        _y = RTEXT_TOP+(font_measuring.y * i) + (RFONT_SPACING * i);

        if (i == cursor.selection_line_begin) { //first line
          size_t _plsize = cursor.selection_begin-lines.lines[i].start;
          Vector2 _selection_l_measure = measure_text_part(&text, font, lines.lines[i].start, _plsize);
          Vector2 _selection_measure = measure_text_part(&text, font, cursor.selection_begin, lines.lines[i].end - cursor.selection_begin);
          _x = RTEXT_LEFT+_selection_l_measure.x;
          _w = _selection_measure.x;
          DrawRectangle(_x, _y, _w, _h, select_color);
        }
        else if (i == cursor.selection_line_end) { //last line
          Vector2 _selection_measure = measure_text_part(&text, font, lines.lines[i].start, cursor.selection_end-lines.lines[i].start);
          _x = RTEXT_LEFT;
          _w = _selection_measure.x;
          DrawRectangle(_x, _y, _w, _h, select_color);
        }
        else { //between lines
          _x = RTEXT_LEFT;
          size_t _range = lines.lines[i].end - lines.lines[i].start;
          Vector2 _line_measure = measure_text_part(&text, font, lines.lines[i].start, _range);
          _w = _line_measure.x;
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
