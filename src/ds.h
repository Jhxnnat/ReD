#ifndef DS_H
#define DS_H

#include <stdlib.h>
#include <stdbool.h>
#include "../raylib/include/raylib.h"

#define NAME "Retro eDitor - [0.0.1]"
#define GW 800
#define GH 600
#define RTEXT_LEFT 72
#define RTEXT_TOP 22
#define RTEXT_LEFT_LINES 10
#define RFONT_SPACING 2
#define RFONT_SIZE 24
#define MAX_LINES 11

//TODO define colors

typedef struct {
    size_t capacity;
    size_t size;
    char   *text;
} Text;

typedef struct {
    size_t start;
    size_t end;
} Line;

typedef struct {
    size_t size;
    size_t capacity;
    Line   *lines;
    size_t offset; //WARNING this can be vert_offset on Editor
} Lines;

typedef struct {
    size_t pos; 
    size_t line_pos; //TODO change this to: column
    size_t current_line;
    bool is_selecting;
    size_t selection_begin; 
    size_t selection_end;
    size_t selection_line_begin;
    size_t selection_line_end;
} Cursor;

typedef struct {
    Cursor *cursor;
    Lines *lines;
    Text *text;

    Font font;
    Vector2 font_measuring;

    Vector2 cursor_display;
    int max_lines;
    int hori_offset;

    bool write_mode;
} Editor;

void init_text(Text *t, size_t size);
void free_text(Text *t);
void init_cursor(Cursor *c);
void init_lines(Lines *lines, size_t initial_capacity);
void free_lines(Lines *lines);

void init_editor(Editor *editor, Cursor *cursor, Lines *lines, Text *text, Font font, int window_w, int window_h, bool write_mode);

#endif // !DS_H

