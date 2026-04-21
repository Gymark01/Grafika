#include "../includes/map.h"
#include "../includes/camera.h"
#include "../includes/car.h"
#include "../includes/lighting.h"
#include "../includes/assets.h"
#include "../includes/config.h"

#include <GL/freeglut.h>

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <windows.h>
#include <mmsystem.h>
#include <direct.h>

typedef enum {
    MENU_MAIN,
    MENU_DESCRIPTION,
    MENU_CONTROLS,
    MENU_CREDITS
} MenuState;

MenuState menuState = MENU_MAIN;
bool showMenu = false;
int selectedMenuItem = 0;
Camera camera;
Car car;
bool keyW = false, keyS = false, keyA = false, keyD = false, keySpace = false;
bool gameOver = false;
int selectedGameOverItem = 0;
float previousStartSide = 0.0f;
int startLineReady = 0;
int hitFlashTimer = 0;
int score = 0;
float brightness = 1.0f;

float getCarStartSide(float carX, float carZ)
{
    float sx, sz, dirX, dirZ;
    getStartTransform(&sx, &sz, &dirX, &dirZ);

    float nx = -dirZ;
    float nz = dirX;

    float dx = carX - sx;
    float dz = carZ - sz;

    return dx * nx + dz * nz;
}

void drawText(float x, float y, const char* text)
{
    glRasterPos2f(x, y);

    for (int i = 0; text[i] != '\0'; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
    }
}

void drawHeart(float x, float y, float size, int filled)
{
    if (filled) {
        glColor3f(1.0f, 0.0f, 0.0f);
    } else {
        glColor3f(1.0f, 1.0f, 1.0f);
    }

    glBegin(GL_POLYGON);

    for (int i = 0; i < 100; i++) {
        float t = 2.0f * 3.1415926f * i / 100.0f;

        float hx = 16.0f * sinf(t) * sinf(t) * sinf(t);
        float hy = 13.0f * cosf(t) - 5.0f * cosf(2.0f * t)
                 - 2.0f * cosf(3.0f * t) - cosf(4.0f * t);

        glVertex2f(x + hx * size, y + hy * size);
    }

    glEnd();
}

void renderLivesHUD()
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 800, 0, 600);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);

    float time = glutGet(GLUT_ELAPSED_TIME) * 0.005f;
    float pulse = 1.0f + 0.08f * sinf(time);

    drawHeart(700, 550, (car.lives >= 1) ? 0.8f * pulse : 0.8f, car.lives >= 1);
    drawHeart(740, 550, (car.lives >= 2) ? 0.8f * pulse : 0.8f, car.lives >= 2);
    drawHeart(780, 550, (car.lives >= 3) ? 0.8f * pulse : 0.8f, car.lives >= 3);

    char text[64];
    sprintf(text, "Score: %d", score);
    drawText(20, 560, text);

    glEnable(GL_DEPTH_TEST);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void drawBigText(float x, float y, float scale, const char* text)
{
    glPushMatrix();

    glTranslatef(x, y, 0);
    glScalef(scale, scale, 1.0f);

    glLineWidth(3.5f);

    for (int i = 0; text[i] != '\0'; i++) {
        glutStrokeCharacter(GLUT_STROKE_ROMAN, text[i]);
    }

    glLineWidth(1.0);

    glPopMatrix();
}

void renderOilSlipText()
{
    if (car.oilSlipTimer <= 0) {
        return;
    }

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 800, 0, 600);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);

    int time = glutGet(GLUT_ELAPSED_TIME);
    int blink = (time / 150) % 2;

    if (blink) {

        glColor3f(1.0f, 1.0f, 0.0f);
        drawBigText(300, 450, 0.3f, "OIL SLIP!");

    }

    glEnable(GL_DEPTH_TEST);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void renderHitFlash()
{
    if (hitFlashTimer <= 0) {
        return;
    }

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 800, 0, 600);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float alpha = 0.35f * ((float)hitFlashTimer / 10.0f);
    if (alpha > 0.35f) alpha = 0.35f;

    glColor4f(1.0f, 0.0f, 0.0f, alpha);

    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(800, 0);
    glVertex2f(800, 600);
    glVertex2f(0, 600);
    glEnd();

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void renderGameOverOverlay()
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 800, 0, 600);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);

    glColor4f(0.0f, 0.0f, 0.0f, 0.85f);
    glBegin(GL_QUADS);
    glVertex2f(180, 140);
    glVertex2f(620, 140);
    glVertex2f(620, 460);
    glVertex2f(180, 460);
    glEnd();

    glColor3f(1.0f, 0.2f, 0.2f);
    drawText(335, 390, "GAME OVER");

    const char* items[2] = { "Restart", "Exit" };

    for (int i = 0; i < 2; i++) {
        if (i == selectedGameOverItem) {
            glColor3f(1.0f, 1.0f, 0.0f);
        } else {
            glColor3f(1.0f, 1.0f, 1.0f);
        }

        drawText(350, 300 - i * 60, items[i]);
    }

    glColor3f(0.7f, 0.7f, 0.7f);
    drawText(250, 180, "UP/DOWN: Select   Enter: Confirm");

    glEnable(GL_DEPTH_TEST);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void renderMenuOverlay()
{

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

void handleMenuSelection()
{

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

void specialKeys(int key, int x, int y)
{
    if (gameOver) {
        if (key == GLUT_KEY_UP || key == GLUT_KEY_DOWN) {
            selectedGameOverItem = 1 - selectedGameOverItem;
        }
        return;
    }

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

void keyboard(unsigned char key, int x, int y)
{
    if (gameOver){
        if (key == 13){
            if (selectedGameOverItem == 0){
                float startX, startY, startZ;
                float startAngle;

                getStartPosition(&startX, &startY, &startZ);
                startAngle = getStartAngle();

                setCarSpawn(&car, startX, startY, startZ, startAngle);
                snapCameraToTarget(&camera, car.position, car.angle);

                previousStartSide = getCarStartSide(car.position.x, car.position.z);
                startLineReady = 1;

                gameOver = false;
                selectedGameOverItem = 0;
            }else {exit (0);}
        }
        return;
    }

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

void keyboardUp(unsigned char key, int x, int y)
{
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

void display()
{

    float skyR, skyG, skyB;
    getSkyColor(&skyR, &skyG, &skyB);

    float carRadius = config.car.carRadius;

    glClearColor(skyR * brightness, skyG * brightness, skyB * brightness, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!showMenu && !gameOver && !car.isSinking) {
        updateCar(&car, keyW, keyS, keyA, keyD, keySpace);

        if (checkOilPatchCollision(car.position.x, car.position.z, carRadius)) {
            car.oilSlipTimer = config.timing.oilSlipDuration;
        }

        if (car.oilSlipTimer > 0) {
            car.oilSlipTimer--;
        }

        if (startLineReady) {
            float currentStartSide = getCarStartSide(car.position.x, car.position.z);

            if (fabs(car.speed) > 0.2f &&
                previousStartSide < 0.0f &&
                currentStartSide >= 0.0f) {
                generateRoadCoins(100);
            }

            previousStartSide = currentStartSide;
        }

        if (collectCoinAt(car.position.x, car.position.z, carRadius)) {
            score += config.game.coinValue;
            PlaySound(TEXT("assets/Sounds/coin.wav"), NULL, SND_ASYNC | SND_FILENAME);
        }

        if (car.hitCooldown > 0) {
            car.hitCooldown--;
        }

        if (checkLakeCollision(car.position.x, car.position.z)) {
            car.isSinking = 1;
            car.speed = 0.0f;
        }
        else if (car.hitCooldown == 0 && car.justHitObstacle){
            car.lives--;
            car.hitCooldown = config.timing.hitCooldown;
            car.speed = 0.0f;
            hitFlashTimer = config.timing.hitFlashDuration;
            PlaySound(TEXT("assets/Sounds/hit.wav"), NULL, SND_ASYNC | SND_FILENAME);


            if (car.lives <= 0){
                gameOver = true;
            }

        }
    }

    renderHitFlash();

    if (hitFlashTimer > 0) {
        hitFlashTimer--;
    }

    if (car.isSinking){
        car.position.y -= 0.03f;

        if (car.position.y < -0.5f){
            car.isSinking = 0;
            gameOver = true;
        }
    }

    updateCamera(&camera, car.position, car.angle, car.speed);
    setView(&camera, car.position);

    renderMap();
    renderCarShadow(&car);
    renderCar(&car);

    if (!gameOver){
        renderLivesHUD();
        renderOilSlipText();
    }

    if(showMenu){
        renderMenuOverlay();
    }

    if(gameOver){
        renderGameOverOverlay();
    }

    glutSwapBuffers();
}

void reshape(int w, int h)
{
    if(h == 0) h = 1;

    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (float)w / (float)h, 1.0, 200.0);

    glMatrixMode(GL_MODELVIEW);
}

void init()
{
    loadConfig("bin/Debug/config.json");
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_POINT_SMOOTH);
    glPointSize(3);


    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    initMap();

    loadAssets();
    printf("tree1 vertices: %d faces: %d\n", treeModel1.vertexCount, treeModel1.faceCount);
    printf("car vertices: %d faces: %d\n", carModel.vertexCount, carModel.faceCount);
    printf("cloud vertices: %d faces: %d\n", cloudModel.vertexCount, cloudModel.faceCount);

    generateRandomRoad(config.road.pointCount, config.road.radius);
    generateRandomLakes(config.objects.lakes);
    generateRandomTrees(config.objects.trees);
    generateRandomClouds(config.objects.clouds);
    generateRoadCoins(config.objects.coins);
    generateRandomRainZones(config.objects.rainZones);
    generateRandomOilPatches(config.objects.oilPatches);

    brightness = config.graphics.brightness;
    score = 0;
    hitFlashTimer = 0;


    initCamera(&camera);
    initCar(&car);

    float startX, startY, startZ;
    float startAngle;

    getStartPosition(&startX, &startY, &startZ);
    startAngle = getStartAngle();

    setCarSpawn(&car, startX, startY, startZ, startAngle);
    snapCameraToTarget(&camera, car.position, car.angle);

    printf("Spawn x=%f z=%f\n", car.position.x, car.position.z);
    printf("Tree collision: %d\n", checkTreeCollision(car.position.x, car.position.z, config.car.carRadius));
    printf("Pole collision: %d\n", checkStartPoleCollision(car.position.x, car.position.z, config.car.carRadius));
    printf("Ground collision: %d\n", checkGroundCollision(car.position.x, car.position.z, config.car.carRadius));

    printf("Spawn x=%f z=%f\n", car.position.x, car.position.z);
    printf("Ground half width=%f half depth=%f\n",
           config.world.width / 2.0f,
           config.world.depth / 2.0f);
    printf("Ground collision at spawn: %d\n",
           checkGroundCollision(car.position.x, car.position.z, config.car.carRadius));

    previousStartSide = getCarStartSide(car.position.x, car.position.z);
    startLineReady = 1;

    glClearColor(0.4f, 0.7f, 1.0f, 1.0f);

}

int main(int argc, char** argv)
{
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
