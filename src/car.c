#include "../includes/car.h"
#include "../includes/lighting.h"
#include "../includes/assets.h"

#include <GL/freeglut.h>

#include <math.h>

// Car acceleration per update step
static float acceleration = 0.06f;

// Maximum forward and reverse speed
static float maxSpeed = 2.0f;

// Friction factor applied every frame
static float friction = 0.98f;

// Initializes the car with default position and movement values
void initCar(Car* car){
    car->position.x = 0.0f;
    car->position.y = 0.0f;
    car->position.z = 0.0f;

    car->angle = 0.0f;
    car->speed = 0.0f;
}

// Updates car movement and rotation based on keyboard input
void updateCar(Car* car, int keyW, int keyS, int keyA, int keyD, int keySpace)
{
    float boostMultiplier = keySpace ? 1.5f : 1.0f;

    float currentAcceleration = acceleration * boostMultiplier;
    float currentmaxSpeed = maxSpeed * boostMultiplier;

    float turnSpeed = 0.0f;

    // Allow turning only when the car is moving fast enough
    // The faster the car moves, the faster it can turn
    if (fabs(car->speed) > 0.001f){
        turnSpeed = 6.0f * (fabs(car->speed) / currentmaxSpeed);
    }

    // Accelerate forward
    if (keyW) car->speed += currentAcceleration;

    // Accelerate backward / reverse
    if (keyS) car->speed -= currentAcceleration;

    // Clamp speed to allowed range
    if (car->speed > currentmaxSpeed) car->speed = currentmaxSpeed;
    if (car->speed < -currentmaxSpeed) car->speed = -currentmaxSpeed;

    // Rotate only if the car is actually moving
    if (fabs(car->speed) > 0.001f){
        if (keyA) car->angle += turnSpeed; // turn left
        if (keyD) car->angle -= turnSpeed; // turn right
    }

    // Keep angle in [0, 360)
    if (car->angle >= 360.0f) car->angle -= 360.0f;
    if (car->angle < 0.0f)    car->angle += 360.0f;

    // Apply friction to gradually slow the car down
    if (!keyW && !keyS) {
        car->speed *= friction;

        if (car->speed > -0.0001f && car->speed < 0.0001f) {
            car->speed = 0.0f;
        }
    }

    // Convert angle to radians for movement calculation
    float rad = car->angle * 3.1415926535f / 180.0f;

    // Compute forward direction vector
    float forwardX = -sinf(rad);
    float forwardZ = -cosf(rad);

    // Move the car in the forward direction
    car->position.x += forwardX * car->speed;
    car->position.z += forwardZ * car->speed;
}

// Render the car model using its current world transform.
void renderCar(Car* car)
{
    glPushMatrix();

    glTranslatef(car->position.x, car->position.y, car->position.z);

    // The imported car model faces the opposite direction by default.
    glRotatef(180.0f, 0, 1, 0);
    glRotatef(car->angle, 0, 1, 0);

    glScalef(0.07f, 0.07f, 0.07f);

    // Keep the base color white so the model textures and materials remain unchanged.
    glColor3f(1.0f, 1.0f, 1.0f);
    renderModel(&carModel);

    glPopMatrix();
}

// Render the car shadow using the same transform as the car itself.
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

// Set the initial car position and heading at spawn time.
void setCarSpawn(Car* car, float x, float y, float z, float angle)
{
    car->position.x = x;
    car->position.y = y;
    car->position.z = z;
    car->angle = angle;
    car->speed = 0.0f;
}
