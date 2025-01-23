#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "nav.h"
#include "../raylib/include/raylib.h"
#include "ins.h"
#include "cam.h"
#include "draw.h"
#include "explorer.h"

#define DELAY 5
#define DELAYPOLL 40

#define UPDATEC update_cursor_display(e); update_cam(cam, e)

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
    // printf("[editor.result_pos]: %d :: [editor.result_line]: %d\n", editor->result_pos, editor->result_line);
    editor->cursor->pos = editor->result_pos;
    editor->cursor->current_line = editor->result_line;
    editor->cursor->column = editor->cursor->pos - editor->lines->lines[editor->cursor->current_line].start;
    update_cam(camera, editor);
}

void keyboard_action(int key, Editor *e, Camera2D *cam, Explorer explorer) {
    if (IsKeyDown(RKEY_ACTION)) {
        switch (key) {
            //move cursor
            case KEY_I:
                cursor_move_v(e->cursor, e->lines, -1);
                update_cursor_display(e);
                update_cam(cam, e);
            break;
            case KEY_J:
                cursor_move_h(e->cursor, e->lines, *e->text, true);
                update_cam_offset_up(e->cursor, e->lines);
                update_cursor_display(e);
                update_cam(cam, e);
            break;
            case KEY_K:
                cursor_move_v(e->cursor, e->lines, 1);
                update_cursor_display(e);
                update_cam(cam, e);
            break;
            case KEY_L:
                cursor_move_h(e->cursor, e->lines, *e->text, false);
                update_cam_offset_down(e->cursor, e->lines, e->max_lines);
                update_cursor_display(e);
                update_cam(cam, e);
            break;
            case KEY_W:
                cursor_move_start(e->cursor);
                update_cursor_display(e);
                move_cam_start(cam, e->lines);
            break;
            case KEY_A:
                cursor_move_sol(e->cursor, e->lines);
                update_cursor_display(e);
                move_cam_left(cam, e->cursor_display.x, e->font_measuring.y, e->text_left_pos);
            break;
            case KEY_S:
                cursor_move_end(e->cursor, e->lines);
                update_cursor_display(e);
                update_cam(cam, e);
            break;
            case KEY_D:
                cursor_move_eol(e->cursor, e->lines);
                update_cursor_display(e);
                move_cam_right(cam, e->cursor_display.x, e->font_measuring.y);
            break;
            //copy-paste-cut
            case KEY_C: copy_text(e->text, e->cursor); break;
            case KEY_V: 
                if (paste_text(e->text, e->cursor, e->lines)) {
                    update_cursor_display(e);
                    update_cam(cam, e);
                }
            break;
            case KEY_X:
                if (cut_text(e->text, e->cursor, e->lines)) {
                    update_cursor_display(e);
                    update_cam(cam, e);
                }
            break;
            //other actions
            case KEY_G:
                if (strlen(explorer.current_file)) {
                    char *_s = LoadUTF8(e->text->buff, e->text->size);
                    SaveFileText(explorer.current_file, _s);
                    UnloadUTF8(_s);
                }
            break;
            case KEY_F: e->mode = FIND; break;
            //font resize
            case KEY_COMMA:
                change_font_size(e, RFONT_SIZE-2);
                UPDATEC;
            break;
            case KEY_PERIOD:
                change_font_size(e, RFONT_SIZE+2);
                UPDATEC;
            break;
            //word backspace
            case KEY_BACKSPACE:
                delete_word(e->text, e->cursor, e->lines);
                UPDATEC;
            break;
        }
    } else {
        switch (key) {
            case KEY_ENTER: 
                new_line(e->text, e->lines, e->cursor);
                update_cam_offset_down(e->cursor, e->lines, e->max_lines);
                update_cursor_display(e);
                update_cam(cam, e);
            break;
            case KEY_BACKSPACE: 
                delete_text(e->text, e->cursor, e->lines);
                update_cursor_display(e);
                update_cam(cam, e);
            break;
            case KEY_TAB:
                insert_text(e->text, ' ', e->cursor, e->lines);
                insert_text(e->text, ' ', e->cursor, e->lines);
                update_cursor_display(e);
            break;
            case KEY_DELETE:
                cursor_move_h(e->cursor, e->lines, *e->text, false);
                delete_text(e->text, e->cursor, e->lines);
            break;
        }
    }
}

int get_key() {
    int _key = GetKeyPressed();
    switch (_key) {
        case KEY_LEFT_CONTROL:
        case KEY_RIGHT_CONTROL:
        case KEY_LEFT_SHIFT: 
        case KEY_RIGHT_SHIFT: 
        case KEY_LEFT_ALT: 
            return 0;
    }
    return _key;
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

    Editor editor;
    Explorer explorer;
    explorer_init(&explorer);
    bool start_explorer_open;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(GW, GH, NAME);
    SetWindowMinSize(400, 300);
    SetExitKey(0);
    SetTargetFPS(60);


    if (argc == 1) {
        explorer_load_path(&explorer, ".");
        start_explorer_open = true;
    }
    else if (argc >= 2) {
        const char *path = argv[1];
        if (!DirectoryExists(path) && !FileExists(path)) {
            printf("You may provide a valid file path of directory path\n");
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

    init_editor(&editor, &cursor, &lines, &text, GW, GH, start_explorer_open);
    explorer.lines_amount = (GH/editor.font_measuring.y)-4;

    update_cursor_display(&editor);
    push_undo(&editor, cursor, lines);

    Shader crt = LoadShader(0, editor.config.shader_file);
    float sh_rh = GetShaderLocation(crt, "renderHeight");
    SetShaderValue(crt, sh_rh, &ScreenH, SHADER_UNIFORM_FLOAT);

    RenderTexture2D tex = LoadRenderTexture(GW, GH);

    //key poll
    int time_pressing = 0;
    int time_delay = 0;
    int _key = 0;
    int key_auto = 0;

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_ESCAPE) && editor.mode != FIND) {
            editor.explorer_open = !editor.explorer_open;
        }

        BeginDrawing();
        ClearBackground(BLACK);

        if (editor.explorer_open) {
            switch(editor.mode) {
                case FIND: {
                    static int __pos = -1;
                    if (IsKeyPressed(KEY_ESCAPE)) {
                        editor.mode = WRITE;
                        editor.result_pos = -1;
                        memset(editor.search_promp, '\0', editor.search_len);
                        editor.search_len = 0;
                        break;
                    }
                    int key = GetCharPressed();
                    while (key > 0 && key <= 255 && editor.search_len < 255) {
                        editor.search_promp[editor.search_len++] = key;
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
                        update_cursor_display(&editor);
                        move_cam_right(&camera, editor.cursor_display.x, editor.font_measuring.y);
                        key = GetCharPressed();
                    }

                    // int _key = get_key();
                    // keyboard_action(_key, &editor, &camera, explorer);
                    if (_key == 0) {
                        _key = get_key();
                    }
                    if (key_auto == 0 && _key > 0 && time_pressing == 0) {
                        // printf("[once]: key pressed: %i\n", _key);
                        keyboard_action(_key, &editor, &camera, explorer);
                    } 
                    if (_key > 0 && key_auto == 1) {
                        time_delay++;
                        if (time_delay == DELAY) {
                            time_delay = 0;
                            // printf("[auto]: key pressed: %i\n", _key);
                            keyboard_action(_key, &editor, &camera, explorer);
                        }

                        int __key__ = get_key();
                        if (__key__ > 0 && __key__ != _key) {
                            _key = __key__;
                        }
                    } 
                    if (IsKeyUp(_key)) {
                        // printf("[reset]: key %i\n", _key);
                        time_pressing = 0;
                        _key = 0;
                        key_auto = 0;
                    } else if (IsKeyDown(_key)) {
                        time_pressing++;
                        if (time_pressing > DELAYPOLL) {
                            time_pressing = 0;
                            key_auto = 1;
                        }
                    }

                    break;
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

            camera.target.y = editor.font_measuring.y*lines.offset + RFONT_SPACING*lines.offset;
            BeginTextureMode(tex);
            DrawRectangle(0, 0, GW, GH, RBLACK);
            BeginMode2D(camera);

            if (editor.config.show_hightlight) {
                draw_text_tokenized_optimized(text.buff, 
                                              lines, editor.font, editor.font_measuring, 
                                              (Vector2){ editor.text_left_pos, RTEXT_TOP }, 
                                              editor.font.baseSize, RFONT_SPACING, editor.max_lines);
            } else {
                draw_text_optimized(editor, (Vector2){ editor.text_left_pos, RTEXT_TOP }, RWHITE);
            }

            DrawRectangle(editor.cursor_display.x, editor.cursor_display.y, editor.font_measuring.x, editor.font_measuring.y, RWHITE);
            if (text.buff[cursor.pos] != '\n' && text.size > cursor.pos) {
                DrawTextCodepoint(editor.font, text.buff[cursor.pos], editor.cursor_display, editor.font.baseSize, RBLACK);
            }

            draw_selection(cursor, lines, editor.font_measuring, editor.text_left_pos);

            //statusbar
            const char *status_text = TextFormat("[%c] %s  %ix%i, %zu", 
                                        mode_to_str(editor.mode),
                                        explorer.current_file, 
                                        (int)ScreenW, (int)ScreenH, 
                                        cursor.column);
            Vector2 status_measure = MeasureTextEx(editor.font, status_text, editor.font.baseSize, RFONT_SPACING);
            Vector2 status_pos = {
                camera.target.x + GW - status_measure.x - 10, 
                camera.target.y + GH - editor.font_measuring.y - 10
            };
            DrawTextEx(editor.font, status_text, status_pos, editor.font.baseSize, RFONT_SPACING, RGRAY);

            DrawRectangle(camera.target.x, camera.target.y, editor.text_left_pos - 3, GH, RBLACK);
            draw_line_numbers(camera, editor.font, editor.font_measuring, lines, editor.text_left_pos);

            if (editor.mode == FIND) {
                //find-highlight
                if (editor.result.np > 0) {
                    int _m = (editor.font_measuring.x + RFONT_SPACING) * editor.search_len;
                    DrawRectangle(editor.cursor_display.x, editor.cursor_display.y, _m, editor.font_measuring.y, RORANGE);
                    DrawTextCodepoints(editor.font, editor.search_promp, editor.search_len, editor.cursor_display, editor.font.baseSize, RFONT_SPACING, RBLACK);
                }
                //find-status
                Vector2 symbol_pos = {
                    camera.target.x + editor.text_left_pos + 10, 
                    camera.target.y + GH - editor.font_measuring.y - 10
                };
                Vector2 find_pos = {
                    symbol_pos.x + 10 + RFONT_SPACING,
                    symbol_pos.y
                };
                DrawTextEx(editor.font, "/", symbol_pos, editor.font.baseSize, RFONT_SPACING, RGRAY);
                DrawTextCodepoints(editor.font, editor.search_promp, editor.search_len, find_pos, editor.font.baseSize, RFONT_SPACING, RGRAY);
            }

            EndMode2D();
            EndTextureMode();

            if (editor.config.show_shader) {
                BeginShaderMode(crt);
                DrawTextureRec(tex.texture, (Rectangle){0,0,tex.texture.width,-tex.texture.height}, (Vector2){0,0}, WHITE);
                EndShaderMode();
            } else {
                DrawTextureRec(tex.texture, (Rectangle){0,0,tex.texture.width,-tex.texture.height}, (Vector2){0,0}, WHITE);
            }
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
                explorer_draw(&explorer, editor.font, editor.font_measuring);
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
    UnloadFont(editor.font);
    CloseWindow();
    free_undo(&editor);
    explorer_free(&explorer);
    free_text(&text);
    free_lines(&lines);
    return 0;
}

