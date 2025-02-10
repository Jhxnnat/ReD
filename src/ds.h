#ifndef DS_H
#define DS_H

#include <stdlib.h>
#include <stdbool.h>
#include "../raylib/include/raylib.h"

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
#define RBLACK (Color){ 29, 32, 33, 255 }
#define RGRAY (Color){ 146, 131, 116, 255 }
#define RWHITE (Color){ 242, 229, 188, 255 }
#define RRED (Color){ 204, 36, 29, 255 }
#define RGREEN (Color){ 152, 151, 26, 255 }
#define RYELLOW (Color){ 215, 153, 33, 255 }
#define RBLUE (Color){ 69, 133, 136, 255 }
#define RPURPLE (Color){ 177, 98, 134, 255 }
#define RAQUA (Color){ 104, 157, 106, 255 }
#define RORANGE (Color){ 214, 93, 14, 255 }

#define TEXT_INIT_SIZE 8
#define LINES_INIT_SIZE 10

#define RKEY_ACTION KEY_LEFT_CONTROL

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
    int show_hightlight;
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

#endif // !DS_H

