#ifndef MAP_H
#define MAP_H

#define MAX_ROAD_POINTS 100
#define MAX_OBSTACLES 100
#define MAX_LAKES 50
#define MAX_RAIN_ZONES 50
#define NUM_RAIN 1000

// Represents a point on the road (2D: x and z)
typedef struct {
    float x;
    float z;
} RoadPoint;

// Represents a 3D obstacle (box)
typedef struct {
    float x, y, z;           // position
    float width, height, depth; // dimensions
} Obstacle;

// Represents a lake (flat rectangular surface)
typedef struct {
    float x, y, z;     // position
    float width, depth; // size
} Lake;

// Represents a rain zone area
typedef struct {
    float x, z;        // center position (2D)
    float width, depth; // size of the rain area
} RainZone;

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
} MapData;

// Initializes map data (clears memory, sets defaults)
void initMap();

// Loads map data from a JSON file
// Returns 1 on success, 0 on failure
int loadMapFromJson(const char* filename);

// Renders all map elements (road, obstacles, lakes, rain)
void renderMap();

#endif
