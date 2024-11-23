#ifndef CAM_H
#define CAM_H

#include "raylib.h"
#include "ds.h"

void init_camera(Camera2D *camera);
// void update_cam_offset(Cursor *cursor, Lines *lines, int dir); //TODO should this be 2 different funcions??
void update_cam_offset_up(Cursor *cursor, Lines *lines);
void update_cam_offset_down(Cursor *cursor, Lines *lines);

#endif // !CAM_H
