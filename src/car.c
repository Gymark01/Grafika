#include "../includes/car.h"
#include "../includes/lighting.h"
#include "../includes/assets.h"
#include "../includes/map.h"
#include "../includes/config.h"

#include <GL/freeglut.h>

#include <math.h>
#include <stdio.h>

void initCar(Car* car){
    car->position.x = 0.0f;
    car->position.y = 0.0f;
    car->position.z = 0.0f;

    car->angle = 0.0f;
    car->speed = 0.0f;

    car->isSinking = 0;
    car->lives = config.game.lives;
    car->hitCooldown = 0;
    car->justHitObstacle = 0;
    car->oilSlipTimer = 0;
}

void updateCar(Car* car, int keyW, int keyS, int keyA, int keyD, int keySpace)
{

    float acceleration = config.car.acceleration;
    float maxSpeed = config.car.maxSpeed;
    float friction = config.car.friction;

    int inRain = isPointInRainZone(car->position.x, car->position.z);
    float boostMultiplier = keySpace ? 1.9f : 1.0f;
    float traction = inRain ? 0.7f : 1.0f;
    float currentAcceleration = acceleration * boostMultiplier * traction;
    float currentMaxSpeed = maxSpeed * boostMultiplier;
    int inOilSlip = (car->oilSlipTimer > 0);

    float turnSpeed = 0.0f;

    car->justHitObstacle = 0;

    if (fabs(car->speed) > 0.001f){
        turnSpeed = 6.0f * (fabs(car->speed) / maxSpeed);

        if (inRain) {
            turnSpeed *= config.effects.rainTurnMultiplier;
        }

        if (inOilSlip) {
            turnSpeed *= config.effects.oilTurnMultiplier;
        }
    }

    if (keyW) car->speed += currentAcceleration;

    if (keyS) car->speed -= currentAcceleration;

    if (car->speed > currentMaxSpeed) car->speed = currentMaxSpeed;
    if (car->speed < -currentMaxSpeed) car->speed = -currentMaxSpeed;

    if (fabs(car->speed) > 0.001f){
        if (keyA) car->angle += turnSpeed;
        if (keyD) car->angle -= turnSpeed;
    }

    if (car->angle >= 360.0f) car->angle -= 360.0f;
    if (car->angle < 0.0f)    car->angle += 360.0f;

    float currentFriction = friction;

    if (inRain) {
        currentFriction = config.effects.rainFriction;
    }

    if (inOilSlip) {
        currentFriction = config.effects.oilFriction;
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

    float carRadius = config.car.carRadius;

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
    car->lives = config.game.lives;
    car->hitCooldown = 0;
    car->justHitObstacle = 0;
    car->oilSlipTimer = 0;
}
