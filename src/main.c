#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "nav.h"
// #include "raylib.h"
#include "../raylib/include/raylib.h"
#include "ins.h"
#include "cam.h"
#include "draw.h"

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

size_t clamp(size_t n, size_t min, size_t max){
  size_t m = n;
  if (n > max) m = max;
  else if (n < min) m = min;
  return m;
}

int main(int argc, char **argv) 
{
  Text text;
  init_text(&text, 8); //BUG if text is initialized after loading file some bug causes text to have problems when drawing
  Lines lines;
  init_lines(&lines, 10);
  // add_line(&lines,0, 0, 0);
  Cursor cursor;
  init_cursor(&cursor);

  Camera2D camera = { 0 };
  init_camera(&camera);
  Vector2 cursor_display;
  cursor_display.x = 0;
  cursor_display.y = 0;

  Editor editor;

  char *FILE_PATH;
  if (argc == 1) {
    FILE_PATH = NULL;
  }
  else if (argc == 2) {
    FILE_PATH = argv[1];
    printf("\nFILE_PATH: %s\n", FILE_PATH);
    char *source = LoadFileText(FILE_PATH);
    size_t len = strlen(source);
    for (size_t i = 0; i < len; ++i) {
      if (source[i] == '\n' || source[i] == '\r') new_line(&text, &lines, &cursor);
      else if (source[i] == '\t') {
        insert_text(&text, '-', &cursor, &lines);
        insert_text(&text, '-', &cursor, &lines);
      }
      else if (source[i] == '\0') break;
      else insert_text(&text, source[i], &cursor, &lines);
    }
    cursor_move_start(&cursor);
    UnloadFileText(source);
  }
  else {
    printf("\nUSAGE: red [FILE_PATH]\n");
    exit(1);
  }

  SetConfigFlags(FLAG_WINDOW_UNDECORATED);
  InitWindow(GW, GH, NAME);
  
  Font font = LoadFontEx("./assets/big-blue-term.ttf", RFONT_SIZE, NULL, 0);
  if (!IsFontValid(font)) {
    font = GetFontDefault();
  }
  float input_delay = 0.1f; //when key is constantly pressed
  float input_lasttime = 0.0f;
  Vector2 font_measuring = MeasureTextEx(font, "M", font.baseSize, RFONT_SPACING);

  init_editor(&editor, GW, GH, font_measuring.y);

  update_cursor_display(&cursor_display, &text, &cursor, &lines, font, font_measuring);

  // scanner(
  //   "#include <stdio.h>\n"
  //   "\n"
  //   "int main (void) {\n"
  //   "   printf(\"holaaa\")\n"
  //   "   return 0;\n"
  //   "}\n"
  // );

  while (!WindowShouldClose()) {
    //writing
    int key = GetCharPressed();
    while (key > 0) {
      if (key >= 0 && key <= 255) {
        insert_text(&text, (char)key, &cursor, &lines);
        update_cursor_display(&cursor_display, &text, &cursor, &lines, font, font_measuring);
        move_cam_right(&camera, cursor_display.x, font_measuring.y);
      }
      key = GetCharPressed();
    }

    float current_time = GetTime();
    if (current_time - input_lasttime >= input_delay) {
      if (IsKeyDown(KEY_RIGHT) && cursor.pos < text.capacity) { 
        cursor_move_h(&cursor, &lines, false);
        input_lasttime = current_time;

        update_cam_offset_down(&cursor, &lines, editor.max_lines);
        move_cam_right(&camera, cursor_display.x, font_measuring.y);

        update_cursor_display(&cursor_display, &text, &cursor, &lines, font, font_measuring);
        if (cursor.line_pos == 0) {
          move_cam_left(&camera, cursor_display.x, font_measuring.y);
        }
      } 
      else if (IsKeyDown(KEY_LEFT) && cursor.pos > 0) {
        cursor_move_h(&cursor, &lines, true);
        input_lasttime = current_time;

        update_cam_offset_up(&cursor, &lines);
        move_cam_left(&camera, cursor_display.x, font_measuring.y);

        update_cursor_display(&cursor_display, &text, &cursor, &lines, font, font_measuring);
        if (cursor.line_pos >= lines.lines[cursor.current_line].end - lines.lines[cursor.current_line].start - 1) {
          move_cam_right(&camera, cursor_display.x, font_measuring.y);
        }
      }
      else if (IsKeyDown(KEY_UP)) {
        input_lasttime = current_time;
        cursor_move_v(&cursor, &lines, -1);

        update_cam_offset_up(&cursor, &lines);
      }
      else if (IsKeyDown(KEY_DOWN)) {
        input_lasttime = current_time;
        cursor_move_v(&cursor, &lines, 1); //WARNING make cursor_move_up/down

        update_cam_offset_down(&cursor, &lines, editor.max_lines);
      }

      if (IsKeyDown(KEY_ENTER)) {
        new_line(&text, &lines, &cursor);
        input_lasttime = current_time;

        update_cam_offset_down(&cursor, &lines, editor.max_lines);

        update_cursor_display(&cursor_display, &text, &cursor, &lines, font, font_measuring);
        move_cam_left(&camera, cursor_display.x, font_measuring.y);
      }

      if (IsKeyDown(KEY_BACKSPACE)) {
        delete_text(&text, &cursor, &lines);
        input_lasttime = current_time;

        update_cursor_display(&cursor_display, &text, &cursor, &lines, font, font_measuring);//----- WARNING Duplicated Code
        move_cam_left(&camera, cursor_display.x, font_measuring.y); //------------------------------ WARNING Duplicated Code
        if (cursor.line_pos >= lines.lines[cursor.current_line].end - lines.lines[cursor.current_line].start - 1) { //------
          move_cam_right(&camera, cursor_display.x, font_measuring.y);
        }
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
        if (IsKeyPressed(KEY_V)) {
          paste_text(&text, &cursor, &lines);
        }
        if (IsKeyPressed(KEY_C)) {
          copy_text(&text, &cursor);
        }
        if (IsKeyPressed(KEY_X)) {
          cut_text(&text, &cursor, &lines);
        }

        if (IsKeyPressed(KEY_HOME)) {
          cursor_move_start(&cursor);
          
          move_cam_start(&camera, &lines);
        }
        if (IsKeyPressed(KEY_END)) {
          cursor_move_end(&cursor, &lines);

          update_cursor_display(&cursor_display, &text, &cursor, &lines, font, font_measuring);
          move_cam_end(&camera, &lines, editor.max_lines);
          move_cam_right(&camera, cursor_display.x, font_measuring.y);
        }
      }

      if (IsKeyPressed(KEY_HOME)) {
        cursor_move_sol(&cursor, &lines);

        update_cursor_display(&cursor_display, &text, &cursor, &lines, font, font_measuring);
        move_cam_left(&camera, cursor_display.x, font_measuring.y);
      }
      if (IsKeyPressed(KEY_END)) {
        cursor_move_eol(&cursor, &lines);

        update_cursor_display(&cursor_display, &text, &cursor, &lines, font, font_measuring);
        move_cam_right(&camera, cursor_display.x, font_measuring.y);
      }
    }

    //WARNING put this on input events
    update_cursor_display(&cursor_display, &text, &cursor, &lines, font, font_measuring);

    //Drawing
    //---------------------------------------------
    BeginDrawing();
    ClearBackground(BLACK);
    
    BeginMode2D(camera);
    camera.target.y = font_measuring.y*lines.offset + RFONT_SPACING*lines.offset;

    ////Main Text
    /// 
    scanner(text.text, font, (Vector2){RTEXT_LEFT, RTEXT_TOP}, (float)font.baseSize, RFONT_SPACING );

    ////Lines num-------------------------------NOTE consider draw a part of the lines to optimize 
    //back
    DrawRectangle(camera.target.x, camera.target.y, RTEXT_LEFT - 3, GH, BLACK);
    // lines
    DrawRectangle(camera.target.x + RTEXT_LEFT-6, camera.target.y, 3, GH, ORANGE);
    DrawRectangle(camera.target.x - 3, camera.target.y, 3, GH, ORANGE);
    DrawRectangle(camera.target.x + GW-3, camera.target.y, 3, GH, ORANGE);

    for (size_t i = 1; i < lines.size+1; ++i) {
      const char *t = TextFormat("%d", i);
      Vector2 _meassure_t = MeasureTextEx(font, t, font.baseSize, RFONT_SPACING);
      Vector2 pos = { camera.target.x + RTEXT_LEFT-(_meassure_t.x)-10, RTEXT_TOP+(font_measuring.y*(i-1))+(RFONT_SPACING*(i-1)) };
      DrawTextEx(font, t, pos, (float)font.baseSize, RFONT_SPACING, GRAY);
    }

    ////Cursor
    DrawText("|", cursor_display.x, cursor_display.y, font.baseSize, RED);

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

    EndMode2D();

    // Status bar 
    
    DrawRectangle(0, GH - 40, GW, 40, BLUE);
    DrawText(TextFormat(
      "%d--col:%d,row:%d--[%d, %d]--select: %d,%d--off:%d", 
      cursor.pos, cursor.line_pos, cursor.current_line,
      lines.lines[cursor.current_line].start, lines.lines[cursor.current_line].end,
      cursor.selection_begin, cursor.selection_end,
      lines.offset
    ), 140, GH - 30, 20, BLACK);

    DrawText(TextFormat("%d", GetFPS()), 20, GH-30, 20, BLACK);
    // DrawFPS(20, GH-30);

    EndDrawing();
  }
  
  UnloadFont(font);
  CloseWindow();
  free_text(&text);
  free_lines(&lines);
  return 0;
}

