#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "nav.h"
#include "../raylib/include/raylib.h"
#include "ins.h"
#include "cam.h"
#include "draw.h"
#include "explorer.h"

//undo & redo stacks with some max size
// when to push to undo stack?
// -> on (before) backspace
// -> on inserting {} [] () 
// -> on inserting two \n and writting?
//

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
        if (!DirectoryExists(path) && !FileExists(path)) {
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

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(GW, GH, NAME);
    SetWindowMinSize(400, 300);
    SetExitKey(0);
    SetTargetFPS(60);

    Shader crt = LoadShader(0, "./assets/shader/crt.glsl");
    float sh_rh = GetShaderLocation(crt, "renderHeight");
    SetShaderValue(crt, sh_rh, &ScreenH, SHADER_UNIFORM_FLOAT);

    Font font = LoadFontEx("./assets/fonts/BigBlueTerminal/BigBlueTermPlusNerdFontMono-Regular.ttf", RFONT_SIZE, NULL, 250);
    if (!IsFontValid(font)) {
        font = GetFontDefault();
    }

    Vector2 font_measuring = MeasureTextEx(font, "M", font.baseSize, RFONT_SPACING);

    explorer.lines_amount = (GH/font_measuring.y)-4;
    init_editor(&editor, &cursor, &lines, &text, font, GW, GH, start_write_mode);
    update_cursor_display(&cursor_display, &text, &cursor, &lines, font, font_measuring);
    push_undo(&editor, cursor, lines, text.text);

    RenderTexture2D tex = LoadRenderTexture(GW, GH);

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_ESCAPE)) {
            editor.write_mode = !editor.write_mode;
        }

        BeginDrawing();
        ClearBackground(BLACK);

        if (editor.write_mode) {
            int key = GetCharPressed();
            while (key > 0) {
                if (key >= 0 && key <= 255) {
                    insert_text(&text, (char)key, &cursor, &lines);
                    update_cursor_display(&cursor_display, &text, &cursor, &lines, font, font_measuring);
                    move_cam_right(&camera, cursor_display.x, font_measuring.y);
                }
                key = GetCharPressed();
            }

            if (IsKeyDown(KEY_LEFT_ALT)) {
                if (IsKeyPressed(KEY_L) && cursor.pos < text.size) { 
                    cursor_move_h(&cursor, &lines, false);
                    update_cam_offset_down(&cursor, &lines, editor.max_lines);
                    move_cam_right(&camera, cursor_display.x, font_measuring.y);
                    update_cursor_display(&cursor_display, &text, &cursor, &lines, font, font_measuring);
                    if (cursor.column == 0) {
                        move_cam_left(&camera, cursor_display.x, font_measuring.y);
                    }
                } 
                else if (IsKeyPressed(KEY_J) && cursor.pos > 0) {
                    cursor_move_h(&cursor, &lines, true);
                    update_cam_offset_up(&cursor, &lines);
                    move_cam_left(&camera, cursor_display.x, font_measuring.y);
                    update_cursor_display(&cursor_display, &text, &cursor, &lines, font, font_measuring);
                    if (cursor.column >= lines.lines[cursor.current_line].end - lines.lines[cursor.current_line].start - 1) {
                        move_cam_right(&camera, cursor_display.x, font_measuring.y);
                    }
                }
                else if (IsKeyPressed(KEY_I)) {
                    cursor_move_v(&cursor, &lines, -1);
                    update_cursor_display(&cursor_display, &text, &cursor, &lines, font, font_measuring);
                    update_cam_offset_up(&cursor, &lines);
                }
                else if (IsKeyPressed(KEY_K)) {
                    cursor_move_v(&cursor, &lines, 1);
                    update_cursor_display(&cursor_display, &text, &cursor, &lines, font, font_measuring);
                    update_cam_offset_down(&cursor, &lines, editor.max_lines);
                }

                else if (IsKeyPressed(KEY_C)) copy_text(&text, &cursor);
                else if (IsKeyPressed(KEY_V) && paste_text(&text, &cursor, &lines)) {
                    update_cursor_display(&cursor_display, &text, &cursor, &lines, font, font_measuring);
                }
                else if (IsKeyPressed(KEY_X) && cut_text(&text, &cursor, &lines)) {
                    update_cursor_display(&cursor_display, &text, &cursor, &lines, font, font_measuring);
                }

                else if (IsKeyPressed(KEY_A)) {
                    cursor_move_sol(&cursor, &lines);
                    update_cursor_display(&cursor_display, &text, &cursor, &lines, font, font_measuring);
                    move_cam_left(&camera, cursor_display.x, font_measuring.y);
                }
                else if (IsKeyPressed(KEY_D)) {
                    cursor_move_eol(&cursor, &lines);
                    update_cursor_display(&cursor_display, &text, &cursor, &lines, font, font_measuring);
                    move_cam_right(&camera, cursor_display.x, font_measuring.y);
                }
                else if (IsKeyPressed(KEY_W)) {
                    cursor_move_start(&cursor);
                    update_cursor_display(&cursor_display, &text, &cursor, &lines, font, font_measuring);
                    move_cam_start(&camera, &lines);
                }
                else if (IsKeyPressed(KEY_S)) {
                    cursor_move_end(&cursor, &lines);
                    cursor_move_eol(&cursor, &lines);
                    update_cursor_display(&cursor_display, &text, &cursor, &lines, font, font_measuring);
                    move_cam_end(&camera, &lines, editor.max_lines);
                    move_cam_right(&camera, cursor_display.x, font_measuring.y);
                }
            }
      
            if (IsKeyPressed(KEY_ENTER)) {
                new_line(&text, &lines, &cursor);
                update_cam_offset_down(&cursor, &lines, editor.max_lines);
                update_cursor_display(&cursor_display, &text, &cursor, &lines, font, font_measuring);
                move_cam_left(&camera, cursor_display.x, font_measuring.y);

                push_undo(&editor, cursor, lines, text.text);
            }

            if (IsKeyPressed(KEY_BACKSPACE)) {
                delete_text(&text, &cursor, &lines);
                update_cursor_display(&cursor_display, &text, &cursor, &lines, font, font_measuring); 
                move_cam_left(&camera, cursor_display.x, font_measuring.y); 
                if (cursor.column >= lines.lines[cursor.current_line].end - lines.lines[cursor.current_line].start - 1) {
                    move_cam_right(&camera, cursor_display.x, font_measuring.y);
                }
            }

            if (IsKeyPressed(KEY_TAB)) {
                insert_text(&text, ' ', &cursor, &lines);
                insert_text(&text, ' ', &cursor, &lines);
                update_cursor_display(&cursor_display, &text, &cursor, &lines, font, font_measuring);
            }

            if (IsKeyPressed(KEY_DELETE) && text.size > 0 && cursor.pos < text.size) { 
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
                if (IsKeyPressed(KEY_Z) && editor.stack_top > 0) {
                    push_redo(&editor, cursor, lines, text.text);
                    Change undo = editor.stack[--editor.stack_top];
                    printf("[undo]::\n%s\n", undo.text);

                    memset(text.text, '\0', text.size); 
                    lines.offset = 0;
                    lines.size = 1;
                    text.size = 0;
                    init_cursor(&cursor); // safe, no malloc stuff
                    init_camera(&camera); // ... //
                    
                    for (size_t i = 0; i < strlen(undo.text); ++i) {
                        if (undo.text[i] == '\n') new_line(&text, &lines, &cursor);
                        else insert_text(&text, undo.text[i], &cursor, &lines);
                    }

                    cursor.pos = undo.cursor_pos;
                    cursor.column = undo.col;
                    cursor.current_line = undo.line;
                    //hori_off...
                    lines.offset = undo.vert_off;

                    update_cursor_display(&cursor_display, &text, &cursor, &lines, font, font_measuring);

                    free(undo.text);
                } 
                else if (IsKeyPressed(KEY_Y) && editor.stack_top_redo > 0) {
                    push_undo(&editor, cursor, lines, text.text);
                    Change redo = editor.stack_r[--editor.stack_top_redo];
                    printf("[redo]:: %s\n", redo.text);

                    //TODO make a reset function of something similar
                    memset(text.text, '\0', text.size); 
                    lines.offset = 0;
                    lines.size = 1;
                    text.size = 0;
                    init_cursor(&cursor); // safe, no malloc stuff
                    init_camera(&camera); // ... //

                    for (size_t i = 0; i < strlen(redo.text); ++i) {
                        if (redo.text[i] == '\n') new_line(&text, &lines, &cursor);
                        else insert_text(&text, redo.text[i], &cursor, &lines);
                    }

                    cursor.pos = redo.cursor_pos;
                    cursor.column = redo.col;
                    cursor.current_line = redo.line;
                    lines.offset = redo.vert_off;

                    update_cursor_display(&cursor_display, &text, &cursor, &lines, font, font_measuring);
                    free(redo.text);
                }
                
            }

            camera.target.y = font_measuring.y*lines.offset + RFONT_SPACING*lines.offset;
            BeginTextureMode(tex);
            DrawRectangle(0, 0, GW, GH, RBLACK);
            BeginMode2D(camera);

            draw_text_tokenized_optimized(
                text.text, lines,
                font, font_measuring,
                (Vector2){RTEXT_LEFT, RTEXT_TOP},
                (float)font.baseSize,
                RFONT_SPACING
            );

            DrawRectangle(cursor_display.x, cursor_display.y, font_measuring.x, font_measuring.y, RWHITE);
            const char cu = text.text[cursor.pos];
            DrawTextEx(font, TextFormat("%c", cu), cursor_display, font.baseSize, RFONT_SPACING, RBLACK);

            draw_selection(cursor, lines, text, font, font_measuring);

            const char *status_text = TextFormat("%ix%i, %zu", (int)ScreenW, (int)ScreenH, cursor.column);
            Vector2 status_measure = MeasureTextEx(font, status_text, font.baseSize, RFONT_SPACING);
            Vector2 status_pos = {
                camera.target.x + GW - status_measure.x - 10, 
                camera.target.y + GH - font_measuring.y - 10
            };
            DrawTextEx(font, status_text, status_pos, font.baseSize, RFONT_SPACING, RGRAY);

            DrawRectangle(camera.target.x, camera.target.y, RTEXT_LEFT - 3, GH, RBLACK);
            draw_line_numbers(camera, font, font_measuring, lines);

            EndMode2D();
            EndTextureMode();

            BeginShaderMode(crt);
                DrawTextureRec(tex.texture, (Rectangle){0,0,tex.texture.width,-tex.texture.height}, (Vector2){0,0}, WHITE);
            EndShaderMode();
        } 
        else {
            explorer_input(&explorer);
            if (IsKeyReleased(KEY_ENTER) && explorer.cursor == -1) {
                explorer_load_prevpath(&explorer);
            }
            else if (IsKeyReleased(KEY_ENTER) && explorer.cursor > -1) {
                char *selected_path = explorer.filepath_list.paths[explorer.cursor];
                if (IsPathFile(selected_path) && FileExists(selected_path)) {
                    memset(text.text, '\0', text.size); 
                    lines.offset = 0;
                    lines.size = 1;
                    text.size = 0;
                    init_cursor(&cursor); // safe, no malloc stuff
                    init_camera(&camera); // ... //
                    init_editor(&editor, &cursor, &lines, &text, font, GW, GH, true);

                    insert_text_from_file(selected_path, &text, &lines, &cursor);
                    cursor_move_start(&cursor);
                    update_cursor_display(&cursor_display, &text, &cursor, &lines, font, font_measuring);
                } 
                else if (DirectoryExists(selected_path)) {
                    explorer_load_path(&explorer, selected_path);
                }
            }

            BeginTextureMode(tex);
                DrawRectangle(0, 0, GW, GH, RBLACK);
                explorer_draw(&explorer, font, font_measuring);
            EndTextureMode();
            BeginShaderMode(crt);
                DrawTextureRec(tex.texture, (Rectangle){0,0,tex.texture.width,-tex.texture.height}, (Vector2){0,0}, WHITE);
            EndShaderMode();
        }

    
        // DrawRectangle(0, GH - 40, GW, 40, BLUE);
        // DrawText(TextFormat(
        //   "%d--col:%d,row:%d--[%d, %d]--select: %d,%d--off:%d", 
        //   cursor.pos, cursor.column, cursor.current_line,
        //   lines.lines[cursor.current_line].start, lines.lines[cursor.current_line].end,
        //   cursor.selection_begin, cursor.selection_end,
        //   lines.offset
        // ), 140, GH - 30, 20, BLACK);
        DrawText(TextFormat("%d - offset: %zu", GetFPS(), lines.offset), 20, GH-30, 20, RBLUE);

        EndDrawing();

        if (IsWindowResized()) {
            ScreenW = GetScreenWidth();
            ScreenH = GetScreenHeight();
            UnloadRenderTexture(tex);
            tex = LoadRenderTexture(GW, GH);

            SetShaderValue(crt, sh_rh, &ScreenH, SHADER_UNIFORM_FLOAT);
        }

    }

    UnloadRenderTexture(tex);
  
    UnloadShader(crt);
    UnloadFont(font);
    CloseWindow();
    free_undo(&editor);
    explorer_free(&explorer);
    free_text(&text);
    free_lines(&lines);
    return 0;
}

