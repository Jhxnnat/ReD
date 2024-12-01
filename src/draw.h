#ifndef DRAW_H
#define DRAW_H

#include <stdlib.h>

#include "../raylib//include/raylib.h"
#include "ds.h"

void draw_text_tokenized(const char *text, Font font, Vector2 position, float fontSize, float spacing);
void draw_line_numbers(Camera2D camera, Font font, Vector2 font_measuring, Lines lines);
void draw_selection(Cursor cursor, Lines lines, Text text, Font font, Vector2 font_measuring);

#endif // !DRAW_H
