#ifndef EXPLORER_H
#define EXPLORER_H

#include <stdlib.h>
#include "../raylib/include/raylib.h"

#define MAX_PATH_LEN 512
#define MAX_FILEPATH_LEN 128
#define MAX_BUFF_EXPLORER 64

typedef enum {
    NORMAL,
    RENAME,
    CREATE,
    DELETE
} ExplorerMode;

typedef struct {
    char path[MAX_PATH_LEN];
    size_t path_capacity;

    char current_file[MAX_FILEPATH_LEN];
    FilePathList filepath_list;
    bool should_free;

    ExplorerMode mode;
    int buffer[MAX_BUFF_EXPLORER];
    int buff_cursor;

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
int explorer_cacl_lines(float font_measuring_y);

#endif // !EXPLORER_H
