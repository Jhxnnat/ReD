#ifndef EXPLORER_H
#define EXPLORER_H

#include <stdlib.h>
#include "../raylib/include/raylib.h"

typedef struct {
    char path[255];
    size_t path_capacity;

    char working_directory[255];
    char current_file[255];
    FilePathList filepath_list;
    bool should_free;

    // graphical
    int cursor;
    int y_offset;
    int lines_amount;
} Explorer;

void explorer_init(Explorer *e);
void explorer_load_path(Explorer *e, const char *path);
void explorer_load_prevpath(Explorer *e);
void explorer_draw(Explorer *e, Font font, Vector2 font_measuring);
void explorer_free(Explorer *e);
void explorer_input(Explorer *e);

#endif // !EXPLORER_H
