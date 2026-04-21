#include "../includes/map.h"
#include "../includes/lighting.h"
#include "../includes/assets.h"
#include "../includes/config.h"

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/freeglut.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define MAX_SPLINE_SAMPLES 4000
#define SAMPLES_PER_SEGMENT 25

static MapData currentMap;
static float rain[NUM_RAIN][3];

static RoadPoint catmullRomPoint(RoadPoint p0, RoadPoint p1, RoadPoint p2, RoadPoint p3, float t);
static int wrapIndex(int i, int count);
static void initRain(void);

static float distance2D(float x1, float z1, float x2, float z2);

static int isNearRoad(float x, float z);
static int isLakeNearRoad(float centerX, float centerZ, float width, float depth);
static int pointInLake(Lake l, float px, float pz);
static int isPointInAnyLake(float x, float z);

static void renderGround(void);
static void renderSingleCloud(const Cloud* c);
static void renderClouds(void);
static void renderLake(const Lake* l);
static void renderSingleTree(const Tree* t);
static void renderSingleTreeShadow(const Tree* t);
static void renderTrees(void);
static void renderTreeShadows(void);
static void renderRainZone(const RainZone* zone);
static void drawCenterDashedLine(const RoadPoint* samples, int sampleCount);
static void renderRoad(void);
static void renderSingleCoin(const Coin* c);
static void renderCoins(void);
static void renderSingleOilPatch(const OilPatch* patch);
static void renderOilPatches(void);


static RoadPoint catmullRomPoint(RoadPoint p0, RoadPoint p1, RoadPoint p2, RoadPoint p3, float t)
{
    RoadPoint result;

    float t2 = t * t;
    float t3 = t2 * t;

    result.x = 0.5f * (
        (2.0f * p1.x) +
        (-p0.x + p2.x) * t +
        (2.0f * p0.x - 5.0f * p1.x + 4.0f * p2.x - p3.x) * t2 +
        (-p0.x + 3.0f * p1.x - 3.0f * p2.x + p3.x) * t3
    );

    result.z = 0.5f * (
        (2.0f * p1.z) +
        (-p0.z + p2.z) * t +
        (2.0f * p0.z - 5.0f * p1.z + 4.0f * p2.z - p3.z) * t2 +
        (-p0.z + 3.0f * p1.z - 3.0f * p2.z + p3.z) * t3
    );

    return result;
}

static int wrapIndex(int i, int count)
{
    while (i < 0) i += count;
    while (i >= count) i -= count;
    return i;
}

static float distance2D(float x1, float z1, float x2, float z2)
{
    float dx = x2 - x1;
    float dz = z2 - z1;
    return sqrtf(dx * dx + dz * dz);
}

static void initRain()
{
    for (int i = 0; i < NUM_RAIN; i++) {
        rain[i][0] = (float)(rand() % 50 - 25);
        rain[i][1] = (float)(rand() % 20 + 5);
        rain[i][2] = (float)(rand() % 50 - 50);
    }
}

void initMap()
{
    memset(&currentMap, 0, sizeof(MapData));
    initRain();

    currentMap.ground.width = config.world.width;
    currentMap.ground.depth = config.world.depth;
    currentMap.ground.color.r = config.ground.r;
    currentMap.ground.color.g = config.ground.g;
    currentMap.ground.color.b = config.ground.b;

    currentMap.sky.color.r = config.sky.r;
    currentMap.sky.color.g = config.sky.g;
    currentMap.sky.color.b = config.sky.b;

    currentMap.roadWidth = config.road.width;
    currentMap.closed = config.road.closed;
}

void getSkyColor(float* r, float* g, float* b)
{
    *r = currentMap.sky.color.r;
    *g = currentMap.sky.color.g;
    *b = currentMap.sky.color.b;
}

void generateRandomRoad(int pointCount, float radius)
{

    float maxOffset = fabsf(config.road.randomOffsetMin);
    if (fabsf(config.road.randomOffsetMax) > maxOffset) {
        maxOffset = fabsf(config.road.randomOffsetMax);
    }

    float safeHalfW = currentMap.ground.width / 2.0f - maxOffset - config.car.carRadius - 10.0f;
    float safeHalfD = currentMap.ground.depth / 2.0f - maxOffset - config.car.carRadius - 10.0f;
    float safeRadius = safeHalfW < safeHalfD ? safeHalfW : safeHalfD;

    if (radius > safeRadius) {
        radius = safeRadius;
    }

    currentMap.pointCount = pointCount;
    currentMap.closed = config.road.closed;
    currentMap.roadWidth = config.road.width;

    for (int i = 0; i < pointCount; i++) {
        float angle = ((float)i / pointCount) * 2.0f * 3.1415926f;

        float baseX = cosf(angle) * radius;
        float baseZ = sinf(angle) * radius;

        float offsetRange = config.road.randomOffsetMax - config.road.randomOffsetMin;
        float offsetX = config.road.randomOffsetMin + ((float)rand() / RAND_MAX) * offsetRange;
        float offsetZ = config.road.randomOffsetMin + ((float)rand() / RAND_MAX) * offsetRange;

        currentMap.points[i].x = baseX + offsetX;
        currentMap.points[i].z = baseZ + offsetZ;
    }
}

void generateRoadCoins(int maxCount)
{
    currentMap.coinCount = 0;

    if (currentMap.pointCount < 4) {
        return;
    }

    RoadPoint samples[MAX_SPLINE_SAMPLES];
    int sampleCount = 0;

    for (int i = 0; i < currentMap.pointCount; i++) {
        int i0 = wrapIndex(i - 1, currentMap.pointCount);
        int i1 = wrapIndex(i,     currentMap.pointCount);
        int i2 = wrapIndex(i + 1, currentMap.pointCount);
        int i3 = wrapIndex(i + 2, currentMap.pointCount);

        RoadPoint p0 = currentMap.points[i0];
        RoadPoint p1 = currentMap.points[i1];
        RoadPoint p2 = currentMap.points[i2];
        RoadPoint p3 = currentMap.points[i3];

        for (int s = 0; s < SAMPLES_PER_SEGMENT; s++) {
            if (sampleCount >= MAX_SPLINE_SAMPLES) break;

            float t = (float)s / (float)SAMPLES_PER_SEGMENT;
            samples[sampleCount++] = catmullRomPoint(p0, p1, p2, p3, t);
        }
    }

    if (sampleCount < 2) {
        return;
    }

    int index = rand() % 20;

    while (index < sampleCount && currentMap.coinCount < maxCount) {
        currentMap.coins[currentMap.coinCount].x = samples[index].x;
        currentMap.coins[currentMap.coinCount].y = 0.8f;
        currentMap.coins[currentMap.coinCount].z = samples[index].z;
        currentMap.coins[currentMap.coinCount].active = 1;
        currentMap.coinCount++;

        int randomStep = 15 + rand() % 35;
        index += randomStep;
    }
}

void generateRandomLakes(int count)
{
    currentMap.lakeCount = 0;

    int attempts = 0;
    int maxAttempts = count * 100;

    float halfGroundW = currentMap.ground.width / 2.0f;
    float halfGroundD = currentMap.ground.depth / 2.0f;

    while (currentMap.lakeCount < count && attempts < maxAttempts) {
        attempts++;

        float cx = ((float)rand() / RAND_MAX) * (halfGroundW * 2.0f) - halfGroundW;
        float cz = ((float)rand() / RAND_MAX) * (halfGroundD * 2.0f) - halfGroundD;

        int pointCount = 24;

        float radiusRange = config.lake.baseRadiusMax - config.lake.baseRadiusMin;
        float baseRadius = config.lake.baseRadiusMin + ((float)rand() / RAND_MAX) * radiusRange;

        Lake lake;
        lake.x = cx;
        lake.z = cz;
        lake.pointCount = pointCount;

        float radii[MAX_LAKE_POINTS];
        float smoothRadii[MAX_LAKE_POINTS];

        for (int i = 0; i < pointCount; i++) {
            float randomFactor = 0.85f + ((float)rand() / RAND_MAX) * 0.3f;
            radii[i] = baseRadius * randomFactor;
        }

        for (int i = 0; i < pointCount; i++) {
            int prev = (i - 1 + pointCount) % pointCount;
            int next = (i + 1) % pointCount;

            smoothRadii[i] = (radii[prev] + radii[i] + radii[next]) / 3.0f;
        }

        for (int i = 0; i < pointCount; i++) {
            float angle = (float)i / pointCount * 2.0f * M_PI;
            float r = smoothRadii[i];

            lake.points[i][0] = cosf(angle) * r;
            lake.points[i][1] = sinf(angle) * r;
        }

        if (isLakeNearRoad(cx, cz, baseRadius * 1.2f, baseRadius * 1.2f)) {
            continue;
        }

        currentMap.lakes[currentMap.lakeCount++] = lake;
    }
}

void generateRandomTrees(int count)
{
    currentMap.treeCount = 0;

    int attempts = 0;
    int maxAttempts = count * 100;
    float width = currentMap.ground.width;
    float depth = currentMap.ground.depth;

    while (currentMap.treeCount < count && attempts < maxAttempts) {
        attempts++;

        float x = ((float)rand() / RAND_MAX) * width - (width / 2.0f);
        float z = ((float)rand() / RAND_MAX) * depth - (depth / 2.0f);

        if (isNearRoad(x, z)) {
            continue;
        }

        if (isPointInAnyLake(x, z)){
            continue;
        }

        currentMap.trees[currentMap.treeCount].x = x;
        currentMap.trees[currentMap.treeCount].z = z;

        float scaleRange = config.tree.scaleMax - config.tree.scaleMin;
        currentMap.trees[currentMap.treeCount].scale =
        config.tree.scaleMin + ((float)rand() / RAND_MAX) * scaleRange;

        currentMap.trees[currentMap.treeCount].type = rand() % 3;

        currentMap.treeCount++;
    }
}

void generateRandomClouds(int count)
{
    currentMap.cloudCount = count;

    float width = currentMap.ground.width;
    float depth = currentMap.ground.depth;

    for (int i = 0; i < count; i++) {

        currentMap.clouds[i].x = ((float)rand() / RAND_MAX) * width - (width / 2.0f);
        currentMap.clouds[i].z = ((float)rand() / RAND_MAX) * depth - (depth / 2.0f);
        float heightRange = config.cloud.heightMax - config.cloud.heightMin;
        float scaleRange = config.cloud.scaleMax - config.cloud.scaleMin;

        currentMap.clouds[i].y =
            config.cloud.heightMin + ((float)rand() / RAND_MAX) * heightRange;

        currentMap.clouds[i].scale =
            config.cloud.scaleMin + ((float)rand() / RAND_MAX) * scaleRange;
    }
}

void generateRandomRainZones(int count)
{
    currentMap.rainZoneCount = 0;

    if (currentMap.pointCount < 4) {
        return;
    }

    for (int i = 0; i < count && currentMap.rainZoneCount < MAX_RAIN_ZONES; i++) {
        int roadIndex = rand() % currentMap.pointCount;

        int i0 = wrapIndex(roadIndex - 1, currentMap.pointCount);
        int i1 = wrapIndex(roadIndex,     currentMap.pointCount);
        int i2 = wrapIndex(roadIndex + 1, currentMap.pointCount);
        int i3 = wrapIndex(roadIndex + 2, currentMap.pointCount);

        RoadPoint p0 = currentMap.points[i0];
        RoadPoint p1 = currentMap.points[i1];
        RoadPoint p2 = currentMap.points[i2];
        RoadPoint p3 = currentMap.points[i3];

        float t = (float)(rand() % 100) / 100.0f;
        RoadPoint center = catmullRomPoint(p0, p1, p2, p3, t);

        RoadPoint ahead = catmullRomPoint(p0, p1, p2, p3, t + 0.03f);
        float dx = ahead.x - center.x;
        float dz = ahead.z - center.z;

        float angle = atan2f(dz, dx) * 180.0f / 3.1415926535f;

        RainZone zone;
        zone.x = center.x;
        zone.z = center.z;

        /* Wider than the road, so rain is not only directly above it. */
        zone.width = currentMap.roadWidth + 35.0f + (float)(rand() % 15);

        /* Long zone stretched along the road direction. */
        zone.depth = 70.0f + (float)(rand() % 50);

        zone.angle = angle;

        currentMap.rainZones[currentMap.rainZoneCount++] = zone;
    }
}

void generateRandomOilPatches(int count)
{
    currentMap.oilPatchCount = 0;

    if (currentMap.pointCount < 4) {
        return;
    }

    RoadPoint samples[MAX_SPLINE_SAMPLES];
    int sampleCount = 0;

    for (int i = 0; i < currentMap.pointCount; i++) {
        int i0 = wrapIndex(i - 1, currentMap.pointCount);
        int i1 = wrapIndex(i,     currentMap.pointCount);
        int i2 = wrapIndex(i + 1, currentMap.pointCount);
        int i3 = wrapIndex(i + 2, currentMap.pointCount);

        RoadPoint p0 = currentMap.points[i0];
        RoadPoint p1 = currentMap.points[i1];
        RoadPoint p2 = currentMap.points[i2];
        RoadPoint p3 = currentMap.points[i3];

        for (int s = 0; s < SAMPLES_PER_SEGMENT; s++) {
            if (sampleCount >= MAX_SPLINE_SAMPLES) break;

            float t = (float)s / (float)SAMPLES_PER_SEGMENT;
            samples[sampleCount++] = catmullRomPoint(p0, p1, p2, p3, t);
        }
    }

    if (sampleCount < 10) {
        return;
    }

    int index = 20 + rand() % 20;

    while (index < sampleCount && currentMap.oilPatchCount < count) {
        OilPatch patch;
        patch.x = samples[index].x;
        patch.z = samples[index].z;
        float radiusRange = config.oilPatch.radiusMax - config.oilPatch.radiusMin;
        patch.radius = config.oilPatch.radiusMin + ((float)rand() / RAND_MAX) * radiusRange;
        patch.active = 1;

        currentMap.oilPatches[currentMap.oilPatchCount++] = patch;

        index += 60 + rand() % 80;
    }
}

int isPointInRainZone(float x, float z)
{
    for (int i = 0; i < currentMap.rainZoneCount; i++) {
        RainZone zone = currentMap.rainZones[i];

        float dx = x - zone.x;
        float dz = z - zone.z;

        float rad = -zone.angle * 3.1415926535f / 180.0f;
        float localX = dx * cosf(rad) - dz * sinf(rad);
        float localZ = dx * sinf(rad) + dz * cosf(rad);

        float halfW = zone.width / 2.0f;
        float halfD = zone.depth / 2.0f;

        /* Middle rectangle */
        if (localX >= -halfD && localX <= halfD &&
            localZ >= -halfW && localZ <= halfW) {
            return 1;
        }

        /* Rounded end caps */
        float frontDx = localX - halfD;
        float backDx  = localX + halfD;

        if ((frontDx * frontDx + localZ * localZ) <= (halfW * halfW)) {
            return 1;
        }

        if ((backDx * backDx + localZ * localZ) <= (halfW * halfW)) {
            return 1;
        }
    }

    return 0;
}

static int isNearRoad(float x, float z)
{
    if (currentMap.pointCount < 4) return 0;

    const int samplesPerSegment = 25;
    float minDist = 999999.0f;

    for (int i = 0; i < currentMap.pointCount; i++) {
        int i0 = wrapIndex(i - 1, currentMap.pointCount);
        int i1 = wrapIndex(i,     currentMap.pointCount);
        int i2 = wrapIndex(i + 1, currentMap.pointCount);
        int i3 = wrapIndex(i + 2, currentMap.pointCount);

        RoadPoint p0 = currentMap.points[i0];
        RoadPoint p1 = currentMap.points[i1];
        RoadPoint p2 = currentMap.points[i2];
        RoadPoint p3 = currentMap.points[i3];

        for (int s = 0; s <= samplesPerSegment; s++) {
            float t = (float)s / (float)samplesPerSegment;
            RoadPoint rp = catmullRomPoint(p0, p1, p2, p3, t);

            float dx = x - rp.x;
            float dz = z - rp.z;
            float dist = sqrtf(dx * dx + dz * dz);

            if (dist < minDist) {
                minDist = dist;
            }
        }
    }

    return minDist < (currentMap.roadWidth * 0.5f + config.tree.roadPadding);
}

static int isLakeNearRoad(float centerX, float centerZ, float width, float depth)
{
    if (currentMap.pointCount < 4) return 0;

    const int samplesPerSegment = 25;

    float halfW = width / 2.0f;
    float halfD = depth / 2.0f;

    float testPoints[9][2] = {
        { centerX, centerZ },
        { centerX - halfW, centerZ - halfD },
        { centerX + halfW, centerZ - halfD },
        { centerX - halfW, centerZ + halfD },
        { centerX + halfW, centerZ + halfD },
        { centerX - halfW, centerZ },
        { centerX + halfW, centerZ },
        { centerX, centerZ - halfD },
        { centerX, centerZ + halfD }
    };

    for (int p = 0; p < 9; p++) {
        float x = testPoints[p][0];
        float z = testPoints[p][1];

        float minDist = 999999.0f;

        for (int i = 0; i < currentMap.pointCount; i++) {
            int i0 = wrapIndex(i - 1, currentMap.pointCount);
            int i1 = wrapIndex(i,     currentMap.pointCount);
            int i2 = wrapIndex(i + 1, currentMap.pointCount);
            int i3 = wrapIndex(i + 2, currentMap.pointCount);

            RoadPoint p0 = currentMap.points[i0];
            RoadPoint p1 = currentMap.points[i1];
            RoadPoint p2 = currentMap.points[i2];
            RoadPoint p3 = currentMap.points[i3];

            for (int s = 0; s <= samplesPerSegment; s++) {
                float t = (float)s / (float)samplesPerSegment;
                RoadPoint rp = catmullRomPoint(p0, p1, p2, p3, t);

                float dx = x - rp.x;
                float dz = z - rp.z;
                float dist = sqrtf(dx * dx + dz * dz);

                if (dist < minDist) {
                    minDist = dist;
                }
            }
        }

        if (minDist < (currentMap.roadWidth * 0.5f + config.lake.shorePadding)) {
            return 1;
        }
    }

    return 0;
}

static int pointInLake(Lake l, float px, float pz)
{
    int inside = 0;

    for (int i = 0, j = l.pointCount - 1; i < l.pointCount; j = i++) {
        float xi = l.points[i][0] + l.x;
        float zi = l.points[i][1] + l.z;

        float xj = l.points[j][0] + l.x;
        float zj = l.points[j][1] + l.z;

        int intersect =
            ((zi > pz) != (zj > pz)) &&
            (px < (xj - xi) * (pz - zi) / (zj - zi + 0.00001f) + xi);

        if (intersect)
            inside = !inside;
    }

    return inside;
}

static int isPointInAnyLake(float x, float z)
{
    for (int i = 0; i < currentMap.lakeCount; i++) {
        if (pointInLake(currentMap.lakes[i], x, z)) {
            return 1;
        }
    }

    return 0;
}

int checkLakeCollision(float x, float z)
{
    for (int i = 0; i < currentMap.lakeCount; i++) {
        if (pointInLake(currentMap.lakes[i], x, z)) {
            return 1;
        }
    }
    return 0;
}

int checkTreeCollision(float x, float z, float radius)
{
    for (int i = 0; i < currentMap.treeCount; i++){
        float treeRadius = config.tree.collisionRadiusFactor * currentMap.trees[i].scale;
        float dist = distance2D(x, z, currentMap.trees[i].x, currentMap.trees[i].z);

        if ( dist < radius + treeRadius){
            return 1;
        }
    }

    return 0;
}

int checkStartPoleCollision(float x, float z, float radius)
{
    float sx, sz, dirX, dirZ;
    getStartTransform(&sx, &sz, &dirX, &dirZ);

    float nx = -dirZ;
    float nz = dirX;

    float halfWidth = currentMap.roadWidth / 2.0f + 0.5f;

    float leftX = sx + nx * halfWidth;
    float leftZ = sz + nz * halfWidth;
    float rightX = sx - nx * halfWidth;
    float rightZ = sz - nz * halfWidth;

    float poleRadius = 0.3f;

    float leftDist = distance2D(x, z, leftX, leftZ);
    float rightDist = distance2D(x, z, rightX, rightZ);

    if (leftDist < radius + poleRadius) return 1;
    if (rightDist < radius + poleRadius) return 1;

    return 0;
}

int checkGroundCollision(float x, float z, float radius)
{
    float halfWidth = currentMap.ground.width / 2.0f;
    float halfDepth = currentMap.ground.depth / 2.0f;

    if (x - radius < -halfWidth) return 1;
    if (x + radius >  halfWidth) return 1;
    if (z - radius < -halfDepth) return 1;
    if (z + radius >  halfDepth) return 1;

    return 0;
}

int checkMapCollision(float x, float z, float radius)
{
    if (checkTreeCollision(x, z, radius)) {
        return 1;
    }

    if (checkStartPoleCollision(x, z, radius)) {
        return 1;
    }

    if (checkGroundCollision(x, z, radius)) {
        return 1;
    }

    return 0;
}

int checkOilPatchCollision(float x, float z, float radius)
{
    for (int i = 0; i < currentMap.oilPatchCount; i++) {
        if (!currentMap.oilPatches[i].active) {
            continue;
        }

        float dx = x - currentMap.oilPatches[i].x;
        float dz = z - currentMap.oilPatches[i].z;
        float dist = sqrtf(dx * dx + dz * dz);

        if (dist < radius + currentMap.oilPatches[i].radius) {
            currentMap.oilPatches[i].active = 0;
            return 1;
        }
    }

    return 0;
}

void getStartTransform(float* sx, float* sz, float* dirX, float* dirZ)
{
    if (currentMap.pointCount < 4) {
        *sx = 0.0f;
        *sz = 0.0f;
        *dirX = 0.0f;
        *dirZ = -1.0f;
        return;
    }

    int i0 = wrapIndex(-1, currentMap.pointCount);
    int i1 = wrapIndex(0,  currentMap.pointCount);
    int i2 = wrapIndex(1,  currentMap.pointCount);
    int i3 = wrapIndex(2,  currentMap.pointCount);

    RoadPoint p0 = currentMap.points[i0];
    RoadPoint p1 = currentMap.points[i1];
    RoadPoint p2 = currentMap.points[i2];
    RoadPoint p3 = currentMap.points[i3];

    RoadPoint a = catmullRomPoint(p0, p1, p2, p3, 0.0f);
    RoadPoint b = catmullRomPoint(p0, p1, p2, p3, 0.03f);

    float dx = b.x - a.x;
    float dz = b.z - a.z;
    float len = sqrtf(dx * dx + dz * dz);

    if (len <= 0.0001f) {
        dx = 0.0f;
        dz = -1.0f;
        len = 1.0f;
    }

    dx /= len;
    dz /= len;

    *sx = a.x;
    *sz = a.z;
    *dirX = dx;
    *dirZ = dz;
}

void getStartPosition(float* x, float* y, float* z)
{
    float sx, sz, dirX, dirZ;
    getStartTransform(&sx, &sz, &dirX, &dirZ);

    float backOffset = 2.0f;

    *x = sx - dirX * backOffset;
    *y = 0.0f;
    *z = sz - dirZ * backOffset;
}

float getStartAngle()
{
    float sx, sz, dirX, dirZ;
    getStartTransform(&sx, &sz, &dirX, &dirZ);

    return atan2f(-dirX, -dirZ) * 180.0f / 3.1415926535f;
}

static void renderGround()
{
    float r = currentMap.ground.color.r * brightness;
    float g = currentMap.ground.color.g * brightness;
    float b = currentMap.ground.color.b * brightness;

    if (r > 1.0f) r = 1.0f;
    if (g > 1.0f) g = 1.0f;
    if (b > 1.0f) b = 1.0f;

    glColor3f(r, g, b);

    float hw = currentMap.ground.width / 2.0f;
    float hd = currentMap.ground.depth / 2.0f;

    glBegin(GL_QUADS);
    glVertex3f(-hw, -0.01f, -hd);
    glVertex3f( hw, -0.01f, -hd);
    glVertex3f( hw, -0.01f,  hd);
    glVertex3f(-hw, -0.01f,  hd);
    glEnd();
}

static void renderSingleCloud(const Cloud* c)
{
    glPushMatrix();

    float offset = glutGet(GLUT_ELAPSED_TIME) * config.cloud.driftSpeed;
    glTranslatef(c->x + offset, c->y, c->z);
    glScalef(0.01f * c->scale, 0.01f * c->scale, 0.01f * c->scale);

    glColor3f(1.0f, 1.0f, 1.0f);
    renderModel(&cloudModel);

    glPopMatrix();
}

static void renderClouds()
{
    for (int i = 0; i < currentMap.cloudCount; i++)
    {
        renderSingleCloud(&currentMap.clouds[i]);
    }
}

static void renderLake(const Lake* l)
{
    float sandR = 0.76f * brightness;
    float sandG = 0.68f * brightness;
    float sandB = 0.50f * brightness;

    float wetR = 0.55f * brightness;
    float wetG = 0.45f * brightness;
    float wetB = 0.30f * brightness;

    float waterR = 0.08f * brightness;
    float waterG = 0.40f * brightness;
    float waterB = 0.85f * brightness;

    if (sandR > 1.0f) sandR = 1.0f;
    if (sandG > 1.0f) sandG = 1.0f;
    if (sandB > 1.0f) sandB = 1.0f;

    if (wetR > 1.0f) wetR = 1.0f;
    if (wetG > 1.0f) wetG = 1.0f;
    if (wetB > 1.0f) wetB = 1.0f;

    if (waterR > 1.0f) waterR = 1.0f;
    if (waterG > 1.0f) waterG = 1.0f;
    if (waterB > 1.0f) waterB = 1.0f;

    glPushMatrix();
    glTranslatef(l->x, 0.0f, l->z);

    glColor3f(sandR, sandG, sandB);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0f, 0.004f, 0.0f);
    for (int i = 0; i <= l->pointCount; i++) {
        int idx = i % l->pointCount;
        glVertex3f(l->points[idx][0] * 1.22f, 0.004f, l->points[idx][1] * 1.22f);
    }
    glEnd();

    glColor3f(wetR, wetG, wetB);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0f, 0.007f, 0.0f);
    for (int i = 0; i <= l->pointCount; i++) {
        int idx = i % l->pointCount;
        glVertex3f(l->points[idx][0] * 1.08f, 0.007f, l->points[idx][1] * 1.08f);
    }
    glEnd();

    glColor3f(waterR, waterG, waterB);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0f, 0.010f, 0.0f);
    for (int i = 0; i <= l->pointCount; i++) {
        int idx = i % l->pointCount;
        glVertex3f(l->points[idx][0], 0.010f, l->points[idx][1]);
    }
    glEnd();

    glPopMatrix();
}

static void renderSingleOilPatch(const OilPatch* patch)
{
    if (!patch->active) {
        return;
    }

    glColor3f(0.05f, 0.05f, 0.05f);

    glPushMatrix();
    glTranslatef(patch->x, 0.015f, patch->z);

    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0f, 0.0f, 0.0f);

    for (int i = 0; i <= 24; i++) {
        float angle = (2.0f * 3.1415926535f * i) / 24.0f;
        float r = patch->radius * (0.85f + 0.15f * sinf(i * 1.7f));
        glVertex3f(cosf(angle) * r, 0.0f, sinf(angle) * r);
    }

    glEnd();
    glPopMatrix();
}

static void renderOilPatches(void)
{
    for (int i = 0; i < currentMap.oilPatchCount; i++) {
        renderSingleOilPatch(&currentMap.oilPatches[i]);
    }
}

static void renderSingleTree(const Tree* t)
{
    glPushMatrix();
    glTranslatef(t->x, 0.0f, t->z);
    glScalef(0.08f * t->scale, 0.08f * t->scale, 0.08f * t->scale);
    renderModel(&treeModel1);
    glPopMatrix();
}

static void renderSingleTreeShadow(const Tree* t)
{
    glPushMatrix();
    glTranslatef(t->x, 0.0f, t->z);
    glScalef(0.08f * t->scale, 0.08f * t->scale, 0.08f * t->scale);
    renderShadowModel(&treeModel1, 1.0f, 2.0f, 0.7f);
    glPopMatrix();
}

static void renderTreeShadows()
{
    for (int i = 0; i < currentMap.treeCount; i++) {
        renderSingleTreeShadow(&currentMap.trees[i]);
    }
}

static void renderTrees()
{
    for (int i = 0; i < currentMap.treeCount; i++){
        renderSingleTree(&currentMap.trees[i]);
    }
}

static void renderRainZone(const RainZone* zone)
{
    float halfW = zone->width / 2.0f;
    float halfD = zone->depth / 2.0f;

    glPushMatrix();
    glTranslatef(zone->x, 0.0f, zone->z);
    glRotatef(zone->angle, 0.0f, 1.0f, 0.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(0.2f, 0.2f, 0.25f, 0.16f);
    glBegin(GL_QUADS);
    glVertex3f(-halfD, 0.002f, -halfW);
    glVertex3f( halfD, 0.002f, -halfW);
    glVertex3f( halfD, 0.002f,  halfW);
    glVertex3f(-halfD, 0.002f,  halfW);
    glEnd();

    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(halfD, 0.002f, 0.0f);
    for (int i = 0; i <= 20; i++) {
        float a = -3.1415926535f / 2.0f + (3.1415926535f * i / 20.0f);
        float x = halfD + cosf(a) * halfW;
        float z = sinf(a) * halfW;
        glVertex3f(x, 0.002f, z);
    }
    glEnd();

    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(-halfD, 0.002f, 0.0f);
    for (int i = 0; i <= 20; i++) {
        float a = 3.1415926535f / 2.0f + (3.1415926535f * i / 20.0f);
        float x = -halfD + cosf(a) * halfW;
        float z = sinf(a) * halfW;
        glVertex3f(x, 0.002f, z);
    }
    glEnd();

    glColor3f(0.8f, 0.8f, 1.0f);

    glBegin(GL_LINES);

    for (int i = 0; i < NUM_RAIN; i++) {
        float localX = ((float)(rand() % 1000) / 1000.0f - 0.5f) * (zone->depth + halfW);
        float localZ = ((float)(rand() % 1000) / 1000.0f - 0.5f) * zone->width;
        float y = rain[i][1];

        glVertex3f(localX, y, localZ);
        glVertex3f(localX, y - 0.8f, localZ);

        rain[i][1] -= 0.35f;

        if (rain[i][1] < 0.0f) {
            rain[i][1] = (float)(rand() % 20 + 8);
        }
    }
    glEnd();

    glDisable(GL_BLEND);
    glPopMatrix();
}

static void renderSingleCoin(const Coin* c)
{
    if (!c->active) {
        return;
    }

    glPushMatrix();

    float rot = glutGet(GLUT_ELAPSED_TIME) * 0.2f;

    glTranslatef(c->x, c->y, c->z);
    glRotatef(rot, 0.0f, 1.0f, 0.0f);
    glScalef(0.7f, 0.7f, 0.7f);

    renderModel(&coinModel);

    glPopMatrix();
}

static void renderCoins(void)
{
    for (int i = 0; i < currentMap.coinCount; i++) {
        renderSingleCoin(&currentMap.coins[i]);
    }
}

int collectCoinAt(float x, float z, float radius)
{
    for (int i = 0; i < currentMap.coinCount; i++) {
        if (!currentMap.coins[i].active) continue;

        float dx = x - currentMap.coins[i].x;
        float dz = z - currentMap.coins[i].z;
        float dist = sqrtf(dx * dx + dz * dz);

        float coinRadius = 1.2f;

        if (dist < radius + coinRadius) {
            currentMap.coins[i].active = 0;
            return 1;
        }
    }
    return 0;
}

static void drawCenterDashedLine(const RoadPoint* samples, int sampleCount)
{
    if (sampleCount < 2) return;

    float dashLength = 1.5f;
    float gapLength = 1.0f;

    glColor3f(1.0f, 1.0f, 1.0f);

    float accumulated = 0.0f;
    int draw = 1;

    glBegin(GL_LINES);

    for (int i = 0; i < sampleCount - 1; i++) {
        float dx = samples[i + 1].x - samples[i].x;
        float dz = samples[i + 1].z - samples[i].z;
        float segLen = sqrtf(dx * dx + dz * dz);

        if (segLen <= 0.0001f) continue;

        float dirx = dx / segLen;
        float dirz = dz / segLen;

        float localPos = 0.0f;

        while (localPos < segLen) {

            float needed = (draw ? dashLength : gapLength) - accumulated;
            float step = segLen - localPos;

            if (step > needed) step = needed;

            if (draw) {
                float sx = samples[i].x + dirx * localPos;
                float sz = samples[i].z + dirz * localPos;
                float ex = samples[i].x + dirx * (localPos + step);
                float ez = samples[i].z + dirz * (localPos + step);

                glVertex3f(sx, 0.02f, sz);
                glVertex3f(ex, 0.02f, ez);
            }

            localPos += step;
            accumulated += step;

            if (draw && accumulated >= dashLength) {
                draw = 0;
                accumulated = 0.0f;
            }
            else if (!draw && accumulated >= gapLength) {
                draw = 1;
                accumulated = 0.0f;
            }
        }
    }

    glEnd();
}

void renderCheckeredStartLine()
{
    float sx, sz, dirX, dirZ;
    getStartTransform(&sx, &sz, &dirX, &dirZ);

    float nx = -dirZ;
    float nz = dirX;

    float roadHalf = currentMap.roadWidth / 2.0f;
    float lineDepth = 2.0f;
    float tile = 1.0f;

    int cols = (int)(currentMap.roadWidth / tile);
    int rows = (int)(lineDepth / tile);

    float startLeftX = sx + nx * roadHalf;
    float startLeftZ = sz + nz * roadHalf;

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if ((r + c) % 2 == 0) {
                float white = 1.0f * brightness;
                if (white > 1.0f) white = 1.0f;
                glColor3f(white, white, white);
            } else {
                glColor3f(0.0f, 0.0f, 0.0f);
            }

            float ox = startLeftX - nx * (c * tile);
            float oz = startLeftZ - nz * (c * tile);

            float px = ox + dirX * (r * tile);
            float pz = oz + dirZ * (r * tile);

            glBegin(GL_QUADS);
            glVertex3f(px, 0.03f, pz);
            glVertex3f(px - nx * tile, 0.03f, pz - nz * tile);
            glVertex3f(px - nx * tile + dirX * tile, 0.03f, pz - nz * tile + dirZ * tile);
            glVertex3f(px + dirX * tile, 0.03f, pz + dirZ * tile);
            glEnd();
        }
    }
}

void renderStartPoles()
{
    float sx, sz, dirX, dirZ;
    getStartTransform(&sx, &sz, &dirX, &dirZ);

    float nx = -dirZ;
    float nz = dirX;

    float halfWidth = currentMap.roadWidth / 2.0f + 0.5f;

    float leftX  = sx + nx * halfWidth;
    float leftZ  = sz + nz * halfWidth;
    float rightX = sx - nx * halfWidth;
    float rightZ = sz - nz * halfWidth;

    float c = 0.8f * brightness;
    if (c > 1.0f) c = 1.0f;
    glColor3f(c, c, c);

    glPushMatrix();
    glTranslatef(leftX, 1.5f, leftZ);
    glScalef(0.2f, 3.0f, 0.2f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(rightX, 1.5f, rightZ);
    glScalef(0.2f, 3.0f, 0.2f);
    glutSolidCube(1.0f);
    glPopMatrix();
}

static void renderRoad()
{
    if (currentMap.pointCount < 4) return;

    RoadPoint samples[MAX_SPLINE_SAMPLES];
    int sampleCount = 0;

    for (int i = 0; i < currentMap.pointCount; i++) {
        int i0 = wrapIndex(i - 1, currentMap.pointCount);
        int i1 = wrapIndex(i,     currentMap.pointCount);
        int i2 = wrapIndex(i + 1, currentMap.pointCount);
        int i3 = wrapIndex(i + 2, currentMap.pointCount);

        RoadPoint p0 = currentMap.points[i0];
        RoadPoint p1 = currentMap.points[i1];
        RoadPoint p2 = currentMap.points[i2];
        RoadPoint p3 = currentMap.points[i3];

        for (int s = 0; s < SAMPLES_PER_SEGMENT; s++) {
            if (sampleCount >= MAX_SPLINE_SAMPLES) break;

            float t = (float)s / (float)SAMPLES_PER_SEGMENT;
            samples[sampleCount++] = catmullRomPoint(p0, p1, p2, p3, t);
        }
    }

    if (sampleCount < 2) return;

    float c = 0.2f * brightness;
    if (c > 1.0f) c = 1.0f;
    glColor3f(c, c, c);

    float halfWidth = currentMap.roadWidth / 2.0f;

    glBegin(GL_TRIANGLE_STRIP);

    for (int i = 0; i < sampleCount; i++) {
        int prev = (i - 1 + sampleCount) % sampleCount;
        int next = (i + 1) % sampleCount;

        float dx = samples[next].x - samples[prev].x;
        float dz = samples[next].z - samples[prev].z;
        float len = sqrtf(dx * dx + dz * dz);

        if (len <= 0.0001f) continue;

        float nx = -dz / len;
        float nz =  dx / len;

        float lx = samples[i].x + nx * halfWidth;
        float lz = samples[i].z + nz * halfWidth;

        float rx = samples[i].x - nx * halfWidth;
        float rz = samples[i].z - nz * halfWidth;

        glVertex3f(lx, 0.0f, lz);
        glVertex3f(rx, 0.0f, rz);
    }


    int i = 0;
    int prev = (i - 1 + sampleCount) % sampleCount;
    int next = (i + 1) % sampleCount;

    float dx = samples[next].x - samples[prev].x;
    float dz = samples[next].z - samples[prev].z;
    float len = sqrtf(dx * dx + dz * dz);

    if (len > 0.0001f) {
        float nx = -dz / len;
        float nz =  dx / len;

        float lx = samples[i].x + nx * halfWidth;
        float lz = samples[i].z + nz * halfWidth;

        float rx = samples[i].x - nx * halfWidth;
        float rz = samples[i].z - nz * halfWidth;

        glVertex3f(lx, 0.0f, lz);
        glVertex3f(rx, 0.0f, rz);
    }


    glEnd();

    drawCenterDashedLine(samples, sampleCount);
}

void renderMap()
{
    renderGround();
    renderClouds();
    renderRoad();
    renderOilPatches();
    renderCoins();
    renderCheckeredStartLine();
    renderStartPoles();

    for (int i = 0; i < currentMap.lakeCount; i++) {
        renderLake(&currentMap.lakes[i]);
    }

    for (int i = 0; i < currentMap.rainZoneCount; i++) {
        renderRainZone(&currentMap.rainZones[i]);
    }

    renderTreeShadows();
    renderTrees();
}
