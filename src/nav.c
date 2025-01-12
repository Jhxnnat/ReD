#include <stdio.h>
#include <string.h>

#include "nav.h"

void swap_values(size_t *x, size_t *y) {
    size_t temp = *x;
    *x = *y;
    *y = temp;
}

void selection_reset(Cursor *cursor) {
    cursor->selection_begin = cursor->pos;
    cursor->selection_end = cursor->pos;
    cursor->selection_line_begin = cursor->current_line;
    cursor->selection_line_end = cursor->current_line;
}

void __selection_move_left(Cursor *cursor) {
    if (cursor->is_selecting && cursor->selection_begin < cursor->pos) {
        cursor->selection_end = cursor->pos;
        cursor->selection_line_end = cursor->current_line;
    }
    else if (cursor->is_selecting && cursor->selection_begin >= cursor->pos) {
        cursor->selection_begin = cursor->pos;
        cursor->selection_line_begin = cursor->current_line;
    }
}

void __selection_move_right(Cursor *cursor) {
    if (cursor->is_selecting && cursor->selection_end > cursor->pos) {
        cursor->selection_begin = cursor->pos;
        cursor->selection_line_begin = cursor->current_line;
    }
    else if (cursor->is_selecting && cursor->selection_end <= cursor->pos) {
        cursor->selection_end = cursor->pos;
        cursor->selection_line_end = cursor->current_line;
    }
}

void __selection_move_up(Cursor *cursor, size_t prev_pos) {
    if (cursor->selection_end == prev_pos) {
        cursor->selection_end = cursor->pos;
        cursor->selection_line_end = cursor->current_line;
        if (cursor->selection_end < cursor->selection_begin) {
            swap_values(&cursor->selection_end, &cursor->selection_begin);
            swap_values(&cursor->selection_line_end, &cursor->selection_line_begin);
        }
    }
    else if (cursor->selection_begin == prev_pos) {
        cursor->selection_begin = cursor->pos;
        cursor->selection_line_begin = cursor->current_line;
    }
}

void __selection_move_down(Cursor *cursor, size_t prev_pos) {
    if (cursor->selection_end == prev_pos) { 
        cursor->selection_end = cursor->pos;
        cursor->selection_line_end = cursor->current_line;
    }
    else if (cursor->selection_begin == prev_pos) {
        cursor->selection_begin = cursor->pos;
        cursor->selection_line_begin = cursor->current_line;
        if (cursor->selection_begin > cursor->selection_end) {
            swap_values(&cursor->selection_end, &cursor->selection_begin);
            swap_values(&cursor->selection_line_end, &cursor->selection_line_begin);
        }
    }
}

void cursor_move_h(Cursor *cursor, Lines *lines, bool left) {
    if (left) {
        if (cursor->column == 0 && cursor->current_line > 0) {
            cursor->current_line--;
            cursor->column = lines->lines[cursor->current_line].end-lines->lines[cursor->current_line].start-1;
            cursor->pos--;
        } else {
            cursor->pos--;
            cursor->column--;
        }

        __selection_move_left(cursor);
    } 
    else {
        if (cursor->pos == lines->lines[cursor->current_line].end-1 && cursor->current_line < lines->size-1) {
            cursor->current_line++;
            cursor->column = 0;
            cursor->pos++;
        } else {
            cursor->pos++;
            cursor->column++;
        }

        __selection_move_right(cursor);
    }

    if (!cursor->is_selecting) {
        selection_reset(cursor);
    }
}

void cursor_move_v(Cursor *cursor, Lines *lines, int dir) {
    if (dir > 0 && cursor->current_line == lines->size-1) return;
    if (dir < 0 && cursor->current_line == 0) return;

    size_t prev_pos = cursor->pos;
    cursor->current_line += dir;
    size_t current_line = cursor->current_line;
    size_t next_line_len = lines->lines[current_line].end - lines->lines[current_line].start;

    if (cursor->column > next_line_len) {
        cursor->column = next_line_len - 1;
        cursor->pos = lines->lines[current_line].end-1;
    }
    else {
        cursor->pos = lines->lines[current_line].start + cursor->column;
    }

    if (!cursor->is_selecting) {
        selection_reset(cursor);
        return;
    }

    if (dir > 0) { __selection_move_down(cursor, prev_pos); }
    else {__selection_move_up(cursor, prev_pos); }
}

void cursor_move_sol(Cursor *cursor, Lines *lines) {
    cursor->pos = lines->lines[cursor->current_line].start;
    cursor->column = 0;

    if (!cursor->is_selecting) selection_reset(cursor);
    else __selection_move_left(cursor);
}

void cursor_move_eol(Cursor *cursor, Lines *lines) {
    size_t _len = lines->lines[cursor->current_line].end - lines->lines[cursor->current_line].start;
    if (_len > 0) {
        int _off = 1;
        if (lines->size <= 1 || cursor->current_line == lines->size-1) {
            _off = 0;
        }
        cursor->column = _len;
        cursor->pos = lines->lines[cursor->current_line].end - _off;
    } else {
        cursor->column = 0;
        cursor->pos = lines->lines[cursor->current_line].end;
    }

    if (!cursor->is_selecting) selection_reset(cursor);
    else __selection_move_right(cursor);
}

void cursor_move_start(Cursor *cursor) {
    size_t prev_pos = cursor->pos;
    cursor->pos = 0;
    cursor->column = 0;
    cursor->current_line = 0;

    // lines->offset = 0;
    if (!cursor->is_selecting) selection_reset(cursor);
    else __selection_move_up(cursor, prev_pos); 
}

void cursor_move_end(Cursor *cursor, Lines *lines) {
    size_t prev_pos = cursor->pos;
    cursor->current_line = lines->size-1;
    cursor->pos = lines->lines[cursor->current_line].start;
    cursor->column = 0;

    if (!cursor->is_selecting) selection_reset(cursor);
    else __selection_move_down(cursor, prev_pos);
}

void update_cursor_display(Editor *e) {
    size_t _range = e->cursor->pos - e->lines->lines[e->cursor->current_line].start;
    if (_range <= 0) {
        e->cursor_display.x = RTEXT_LEFT;
    } else {
        char _part[_range];
        strncpy(_part, e->text->text + e->lines->lines[e->cursor->current_line].start, _range);
        _part[_range] = '\0';
        Vector2 _text_measure = MeasureTextEx(e->font, _part, e->font.baseSize, RFONT_SPACING);
        e->cursor_display.x = RTEXT_LEFT + _text_measure.x;
    }
    e->cursor_display.y = RTEXT_TOP + (e->font_measuring.y * (e->cursor->current_line)) + (RFONT_SPACING * e->cursor->current_line);
}
