#ifndef TEXT_H

#include <stdlib.h>
#include "lines.h"
#include "cursor.h"

typedef struct {
  size_t capacity;
  size_t size;
  char   *text;
} Text;

void init_text(Text *t, size_t size);
void insert_text(Text *t, char c, Cursor *cu, Lines *lines);
void delete_text(Text *t, Cursor *cur, Lines *lines);
void free_text(Text *t);

#endif // ! TEXT_H
