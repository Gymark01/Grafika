#ifndef MAP_H
#define MAP_H

#define MAX_ROAD_POINTS 200
#define MAX_OBSTACLES 100
#define MAX_LAKES 50
#define MAX_RAIN_ZONES 50
#define NUM_RAIN 1000
#define MAX_TREES 200
#define MAX_CLOUDS 100

typedef struct {
    float x, z;
} RoadPoint;

typedef struct {
    float x, y, z;
    float width, height, depth;
} Obstacle;

typedef struct {
    float x, y, z;
    float width, depth;
} Lake;

typedef struct {
    float x, z;
    float width, depth;
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


void getSkyColor(float* r, float* g, float* b);


// Stores all map-related data
typedef struct {
    // Road data
    RoadPoint points[MAX_ROAD_POINTS]; // control points for spline road
    int pointCount;                   // number of valid points
    float roadWidth;                  // width of the road
    int closed;                       // whether the road is a loop

    // Obstacles
    Obstacle obstacles[MAX_OBSTACLES];
    int obstacleCount;

    // Lakes
    Lake lakes[MAX_LAKES];
    int lakeCount;

    // Rain zones
    RainZone rainZones[MAX_RAIN_ZONES];
    int rainZoneCount;

    Ground ground;

    Sky sky;

    Cloud clouds[MAX_CLOUDS];
    int cloudCount;

    Tree trees[MAX_TREES];
    int treeCount;


} MapData;

void generateRandomRoad(int pointCount, float radius);

void generateRandomTrees(int count);

void generateRandomClouds(int count);

void getStartPosition(float* x, float* y, float* z);

float getStartAngle();

void getStartTransform(float* sx, float* sz, float* dirX, float* dirZ);

// Initializes map data (clears memory, sets defaults)
void initMap();

// Loads map data from a JSON file
// Returns 1 on success, 0 on failure
int loadMapFromJson(const char* filename);

// Renders all map elements (road, obstacles, lakes, rain)
void renderMap();

#endif
