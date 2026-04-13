#include "../includes/assets.h"
#include <stdio.h>

Model treeModel1;
//Model treeModel2;
//Model treeModel3;
Model cloudModel;
Model carModel;

void loadAssets()
{
    if (!loadOBJ("assets/tree/model/tree2.obj", &treeModel1)) {
        printf("Error: tree1.obj\n");
    }

    /*if (!loadOBJ("assets/tree/model/tree2.obj", &treeModel2)) {
        printf("Error: tree2.obj\n");
    }

    if (!loadOBJ("assets/tree/model/tree3.obj", &treeModel3)) {
        printf("Error: tree3.obj\n");
    }*/

    if (!loadOBJ("assets/cloud/model/cloud.obj", &cloudModel)) {
        printf("Error: cloud.obj\n");
    }

    if (!loadOBJ("assets/car/model/car.obj", &carModel)) {
        printf("Error: car.obj\n");
    }
}

void freeAssets()
{
    freeModel(&treeModel1);
    //freeModel(&treeModel2);
    //freeModel(&treeModel3);
    freeModel(&cloudModel);
    freeModel(&carModel);
}
