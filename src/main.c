#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "nav.h"
#include "../raylib/include/raylib.h"
#include "ins.h"
#include "cam.h"
#include "draw.h"
#include "explorer.h"

int main(int argc, char **argv) 
{
  Text text;
  init_text(&text, 8);
  Lines lines;
  init_lines(&lines, 10);
  Cursor cursor;
  init_cursor(&cursor);

  Camera2D camera = { 0 };
  init_camera(&camera);
  Vector2 cursor_display;
  cursor_display.x = 0;
  cursor_display.y = 0;

  Editor editor;
  Explorer explorer;
  explorer_init(&explorer);
  bool start_write_mode;

  if (argc == 1) {
    explorer_load_path(&explorer, ".");
    start_write_mode = true;
  }
  else if (argc >= 2) {
    const char *path = argv[1];
    if (!DirectoryExists(path) || !FileExists(path)) {
      puts("You may provide a valid file path of directory path");
      exit(69);
    }

    if (IsPathFile(path)) {
      insert_text_from_file(path, &text, &lines, &cursor);
      cursor_move_start(&cursor);
      start_write_mode = true;

      const char *directory = GetDirectoryPath(path);
      explorer_load_path(&explorer, directory);
    }
    else {
      explorer_load_path(&explorer, path);
      start_write_mode = false;
    }
  }

  SetConfigFlags(FLAG_WINDOW_UNDECORATED);
  InitWindow(GW, GH, NAME);
  SetExitKey(0);
  
  Font font = LoadFontEx("./assets/big-blue-term.ttf", RFONT_SIZE, NULL, 0);
  if (!IsFontValid(font)) {
    font = GetFontDefault();
  }
  Vector2 font_measuring = MeasureTextEx(font, "M", font.baseSize, RFONT_SPACING);

  init_editor(&editor, &cursor, &lines, &text, font, GW, GH, start_write_mode);

  update_cursor_display(&cursor_display, &text, &cursor, &lines, font, font_measuring);

  while (!WindowShouldClose()) {
    
    if (IsKeyPressed(KEY_ESCAPE)) {
      editor.write_mode = !editor.write_mode;
    }

    //WARNING put this on input events
    // update_cursor_display(&cursor_display, &text, &cursor, &lines, font, font_measuring);

    //Drawing
    //---------------------------------------------
    BeginDrawing();
    ClearBackground(BLACK);

    if (editor.write_mode) {

      int key = GetCharPressed();
      while (key > 0) {
        printf("-> key: %d\n", key);
        if (key >= 0 && key <= 255) {
          insert_text(&text, (char)key, &cursor, &lines);
          update_cursor_display(&cursor_display, &text, &cursor, &lines, font, font_measuring);
          move_cam_right(&camera, cursor_display.x, font_measuring.y);
        }
        key = GetCharPressed();
      }


      if (IsKeyPressed(KEY_RIGHT) && cursor.pos < text.capacity) { 
        cursor_move_h(&cursor, &lines, false);
        update_cam_offset_down(&cursor, &lines, editor.max_lines);
        move_cam_right(&camera, cursor_display.x, font_measuring.y);
        update_cursor_display(&cursor_display, &text, &cursor, &lines, font, font_measuring);
        if (cursor.line_pos == 0) {
          move_cam_left(&camera, cursor_display.x, font_measuring.y);
        }
      } 
      else if (IsKeyPressed(KEY_LEFT) && cursor.pos > 0) {
        cursor_move_h(&cursor, &lines, true);
        update_cam_offset_up(&cursor, &lines);
        move_cam_left(&camera, cursor_display.x, font_measuring.y);
        update_cursor_display(&cursor_display, &text, &cursor, &lines, font, font_measuring);
        if (cursor.line_pos >= lines.lines[cursor.current_line].end - lines.lines[cursor.current_line].start - 1) {
          move_cam_right(&camera, cursor_display.x, font_measuring.y);
        }
      }
      else if (IsKeyPressed(KEY_UP)) {
        cursor_move_v(&cursor, &lines, -1);
        update_cam_offset_up(&cursor, &lines);
      }
      else if (IsKeyPressed(KEY_DOWN)) {
        cursor_move_v(&cursor, &lines, 1);
        update_cam_offset_down(&cursor, &lines, editor.max_lines);
      }

      if (IsKeyPressed(KEY_ENTER)) {
        new_line(&text, &lines, &cursor);
        update_cam_offset_down(&cursor, &lines, editor.max_lines);
        update_cursor_display(&cursor_display, &text, &cursor, &lines, font, font_measuring);
        move_cam_left(&camera, cursor_display.x, font_measuring.y);
      }

      if (IsKeyPressed(KEY_BACKSPACE)) {
        delete_text(&text, &cursor, &lines);
        update_cursor_display(&cursor_display, &text, &cursor, &lines, font, font_measuring); 
        move_cam_left(&camera, cursor_display.x, font_measuring.y); 
        if (cursor.line_pos >= lines.lines[cursor.current_line].end - lines.lines[cursor.current_line].start - 1) {
          move_cam_right(&camera, cursor_display.x, font_measuring.y);
        }
      }

      if (IsKeyPressed(KEY_TAB)) {
        insert_text(&text, ' ', &cursor, &lines);
        insert_text(&text, ' ', &cursor, &lines);
      }

      if (IsKeyPressed(KEY_DELETE) && text.capacity > 0 && cursor.pos < text.capacity) { 
        cursor_move_h(&cursor, &lines, false);
        delete_text(&text, &cursor, &lines);
      }

      if (IsKeyPressed(KEY_LEFT_SHIFT)) {
        cursor.is_selecting = true;
        if (cursor.selection_end - cursor.selection_begin <= 0) {
          cursor.selection_begin = cursor.pos;
          cursor.selection_end = cursor.pos;
        }
      }
      if (IsKeyReleased(KEY_LEFT_SHIFT)) cursor.is_selecting = false;

      if (IsKeyDown(KEY_LEFT_CONTROL)) {
        if (IsKeyPressed(KEY_C)) copy_text(&text, &cursor);
        else if (IsKeyPressed(KEY_V) && paste_text(&text, &cursor, &lines)) {
          update_cursor_display(&cursor_display, &text, &cursor, &lines, font, font_measuring);
        }
        else if (IsKeyPressed(KEY_X) && cut_text(&text, &cursor, &lines)) {
          update_cursor_display(&cursor_display, &text, &cursor, &lines, font, font_measuring);
        }
        else if (IsKeyPressed(KEY_HOME)) {
          cursor_move_start(&cursor);
          update_cursor_display(&cursor_display, &text, &cursor, &lines, font, font_measuring);
          move_cam_start(&camera, &lines);
        }
        else if (IsKeyPressed(KEY_END)) {
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


      camera.target.y = font_measuring.y*lines.offset + RFONT_SPACING*lines.offset;
      BeginMode2D(camera);

      draw_text_tokenized(text.text, font, (Vector2){RTEXT_LEFT, RTEXT_TOP}, (float)font.baseSize, RFONT_SPACING );
      draw_line_numbers(camera, font, font_measuring, lines);
      DrawText("|", cursor_display.x, cursor_display.y, font.baseSize, RED);
      draw_selection(cursor, lines, text, font, font_measuring);

      EndMode2D();
    } 
    else {
      explorer_input(&explorer);
      if (IsKeyReleased(KEY_ENTER) && explorer.cursor == -1) {
        explorer_load_prevpath(&explorer);
      }
      else if (IsKeyReleased(KEY_ENTER) && explorer.cursor > -1) {
        char *selected_path = explorer.filepath_list.paths[explorer.cursor];
        if (IsPathFile(selected_path) && FileExists(selected_path)) {
          memset(text.text, '\0', text.capacity);
          lines.offset = 0;
          lines.size = 1;
          init_cursor(&cursor); // safe, no malloc stuff
          init_camera(&camera); // ... //
          init_editor(&editor, &cursor, &lines, &text, font, GW, GH, true);
          cursor_display.x = 0; //NOTE cursor_display should be part of editor
          cursor_display.y = 0;
          
          insert_text_from_file(selected_path, &text, &lines, &cursor);
          cursor_move_start(&cursor);
        } 
        else if (DirectoryExists(selected_path)) {
          explorer_load_path(&explorer, selected_path);
        }

      }
      explorer_draw(&explorer, font, font_measuring);
    }
    
    DrawRectangle(0, GH - 40, GW, 40, BLUE);
    DrawText(TextFormat(
      "%d--col:%d,row:%d--[%d, %d]--select: %d,%d--off:%d", 
      cursor.pos, cursor.line_pos, cursor.current_line,
      lines.lines[cursor.current_line].start, lines.lines[cursor.current_line].end,
      cursor.selection_begin, cursor.selection_end,
      lines.offset
    ), 140, GH - 30, 20, BLACK);
    DrawText(TextFormat("%d", GetFPS()), 20, GH-30, 20, BLACK);

    EndDrawing();
  }
  
  UnloadFont(font);
  CloseWindow();
  explorer_free(&explorer);
  free_text(&text);
  free_lines(&lines);
  return 0;
}

