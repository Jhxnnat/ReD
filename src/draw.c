// #include <ctype.h>
#include <stdio.h>
// #include <stdlib.h>
#include <string.h>

// #include "ds.h"
#include "draw.h"
#include "lexer.h"

void draw_text_line_token(Token token, float *textOffsetX, Font font, Vector2 position, float fontSize, float spacing) {
  Color tint = token_color(token.type);
  const int textLineSpacing = 2;
  if (font.texture.id == 0) font = GetFontDefault();
  float textOffsetY = (fontSize + textLineSpacing) * (token.line - 1);
  float scaleFactor = fontSize / font.baseSize;

  for (size_t i = 0; i < token.len;) {
    int codepointByteCount = 0;
    int codepoint = GetCodepointNext(&token.start[i], &codepointByteCount);
    int index = GetGlyphIndex(font, codepoint);
    if (codepoint == '\n') break;
    else {
      if ((codepoint != ' ') && (codepoint != '\t')) {
        DrawTextCodepoint(font, codepoint,
          (Vector2){ position.x + *textOffsetX, position.y + textOffsetY },
          fontSize, tint);
      }
      if (font.glyphs[index].advanceX == 0)
        *textOffsetX += ((float)font.recs[index].width * scaleFactor + spacing);
      else
        *textOffsetX += ((float)font.glyphs[index].advanceX * scaleFactor + spacing);
    }
    i += codepointByteCount;
  }
}

void draw_text_tokenized(const char *text, Font font, Vector2 position, float fontSize, float spacing){
  Scanner scanner;
  scanner.start = text;
  scanner.current = text;
  scanner.cursor = 0;
  scanner.line = 1;

  int line = -1;
  float textOffsetX = 0.0f;
  for (;;) {
    Token token = scan_token(&scanner);
    if ((int)token.line != line) {
      line = token.line;
      textOffsetX = 0.0f;
    }
    draw_text_line_token(token, &textOffsetX, font, position, fontSize, spacing);
    if (token.type == TOKEN_EOL) break;
  }
}

void draw_line_numbers(Camera2D camera, Font font, Vector2 font_measuring, Lines lines){
  ////Lines num NOTE consider draw a part of the lines to optimize 

  for (size_t i = 1; i < lines.size+1; ++i) {
    const char *t = TextFormat("%d", i);
    Vector2 _meassure_t = MeasureTextEx(font, t, font.baseSize, RFONT_SPACING);
    Vector2 pos = { camera.target.x + RTEXT_LEFT-(_meassure_t.x)-10, RTEXT_TOP+(font_measuring.y*(i-1))+(RFONT_SPACING*(i-1)) };
    DrawTextEx(font, t, pos, (float)font.baseSize, RFONT_SPACING, GRAY);
  }
}

Vector2 _measure_text_part(Text *text, Font font, size_t start, size_t range) {
  if (range <= 0) {
    Vector2 measurement_ = { 0, 0 };
    return measurement_;
  }
  char part[range];
  strncpy(part, text->text+start, range);
  part[range] = '\0';
  Vector2 measurement = MeasureTextEx(font, part, font.baseSize, RFONT_SPACING);
  return measurement;
}

void draw_selection(Cursor cursor, Lines lines, Text text, Font font, Vector2 font_measuring){
  //selection
  size_t select_range = cursor.selection_end - cursor.selection_begin;
  size_t select_line_range = cursor.selection_line_end - cursor.selection_line_begin;
  Color select_color = { 255, 255, 255, 80 };
  int _x, _w, _y, _h = font_measuring.y;
  //one line selected
  if (select_line_range == 0 && select_range > 0) {
    size_t _plsize = cursor.selection_begin-lines.lines[cursor.current_line].start;
    Vector2 _selection_l_measure = _measure_text_part(&text, font, lines.lines[cursor.current_line].start, _plsize);
    Vector2 _selection_measure = _measure_text_part(&text, font, cursor.selection_begin, select_range);
    _x = RTEXT_LEFT+(_selection_l_measure.x);
    _w = _selection_measure.x;
    _y = RTEXT_TOP+(font_measuring.y*cursor.current_line)+(RFONT_SPACING*cursor.current_line);
    DrawRectangle(_x, _y, _w, _h, select_color);
  }
  else if (select_line_range > 0 && select_range > 0) {
    for (size_t i = cursor.selection_line_begin; i <= cursor.selection_line_end; ++i) {
      _y = RTEXT_TOP+(font_measuring.y * i) + (RFONT_SPACING * i);

      if (i == cursor.selection_line_begin) { //first line
        size_t _plsize = cursor.selection_begin-lines.lines[i].start;
        Vector2 _selection_l_measure = _measure_text_part(&text, font, lines.lines[i].start, _plsize);
        Vector2 _selection_measure = _measure_text_part(&text, font, cursor.selection_begin, lines.lines[i].end - cursor.selection_begin);
        _x = RTEXT_LEFT+_selection_l_measure.x;
        _w = _selection_measure.x;
        DrawRectangle(_x, _y, _w, _h, select_color);
      }
      else if (i == cursor.selection_line_end) { //last line
        Vector2 _selection_measure = _measure_text_part(&text, font, lines.lines[i].start, cursor.selection_end-lines.lines[i].start);
        _x = RTEXT_LEFT;
        _w = _selection_measure.x;
        DrawRectangle(_x, _y, _w, _h, select_color);
      }
      else { //between lines
        _x = RTEXT_LEFT;
        size_t _range = lines.lines[i].end - lines.lines[i].start;
        Vector2 _line_measure = _measure_text_part(&text, font, lines.lines[i].start, _range);
        _w = _line_measure.x;
        DrawRectangle(_x, _y, _w, _h, select_color);
      }
    }
  }
}
