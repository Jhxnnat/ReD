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

void cursor_move_h(Cursor *cursor, Lines *lines, Text text, bool left) {
    if (text.size < 1) return;

    if (left && cursor->pos > 0) {
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
    else if (!left && cursor->pos < text.size) {
        if (cursor->current_line < lines->size-1 && cursor->pos == lines->lines[cursor->current_line].end-1) {
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

    if (next_line_len <= 1) {
        cursor->column = 0;
        cursor->pos = lines->lines[current_line].start;
    }
    else if (cursor->column > next_line_len)  {
        cursor->column = next_line_len-1;
        cursor->pos = lines->lines[current_line].end-1;
    } else {
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
    if (cursor->current_line == lines->size-1) {
        cursor->pos = lines->lines[cursor->current_line].end;
    } else {
        cursor->pos = lines->lines[cursor->current_line].end - 1;
    }
    cursor->column = cursor->pos - lines->lines[cursor->current_line].start;
    if (!cursor->is_selecting) selection_reset(cursor);
    else __selection_move_right(cursor);
}

void cursor_move_start(Cursor *cursor) {
    size_t prev_pos = cursor->pos;
    cursor->pos = 0;
    cursor->column = 0;
    cursor->current_line = 0;
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

void update_cursor_display(Editor *e) { //NOTE only works with monospaced fonts
    e->cursor_display.x = e->text_left_pos + e->cursor->column * (e->font_measuring.x + RFONT_SPACING);
    e->cursor_display.y = RTEXT_TOP + (e->font_measuring.y * (e->cursor->current_line)) + (RFONT_SPACING * e->cursor->current_line);
}
