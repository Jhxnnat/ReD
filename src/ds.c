#include <stdio.h>
#include <string.h>

#include "ds.h"

float ScreenW = 1000;
float ScreenH = 600;

int FontSize = 38;

void init_text(Text *t, size_t size) {
    t->buff = malloc(size * sizeof(int));
    t->capacity = size;
    t->size = 0;
}

void free_text(Text *t) {
    free(t->buff);
    t->buff = NULL;
    t->size = 0;
    t->capacity = 0;
}

void init_cursor(Cursor *c) {
    c->pos = 0;
    c->column = 0;
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

void init_editor(Editor *editor, Cursor *cursor, Lines *lines, Text *text, int window_w, int window_h) {
    if (window_h <= 0 || window_w <= 0) {
        printf("negative windows haven't been invented yet\n");
        exit(69);
    }

    editor->appdir = GetApplicationDirectory();
    sprintf(editor->config.shader_file, "%s/%s", editor->appdir, "assets/shader/crt.glsl");
    sprintf(editor->config.font_file, "%s/%s", editor->appdir, "assets/fonts/IosevkaTerm/IosevkaTermNerdFontMono-Regular.ttf");
    //NOTE: only mono spaced fonts works correctly

    editor->config.show_shader = false;
    editor->config.show_hightlight = true;
    editor->config.show_decorations = true;

    editor->font = LoadFontEx(editor->config.font_file, RFONT_SIZE, NULL, 250);
    if (!IsFontValid(editor->font)) {
        editor->font = GetFontDefault();
    }
    editor->font_measuring = MeasureTextEx(editor->font, "M", editor->font.baseSize, RFONT_SPACING);

    editor->max_lines = ((window_h-RTEXT_TOP) / editor->font_measuring.y)-1;
    editor->hori_offset = 0;
    editor->cursor_display.x = 0;
    editor->cursor_display.y = 0;
    editor->text_left_pos = (editor->font_measuring.x + RFONT_SPACING) * 4;

    editor->lines = lines;
    editor->text = text;
    editor->cursor = cursor;

    editor->mode = WRITE;

    editor->search_len = 0;
    editor->result_pos = -1;
    editor->result_line = -1;

    editor->stack_top = 0;
    editor->stack_top_redo = 0;


}

void push_undo(Editor *editor, Cursor c, Lines l) {
    if (editor->stack_top >= STACK_MAX_SIZE) {
        printf("[stack] full\n");
        return;
    }
    size_t text_size = editor->text->size;
    editor->stack[editor->stack_top].size = text_size;
    editor->stack[editor->stack_top].text = malloc(text_size * sizeof(int));
    for (size_t i = 0; i < text_size; ++i) {
        editor->stack[editor->stack_top].text[i] = editor->text->buff[i];
    }
    editor->stack[editor->stack_top].cursor_pos = c.pos;
    editor->stack[editor->stack_top].col = c.column;
    editor->stack[editor->stack_top].line = c.current_line;
    // editor->stack[editor->stack_top].hori_off = editor->hori_offset;
    editor->stack[editor->stack_top].vert_off = l.offset;

    editor->stack_top++;
}

void push_redo(Editor *editor, Cursor c, Lines l) {
    if (editor->stack_top_redo >= STACK_MAX_SIZE) {
        printf("[stack] full!\n");
        return;
    }
    size_t text_size = editor->text->size;
    editor->stack_r[editor->stack_top_redo].size = text_size;
    editor->stack_r[editor->stack_top_redo].text = malloc(text_size * sizeof(int));
    for (size_t i = 0; i < text_size; ++i) {
        editor->stack_r[editor->stack_top_redo].text[i] = editor->text->buff[i];
    }
    editor->stack_r[editor->stack_top_redo].cursor_pos = c.pos;
    editor->stack_r[editor->stack_top_redo].col = c.column;
    editor->stack_r[editor->stack_top_redo].line = c.current_line;
    // editor->stack_r[editor->stack_top_redo].hori_off = editor->hori_offset;
    editor->stack_r[editor->stack_top_redo].vert_off = l.offset;

    editor->stack_top_redo++;
}

void free_undo(Editor *editor) {
    for (int i = 0; i < editor->stack_top; ++i) {
        free(editor->stack[i].text);
    }
}

void free_redo(Editor *editor) {
    for (int i = 0; i < editor->stack_top_redo; ++i) {
        free(editor->stack[i].text);
    }
}

KmpTable __kmp_table(const int *word, int word_len) {
    int position = 1, candidate = 0;
    KmpTable T;
    T.t[0] = -1;
    while(position < word_len) {
        if (word[position] == word[candidate]) {
            T.t[position] = T.t[candidate];
        } else {
            T.t[position] = candidate;
            while (candidate >= 0 && word[position] != word[candidate]) {
                candidate = T.t[candidate];
            }
        }
        position++;
        candidate++;
    }
    T.t[position] = candidate;
    return T;
}

SearchResult kmp_search(Editor e, const int *word, int word_len) {
    int textpos = 0, wordpos = 0;
    SearchResult positions = { .np = 0 };
    KmpTable T = __kmp_table(word, word_len);
    int l = 0;
    while (textpos < (int)e.text->size) {
        if (textpos > (int)e.lines->lines[l].end) {
            l++;
        }
        if (word[wordpos] == (e.text->buff[textpos])) {
            wordpos++;
            textpos++;
            if (wordpos == word_len) {
                positions.p[positions.np++] = textpos - wordpos;
                positions.l[positions.np-1] = l;
                wordpos = T.t[wordpos];
            }
        } else {
            wordpos = T.t[wordpos];
            if (wordpos < 0) {
                wordpos++;
                textpos++;
            }
        }
    }
    return positions;
}

void change_font_size(Editor *editor, int amount) {
    if (amount < 8 || amount > 48) return;
    UnloadFont(editor->font);
    RFONT_SIZE = amount;
    editor->font = LoadFontEx(editor->config.font_file, RFONT_SIZE, NULL, 250);
    if (!IsFontValid(editor->font)) {
        editor->font = GetFontDefault();
    }
    editor->font_measuring = MeasureTextEx(editor->font, "M", editor->font.baseSize, RFONT_SPACING);
    editor->text_left_pos = (editor->font_measuring.x + RFONT_SPACING) * 4;
    editor->max_lines = ((GH - RTEXT_TOP) / editor->font_measuring.y)-1;
}
