#ifndef CAMERA_H
#define CAMERA_H

#include "../includes/math3d.h"
#include <stdbool.h>

typedef struct Camera
{
    vec3 position;

    float horizontal; // rotate (yaw)
    float vertical;   // up/down (pitch)
    float distance;   // distance from car

} Camera;

//init
void init_camera(Camera* camera);

//update camera based on car
void update_camera(Camera* camera, vec3 target, float carAngle);

//using camera
void set_view(Camera* camera, vec3 target);

#endif
