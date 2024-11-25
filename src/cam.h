#ifndef CAM_H
#define CAM_H

#include "raylib.h"
#include "ds.h"

void init_camera(Camera2D *camera);
void update_cam_offset_up(Cursor *cursor, Lines *lines); //TODO rename this 
void update_cam_offset_down(Cursor *cursor, Lines *lines, int max_lines);

void move_cam_left(Camera2D *camera, int cursor_x, int font_height);
void move_cam_right(Camera2D *camera, int cursor_x, int font_height);
void move_cam_start(Lines *lines);
void move_cam_end(Lines *lines, int max_lines);

// bool is_cursor_outbound_top(int cursor_y, int vert_margin);
// bool is_cursor_outbound_bottom(int cursor_y, int vert_margin);

#endif // !CAM_H
