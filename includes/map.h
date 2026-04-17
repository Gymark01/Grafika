#ifndef MAP_H
#define MAP_H

#define MAX_ROAD_POINTS 200
#define MAX_LAKES 50
#define MAX_RAIN_ZONES 50
#define NUM_RAIN 1000
#define MAX_TREES 1000
#define MAX_CLOUDS 100
#define MAX_LAKE_POINTS 32
#define MAX_COINS 500
#define MAX_OIL_PATCHES 20

typedef struct {
    float x, z;
} RoadPoint;

typedef struct {
    float x, z;
    int pointCount;
    float points[MAX_LAKE_POINTS][2];
} Lake;

typedef struct {
    float x, z;
    float width, depth;
    float angle;
} RainZone;

typedef struct{
    float r, g, b;
}Color3;

typedef struct{
    float width, depth;
    Color3 color;
}Ground;

typedef struct{
    Color3 color;
}Sky;

typedef struct{
    float x, y, z;
    float scale;
}Cloud;

typedef struct{
    float x, z;
    float scale;
    int type;
}Tree;

typedef struct{
    float x, y, z;
    int active;
}Coin;

typedef struct {
    float x, z;
    float radius;
    int active;
} OilPatch;

void getSkyColor(float* r, float* g, float* b);

// Stores all map-related data
typedef struct {
    RoadPoint points[MAX_ROAD_POINTS];
    int pointCount;
    float roadWidth;
    int closed;

    Lake lakes[MAX_LAKES];
    int lakeCount;

    RainZone rainZones[MAX_RAIN_ZONES];
    int rainZoneCount;

    Sky sky;
    Cloud clouds[MAX_CLOUDS];
    int cloudCount;

    Ground ground;
    Tree trees[MAX_TREES];
    int treeCount;

    Coin coins[MAX_COINS];
    int coinCount;

    OilPatch oilPatches[MAX_OIL_PATCHES];
    int oilPatchCount;

} MapData;

void getStartPosition(float* x, float* y, float* z);

float getStartAngle();

void getStartTransform(float* sx, float* sz, float* dirX, float* dirZ);

void renderCheckeredStartLine();

void renderStartPoles();

void generateRandomRoad(int pointCount, float radius);

void generateRandomTrees(int count);

void generateRandomClouds(int count);

// Generates coins randomly across the spline of the road
void generateRoadCoins(int maxCount);

void generateRandomRainZones(int count);

void getSkyColor(float* r, float* g, float* b);

int checkTreeCollision(float x, float z, float radius);

int checkStartPoleCollision(float x, float z, float radius);

int checkMapCollision(float x, float z, float radius);

int checkGroundCollision(float x, float z, float radius);

int checkLakeCollision(float x, float y, float radius);

int collectCoinAt(float x, float z, float radius);

int isPointInRainZone(float x, float z);

void generateRandomOilPatches(int count);

int checkOilPatchCollision(float x, float z, float radius);

// Initializes map data (clears memory, sets defaults)
void initMap();

// Renders all map elements (road, obstacles, lakes, rain)
void renderMap();

#endif
