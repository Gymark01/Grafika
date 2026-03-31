#include "../includes/car.h"
#include "../includes/lighting.h"

#include <GL/freeglut.h>

#include <math.h>

// Car acceleration per update step
static float acceleration = 0.001f;

// Maximum forward and reverse speed
static float maxSpeed = 0.015f;

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
void updateCar(Car* car, int keyW, int keyS, int keyA, int keyD)
{
    float turnSpeed = 0.0f;

    // Allow turning only when the car is moving fast enough
    // The faster the car moves, the faster it can turn
    if (fabs(car->speed) > 0.001){
        turnSpeed = 0.1f * (fabs(car->speed) / maxSpeed);
    }

    // Accelerate forward
    if (keyW) car->speed += acceleration;

    // Accelerate backward / reverse
    if (keyS) car->speed -= acceleration;

    // Clamp speed to allowed range
    if (car->speed > maxSpeed) car->speed = maxSpeed;
    if (car->speed < -maxSpeed) car->speed = -maxSpeed;

    // Rotate only if the car is actually moving
    if (fabs(car->speed) > 0.001f){
        if (keyA) car->angle += turnSpeed; // turn left
        if (keyD) car->angle -= turnSpeed; // turn right
    }

    // Keep angle in [0, 360)
    if (car->angle >= 360.0f) car->angle -= 360.0f;
    if (car->angle < 0.0f)    car->angle += 360.0f;

    // Apply friction to gradually slow the car down
    car->speed *= friction;

    // Snap very small speed values to zero
    if (car->speed > -0.0001f && car->speed < 0.0001f) {
        car->speed = 0.0f;
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

// Renders the car model
void renderCar(Car* car){

    glPushMatrix();

    // Move and rotate the whole car into world position
    glTranslatef(car->position.x, car->position.y, car->position.z);
    glRotatef(car->angle, 0, 1, 0);

    // Main body of the car
    glColor3f(1.0f * brightness, 0, 0);
    glPushMatrix();
    glScalef(1.5f, 0.5f, 3.0f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Cabin / upper part of the car
    glColor3f(0.8f * brightness, 0, 0);
    glPushMatrix();
    glTranslatef(0, 0.5f, -0.3f);
    glScalef(1.2f, 0.5f, 1.5f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Left headlight
    glColor3f(1.0f * brightness, 1.0f * brightness, 0.0f);
    glPushMatrix();
    glTranslatef(-0.45f, 0.15f, -1.6f);
    glScalef(0.25f, 0.2f, 0.2f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Right headlight
    glPushMatrix();
    glTranslatef(0.45f, 0.15f, -1.6f);
    glScalef(0.25f, 0.2f, 0.2f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Tires / wheels
    glColor3f(0.0f, 0.0f, 0.0f);

    float wheelOffsetX = 0.8f;
    float wheelOffsetZ = 1.2f;

    // Draw 4 wheels using mirrored positions
    for(int i = -1; i <= 1; i += 2){
        for(int j = -1; j <= 1; j += 2){
            glPushMatrix();
            glTranslatef(i * wheelOffsetX, -0.25f, j * wheelOffsetZ);
            glutSolidSphere(0.3f, 10, 10);
            glPopMatrix();
        }
    }

    glPopMatrix();
}
