#include "../includes/camera.h"
#include "../includes/config.h"

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
    camera->distance = config.camera.baseDistance;
    camera->followDirection = 1.0f;

    camera->position.x = 0.0f;
    camera->position.y = config.camera.baseHeight;
    camera->position.z = config.camera.baseDistance;
}

void updateCamera(Camera* camera, vec3 target, float carAngle, float speed)
{
    float rad = degreeToRadian(carAngle);
    float absSpeed = fabs(speed);

    float baseDistance = config.camera.baseDistance;
    float maxDistance = config.camera.maxDistance;

    float distance = baseDistance + absSpeed * config.camera.distanceSpeedFactor;
    if (distance > maxDistance) {
        distance = maxDistance;
    }

    float height = config.camera.baseHeight + absSpeed * config.camera.heightSpeedFactor;

    float targetFollowDirection =
        (speed < config.camera.reverseThreshold) ? -1.0f : 1.0f;

    camera->followDirection +=
        (targetFollowDirection - camera->followDirection) * config.camera.directionSmooth;

    vec3 desired;

    desired.x = target.x + sinf(rad) * distance * camera->followDirection;
    desired.z = target.z + cosf(rad) * distance * camera->followDirection;
    desired.y = target.y + height;

    float smooth = config.camera.baseSmooth + absSpeed * config.camera.smoothSpeedFactor;
    if (smooth > config.camera.maxSmooth) {
        smooth = config.camera.maxSmooth;
    }

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

    if(camera->vertical > config.camera.maxVertical) camera->vertical = config.camera.maxVertical;
    if(camera->vertical < config.camera.minVertical) camera->vertical = config.camera.minVertical;
}

void snapCameraToTarget(Camera* camera, vec3 target, float carAngle)
{
    float rad = degreeToRadian(carAngle);

    float distance = config.camera.baseDistance;
    float height = config.camera.baseHeight;

    camera->followDirection = 1.0f;

    camera->position.x = target.x + sinf(rad) * distance;
    camera->position.z = target.z + cosf(rad) * distance;
    camera->position.y = target.y + height;

}
