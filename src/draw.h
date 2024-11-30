#ifndef DRAW_H
#define DRAW_H

#include <stdlib.h>
#include "../raylib//include/raylib.h"

typedef enum {
  TOKEN_KEYWORD = 0,
  TOKEN_IDENTIFIER,
  TOKEN_PREPROC,
  TOKEN_STRING,
  TOKEN_COMMENT,
  TOKEN_NUMBER,
  TOKEN_EOL,
  TOKEN_OTHER,
  TOKEN_BREAKLINE,
} TokenType;

typedef struct {
  TokenType type;
  const char *start;
  size_t len;
  size_t line;

  size_t pos;
  Color color;
} Token;

typedef struct {
  const char* start;
  const char* current;
  size_t cursor;
  int line;
} Scanner;

// typedef struct {
//   Token *items;
//   size_t size;
//   size_t capacity;
// } TokenList;

void draw_text_tokenized(const char *text, Font font, Vector2 position, float fontSize, float spacing);

#endif // !DRAW_H
