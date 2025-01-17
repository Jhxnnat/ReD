#include <string.h>
#include <stdio.h>

#include "ins.h"

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

    selection_reset(c);

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
    selection_reset(c);
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
    cursor->selection_begin = cursor->pos;
    cursor->selection_end = cursor->pos;
    cursor->selection_line_begin = cursor->current_line;
    cursor->selection_line_end = cursor->current_line;
    UnloadUTF8(copied_text);
}

bool cut_text(Text *text, Cursor *cursor, Lines *lines) {
    const int selection_range = cursor->selection_end - cursor->selection_begin;
    if (selection_range <= 0) return false;
    copy_text(text, cursor);
    delete_text(text, cursor, lines);
    return true;
}
