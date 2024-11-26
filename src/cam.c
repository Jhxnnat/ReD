// #include "raylib.h"
#include "../raylib/include/raylib.h"
#include "cam.h"

void init_camera(Camera2D *camera) {
  camera->target = (Vector2){ 0, 0 };
  camera->rotation = 0.0f;
  camera->zoom = 1.0f;
}
void update_cam_offset_up(Cursor *cursor, Lines *lines){
  size_t relative_line = cursor->current_line - lines->offset;
  if (lines->offset > 0 && relative_line <= 0) {
    lines->offset--;
  }
}
void update_cam_offset_down(Cursor *cursor, Lines *lines, int max_lines){
  size_t relative_line = cursor->current_line - lines->offset;
  if (lines->offset < lines->size && (int)relative_line > max_lines - 1) {
    lines->offset++;
  }
}

//WARNING, positioning BUG keep working on this------------
void move_cam_left(Camera2D *camera, int cursor_x, int font_height) {
  int cam_left = camera->target.x + RTEXT_LEFT + font_height;
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

