#ifndef CAR_H
#define CAR_H

#include "math3d.h"

typedef struct {
    vec3 position;
    float angle; //rotate
    float speed; //forward/backward speed
} Car;

void initCar(Car* car);

void updateCar(Car* car, int keyW, int keyS, int keyA, int keyD, int keySpace);

void renderCar(Car* car);

void renderCarShadow(Car* car);

void setCarSpawn(Car* car, float x, float y, float z, float angle);

#endif
