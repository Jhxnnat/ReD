#include <string.h>
#include <stdio.h>

// #include "raylib.h"
#include "ins.h"
// #include "cam.h"

void insert_text(Text *t, char c, Cursor *cu, Lines *lines) {
    if (cu->pos > t->size) cu->pos = t->size;

    const size_t selection_range = cu->selection_end - cu->selection_begin;
    if (selection_range > 0) {
        delete_text(t, cu, lines);
    }

    if (t->size >= t->capacity) {
        t->capacity *= 2;
        char *temp = realloc(t->text, (t->capacity) * sizeof(char));
        if (temp == NULL) {
            printf("Error realloc!!!\n");
            exit(0);
        } else { 
            t->text = temp;
        }
    }

    t->size++;
    memmove(t->text + cu->pos+1, t->text + cu->pos, t->size - cu->pos+1);
    t->text[cu->pos] = c;
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
    size_t len = TextLength(source);
    for (size_t i = 0; i < len; ++i) {
        if (source[i] == '\n') new_line(text, lines, cursor);
        else if (source[i] == '\r') insert_text(text, ' ', cursor, lines);
        else if (source[i] == '\t') {
            insert_text(text, ' ', cursor, lines);
            insert_text(text, ' ', cursor, lines);
        }
        else if (source[i] == '\0') break; //NOTE is this needed?
        else insert_text(text, source[i], cursor, lines);
    }
    UnloadFileText(source);
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
        strncpy(deleted, t->text + c->selection_begin, selection_range);
    }

    while (repeat > 0) {
        repeat--;

        _delete_recalculate_lines(c, l);

        c->pos--;
        memmove(t->text + c->pos, t->text + c->pos+1, t->size - c->pos+1);
        t->text[t->size] = '\0';
        t->size--;
    }

    selection_reset(c);

    return deleted;
}

void delete_text(Text *t, Cursor *c, Lines *l) {
/// Deleting a selection: 
// dumb way: put cursor in selection_end, delete char per char in a loop
// not so dumb easy way: 
//   if there one of two line selected just do the easy way
//   if there are multiple lines selected we:
//      - easy way on final line 
//      - delete all betwen lines, update selection lines.lines etc 
//      - easy way on first line 
//      - memmove?

    //lets go easy (dumb) way for now 
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
        memmove(t->text + c->pos, t->text + c->pos+1, t->size - c->pos+1);
        t->text[t->size] = '\0';
        t->size--;
    }

    selection_reset(c);
}


void resize_lines(Lines *lines) {
    const size_t new_capacity = lines->capacity * 2;
    Line *new_lines = realloc(lines->lines, sizeof(Line) * new_capacity);
    if (new_lines == NULL) {
        fprintf(stderr, "Memory reallocation failed\n");
        free(lines->lines); 
        exit(1);
    }
    lines->lines = new_lines;
    lines->capacity = new_capacity;
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
    const size_t len = strlen(source);
    if (len == 0) return false;

    for (size_t i = 0; i < len; ++i) {
        if (source[i] == '\n') new_line(text, lines, cursor);
        else if (source[i] == '\0') break;
        else insert_text(text, source[i], cursor, lines);
    }
    return true;
}

void copy_text(Text *text, Cursor *cursor) {
    const size_t range = cursor->selection_end - cursor->selection_begin;
    if (range <= 0) return;
    char copied_text[range];
    strncpy(copied_text, text->text + cursor->selection_begin, range);
    copied_text[range] = '\0';
    printf("copied_text: %s\n", copied_text);
    SetClipboardText(copied_text);
    cursor->selection_begin = cursor->pos;
    cursor->selection_end = cursor->pos;
    cursor->selection_line_begin = cursor->current_line;
    cursor->selection_line_end = cursor->current_line;
}

bool cut_text(Text *text, Cursor *cursor, Lines *lines) {
    size_t selection_range = cursor->selection_end - cursor->selection_begin; //NOTE be careful when begin > end, selection_range cound end up being a big num
    if (selection_range == 0) return false;
    char *_d = _delete_text(text, cursor, lines);
    SetClipboardText(_d);
    free(_d);
    return true;
}
