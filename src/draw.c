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

    for (size_t i = 0; i < token.len; ++i) {
        // int codepointByteCount = 0;
        int codepoint = token.start[i];// GetCodepointNext(&token.start[i], &codepointByteCount);
        int index = GetGlyphIndex(font, codepoint);
        if (codepoint == '\n') break;
        else {
            if ((codepoint != ' ') && (codepoint != '\t')) {
                DrawTextCodepoint(font, codepoint, (Vector2){ position.x + *textOffsetX, position.y + textOffsetY }, fontSize, tint);
            }
            if (font.glyphs[index].advanceX == 0) { *textOffsetX += ((float)font.recs[index].width * scaleFactor + spacing); }
            else { *textOffsetX += ((float)font.glyphs[index].advanceX * scaleFactor + spacing); }
        }
        // i += codepointByteCount;
    }
}

void draw_text_tokenized(const int *text, Font font, Vector2 position, float fontSize, float spacing){
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

void draw_text_tokenized_optimized(const int *text, Lines lines, 
                                   Font font, Vector2 font_measuring, Vector2 position, float fontSize, float spacing) {
    Scanner scanner;
    scanner.cursor = 0;
    scanner.line = 1;
    
    size_t text_from = lines.lines[lines.offset].start;
    scanner.start = text+text_from;
    scanner.current = text+text_from;

    int max_lines = (GH/font_measuring.y) - 3;

    int line = -1;
    float textOffsetX = 0.0f;
    for (;;) {
        Token token = scan_token(&scanner);
        if ((int)token.line != line) {
            line = token.line;
            textOffsetX = 0.0f;
        }
        Vector2 new_position = {
            position.x,
            position.y + (lines.offset * font_measuring.y) + (lines.offset * spacing)
        };
        draw_text_line_token(token, &textOffsetX, font, new_position, fontSize, spacing);
        if (token.type == TOKEN_EOL || line >= max_lines) break;
    }
}

void draw_text_optimized(Text text, Lines lines, Font font, Vector2 font_measuring, Vector2 position, float fontSize, float spacing, Color color) {
    size_t text_start = lines.lines[lines.offset].start;
    int max_lines = (GH/font_measuring.y) - 3;
    size_t text_end;
    if (lines.offset + max_lines >= lines.size) { text_end = lines.lines[lines.size-1].end; }
    else { text_end = lines.lines[lines.offset + max_lines].end; }

    Vector2 newpos = position;
    newpos.y = newpos.y + (lines.offset * font_measuring.y) + (lines.offset * spacing);
    DrawTextCodepoints(font, text.buff+text_start, text_end - text_start, newpos, fontSize, spacing, color);
}

void draw_line_numbers(Camera2D camera, Font font, Vector2 font_measuring, Lines lines){
    size_t max_lines = (GH/font_measuring.y) - 3 + lines.offset; //NOTE yes, arbitrary num '3'
    if (max_lines > lines.size + 1) { max_lines = lines.size + 1; } 
    size_t top = lines.offset;

    for (size_t i = top; i < max_lines; ++i) {
        const char *t = TextFormat("%d", i);
        Vector2 _meassure_t = MeasureTextEx(font, t, font.baseSize, RFONT_SPACING);
        Vector2 pos = {
            camera.target.x + RTEXT_LEFT-(_meassure_t.x)-10,
            RTEXT_TOP+(font_measuring.y*(i-1))+(RFONT_SPACING*(i-1))
        };
        DrawTextEx(font, t, pos, (float)font.baseSize, RFONT_SPACING, RGRAY);
    }
}

void draw_selection(Cursor cursor, Lines lines, Vector2 font_measuring){
    int select_range = cursor.selection_end - cursor.selection_begin;
    if (select_range <= 0) return;
    Color select_color = { 255, 255, 255, 80 };
    int _range, _x, _w, _y, _h = font_measuring.y;
    for (size_t i = cursor.selection_line_begin; i <= cursor.selection_line_end; ++i) {
        if (i == cursor.selection_line_begin) { //first line
            _x = RTEXT_LEFT + ((cursor.selection_begin - lines.lines[i].start) * (font_measuring.x + RFONT_SPACING));
            if (i == cursor.selection_line_end) _range = cursor.selection_end - cursor.selection_begin;
            else _range = lines.lines[i].end - cursor.selection_begin;
        }
        else if (i == cursor.selection_line_end) { //last line
            _x = RTEXT_LEFT;
            _range = cursor.selection_end - lines.lines[i].start;
        }
        else { //between lines
            _x = RTEXT_LEFT;
            _range = lines.lines[i].end - lines.lines[i].start;
        }
        _y = RTEXT_TOP+(font_measuring.y * i) + (RFONT_SPACING * i);
        _w = (font_measuring.x + RFONT_SPACING) * (_range);
        DrawRectangle(_x, _y, _w, _h, select_color);
    }
}
