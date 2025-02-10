#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "nav.h"
#include "../raylib/include/raylib.h"
#include "ins.h"
#include "cam.h"
#include "draw.h"
#include "explorer.h"

#define DELAY 2
#define DELAYINIT 30

#define UPDATEC(editor, camera) do { update_cursor_display(editor); update_cam(camera, editor); } while (0);

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

void capture_input_simple(int *buffer, int *cursor, int max_len) {
    int _key = GetCharPressed();
    while (_key > 0 && *cursor < max_len) {
        buffer[*cursor] = _key;
        *cursor += 1;
        _key = GetCharPressed();
    }
    if (IsKeyPressed(KEY_BACKSPACE) && *cursor > 0) {
        *cursor -= 1;
        buffer[*cursor] = 0;
    }
}

bool check_if_dir(char *filename, int len) {
    bool is = false;
    if (filename[len-1] == '/') is = true;
    return is;
}

void result_update(Editor *editor, Camera2D *camera){
    // printf("[editor.result_pos]: %d :: [editor.result_line]: %d\n", editor->result_pos, editor->result_line);
    editor->cursor->pos = editor->result_pos;
    editor->cursor->current_line = editor->result_line;
    editor->cursor->column = editor->cursor->pos - editor->lines->lines[editor->cursor->current_line].start;
    update_cam(camera, editor);
}

void keyboard_action(int key, Editor *e, Camera2D *cam, Explorer *explorer) {
    if (IsKeyDown(RKEY_ACTION)) {
        switch (key) {
            //move cursor
            case KEY_I:
                cursor_move_v(e->cursor, e->lines, -1);
                UPDATEC(e, cam);
            break;
            case KEY_J:
                cursor_move_h(e->cursor, e->lines, *e->text, true);
                update_cam_offset_up(e->cursor, e->lines);
                update_cursor_display(e);
                update_cam(cam, e);
            break;
            case KEY_K:
                cursor_move_v(e->cursor, e->lines, 1);
                UPDATEC(e, cam);
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
                UPDATEC(e, cam);
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
                    UPDATEC(e, cam);
                }
            break;
            case KEY_X:
                if (cut_text(e->text, e->cursor, e->lines)) {
                    UPDATEC(e, cam);
                }
            break;
            //other actions
            case KEY_G:
                if (strlen(explorer->current_file)) {
                    char *_s = LoadUTF8(e->text->buff, e->text->size);
                    SaveFileText(explorer->current_file, _s);
                    UnloadUTF8(_s);
                }
            break;
            case KEY_F: e->mode = FIND; break;
            //word backspace
            case KEY_BACKSPACE:
                delete_word(e->text, e->cursor, e->lines);
                UPDATEC(e, cam);
            break;
            //toggle highlight
            case KEY_H: e->config.show_hightlight = !e->config.show_hightlight; break;
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

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(GW, GH, NAME);
    SetWindowMinSize(400, 300);
    SetExitKey(0);
    SetTargetFPS(60);

    init_editor(&editor, &cursor, &lines, &text, GW, GH); //needs to be after InitWindow();
    explorer.lines_amount = explorer_cacl_lines(editor.font_measuring.y);

    if (argc == 1) {
        const char *working_dir = GetWorkingDirectory();
        explorer_load_path(&explorer, working_dir);
        editor.explorer_open = true;
    }
    else if (argc >= 2) {
        const char *path = argv[1];
        if (!DirectoryExists(path) && !FileExists(path)) {
            printf("File of directory does not exists\n");
            exit(69);
        }
        
        if (IsPathFile(path)) {
            insert_text_from_file(path, &text, &lines, &cursor);
            cursor_move_start(&cursor);
            const char *directory = GetDirectoryPath(path);
            explorer_load_path(&explorer, directory);
            strcpy(explorer.current_file, path);
            editor.explorer_open = false;
        }
        else {
            explorer_load_path(&explorer, path);
            editor.explorer_open = true;
        }
    }

    update_cursor_display(&editor);

    Shader crt = LoadShader(0, editor.config.shader_file);
    float sh_rh = GetShaderLocation(crt, "renderHeight");
    SetShaderValue(crt, sh_rh, &ScreenH, SHADER_UNIFORM_FLOAT);

    RenderTexture2D tex = LoadRenderTexture(GW, GH);

    //key poll
    float time_pressing = 0.0f;
    float time_delay = 0.0f;
    int _key = 0;
    int key_auto = 0;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        if (!editor.explorer_open) {
            // Open explorer
            if (IsKeyPressed(KEY_ESCAPE) && editor.mode != FIND) {
                editor.explorer_open = true;
            }

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
                    if (__pos > -1 && IsKeyDown(RKEY_ACTION) && IsKeyPressed(KEY_ENTER)) {
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

                    // Repeat actions when keeping a key pressed
                    float delta = 60 * GetFrameTime();
                    if (_key == 0) {
                        _key = get_key();
                    }
                    if (key_auto == 0 && _key > 0 && time_pressing == 0) {
                        keyboard_action(_key, &editor, &camera, &explorer);
                    } 
                    if (_key > 0 && key_auto == 1) {
                        time_delay += delta;
                        if (time_delay > DELAY) {
                            time_delay = 0.0f;
                            keyboard_action(_key, &editor, &camera, &explorer);
                        }
                        int __key__ = get_key();
                        if (__key__ > 0 && __key__ != _key) { _key = __key__; }
                    } 
                    if (IsKeyUp(_key)) {
                        time_pressing = 0;
                        _key = 0;
                        key_auto = 0;
                    } else if (IsKeyDown(_key)) {
                        time_pressing += delta;
                        if (time_pressing > DELAYINIT) {
                            time_pressing = 0.0f;
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

            draw_selection(cursor, lines, editor.font_measuring, editor.text_left_pos);

            DrawRectangle(camera.target.x, camera.target.y, editor.text_left_pos - 3, GH, RBLACK);
            draw_line_numbers(camera, editor.font, editor.font_measuring, lines, editor.text_left_pos, editor.max_lines);

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

            //draw cursor
            DrawRectangle(editor.cursor_display.x, editor.cursor_display.y, editor.font_measuring.x, editor.font_measuring.y, RWHITE);
            if (text.buff[cursor.pos] != '\n' && text.size > cursor.pos) {
                DrawTextCodepoint(editor.font, text.buff[cursor.pos], editor.cursor_display, editor.font.baseSize, RBLACK);
            }

            EndMode2D();

            //status bar (outside cam drawing to be able to use global coord to draw)
            if (editor.mode == WRITE) {
                const char *status_text = TextFormat(" %d | \"%s\"", cursor.pos, explorer.current_file);
                draw_status(status_text, editor.font, editor.font_measuring, RGRAY, RGRAY, RBLACK);
            }

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
            const char *status_text;
            switch(explorer.mode) {
                case NORMAL: 
                    // Close explorer
                    if (IsKeyPressed(KEY_ESCAPE) && editor.mode != FIND) {
                        editor.explorer_open = false;
                    }
                    else if (IsKeyPressed(KEY_A)) explorer.mode = CREATE;
                    else if (IsKeyPressed(KEY_R)) explorer.mode = RENAME;
                    else if (IsKeyPressed(KEY_D)) explorer.mode = DELETE;

                    status_text = "^A: new - ^R: rename - ^D: delete";
                    explorer_input(&explorer);
                    if (IsKeyReleased(KEY_ENTER) && explorer.cursor == -1) {
                        explorer_load_prevpath(&explorer);
                    }
                    else if (IsKeyPressed(KEY_ENTER) && explorer.cursor > -1) {
                        explorer.y_offset = 0;
                        char *selected_path = explorer.filepath_list.paths[explorer.cursor];
                        if (IsPathFile(selected_path) && FileExists(selected_path)) {
                            strcpy(explorer.current_file, selected_path);
                            editor_reset(&text, &cursor, &lines, &camera);
                            insert_text_from_file(selected_path, &text, &lines, &cursor);
                            cursor_move_start(&cursor);
                            update_cursor_display(&editor);
                            editor.explorer_open = false;
                        } 
                        else if (DirectoryExists(selected_path)) {
                            explorer_load_path(&explorer, selected_path);
                        }
                    }
                break;
                case RENAME:
                    status_text = "rename file [%d]: ";
                    capture_input_simple(explorer.buffer, &explorer.buff_cursor, MAX_BUFF_EXPLORER);
                break;
                case CREATE: 
                    status_text = TextFormat("create file: %s/", explorer.path);
                    capture_input_simple(explorer.buffer, &explorer.buff_cursor, MAX_BUFF_EXPLORER);
                    if (IsKeyPressed(KEY_ENTER) && explorer.buff_cursor > 0) {
                        char *filename = LoadUTF8(explorer.buffer, explorer.buff_cursor);
                        if (filename[explorer.buff_cursor-1] == '/') {
                            const char *_path = TextFormat("%s/%s", explorer.path, filename);
                            int dir = MakeDirectory(_path);
                        } else if (IsFileNameValid(filename)) {
                            const char *_path = TextFormat("%s/%s", explorer.path, filename);
                            bool saved = SaveFileText(_path, "");
                        }
                        memset(explorer.buffer, 0, explorer.buff_cursor);
                        explorer.buff_cursor = 0;
                        if (DirectoryExists(explorer.path)) {
                            explorer.filepath_list = LoadDirectoryFiles(explorer.path);
                            explorer.cursor = 0;
                        }
                        UnloadUTF8(filename);
                        explorer.mode = NORMAL;
                    }
                break;
                case DELETE: 
                    status_text = "sure to delete file [%d]?: %s [y,N]: ";
                    capture_input_simple(explorer.buffer, &explorer.buff_cursor, MAX_BUFF_EXPLORER);
                break;
            }

            if (IsKeyPressed(KEY_ESCAPE) && (explorer.mode == RENAME || explorer.mode == CREATE  || explorer.mode == DELETE)) {
                explorer.mode = NORMAL;
                explorer.buff_cursor = 0;
                memset(explorer.buffer, 0, sizeof(int) * MAX_BUFF_EXPLORER);
            }

            BeginTextureMode(tex);
            DrawRectangle(0, 0, GW, GH, RBLACK);
            explorer_draw(&explorer, editor.font, editor.font_measuring);

            // explorer status 
            draw_status(status_text, editor.font, editor.font_measuring, RGRAY, RGRAY, RBLACK);
            Vector2 _pos = { 
                strlen(status_text) * (editor.font_measuring.x + RFONT_SPACING),
                GH - editor.font_measuring.y - RFONT_SPACING
            };
            if (explorer.mode != NORMAL) {
                DrawTextCodepoints(editor.font, explorer.buffer, explorer.buff_cursor, _pos, RFONT_SIZE, RFONT_SPACING, RGRAY);
            }

            EndTextureMode();

            if (editor.config.show_shader) {
                BeginShaderMode(crt);
                DrawTextureRec(tex.texture, (Rectangle){0,0,tex.texture.width,-tex.texture.height}, (Vector2){0,0}, WHITE);
                EndShaderMode();
            } else DrawTextureRec(tex.texture, (Rectangle){0,0,tex.texture.width,-tex.texture.height}, (Vector2){0,0}, WHITE);
        }

        // global binds
        //font resize
        if (IsKeyPressed(KEY_COMMA) && IsKeyDown(RKEY_ACTION)) {
            change_font_size(&editor, RFONT_SIZE-2);
            editor_calc_lines(&editor);
            explorer.lines_amount = explorer_cacl_lines(editor.font_measuring.y);
            UPDATEC(&editor, &camera);
        } else if (IsKeyPressed(KEY_PERIOD) && IsKeyDown(RKEY_ACTION)) {
            change_font_size(&editor, RFONT_SIZE+2);
            editor_calc_lines(&editor);
            explorer.lines_amount = explorer_cacl_lines(editor.font_measuring.y);
            UPDATEC(&editor, &camera);
        }

        if (IsKeyPressed(KEY_P) && IsKeyDown(KEY_LEFT_CONTROL)) {
            explorer.filepath_list = LoadDirectoryFiles(explorer.path);
            explorer.cursor = 0;
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

            // calculate max-lines again
            editor_calc_lines(&editor);
            explorer.lines_amount = explorer_cacl_lines(editor.font_measuring.y);
        }

    }

    UnloadRenderTexture(tex);
    UnloadShader(crt);
    UnloadFont(editor.font);
    CloseWindow();
    explorer_free(&explorer);
    free_text(&text);
    free_lines(&lines);
    return 0;
}

