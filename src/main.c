#include "../includes/map.h"
#include "../includes/camera.h"
#include "../includes/car.h"
#include "../includes/lighting.h"
#include "../includes/assets.h"

#include <GL/freeglut.h>

#include <stdbool.h>

typedef enum {
    MENU_MAIN,
    MENU_DESCRIPTION,
    MENU_CONTROLS,
    MENU_CREDITS
} MenuState;

MenuState menuState = MENU_MAIN;
bool showMenu = false;
int selectedMenuItem = 0;

void drawText(float x, float y, const char* text) {
    glRasterPos2f(x, y);

    for (int i = 0; text[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
    }
}

void renderMenuOverlay() {

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 800, 0, 600);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);

    glColor4f(0.0f, 0.0f, 0.0f, 0.8f);
    glBegin(GL_QUADS);
    glVertex2f(150, 100);
    glVertex2f(650, 100);
    glVertex2f(650, 500);
    glVertex2f(150, 500);
    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f);

    if (menuState == MENU_MAIN) {
        drawText(390, 450, "MENU");

        const char* items[4] = {
            "1. Description",
            "2. Controls",
            "3. Credits",
            "4. EXIT"
        };

        for (int i = 0; i < 4; i++) {
            if (i == selectedMenuItem) {
                glColor3f(1.0f, 1.0f, 0.0f);
            } else {
                glColor3f(1.0f, 1.0f, 1.0f);
            }

            drawText(280, 380 - i * 60, items[i]);
        }

        glColor3f(0.7f, 0.7f, 0.7f);
        drawText(220, 140, "UP/DOWN: Movement   Enter: Enter   Esc: Back");
    }

    else if (menuState == MENU_DESCRIPTION) {
        glColor3f(1.0f, 1.0f, 1.0f);
        drawText(340, 450, "DESCRIPTION");
        drawText(190, 360, "This is a car game in OpenGL + FreeGLUT.");
        drawText(190, 320, "The purpose of this game to me not to");
        drawText(190, 280, "fail in graphic design class.");
        drawText(190, 240, "For others to enjoy this game.");
        drawText(310, 170, "Esc - Back to menu");
    }

    else if (menuState == MENU_CONTROLS) {
        glColor3f(1.0f, 1.0f, 1.0f);
        drawText(360, 450, "CONTORLS");
        drawText(250, 370, "W - accelerate");
        drawText(250, 330, "S - break / reverse");
        drawText(250, 290, "A - left turn");
        drawText(250, 250, "D - right turn");
        drawText(250, 210, "SPACE - nitro");
        drawText(250, 170, "0 / 1 - brightness settings");
        drawText(300, 120, "Esc - Back to menu");
    }

    else if (menuState == MENU_CREDITS) {
        glColor3f(1.0f, 1.0f, 1.0f);
        drawText(360, 450, "CREDITS");
        drawText(240, 360, "Who made this wonder:");
        drawText(240, 320, "Team member 1 - Gyongyosi Mark - D9RKIN");
        drawText(240, 280, "Team member 2 - Marlboro Touch");
        drawText(240, 240, "Team member 3 - Jameson Crested");
        drawText(320, 170, "Esc - Back to menu");
    }

    glEnable(GL_DEPTH_TEST);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void handleMenuSelection() {

    if (menuState != MENU_MAIN) {
        return;
    }

    switch (selectedMenuItem) {
        case 0:
            menuState = MENU_DESCRIPTION;
            break;
        case 1:
            menuState = MENU_CONTROLS;
            break;
        case 2:
            menuState = MENU_CREDITS;
            break;
        case 3:
            exit(0);
            break;
    }
}

void specialKeys(int key, int x, int y) {

    if (!showMenu || menuState != MENU_MAIN) {
        return;
    }

    if (key == GLUT_KEY_UP) {
        selectedMenuItem--;
        if (selectedMenuItem < 0) selectedMenuItem = 3;
    }

    if (key == GLUT_KEY_DOWN) {
        selectedMenuItem++;
        if (selectedMenuItem > 3) selectedMenuItem = 0;
    }
}

bool keyW = false, keyS = false, keyA = false, keyD = false, keySpace = false;

float brightness = 1.0f;

Camera camera;
Car car;

void keyboard(unsigned char key, int x, int y){
    if (key == 27){
        if(!showMenu){

            showMenu = true;
            menuState = MENU_MAIN;
        }else{

            if (menuState == MENU_MAIN){
                showMenu = false;
            }else {
                menuState = MENU_MAIN;
            }
        }
        return;
    }

    if(showMenu){
        if(key == 13){
            handleMenuSelection();
        }
        return;
    }

    if(key == 'w' || key == 'W') keyW = true;
    if(key == 's' || key == 'S') keyS = true;
    if(key == 'a' || key == 'A') keyA = true;
    if(key == 'd' || key == 'D') keyD = true;
    if(key == ' ') keySpace = true;

    if(key == '0'){
        brightness -= 0.1f;
        if(brightness < 0.2f) brightness = 0.2f;
    }
    if(key == '1'){
        brightness += 0.1f;
        if(brightness > 2.0f) brightness = 2.0f;
    }
}

void keyboardUp(unsigned char key, int x, int y){
    if(key == 'w' || key == 'W') keyW = false;
    if(key == 's' || key == 'S') keyS = false;
    if(key == 'a' || key == 'A') keyA = false;
    if(key == 'd' || key == 'D') keyD = false;
    if(key == ' ' ) keySpace = false;
}

void specialKeyUp(int key, int x, int y)
{
    keySpace = false;
}

void display() {

    float skyR, skyG, skyB;
    getSkyColor(&skyR, &skyG, &skyB);

    glClearColor(skyR * brightness, skyG * brightness, skyB * brightness, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(!showMenu){
        updateCar(&car, keyW, keyS, keyA, keyD, keySpace);
    }

    updateCamera(&camera, car.position, car.angle, car.speed);
    setView(&camera, car.position);

    renderMap();
    renderCarShadow(&car);
    renderCar(&car);

    if(showMenu){
        renderMenuOverlay();
    }

    glutSwapBuffers();
}

void reshape(int w, int h) {
    if(h == 0) h = 1;

    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (float)w / (float)h, 1.0, 200.0);

    glMatrixMode(GL_MODELVIEW);
}

void init() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_POINT_SMOOTH);
    glPointSize(3);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    initMap();
    loadMapFromJson("maps/map1.json");

    loadAssets();
    printf("tree1 vertices: %d faces: %d\n", treeModel1.vertexCount, treeModel1.faceCount);
    printf("car vertices: %d faces: %d\n", carModel.vertexCount, carModel.faceCount);
    printf("cloud vertices: %d faces: %d\n", cloudModel.vertexCount, cloudModel.faceCount);

    generateRandomRoad(40,150.0f);
    generateRandomTrees(100);
    generateRandomClouds(20);

    initCamera(&camera);
    initCar(&car);

    float startX, startY, startZ;
    float startAngle;

    getStartPosition(&startX, &startY, &startZ);
    startAngle = getStartAngle();

    setCarSpawn(&car, startX, startY, startZ, startAngle);
    snapCameraToTarget(&camera, car.position, car.angle);

    glClearColor(0.4f, 0.7f, 1.0f, 1.0f);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Need For Weed");

    init();
    atexit(freeAssets);

    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(specialKeys);
    glutSpecialUpFunc(specialKeyUp);
    glutDisplayFunc(display);
    glutIdleFunc(display);
    glutReshapeFunc(reshape);

    glutMainLoop();
    return 0;
}
