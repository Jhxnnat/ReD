#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include "ds.h"
#include "explorer.h"

void explorer_init(Explorer *e) {
    e->cursor = 0;
    e->working_directory = "";
    e->should_free = false;
    e->y_offset = 0;

    e->path_capacity = 16;
    e->path = malloc(sizeof(char) * e->path_capacity);
}

void explorer_load_path(Explorer *e, const char *path) {
    if (strcmp(e->path, path) != 0) {
        e->filepath_list = LoadDirectoryFiles(path);
        e->should_free = true;

        if (strlen(path) > e->path_capacity) { //TODO introduce a Dstring struct ? (dynamic string)
            e->path_capacity *= 2;
            e->path = realloc(e->path, sizeof(char) * e->path_capacity);
        }
        strcpy(e->path, path); //NOTE be carefull
    }
    e->cursor = 0;
}

void explorer_load_prevpath(Explorer *e) {
    const char *prev_directory = GetPrevDirectoryPath(e->path);
    if (DirectoryExists(prev_directory)) {
        explorer_load_path(e, prev_directory);
    }
}

void explorer_draw(Explorer *e, Font font, Vector2 font_measuring) {
    DrawTextEx(font, "../", (Vector2){20, 20}, font.baseSize, RFONT_SPACING, PURPLE);
    if (e->cursor == -1) {
        DrawRectangle(20, 20, GW-40, font_measuring.y, (Color){255, 255, 255, 100});
    }
    for (size_t i = 0; i < e->filepath_list.count; ++i) {
        DrawTextEx(font, e->filepath_list.paths[i], (Vector2){20, 20 + (font_measuring.y * (i+1)) }, font.baseSize, RFONT_SPACING, WHITE);
        if (e->cursor == (int)i) {
            DrawRectangle(20, 20 + (font_measuring.y * (i+1)), GW-40, font_measuring.y, (Color){255, 255, 255, 100});
        }
    }
}

void explorer_free(Explorer *e) {
    if (e->should_free) {
        UnloadDirectoryFiles(e->filepath_list);
    }
    free(e->path);
}

void explorer_input(Explorer *e) {
    if (IsKeyPressed(KEY_I) && e->cursor > -1) {
        e->cursor--;
    } else if (IsKeyPressed(KEY_K) && e->cursor+1 < (int)e->filepath_list.count) {
        e->cursor++;
    }
}
