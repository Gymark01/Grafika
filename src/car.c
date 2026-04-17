#include "../includes/car.h"
#include "../includes/lighting.h"
#include "../includes/assets.h"
#include "../includes/map.h"

#include <GL/freeglut.h>

#include <math.h>
#include <stdio.h>

static float acceleration = 0.06f;

static float maxSpeed = 2.0f;

static float friction = 0.98f;

void initCar(Car* car){
    car->position.x = 0.0f;
    car->position.y = 0.0f;
    car->position.z = 0.0f;

    car->angle = 0.0f;
    car->speed = 0.0f;

    car->isSinking = 0;
    car->lives = 3;
    car->hitCooldown = 0;
    car->justHitObstacle = 0;
    car->oilSlipTimer = 0;
}

void updateCar(Car* car, int keyW, int keyS, int keyA, int keyD, int keySpace)
{
    int inRain = isPointInRainZone(car->position.x, car->position.z);
    float boostMultiplier = keySpace ? 1.5f : 1.0f;
    float traction = inRain ? 0.8f : 1.0f;
    float currentAcceleration = acceleration * boostMultiplier * traction;
    float currentmaxSpeed = maxSpeed * boostMultiplier;
    int inOilSlip = (car->oilSlipTimer > 0);

    float turnSpeed = 0.0f;

    car->justHitObstacle = 0;

    if (fabs(car->speed) > 0.001f){
        turnSpeed = 6.0f * (fabs(car->speed) / currentmaxSpeed);

        if (inRain) {
            turnSpeed *= 2.0f;
        }

        if (inOilSlip) {
            turnSpeed *= 0.18f;
        }
    }

    if (keyW) car->speed += currentAcceleration;

    if (keyS) car->speed -= currentAcceleration;

    if (car->speed > currentmaxSpeed) car->speed = currentmaxSpeed;
    if (car->speed < -currentmaxSpeed) car->speed = -currentmaxSpeed;

    if (fabs(car->speed) > 0.001f){
        if (keyA) car->angle += turnSpeed;
        if (keyD) car->angle -= turnSpeed;
    }

    if (car->angle >= 360.0f) car->angle -= 360.0f;
    if (car->angle < 0.0f)    car->angle += 360.0f;

    float currentFriction = friction;

    if (inRain) {
        currentFriction = 0.8f;
    }

    if (inOilSlip) {
        currentFriction = 1.0f;
    }

    if (!keyW && !keyS) {
        car->speed *= currentFriction;

        if (car->speed > -0.0001f && car->speed < 0.0001f) {
            car->speed = 0.0f;
        }
    }

    float rad = car->angle * 3.1415926535f / 180.0f;

    float forwardX = -sinf(rad);
    float forwardZ = -cosf(rad);

    float newX = car->position.x + forwardX * car->speed;
    float newZ = car->position.z + forwardZ * car->speed;

    float carRadius = 0.9f;

    if (!checkMapCollision(newX, newZ, carRadius)) {
        car->position.x = newX;
        car->position.z = newZ;
    } else {
        car->speed = 0.0f;
        car->justHitObstacle = 1;
        printf("Obstacle hit!\n");
    }
}

void renderCar(Car* car)
{
    glPushMatrix();

    glTranslatef(car->position.x, car->position.y, car->position.z);

    glRotatef(180.0f, 0, 1, 0);
    glRotatef(car->angle, 0, 1, 0);

    glScalef(0.07f, 0.07f, 0.07f);

    glColor3f(1.0f, 1.0f, 1.0f);
    renderModel(&carModel);

    glPopMatrix();
}

void renderCarShadow(Car* car)
{
    glPushMatrix();

    glTranslatef(car->position.x, car->position.y, car->position.z);
    glRotatef(180.0f, 0, 1, 0);
    glRotatef(car->angle, 0, 1, 0);
    glScalef(0.07f, 0.07f, 0.07f);

    renderShadowModel(&carModel, 1.0f, 2.0f, 0.7f);

    glPopMatrix();
}

void setCarSpawn(Car* car, float x, float y, float z, float angle)
{
    car->position.x = x;
    car->position.y = y;
    car->position.z = z;

    car->angle = angle;
    car->speed = 0.0f;

    car->isSinking = 0;
    car->lives = 3;
    car->hitCooldown = 0;
    car->justHitObstacle = 0;
    car->oilSlipTimer = 0;
}
