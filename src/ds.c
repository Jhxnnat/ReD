#include <stdio.h>

#include "ds.h"

void init_text(Text *t, size_t size) {
    t->text = malloc(size * sizeof(char));
    t->text[size] = '\0';
    t->capacity = size;
    t->size = 0;
}

void free_text(Text *t) {
    free(t->text);
    t->text = NULL;
    t->size = 0;
    t->capacity = 0;
}

void init_cursor(Cursor *c) {
    c->pos = 0;
    c->line_pos = 0;
    c->current_line = 0;
    c->is_selecting = false;
    c->selection_begin = 0;
    c->selection_end = 0;
    c->selection_line_begin = 0;
    c->selection_line_end = 0;
}

void init_lines(Lines *lines, size_t initial_capacity) {
    lines->size = 1;
    lines->capacity = initial_capacity;
    lines->offset = 0;
    lines->lines = (Line *)malloc(sizeof(Line) * initial_capacity);
    if (lines->lines == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
}

void free_lines(Lines *lines) {
    free(lines->lines);
    lines->lines = NULL;
    lines->capacity = 0;
    lines->size = 0;
}

void init_editor(Editor *editor, Cursor *cursor, Lines *lines, Text *text, Font font, int window_w, int window_h, bool write_mode) {
    if (window_h <= 0 || window_w <= 0) {
        printf("negative windows haven't been invented yet\n");
        exit(44);
    }

    Vector2 font_measuring = MeasureTextEx(font, "M", font.baseSize, RFONT_SPACING);
    editor->font_measuring = font_measuring;

    float font_height = font_measuring.y;
    editor->max_lines = window_h / font_height;
    editor->max_lines -= 4;
    editor->hori_offset = 0;
    editor->cursor_display.x = 0;
    editor->cursor_display.y = 0;

    editor->lines = lines;
    editor->text = text;
    editor->cursor = cursor;

    editor->write_mode = write_mode;
}
