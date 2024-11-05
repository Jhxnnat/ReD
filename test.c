
#include <stdlib.h>
#include <stdio.h>

typedef struct {
    size_t start;
    size_t end;
} Line;

typedef struct {
    size_t size;
    size_t capacity;
    Line *lines;
} Lines;

void init_lines(Lines *lines, size_t initial_capacity) {
    lines->size = 0;
    lines->capacity = initial_capacity;
    lines->lines = (Line *)malloc(sizeof(Line) * initial_capacity);
    if (lines->lines == NULL) {
        // Handle malloc failure
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
}

void resize_lines(Lines *lines) {
    size_t new_capacity = lines->capacity * 2;
    Line *new_lines = realloc(lines->lines, sizeof(Line) * new_capacity);
    if (new_lines == NULL) {
        // Handle realloc failure
        fprintf(stderr, "Memory reallocation failed\n");
        free(lines->lines); // free original allocation if realloc fails
        exit(1);
    }

    lines->lines = new_lines;
    lines->capacity = new_capacity;
}

void add_line(Lines *lines, size_t start, size_t end) {
    if (lines->size == lines->capacity) {
        resize_lines(lines);
    }

    lines->lines[lines->size].start = start;
    lines->lines[lines->size].end = end;
    lines->size++;
}

void free_lines(Lines *lines) {
    free(lines->lines);
    lines->lines = NULL;  // Avoid dangling pointer
    lines->size = 0;
    lines->capacity = 0;
}

int main() {
    Lines lines;
    init_lines(&lines, 10);

    // Add some lines
    add_line(&lines, 0, 5);
    add_line(&lines, 6, 10);
    add_line(&lines, 11, 20);

    // Print out the lines
    for (size_t i = 0; i < lines.size; i++) {
        printf("Line %zu: start = %zu, end = %zu\n", i, lines.lines[i].start, lines.lines[i].end);
    }

    free_lines(&lines);
    return 0;
}

