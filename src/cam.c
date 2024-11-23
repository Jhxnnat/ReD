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
void update_cam_offset_down(Cursor *cursor, Lines *lines){
  size_t relative_line = cursor->current_line - lines->offset;
  if (lines->offset < lines->size && relative_line > MAX_LINES-1) {
    lines->offset++;
  }
}
