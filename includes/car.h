#ifndef CAR_H
#define CAR_H

#include "math3d.h"

// Represents the player-controlled car.
typedef struct {
    vec3 position;
    float angle;
    float speed;
    int isSinking;
    int lives;
    int hitCooldown;
    int justHitObstacle;
    int oilSlipTimer;
} Car;

// Initializes the car with default values (position, angle, speed).
void initCar(Car* car);

// Updates car movement and rotation based on input keys.
void updateCar(Car* car, int keyW, int keyS, int keyA, int keyD, int keySpace);

// Renders the car model at its current position and orientation.
void renderCar(Car* car);

// Renders a projected shadow of the car onto the ground.
void renderCarShadow(Car* car);

// Sets the initial spawn position and orientation of the car.
void setCarSpawn(Car* car, float x, float y, float z, float angle);

#endif
