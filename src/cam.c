#include "raylib.h"
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
  if (lines->offset < lines->size && relative_line > max_lines - 1) {
    lines->offset++;
  }
}

void move_cam_left(Camera2D *camera, int cursor_x, int font_height) { //BUG Unfinished
  int cam_left = camera->target.x + RTEXT_LEFT + font_height;
  if (cursor_x < cam_left) {
    camera->target.x -= cam_left - cursor_x;
  }
  else if (cursor_x > cam_left) {
    camera->target.x = cursor_x - cam_left;
  }
}

void move_cam_right(Camera2D *camera, int cursor_x, int font_height) { //TODO keep working on this 
  int cam_right = camera->target.x + GW - font_height;
  if (cursor_x > cam_right) {
    camera->target.x += cursor_x - cam_right;
  }
  else if (cursor_x < cam_right) {
    camera->target.x = 0;
  }
}

void move_cam_start(Lines *lines) {
  lines->offset = 0;
}

void move_cam_end(Lines *lines, int max_lines) {
  long _off = (long)lines->size - max_lines - 1;
  if (_off < 0) _off = 0;
  lines->offset = (size_t)_off;
}

// bool is_cursor_outbound_top(int cursor_y, int vert_margin) {
//   if (cursor_y < vert_margin) {
//     return true;
//   }
//   return false;
// }

// bool is_cursor_outbound_bottom(int cursor_y, int vert_margin) {
//   if (cursor_y > GH-vert_margin) {
//     return true;
//   }
//   return false;
// }
