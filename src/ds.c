#include <stdio.h>
#include <string.h>

#include "ds.h"

float ScreenW = 1000;
float ScreenH = 600;

int FontSize = 24;

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
    // sprintf(editor->config.font_file, "%s/%s", editor->appdir, "assets/fonts/BigBlueTerminal/BigBlueTermPlusNerdFontMono-Regular.ttf");
    //NOTE: only mono spaced fonts works correctly

    editor->config.show_shader = false;
    editor->config.show_hightlight = true;
    editor->config.show_decorations = true;

    editor->font = LoadFontEx(editor->config.font_file, RFONT_SIZE, NULL, 250);
    if (!IsFontValid(editor->font)) {
        editor->font = GetFontDefault();
    }
    editor->font_measuring = MeasureTextEx(editor->font, "M", editor->font.baseSize, RFONT_SPACING);

    editor->hori_offset = 0;
    editor->cursor_display.x = 0;
    editor->cursor_display.y = 0;
    editor_calc_lines(editor);

    editor->lines = lines;
    editor->text = text;
    editor->cursor = cursor;

    editor->mode = WRITE;

    editor->search_len = 0;
    editor->result_pos = -1;
    editor->result_line = -1;
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

//KMP Search: https://en.wikipedia.org/wiki/Knuth%E2%80%93Morris%E2%80%93Pratt_algorithm
//from: https://rosettacode.org/wiki/Knuth-Morris-Pratt_string_search#Python
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

void editor_calc_lines(Editor *editor) {
    editor->text_left_pos = (editor->font_measuring.x + RFONT_SPACING) * 4;
    int _h = editor->font_measuring.y + (RFONT_SPACING * 2);
    editor->max_lines = (int)((GH - RTEXT_TOP - _h) / editor->font_measuring.y);
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
}

// Text Modification
void insert_text(Text *t, int c, Cursor *cu, Lines *lines) {
    if (cu->pos > t->size) cu->pos = t->size;
    const size_t selection_range = cu->selection_end - cu->selection_begin;
    if (selection_range > 0) {
        delete_text(t, cu, lines);
    }
    if (t->size >= t->capacity) {
        t->capacity *= 2;
        int *temp = realloc(t->buff, (t->capacity) * sizeof(int));
        if (temp == NULL) {
            printf("Error realloc!!!\n");
            exit(0);
        } else { 
            t->buff = temp;
        }
    }
    t->size++;
    for (size_t i = t->size; i > cu->pos; --i) {
        t->buff[i] = t->buff[i-1];
    }
    t->buff[cu->pos] = c;
    cu->pos++;
    cu->column++;
    lines->lines[cu->current_line].end += 1;
    if (cu->current_line < lines->size-1) {
        for (size_t i = cu->current_line+1; i < lines->size; ++i) { // move other lines when inserting before last line
            lines->lines[i].start++;
            lines->lines[i].end++;
        }
    }
    cu->selection_begin = cu->pos;
    cu->selection_end = cu->pos;
}

void insert_text_from_file(const char *path, Text *text, Lines *lines, Cursor *cursor) {
    char *source = LoadFileText(path);
    int count;
    int *_source = LoadCodepoints(source, &count);
    for (int i = 0; i < count; ++i) {
        if (_source[i] == 10) {
            new_line(text, lines, cursor);
        } else {
            insert_text(text, _source[i], cursor, lines);
        }
    }
    UnloadFileText(source);
    UnloadCodepoints(_source);
}

void _delete_recalculate_lines(Cursor *c, Lines *l) {
    if (c->column == 0) {
        c->current_line--;
        l->size--;

        int diff = l->lines[c->current_line].end-l->lines[c->current_line].start-1;
        c->column = diff;

        l->lines[c->current_line].end = l->lines[c->current_line+1].end-1;
        for (size_t i = c->current_line+1; i < l->size; ++i) {
            l->lines[i].start = l->lines[i+1].start-1;
            l->lines[i].end = l->lines[i+1].end-1;
        }
    } 
    else {
        c->column--;
        l->lines[c->current_line].end--;
        for (size_t i = c->current_line+1; i <= l->size; ++i) {
            l->lines[i].start--;
            l->lines[i].end--;
        }
    }
}

char *_delete_text(Text *t, Cursor *c, Lines *l) {
    size_t repeat = 1;
    const size_t selection_range = c->selection_end - c->selection_begin;
    if (selection_range > 0) {
        repeat = selection_range;
        c->current_line = c->selection_line_end;
        c->pos = c->selection_end;
        c->column = c->selection_end - l->lines[c->current_line].start;
    }

    char* deleted = (char*)malloc(selection_range+1);

    if ((t->size <= 0) || (c->pos <= 0)) {
        return deleted;
    }

    deleted[selection_range] = '\0';
    if (selection_range > 0) {
        // strncpy(deleted, (char *)t->buff + c->selection_begin, selection_range);
    }

    while (repeat > 0) {
        repeat--;

        _delete_recalculate_lines(c, l);

        c->pos--;
        memmove(t->buff + c->pos, t->buff + c->pos+1, t->size - c->pos+1);
        t->buff[t->size] = '\0';
        t->size--;
    }

    /*selection_reset(c);*/

    return deleted;
}

void delete_text(Text *t, Cursor *c, Lines *l) {
    size_t repeat = 1;
    const int selection_range = c->selection_end - c->selection_begin;
    if (selection_range > 0) {
        repeat = selection_range;
        c->current_line = c->selection_line_end;
        c->pos = c->selection_end;
        c->column = c->selection_end - l->lines[c->current_line].start;
    }
    if ((t->size == 0) || (c->pos == 0)) {
        return;
    }
    while (repeat > 0) {
        repeat--;
        _delete_recalculate_lines(c, l);
        c->pos--;
        for (size_t i = c->pos; i < t->size; ++i) {
            t->buff[i] = t->buff[i+1];
        }
        t->buff[t->size] = 0;
        t->size--;
    }
    /*selection_reset(c);*/
}

void delete_word(Text *t, Cursor *c, Lines *l) {
    //' ' = 32, '\n' = 10, '\t' = 9
    while (true) {
        if (t->size < 1 || c->pos < 1) break;
        delete_text(t,c,l);
        int _c = t->buff[c->pos-1];
        if (_c == ' ' || _c == '\n' || _c == '\t') break;
    }
}

void resize_lines(Lines *lines) {
    // const size_t new_capacity = lines->capacity * 2;
    lines->capacity *= 2;
    Line *new_lines = realloc(lines->lines, sizeof(Line) * lines->capacity);
    if (new_lines == NULL) {
        fprintf(stderr, "Memory reallocation failed\n");
        free(lines->lines); 
        exit(1);
    }
    lines->lines = new_lines;
    // lines->capacity = new_capacity;
}

void new_line(Text *text, Lines *lines, Cursor *c) {
    insert_text(text, '\n', c, lines); 

    c->column = 0;
    c->current_line++;

    lines->size++;
    if (lines->size >= lines->capacity) {
        resize_lines(lines);
    }
    
    for (size_t i = lines->size-1; i > c->current_line; --i) {
        lines->lines[i].start = lines->lines[i-1].start;
        lines->lines[i].end = lines->lines[i-1].end;
    }

    lines->lines[c->current_line].end = lines->lines[c->current_line-1].end;
    lines->lines[c->current_line].start = c->pos;

    lines->lines[c->current_line-1].end = c->pos;
}

bool paste_text(Text *text, Cursor *cursor, Lines *lines) {
    const char *source = GetClipboardText();
    const int len = strlen(source);
    if (len == 0) return false;
    int count;
    int *_source = LoadCodepoints(source, &count);
    for (int i = 0; i < count; ++i) {
        insert_text(text, _source[i], cursor, lines);
    }
    UnloadCodepoints(_source);
    return true;
}

void copy_text(Text *text, Cursor *cursor) {
    const int range = cursor->selection_end - cursor->selection_begin;
    if (range <= 0) return;
    char *copied_text = LoadUTF8(text->buff + cursor->selection_begin, range);
    printf("copied_text: %s\n", copied_text);
    SetClipboardText(copied_text);
    /*cursor->selection_begin = cursor->pos;*/
    /*cursor->selection_end = cursor->pos;*/
    /*cursor->selection_line_begin = cursor->current_line;*/
    /*cursor->selection_line_end = cursor->current_line;*/
    UnloadUTF8(copied_text);
}

bool cut_text(Text *text, Cursor *cursor, Lines *lines) {
    const int selection_range = cursor->selection_end - cursor->selection_begin;
    if (selection_range <= 0) return false;
    copy_text(text, cursor);
    delete_text(text, cursor, lines);
    return true;
}
