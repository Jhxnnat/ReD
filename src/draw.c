// #include <ctype.h>
#include <stdio.h>
// #include <stdlib.h>
#include <string.h>

// #include "ds.h"
#include "draw.h"

const char *token_name(TokenType kind){
  switch (kind) {
    case TOKEN_KEYWORD: return "token keyword";
    case TOKEN_STRING: return "a string";
    case TOKEN_COMMENT: return "a commet";
    case TOKEN_OTHER: return "token other";
    case TOKEN_BREAKLINE: return "a breakline";
    case TOKEN_EOL: return "EOF";
    case TOKEN_IDENTIFIER: return "token identifier";
    case TOKEN_PREPROC: return "token preproc";
    case TOKEN_NUMBER: return "token num";
    default: return "unknown token";
  }
}

Color token_color(TokenType kind){
  switch (kind) {
    case TOKEN_KEYWORD: return ORANGE;
    case TOKEN_STRING: return LIME;
    case TOKEN_COMMENT: return GRAY;
    case TOKEN_OTHER: return WHITE;
    case TOKEN_IDENTIFIER: return SKYBLUE;
    case TOKEN_PREPROC: return RED;
    case TOKEN_NUMBER: return PURPLE;
    default: return WHITE;
  }
}

bool is_at_end(const char *c) {
  return *c == '\0';
}

Token make_token(Scanner *scanner, TokenType type) {
  Token token;
  token.type = type;
  token.start = scanner->start;
  token.len = (size_t)(scanner->current - scanner->start);
  token.line = scanner->line;
  return token;
}

char advance(Scanner *scanner) {
  scanner->current++;
  scanner->cursor++;
  return scanner->current[-1];
}

char peek(Scanner *scanner) {
  return *scanner->current;
}

char peek_next(Scanner *scanner) {
  if (is_at_end(scanner->current)) return '\0';
  return scanner->current[1];
}

bool skip_whitespace(Scanner *scanner) {
  for (;;) {
    char c = peek(scanner);
    switch(c) {
      // case ' ':
      // case '\r':
      // case '\t':
      //   advance(scanner);
      //   break;
      case '\n':
        scanner->line++;
        advance(scanner);
        return true;
        break;
      default:
        return false;
    }
  }
}

Token make_preproc(Scanner *scanner) {
  while (peek(scanner) != '\n' && peek_next(scanner) != '\n' && !is_at_end(scanner->current)) {
    // if (peek(scanner) == '\\') scanner->line++;
    advance(scanner);
  }

  if (is_at_end(scanner->current)) return make_token(scanner, TOKEN_PREPROC);

  advance(scanner);
  return make_token(scanner, TOKEN_PREPROC);
}

Token make_string(Scanner *scanner) { 
  while (peek(scanner) != '"' && !is_at_end(scanner->current)) {
    // if (peek(scanner) == '\n') scanner->line++; //TODO this is causing bugs with breakline token
    if (peek(scanner) == '\n') {
      return make_token(scanner, TOKEN_STRING);
    }
    advance(scanner);
  }

  if (is_at_end(scanner->current)) return make_token(scanner, TOKEN_OTHER);

  advance(scanner);
  return make_token(scanner, TOKEN_STRING);
}

bool is_digit(char c) {
  return c >= '0' && c <= '9';
}

Token make_number(Scanner *scanner) {
  while (is_digit(peek(scanner))) advance(scanner);

  if (peek(scanner) == '.' && is_digit(peek_next(scanner))) {
    advance(scanner);

    while (is_digit(peek(scanner))) advance(scanner);
  }

  return make_token(scanner, TOKEN_NUMBER);
}

bool is_alpha(char c) {
  return (c >= 'a' && c <= 'z') ||
    (c >= 'A' && c <= 'Z') ||
    c == '_';
}

TokenType check_keyword(Scanner *scanner, const char *rest) {
  //TODO make a definition for char len
  const size_t len = strlen(rest);
  const size_t len2 = scanner->current - scanner->start;
  if (len2 == len && memcmp(scanner->start, rest, len) == 0) {
    return TOKEN_KEYWORD;
  }

  return TOKEN_IDENTIFIER;
}

TokenType identifier_type(Scanner *scanner) {
  switch (scanner->start[0]) {
    case 'i':
      if (scanner->current - scanner->start > 1) {
        switch(scanner->start[1]) {
          case 'n': return check_keyword(scanner, "int");
          case 'f': return TOKEN_KEYWORD;
        }
      }
      return check_keyword(scanner, "if");
    case 'c': return check_keyword(scanner, "char");
    case 'e': return check_keyword(scanner, "else");
    case 'p': return check_keyword(scanner, "printf");
    case 'r': return check_keyword(scanner, "return");
    case 'w': return check_keyword(scanner, "while");
    case 'v': return check_keyword(scanner, "void");
    // case 'm': return check_keyword(scanner, "main");
  }
  
  return TOKEN_IDENTIFIER;
}

Token make_identifier(Scanner *scanner) {
  while (is_alpha(peek(scanner)) || is_digit(peek(scanner))) advance(scanner);
  return make_token(scanner, identifier_type(scanner));
}

Token scan_token(Scanner *scanner) {
  scanner->start = scanner->current;
  
  if (is_at_end(scanner->current)) return make_token(scanner, TOKEN_EOL);

  char c = advance(scanner);
  if (is_alpha(c)) return make_identifier(scanner);
  if (is_digit(c)) return make_number(scanner);

  if (c == '\n') {
    scanner->line++;
    return make_token(scanner, TOKEN_BREAKLINE);
  }

  switch (c) {
    case '#': return make_preproc(scanner);
    case '"': return make_string(scanner);
  }

  return make_token(scanner, TOKEN_OTHER);
}


void draw_text_highlighted_token(Token token, float *textOffsetX, Font font, Vector2 position, float fontSize, float spacing) {
  Color tint = token_color(token.type);
  const int textLineSpacing = 2;
  if (font.texture.id == 0) font = GetFontDefault();
  float textOffsetY = (fontSize + textLineSpacing) * (token.line - 1);
  float scaleFactor = fontSize / font.baseSize;

  for (int i = 0; i < token.len;) {
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

void scanner(const char *text, Font font, Vector2 position, float fontSize, float spacing){
  Scanner scanner;
  scanner.start = text;
  scanner.current = text;
  scanner.cursor = 0;
  scanner.line = 1;

  int line = -1;
  float textOffsetX = 0.0f;
  for (;;) {
    Token token = scan_token(&scanner);
    if (token.line != line) {
      // printf("%zu ", token.line);
      line = token.line;
      textOffsetX = 0.0f;
    } else {
      // printf("   | ");
    }
    // printf("%s, cursor: %zu - '%.*s'\n", token_name(token.type), scanner.cursor, (int) token.len, token.start);

    // if (token.type == TOKEN_BREAKLINE) textOffsetX = 0.0f;
    
    // printf("token line: %zu\n", token.line);
    draw_text_highlighted_token(token, &textOffsetX, font, position, fontSize, spacing);

    if (token.type == TOKEN_EOL) break;
  }
}

