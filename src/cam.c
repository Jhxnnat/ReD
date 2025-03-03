// #include "raylib.h"
#include "../raylib/include/raylib.h"
#include "cam.h"

void init_camera(Camera2D *camera) {
    camera->target = (Vector2){ 0, 0 };
    camera->rotation = 0.0f;
    camera->zoom = 1.0f;
}

void update_cam_offset_up(Cursor *cursor, Lines *lines){
    int relative_line = cursor->current_line - lines->offset;
    if (lines->offset > 0 && relative_line <= 0) {
        lines->offset--;
    }
}

void update_cam_offset_down(Cursor *cursor, Lines *lines, int max_lines){
    int relative_line = cursor->current_line - lines->offset;
    if (lines->offset < lines->size && relative_line > max_lines - 1) {
        lines->offset++;
    }
}

//NOTE keep working on this
void move_cam_left(Camera2D *camera, int cursor_x, int font_height, int text_left_pos) {
    int cam_left = camera->target.x + text_left_pos + font_height;
    if (cursor_x < cam_left) {
        camera->target.x -= cam_left - cursor_x;
        if (camera->target.x < 0) camera->target.x = 0; //hardcoded?
    }
}

void move_cam_right(Camera2D *camera, int cursor_x, int font_height) {  
    int cam_right = camera->target.x + GW - font_height;
    if (cursor_x > cam_right) {
        camera->target.x += cursor_x - cam_right;
    }
}
//-------------------------------------

void move_cam_start(Camera2D *camera, Lines *lines) {
    camera->target.x = 0;
    lines->offset = 0;
}

void move_cam_end(Camera2D *camera, Lines *lines, int max_lines) {
    camera->target.x = 0;
    int _off = (int)lines->size - max_lines - 1;
    if (_off < 0) _off = 0;
    lines->offset = (size_t)_off;
}

void update_cam(Camera2D *camera, Editor *editor) {
    //NOTE 2 is a magic number, updating camera vertically works better with it, (just an offset)
    size_t range = editor->lines->offset + editor->max_lines - 2;
    if (editor->cursor->current_line > range) {
        editor->lines->offset += editor->cursor->current_line - range;
    } 
    else if (editor->cursor->current_line < editor->lines->offset) {
        editor->lines->offset -= (editor->lines->offset - editor->cursor->current_line);
    }

    int cam_left = camera->target.x + editor->text_left_pos + editor->font_measuring.y;
    if (editor->cursor_display.x < cam_left) {
        camera->target.x -= cam_left - editor->cursor_display.x;
        if (camera->target.x < 0) camera->target.x = 0;
    } else {
        int cam_right = camera->target.x + GW - editor->font_measuring.y;
        if (editor->cursor_display.x > cam_right) {
            camera->target.x += editor->cursor_display.x - cam_right;
        }
    }
}
