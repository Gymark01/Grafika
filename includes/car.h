#ifndef CAR_H
#define CAR_H

#include "math3d.h"

typedef struct {
    vec3 position;
    float angle; //rotate
    float speed; //forward/backward speed
} Car;

// init
void initCar(Car* car);

// update (movement)
void updateCar(Car* car, int keyW, int keyS, int keyA, int keyD);

// render
void renderCar(Car* car);

#endif
