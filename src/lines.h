#ifndef LINES_H

#include <stdlib.h>

typedef struct {
  size_t start;
  size_t end;
} Line;

typedef struct {
  size_t size;
  size_t capacity;
  Line   *lines;
} Lines;

#endif // !LINES_H
