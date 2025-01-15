#ifndef INS_H
#define INS_H

#include "nav.h"

void insert_text(Text *t, int c, Cursor *cu, Lines *lines);
void delete_text(Text *t, Cursor *c, Lines *lines);
void insert_text_from_file(const char *path, Text *text, Lines *lines, Cursor *cursor);

void copy_text(Text *text, Cursor *cursor);
bool paste_text(Text *text, Cursor *cursor, Lines *lines);
bool cut_text(Text *text, Cursor *cursor, Lines *lines);

void resize_lines(Lines *lines);
void update_lines(Lines *lines, size_t line_num, size_t start, size_t end);
void new_line(Text *text, Lines *lines, Cursor *c);

#endif // !INS_H
