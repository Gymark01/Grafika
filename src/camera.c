#include "../includes/camera.h"

#include <GL/gl.h>
#include <GL/freeglut.h>
#include <GL/glu.h>

#include <math.h>

// Converts an angle from degrees to radians
static inline float degree_to_radian(float deg) {
    return deg * (M_PI / 180.0f);
}

// Initializes the camera with default values
void init_camera(Camera* camera) {
    // Horizontal rotation angle
    camera->horizontal = 0.0f;

    // Vertical rotation angle
    camera->vertical = 0.0f;

    // Distance from the target
    camera->distance = 8.0f;

    // Initial camera position
    camera->position.x = 0.0f;
    camera->position.y = 6.0f;
    camera->position.z = 8.0f;
}

// Updates the camera position so it smoothly follows the car
void update_camera(Camera* camera, vec3 target, float carAngle) {
    // Convert the car angle to radians
    float rad = degree_to_radian(carAngle);

    float distance = camera->distance;
    float height = 6.0f;

    vec3 desired;

    // Compute the desired camera position behind the target
    desired.x = target.x + sinf(rad) * distance;
    desired.z = target.z + cosf(rad) * distance;
    desired.y = target.y + height;

    // Smoothly interpolate current camera position toward desired position
    float smooth = 0.1f;
    camera->position.x += (desired.x - camera->position.x) * smooth;
    camera->position.y += (desired.y - camera->position.y) * smooth;
    camera->position.z += (desired.z - camera->position.z) * smooth;
}

// Sets the OpenGL view matrix using the current camera position
void set_view(Camera* camera, vec3 target) {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Position the camera and make it look at the target
    gluLookAt(camera->position.x, camera->position.y, camera->position.z,
              target.x, target.y, target.z,
              0.0, 1.0, 0.0);
}

// Rotates the camera by changing horizontal and vertical angles
void rotate_camera(Camera* camera, float h, float v) {
    camera->horizontal += h;
    camera->vertical += v;

    // Clamp vertical angle to avoid extreme rotation
    if(camera->vertical > 80.0f) camera->vertical = 80.0f;
    if(camera->vertical < -10.0f) camera->vertical = -10.0f;
}
