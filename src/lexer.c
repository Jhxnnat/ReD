#include <string.h>
#include "ds.h"
#include "lexer.h"

#include <stdio.h>

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
        case TOKEN_KEYWORD: return RKEYWORD;
        case TOKEN_STRING: return RSTRING;
        case TOKEN_COMMENT: return RCOMMENT;
        case TOKEN_OTHER: return ROTHER;
        case TOKEN_IDENTIFIER: return RFG;
        case TOKEN_PREPROC: return RPREPROC;
        case TOKEN_NUMBER: return RNUMBER;
        default: return RFG;
    }
}

bool is_at_end(const int *c) {
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

int advance(Scanner *scanner) {
    scanner->current++;
    scanner->cursor++;
    return scanner->current[-1];
}

int peek(Scanner *scanner) {
    return *scanner->current;
}

int peek_next(Scanner *scanner) {
    if (is_at_end(scanner->current)) return '\0';
    return scanner->current[1];
}

bool skip_whitespace(Scanner *scanner) {
    for (;;) {
        int c = peek(scanner);
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
        if (peek(scanner) == '\n') {
            return make_token(scanner, TOKEN_PREPROC);
        }
        advance(scanner);
    }
    if (is_at_end(scanner->current)) return make_token(scanner, TOKEN_PREPROC);
    advance(scanner);
    return make_token(scanner, TOKEN_PREPROC);
}

Token make_string(Scanner *scanner) { 
    while (peek(scanner) != '"' && !is_at_end(scanner->current)) {
        if (peek(scanner) == '\n') {
            return make_token(scanner, TOKEN_STRING);
        }
        advance(scanner);
    }
    if (is_at_end(scanner->current)) return make_token(scanner, TOKEN_STRING);
    advance(scanner);
    return make_token(scanner, TOKEN_STRING);
}

Token make_comment(Scanner *scanner) {
    while (peek(scanner) != '\n' && peek_next(scanner) != '\n' && !is_at_end(scanner->current)) {
        advance(scanner);
    }
    if (is_at_end(scanner->current)) return make_token(scanner, TOKEN_COMMENT);
    advance(scanner);
    return make_token(scanner, TOKEN_COMMENT);
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
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool compare(const int *_a, const char *_b, int len) {
    for (int i = 0; i < len; ++i) {
        // printf("[compare] a: %d b: %d\n", _a[i], (int)_b[i]);
        if (_a[i] != (int)_b[i]) return false;
    }
    // printf("\n");
    return true;
}

TokenType check_keyword(Scanner *scanner, const char *rest) {
    const int len = strlen(rest);
    const int len2 = scanner->current - scanner->start;
    if (len2 == len && compare(scanner->start, rest, len)) {
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
            break;
        case 's': 
            if (scanner->current - scanner->start > 1) {
                switch(scanner->start[1]) {
                    case 'i': return check_keyword(scanner, "size_t");
                    case 't': return check_keyword(scanner, "struct");
                    case 'w': return check_keyword(scanner, "switch");
                }
            }
            break;
        case 'c':
            if (scanner->current - scanner->start > 1) {
                switch(scanner->start[1]) {
                    case 'h': return check_keyword(scanner, "char");
                    case 'a': return check_keyword(scanner, "case");
                    case 'o': return check_keyword(scanner, "const");
                }
            }
            break;
        case 'e': return check_keyword(scanner, "else");
        case 'p': return check_keyword(scanner, "printf");
        case 'r': return check_keyword(scanner, "return");
        case 'w': return check_keyword(scanner, "while");
        case 'v': return check_keyword(scanner, "void");
        case 't': return check_keyword(scanner, "typedef");
        case 'f':
            if (scanner->current - scanner->start > 1) {
                switch(scanner->start[1]) {
                    case 'o': return check_keyword(scanner, "for");
                    case 'l': return check_keyword(scanner, "float");
                }
            }
            break;
        case 'b': return check_keyword(scanner, "bool");
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

    int c = advance(scanner);
    if (is_alpha(c)) return make_identifier(scanner);
    if (is_digit(c)) return make_number(scanner);

    if (c == '\n') {
        scanner->line++;
        return make_token(scanner, TOKEN_BREAKLINE);
    }

    switch (c) {
        case '#': return make_preproc(scanner);
        case '"': return make_string(scanner);
        case '/':
			if (peek(scanner) == '/') { return make_comment(scanner); }
			break;
    }

    return make_token(scanner, TOKEN_OTHER);
}

