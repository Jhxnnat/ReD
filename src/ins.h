#ifndef INS_H

#include "nav.h"

void insert_text(Text *t, char c, Cursor *cu, Lines *lines);
void delete_text(Text *t, Cursor *cur, Lines *lines);
char *_delete_text(Text *t, Cursor *cur, Lines *lines);

void paste_text(Text *text, Cursor *cursor, Lines *lines);
void copy_text(Text *text, Cursor *cursor, Lines *lines);
void cut_text(Text *text, Cursor *cursor, Lines *lines);

void resize_lines(Lines *lines);
void add_line(Lines *lines, size_t line_num, size_t start, size_t end);
void new_line(Text *text, Lines *lines, Cursor *c);

#endif // !INS_H
