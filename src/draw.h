#ifndef DRAW_H
#define DRAW_H

#include <stdlib.h>

#include "../raylib/include/raylib.h"
#include "ds.h"

void draw_text_tokenized(const int *text, Font font, Vector2 position, float fontSize, float spacing);
void draw_line_numbers(Camera2D camera, Font font, Vector2 font_measuring, Lines lines, int text_left_pos);
void draw_selection(Cursor cursor, Lines lines, Vector2 font_measuring, int text_left_pos);
void draw_text_tokenized_optimized(const int *text, Lines lines, 
                                   Font font, Vector2 font_measuring, Vector2 position, float fontSize, float spacing, int max_lines);
void draw_text_optimized(Editor editor, Vector2 position, Color color);

#endif // !DRAW_H
