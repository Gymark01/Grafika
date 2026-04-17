#include "../includes/camera.h"

#include <GL/gl.h>
#include <GL/freeglut.h>
#include <GL/glu.h>

#include <math.h>

static inline float degreeToRadian(float deg)
{
    return deg * (M_PI / 180.0f);
}

void initCamera(Camera* camera)
{
    camera->horizontal = 0.0f;
    camera->vertical = 0.0f;
    camera->distance = 8.0f;
    camera->followDirection = 1.0f;

    camera->position.x = 0.0f;
    camera->position.y = 4.0f;
    camera->position.z = 10.0f;
}

void updateCamera(Camera* camera, vec3 target, float carAngle, float speed)
{
    float rad = degreeToRadian(carAngle);
    float absSpeed = fabs(speed);

    float baseDistance = 6.0f;
    float maxDistance = 10.0f;

    float distance = baseDistance + absSpeed * 4.0f;
    if (distance > maxDistance) distance = maxDistance;

    float height = 2.0f + absSpeed * 1.5f;

    float targetFollowDirection = (speed < - 0.02f) ? -1.0f : 1.0f;

    float directionSmooth = 0.08f;
    camera->followDirection += (targetFollowDirection - camera->followDirection) * directionSmooth;

    vec3 desired;

    desired.x = target.x + sinf(rad) * distance * camera->followDirection;
    desired.z = target.z + cosf(rad) * distance * camera->followDirection;
    desired.y = target.y + height;

    float smooth = 0.08f + absSpeed * 0.15f;
    if (smooth > 0.3f) smooth = 0.3f;

    camera->position.x += (desired.x - camera->position.x) * smooth;
    camera->position.y += (desired.y - camera->position.y) * smooth;
    camera->position.z += (desired.z - camera->position.z) * smooth;
}

void setView(Camera* camera, vec3 target)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(camera->position.x, camera->position.y, camera->position.z,
              target.x, target.y + 1.0f, target.z,
              0.0, 1.0, 0.0);
}

void rotateCamera(Camera* camera, float h, float v)
{
    camera->horizontal += h;
    camera->vertical += v;

    if(camera->vertical > 80.0f) camera->vertical = 80.0f;
    if(camera->vertical < -10.0f) camera->vertical = -10.0f;
}

void snapCameraToTarget(Camera* camera, vec3 target, float carAngle)
{
    float rad = degreeToRadian(carAngle);

    float distance = camera->distance;
    float height = 4.0f;

    camera->followDirection = 1.0f;

    camera->position.x = target.x + sinf(rad) * distance;
    camera->position.z = target.z + cosf(rad) * distance;
    camera->position.y = target.z + height;

}
