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
void initCamera(Camera* camera);

//update camera based on car
void updateCamera(Camera* camera, vec3 target, float carAngle, float speed);

//using camera
void setView(Camera* camera, vec3 target);

//snapping camera to car at init
void snapCameraToTarget(Camera* camera, vec3 target, float carAngle);

#endif
