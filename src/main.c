#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "nav.h"
#include "../raylib/include/raylib.h"
#include "ins.h"
#include "cam.h"
#include "draw.h"
#include "explorer.h"

void editor_reset(Text *text, Cursor *cursor, Lines *lines, Camera2D *camera) {
    memset(text->buff, 0, text->size); 
    text->size = 0;
    init_cursor(cursor); // no malloc
    init_camera(camera); // ... //
    for (size_t n = 0; n < lines->size; ++n) {
        lines->lines[n].start = 0;
        lines->lines[n].end = 0;
    }
    lines->size = 1;
    lines->offset = 0;
}

char mode_to_str(EditorMode mode) {
    switch (mode) {
        case WRITE: return 'W';
        case FIND: return 'F';
    }
    return '?';
}

void result_update(Editor *editor, Camera2D *camera){
    printf("[editor.result_pos]: %d :: [editor.result_line]: %d\n", editor->result_pos, editor->result_line);
    editor->cursor->pos = editor->result_pos;
    editor->cursor->current_line = editor->result_line;
    editor->cursor->column = editor->cursor->pos - editor->lines->lines[editor->cursor->current_line].start;
    update_cam(camera, editor);
}

int main(int argc, char **argv)
{
    Text text;
    init_text(&text, TEXT_INIT_SIZE);
    Lines lines;
    init_lines(&lines, LINES_INIT_SIZE);
    Cursor cursor;
    init_cursor(&cursor);

    Camera2D camera = { 0 };
    init_camera(&camera);
    // Vector2 cursor_display;
    // cursor_display.x = 0;
    // cursor_display.y = 0;

    Editor editor;
    Explorer explorer;
    explorer_init(&explorer);
    bool start_explorer_open;

    if (argc == 1) {
        explorer_load_path(&explorer, ".");
        start_explorer_open = true;
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
            start_explorer_open = true;

            const char *directory = GetDirectoryPath(path);
            explorer_load_path(&explorer, directory);
        }
        else {
          explorer_load_path(&explorer, path);
          start_explorer_open = false;
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
    init_editor(&editor, &cursor, &lines, &text, font, GW, GH, start_explorer_open);
    update_cursor_display(&editor);
    // push_undo(&editor, cursor, lines, (char *)text.buff);

    RenderTexture2D tex = LoadRenderTexture(GW, GH);

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_ESCAPE) && editor.mode != FIND) {
            editor.explorer_open = !editor.explorer_open;
        }

        BeginDrawing();
        ClearBackground(BLACK);

        if (editor.explorer_open) {
            switch(editor.mode) {
                case FIND: {
                    // SearchResult result;
                    static int __pos = -1;
                    if (IsKeyPressed(KEY_ESCAPE)) {
                        editor.mode = WRITE;
                        editor.result_pos = -1;
                        memset(editor.search_promp, '\0', editor.search_len);
                        editor.search_len = 0;
                        break;
                    }
                    int key = GetCharPressed();
                    while (key > 0 && key <= 255 && strlen(editor.search_promp) < 255) {
                        editor.search_promp[editor.search_len++] = (char)(key);
                        editor.result = kmp_search(editor, editor.search_promp, editor.search_len);
                        if (editor.search_len > 0 && editor.result.np > 0) {
                            __pos = (editor.result.np > 1) ? 1 : 0;
                            editor.result_pos = editor.result.p[0];
                            editor.result_line = editor.result.l[0];
                            result_update(&editor, &camera);
                            update_cursor_display(&editor);
                        }
                        key = GetCharPressed();
                    }
                    if (IsKeyPressed(KEY_BACKSPACE) && editor.search_len > 0) {
                        editor.search_promp[--editor.search_len] = '\0';
                        editor.result = kmp_search(editor, editor.search_promp, editor.search_len);
                        if (editor.search_len > 0 && editor.result.np > 0) {
                            __pos = (editor.result.np > 1) ? 1 : 0;
                            editor.result_pos = editor.result.p[0];
                            editor.result_line = editor.result.l[0];
                            result_update(&editor, &camera);
                            update_cursor_display(&editor);
                        }
                    }
                    if (__pos > -1 && IsKeyDown(RKEY_ACTION) && IsKeyPressed(KEY_N)) {
                        if (__pos == editor.result.np) { 
                            __pos = 1;
                            editor.result_pos = editor.result.p[0];
                            editor.result_line = editor.result.l[0];
                        }
                        else if (__pos < editor.result.np) {
                            editor.result_pos = editor.result.p[__pos++];
                            editor.result_line = editor.result.l[__pos-1];
                        }
                        result_update(&editor, &camera);
                        update_cursor_display(&editor);
                    }
                    break;
                };
                case WRITE: {
                    int key = GetCharPressed();
                    while (key > 0 && key <= 255) {
                        insert_text(&text, key, &cursor, &lines);
                        for (size_t i = 0; i < text.size; ++i) {
                            printf("> %d\n", text.buff[i]);
                        }
                        printf("--\n");
                        update_cursor_display(&editor);
                        move_cam_right(&camera, editor.cursor_display.x, font_measuring.y);
                        key = GetCharPressed();
                    }
                    if (IsKeyPressed(KEY_ENTER)) {
                        new_line(&text, &lines, &cursor);
                        update_cam_offset_down(&cursor, &lines, editor.max_lines);
                        update_cursor_display(&editor);
                        // move_cam_left(&camera, editor.cursor_display.x, font_measuring.y);
                        update_cam(&camera, &editor);

                        push_undo(&editor, cursor, lines, (char *)text.buff);
                    }
                    else if (IsKeyPressed(KEY_BACKSPACE)) {
                        delete_text(&text, &cursor, &lines);
                        update_cursor_display(&editor);
                        update_cam(&camera, &editor);
                        //TODO take on count pressin backspace on first line, cam should go up
                        // move_cam_left(&camera, editor.cursor_display.x, font_measuring.y); 
                        // if (cursor.column >= lines.lines[cursor.current_line].end - lines.lines[cursor.current_line].start - 1) {
                        //     move_cam_right(&camera, editor.cursor_display.x, font_measuring.y);
                        // }
                    }
                    else if (IsKeyPressed(KEY_TAB)) {
                        insert_text(&text, ' ', &cursor, &lines);
                        insert_text(&text, ' ', &cursor, &lines);
                        update_cursor_display(&editor);
                    }
                    else if (IsKeyPressed(KEY_DELETE) && text.size > 0 && cursor.pos < text.size) { 
                        cursor_move_h(&cursor, &lines, false);
                        delete_text(&text, &cursor, &lines);
                    }

                    if (IsKeyDown(RKEY_ACTION)) {
                        if (IsKeyPressed(KEY_L) && cursor.pos < text.size) { 
                            cursor_move_h(&cursor, &lines, false);
                            update_cam_offset_down(&cursor, &lines, editor.max_lines);
                            update_cursor_display(&editor);
                            update_cam(&camera, &editor);
                            // move_cam_right(&camera, editor.cursor_display.x, font_measuring.y);
                            // if (cursor.column == 0) {
                            //     move_cam_left(&camera, editor.cursor_display.x, font_measuring.y);
                            // }
                        } 
                        else if (IsKeyPressed(KEY_J) && cursor.pos > 0) {
                            cursor_move_h(&cursor, &lines, true);
                            update_cam_offset_up(&cursor, &lines);
                            update_cursor_display(&editor);
                            update_cam(&camera, &editor);
                            // move_cam_left(&camera, editor.cursor_display.x, font_measuring.y);
                            // if (cursor.column >= lines.lines[cursor.current_line].end - lines.lines[cursor.current_line].start - 1) {
                            //     move_cam_right(&camera, editor.cursor_display.x, font_measuring.y);
                            // }
                        }
                        else if (IsKeyPressed(KEY_I)) {
                            cursor_move_v(&cursor, &lines, -1);
                            update_cursor_display(&editor);
                            update_cam(&camera, &editor);
                            // update_cam_offset_up(&cursor, &lines);
                        }
                        else if (IsKeyPressed(KEY_K)) {
                            cursor_move_v(&cursor, &lines, 1);
                            update_cursor_display(&editor);
                            update_cam(&camera, &editor);
                            // update_cam_offset_down(&cursor, &lines, editor.max_lines);
                        }

                        else if (IsKeyPressed(KEY_C)) copy_text(&text, &cursor);
                        else if (IsKeyPressed(KEY_V) && paste_text(&text, &cursor, &lines)) {
                            update_cursor_display(&editor);
                            update_cam(&camera, &editor);
                        }
                        else if (IsKeyPressed(KEY_X) && cut_text(&text, &cursor, &lines)) {
                            update_cursor_display(&editor);
                            update_cam(&camera, &editor);
                        }

                        else if (IsKeyPressed(KEY_A)) {
                            cursor_move_sol(&cursor, &lines);
                            update_cursor_display(&editor);
                            move_cam_left(&camera, editor.cursor_display.x, font_measuring.y);
                        }
                        else if (IsKeyPressed(KEY_D)) {
                            cursor_move_eol(&cursor, &lines);
                            update_cursor_display(&editor);
                            move_cam_right(&camera, editor.cursor_display.x, font_measuring.y);
                        }
                        else if (IsKeyPressed(KEY_W)) {
                            cursor_move_start(&cursor);
                            update_cursor_display(&editor);
                            move_cam_start(&camera, &lines);
                        }
                        else if (IsKeyPressed(KEY_S)) {
                            if (IsKeyUp(KEY_LEFT_SHIFT)) {
                                cursor_move_end(&cursor, &lines);
                                update_cursor_display(&editor);
                                update_cam(&camera, &editor);
                                // move_cam_end(&camera, &lines, editor.max_lines);
                            } else if (strlen(explorer.current_file) > 0) {
                                SaveFileText(explorer.current_file, (char *)text.buff);
                            }
                        }
                        else if (IsKeyPressed(KEY_F)) {
                            editor.mode = FIND;
                        }
                        else if (IsKeyPressed(KEY_Z) && editor.stack_top > 0) {
                            push_redo(&editor, cursor, lines, (char *)text.buff);
                            Change undo = editor.stack[--editor.stack_top];
                            printf("[undo]::\n%s\n", undo.text);

                            editor_reset(&text, &cursor, &lines, &camera);
                            
                            for (size_t i = 0; i < strlen(undo.text); ++i) {
                                if (undo.text[i] == '\n') new_line(&text, &lines, &cursor);
                                else insert_text(&text, undo.text[i], &cursor, &lines);
                            }

                            cursor.pos = undo.cursor_pos;
                            cursor.column = undo.col;
                            cursor.current_line = undo.line;
                            //hori_off...
                            lines.offset = undo.vert_off;

                            update_cursor_display(&editor);

                            free(undo.text);
                        } else if (IsKeyPressed(KEY_Y) && editor.stack_top_redo > 0) {
                            push_undo(&editor, cursor, lines, (char *)text.buff);
                            Change redo = editor.stack_r[--editor.stack_top_redo];
                            printf("[redo]:: %s\n", redo.text);

                            editor_reset(&text, &cursor, &lines, &camera);

                            for (size_t i = 0; i < strlen(redo.text); ++i) {
                                if (redo.text[i] == '\n') new_line(&text, &lines, &cursor);
                                else insert_text(&text, redo.text[i], &cursor, &lines);
                            }
                            cursor.pos = redo.cursor_pos;
                            cursor.column = redo.col;
                            cursor.current_line = redo.line;
                            lines.offset = redo.vert_off;
                            update_cursor_display(&editor);
                            free(redo.text);
                        }
                    }
                    if (IsKeyPressed(KEY_LEFT_SHIFT)) {
                        cursor.is_selecting = true;
                        if (cursor.selection_end - cursor.selection_begin <= 0) {
                            cursor.selection_begin = cursor.pos;
                            cursor.selection_end = cursor.pos;
                        }
                    }
                    if (IsKeyReleased(KEY_LEFT_SHIFT)) cursor.is_selecting = false;
                    break;
                }
            }

            camera.target.y = font_measuring.y*lines.offset + RFONT_SPACING*lines.offset;
            BeginTextureMode(tex);
            DrawRectangle(0, 0, GW, GH, RBLACK);
            BeginMode2D(camera);

            if (ACTIVE_HIGHLIGHTING) {
                draw_text_tokenized_optimized(text.buff, lines, font, font_measuring, (Vector2){ RTEXT_LEFT, RTEXT_TOP }, font.baseSize, RFONT_SPACING);
            } else {
                draw_text_optimized(text, lines, font, font_measuring, (Vector2){ RTEXT_LEFT, RTEXT_TOP }, (float)font.baseSize, RFONT_SPACING, RWHITE);
            }

            DrawRectangle(editor.cursor_display.x, editor.cursor_display.y, font_measuring.x, font_measuring.y, RWHITE);
            if (text.buff[cursor.pos] != '\n' && text.size > cursor.pos) {
                DrawTextCodepoint(font, text.buff[cursor.pos], editor.cursor_display, editor.font.baseSize, RBLACK);
            }

            draw_selection(cursor, lines, text, font, font_measuring);

            //statusbar
            const char *status_text = TextFormat("[%c] %s  %ix%i, %zu, :: %f", 
                                        mode_to_str(editor.mode),
                                        explorer.current_file, 
                                        (int)ScreenW, (int)ScreenH, 
                                        cursor.column, editor.cursor_display.x);
            Vector2 status_measure = MeasureTextEx(font, status_text, font.baseSize, RFONT_SPACING);
            Vector2 status_pos = {
                camera.target.x + GW - status_measure.x - 10, 
                camera.target.y + GH - font_measuring.y - 10
            };
            DrawTextEx(font, status_text, status_pos, font.baseSize, RFONT_SPACING, RGRAY);

            DrawRectangle(camera.target.x, camera.target.y, RTEXT_LEFT - 3, GH, RBLACK);
            draw_line_numbers(camera, font, font_measuring, lines);

            if (editor.mode == FIND) {
                //find-highlight
                if (editor.result.np > 0) {
                    Vector2 _m = MeasureTextEx(font, editor.search_promp, font.baseSize, RFONT_SPACING);
                    DrawRectangle(editor.cursor_display.x, editor.cursor_display.y, _m.x, _m.y, RORANGE);
                    DrawTextEx(font, TextFormat("%s", editor.search_promp), editor.cursor_display, font.baseSize, RFONT_SPACING, RBLACK);
                }
                //find-status
                Vector2 find_pos = {
                    camera.target.x + 10, 
                    camera.target.y + GH - font_measuring.y - 10
                };
                DrawTextEx(font, TextFormat("> %s", editor.search_promp), find_pos, font.baseSize, RFONT_SPACING, RGRAY);
            }

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
                    strcpy(explorer.current_file, selected_path);
                    editor_reset(&text, &cursor, &lines, &camera);
                    insert_text_from_file(selected_path, &text, &lines, &cursor);
                    cursor_move_start(&cursor);
                    update_cursor_display(&editor);
                    editor.explorer_open = true;
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

        //debug purpose   
        // DrawRectangle(0, GH - 40, GW, 40, BLUE);
        // DrawText(TextFormat(
        //   "%d--col:%d,row:%d--[%d, %d]--select: %d,%d--off:%d", 
        //   cursor.pos, cursor.column, cursor.current_line,
        //   lines.lines[cursor.current_line].start, lines.lines[cursor.current_line].end,
        //   cursor.selection_begin, cursor.selection_end,
        //   lines.offset
        // ), 140, GH - 30, 20, BLACK);
        // DrawText(TextFormat("%d - offset: %zu", GetFPS(), lines.offset), 20, GH-30, 20, RBLUE);

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

