#ifndef DS_H
#define DS_H

#include <stdlib.h>
#include <stdbool.h>
#include "../raylib/include/raylib.h"

extern float ScreenW;
extern float ScreenH;

extern int FontSize;

#define NAME "Retro eDitor - [0.0.2]"
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
#define STACK_MAX_SIZE 1024

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
    int *text;
    size_t size;
    size_t cursor_pos;
    size_t col;
    size_t line;
    size_t hori_off;
    size_t vert_off;
    bool is_inserted;
} Change;

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
    char *font_file;
    char *shader_file;
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

    bool explorer_open;
    EditorMode mode;

    int search_promp[255];
    int search_len;
    int result_pos;
    int result_line;
    SearchResult result;

    Change stack[STACK_MAX_SIZE];
    Change stack_r[STACK_MAX_SIZE];
    int stack_top;
    int stack_top_redo;

    Config config;
} Editor;

void init_text(Text *t, size_t size);
void free_text(Text *t);
void init_cursor(Cursor *c);
void init_lines(Lines *lines, size_t initial_capacity);
void free_lines(Lines *lines);
void init_editor(Editor *editor, Cursor *cursor, Lines *lines, Text *text, int window_w, int window_h, bool explorer_open);
void push_undo(Editor *editor, Cursor c, Lines l);
void push_redo(Editor *editor, Cursor c, Lines l);
void free_undo(Editor *editor);
void free_redo(Editor *editor);
void change_font_size(Editor *editor, int amount);

//KMP Search: https://en.wikipedia.org/wiki/Knuth%E2%80%93Morris%E2%80%93Pratt_algorithm
//from: https://rosettacode.org/wiki/Knuth-Morris-Pratt_string_search#Python
SearchResult kmp_search(Editor e, const int *word, int word_len);

#endif // !DS_H

