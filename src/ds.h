#ifndef DS_H
#define DS_H

#include <stdlib.h>
#include <stdbool.h>
#include "../raylib/include/raylib.h"
#include "config.h"

//globals
extern float ScreenW;
extern float ScreenH;
extern int FontSize;

#define NAME "ReD"
#define GW ScreenW
#define GH ScreenH
#define RTEXT_LEFT 72
#define RTEXT_TOP 22
#define RFONT_SPACING 2
#define RFONT_SIZE FontSize

#define TEXT_INIT_SIZE 8
#define LINES_INIT_SIZE 10

typedef enum {
    WRITE,
    FIND
} EditorMode;

typedef struct {
    size_t capacity;
    size_t size;
    int *buff; //codepoint buffer
} Text;

typedef struct {
    size_t start;
    size_t end;
} Line;

typedef struct {
    size_t size;
    size_t capacity;
    Line   *lines;

    //draw purpose
    size_t offset;
} Lines;

typedef struct {
    size_t pos; 
    size_t column;
    size_t current_line;
    bool is_selecting;
    size_t selection_begin; 
    size_t selection_end;
    size_t selection_line_begin;
    size_t selection_line_end;
} Cursor;

typedef struct {
    size_t p[1024];
    size_t l[1024];
    int np;
} SearchResult;

typedef struct {
    int t[1024];
} KmpTable;

typedef struct {
    int show_shader;
    int show_highlight;
    int show_decorations;
    char font_file[1024];
    char shader_file[1024];
} Config;

typedef struct {
    Cursor *cursor;
    Lines *lines;
    Text *text;

    Font font;
    Vector2 font_measuring;

    Vector2 cursor_display;
    int max_lines;
    int hori_offset;
    int text_left_pos;

    const char *appdir;
    bool explorer_open;
    EditorMode mode;

    int search_promp[255];
    int search_len;
    int result_pos;
    int result_line;
    SearchResult result;

	//Ustack ustack;

    Config config;
} Editor;

void init_text(Text *t, size_t size);
void free_text(Text *t);
void init_cursor(Cursor *c);
void init_lines(Lines *lines, size_t initial_capacity);
void free_lines(Lines *lines);
void init_editor(Editor *editor, Cursor *cursor, Lines *lines, Text *text, int window_w, int window_h);
void editor_calc_lines(Editor *editor);
void change_font_size(Editor *editor, int amount);
SearchResult kmp_search(Editor e, const int *word, int word_len);
void insert_text(Text *t, int c, Cursor *cu, Lines *lines);
void delete_text(Text *t, Cursor *c, Lines *lines);
void delete_word(Text *t, Cursor *c, Lines *lines);
void insert_text_from_file(const char *path, Text *text, Lines *lines, Cursor *cursor);
void copy_text(Text *text, Cursor *cursor);
bool paste_text(Text *text, Cursor *cursor, Lines *lines);
bool cut_text(Text *text, Cursor *cursor, Lines *lines);
void resize_lines(Lines *lines);
void update_lines(Lines *lines, size_t line_num, size_t start, size_t end);
void new_line(Text *text, Lines *lines, Cursor *c);

int calc_lines_fit(float font_measuring_y);
void editor_reset(Text *text, Cursor *cursor, Lines *lines, Camera2D *camera);

void init_camera(Camera2D *camera);
void update_cam_offset_up(Cursor *cursor, Lines *lines);
void update_cam_offset_down(Cursor *cursor, Lines *lines, int max_lines);
void move_cam_left(Camera2D *camera, int cursor_x, int font_height, int text_left_pos);
void move_cam_right(Camera2D *camera, int cursor_x, int font_height);
void move_cam_start(Camera2D *camera, Lines *lines);
void move_cam_end(Camera2D *camera, Lines *lines, int max_lines);
void update_cam(Camera2D *camera, Editor *editor);

#endif // !DS_H
