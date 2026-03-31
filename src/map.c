#include "../includes/cJSON.h"
#include "../includes/map.h"
#include "../includes/lighting.h"

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

// Catmull-Rom spline interpolation between 4 points
// p0, p1, p2, p3: control points
// t: parameter in range [0,1] that determines the interpolated position
static RoadPoint catmullRomPoint(RoadPoint p0, RoadPoint p1, RoadPoint p2, RoadPoint p3, float t)
{
    RoadPoint result;

    // Precompute powers of t
    float t2 = t * t;
    float t3 = t2 * t;

    // Interpolate X coordinate
    result.x = 0.5f * (
        (2.0f * p1.x) +
        (-p0.x + p2.x) * t +
        (2.0f * p0.x - 5.0f * p1.x + 4.0f * p2.x - p3.x) * t2 +
        (-p0.x + 3.0f * p1.x - 3.0f * p2.x + p3.x) * t3
    );

    // Interpolate Z coordinate
    result.z = 0.5f * (
        (2.0f * p1.z) +
        (-p0.z + p2.z) * t +
        (2.0f * p0.z - 5.0f * p1.z + 4.0f * p2.z - p3.z) * t2 +
        (-p0.z + 3.0f * p1.z - 3.0f * p2.z + p3.z) * t3
    );

    return result;
}

// Wraps an index so it stays within the range [0, count-1]
// For circular paths or looping roads
static int wrapIndex(int i, int count)
{
    while (i < 0) i += count;
    while (i >= count) i -= count;
    return i;
}

// Reads the full content of a file into memory
// Returns a null-terminated string that must be freed by the caller
static char* readFile(const char* filename)
{
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Failed to open: %s\n", filename);
        return NULL;
    }

    // Determine file size
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    // Allocate memory for file content + null terminator
    char* data = (char*)malloc(size + 1);
    if (!data) {
        fclose(file);
        return NULL;
    }

    // Read file into buffer
    fread(data, 1, size, file);
    data[size] = '\0';

    fclose(file);
    return data;
}

// Initializes rain particle positions randomly
static void initRain()
{
    for (int i = 0; i < NUM_RAIN; i++) {
        rain[i][0] = (float)(rand() % 50 - 25);
        rain[i][1] = (float)(rand() % 20 + 5);
        rain[i][2] = (float)(rand() % 50 - 50);
    }
}

// Initializes the map system
void initMap()
{
    memset(&currentMap, 0, sizeof(MapData));
    initRain();
}

// Loads map data from a JSON file
// Returns 1 on success, 0 on failure
int loadMapFromJson(const char* filename)
{
    char* text = readFile(filename);
    if (!text) {
        return 0;
    }

    // Parse JSON text
    cJSON* root = cJSON_Parse(text);
    if (!root) {
        printf("JSON parse error: %s\n", cJSON_GetErrorPtr());
        free(text);
        return 0;
    }

    // Clear current map before loading new data
    memset(&currentMap, 0, sizeof(MapData));

    // ---------------- ROAD ----------------
    cJSON* road = cJSON_GetObjectItem(root, "road");
    if (road) {
        cJSON* width = cJSON_GetObjectItem(road, "width");
        cJSON* closed = cJSON_GetObjectItem(road, "closed");
        cJSON* points = cJSON_GetObjectItem(road, "points");

        // Load road width
        if (cJSON_IsNumber(width)) {
            currentMap.roadWidth = (float)width->valuedouble;
        }

        // Load whether the road is closed/looping
        if (cJSON_IsBool(closed)) {
            currentMap.closed = cJSON_IsTrue(closed);
        }

        // Load road control points
        if (cJSON_IsArray(points)) {
            int count = cJSON_GetArraySize(points);
            if (count > MAX_ROAD_POINTS) {
                count = MAX_ROAD_POINTS;
            }

            currentMap.pointCount = count;

            for (int i = 0; i < count; i++) {
                cJSON* p = cJSON_GetArrayItem(points, i);
                cJSON* x = cJSON_GetObjectItem(p, "x");
                cJSON* z = cJSON_GetObjectItem(p, "z");

                if (cJSON_IsNumber(x) && cJSON_IsNumber(z)) {
                    currentMap.points[i].x = (float)x->valuedouble;
                    currentMap.points[i].z = (float)z->valuedouble;
                }
            }
        }
    }

    // ---------------- OBSTACLES ----------------
    cJSON* obstacles = cJSON_GetObjectItem(root, "obstacles");
    if (cJSON_IsArray(obstacles)) {
        int count = cJSON_GetArraySize(obstacles);
        if (count > MAX_OBSTACLES) {
            count = MAX_OBSTACLES;
        }

        currentMap.obstacleCount = count;

        for (int i = 0; i < count; i++) {
            cJSON* o = cJSON_GetArrayItem(obstacles, i);

            cJSON* x = cJSON_GetObjectItem(o, "x");
            cJSON* y = cJSON_GetObjectItem(o, "y");
            cJSON* z = cJSON_GetObjectItem(o, "z");
            cJSON* w = cJSON_GetObjectItem(o, "width");
            cJSON* h = cJSON_GetObjectItem(o, "height");
            cJSON* d = cJSON_GetObjectItem(o, "depth");

            if (cJSON_IsNumber(x)) currentMap.obstacles[i].x = (float)x->valuedouble;
            if (cJSON_IsNumber(y)) currentMap.obstacles[i].y = (float)y->valuedouble;
            if (cJSON_IsNumber(z)) currentMap.obstacles[i].z = (float)z->valuedouble;
            if (cJSON_IsNumber(w)) currentMap.obstacles[i].width = (float)w->valuedouble;
            if (cJSON_IsNumber(h)) currentMap.obstacles[i].height = (float)h->valuedouble;
            if (cJSON_IsNumber(d)) currentMap.obstacles[i].depth = (float)d->valuedouble;
        }
    }

    // ---------------- LAKES ----------------
    cJSON* lakes = cJSON_GetObjectItem(root, "lakes");
    if (cJSON_IsArray(lakes)) {
        int count = cJSON_GetArraySize(lakes);
        if (count > MAX_LAKES) {
            count = MAX_LAKES;
        }

        currentMap.lakeCount = count;

        for (int i = 0; i < count; i++) {
            cJSON* l = cJSON_GetArrayItem(lakes, i);

            cJSON* x = cJSON_GetObjectItem(l, "x");
            cJSON* y = cJSON_GetObjectItem(l, "y");
            cJSON* z = cJSON_GetObjectItem(l, "z");
            cJSON* w = cJSON_GetObjectItem(l, "width");
            cJSON* d = cJSON_GetObjectItem(l, "depth");

            if (cJSON_IsNumber(x)) currentMap.lakes[i].x = (float)x->valuedouble;
            if (cJSON_IsNumber(y)) currentMap.lakes[i].y = (float)y->valuedouble;
            if (cJSON_IsNumber(z)) currentMap.lakes[i].z = (float)z->valuedouble;
            if (cJSON_IsNumber(w)) currentMap.lakes[i].width = (float)w->valuedouble;
            if (cJSON_IsNumber(d)) currentMap.lakes[i].depth = (float)d->valuedouble;
        }
    }

    // ---------------- RAIN ZONES ----------------
    cJSON* rainZones = cJSON_GetObjectItem(root, "rainZones");
    if (cJSON_IsArray(rainZones)) {
        int count = cJSON_GetArraySize(rainZones);
        if (count > MAX_RAIN_ZONES) {
            count = MAX_RAIN_ZONES;
        }

        currentMap.rainZoneCount = count;

        for (int i = 0; i < count; i++) {
            cJSON* r = cJSON_GetArrayItem(rainZones, i);

            cJSON* x = cJSON_GetObjectItem(r, "x");
            cJSON* z = cJSON_GetObjectItem(r, "z");
            cJSON* w = cJSON_GetObjectItem(r, "width");
            cJSON* d = cJSON_GetObjectItem(r, "depth");

            if (cJSON_IsNumber(x)) currentMap.rainZones[i].x = (float)x->valuedouble;
            if (cJSON_IsNumber(z)) currentMap.rainZones[i].z = (float)z->valuedouble;
            if (cJSON_IsNumber(w)) currentMap.rainZones[i].width = (float)w->valuedouble;
            if (cJSON_IsNumber(d)) currentMap.rainZones[i].depth = (float)d->valuedouble;
        }
    }

    // Free JSON resources
    cJSON_Delete(root);
    free(text);

    // Print summary
    printf("Map successfully loaded: %s\n", filename);
    printf("Road points: %d\n", currentMap.pointCount);
    printf("Obstacles: %d\n", currentMap.obstacleCount);
    printf("Lakes: %d\n", currentMap.lakeCount);
    printf("Rain zones: %d\n", currentMap.rainZoneCount);

    return 1;
}

// Draws a cube centered at the origin with the given dimensions
static void drawCube(float w, float h, float d)
{
    float x = w / 2.0f;
    float y = h / 2.0f;
    float z = d / 2.0f;

    glBegin(GL_QUADS);
    // Front face
    glVertex3f(-x, -y,  z); glVertex3f( x, -y,  z);
    glVertex3f( x,  y,  z); glVertex3f(-x,  y,  z);
    // Back face
    glVertex3f(-x, -y, -z); glVertex3f( x, -y, -z);
    glVertex3f( x,  y, -z); glVertex3f(-x,  y, -z);
    // Left face
    glVertex3f(-x, -y, -z); glVertex3f(-x, -y,  z);
    glVertex3f(-x,  y,  z); glVertex3f(-x,  y, -z);
    // Right face
    glVertex3f( x, -y, -z); glVertex3f( x, -y,  z);
    glVertex3f( x,  y,  z); glVertex3f( x,  y, -z);
    // Top face
    glVertex3f(-x,  y, -z); glVertex3f( x,  y, -z);
    glVertex3f( x,  y,  z); glVertex3f(-x,  y,  z);
    // Bottom face
    glVertex3f(-x, -y, -z); glVertex3f( x, -y, -z);
    glVertex3f( x, -y,  z); glVertex3f(-x, -y,  z);
    glEnd();
}

// Renders a single obstacle as a red box
static void renderObstacle(Obstacle o)
{
    float r = 1.0f * brightness;
    if (r > 1.0f) r = 1.0f;

    glColor3f(r, 0.0f, 0.0f);
    glPushMatrix();
    // Move cube so it sits on the ground
    glTranslatef(o.x, o.y + o.height / 2.0f, o.z);
    drawCube(o.width, o.height, o.depth);
    glPopMatrix();
}

// Renders a lake as a flat blue rectangle
static void renderLake(Lake l)
{
    float b = 1.0f * brightness;
    if (b > 1.0f) b = 1.0f;

    glColor3f(0.0f, 0.0f, b);
    glPushMatrix();
    glTranslatef(l.x, l.y + 0.01f, l.z);

    glBegin(GL_QUADS);
    glVertex3f(-l.width / 2.0f, 0.0f, -l.depth / 2.0f);
    glVertex3f( l.width / 2.0f, 0.0f, -l.depth / 2.0f);
    glVertex3f( l.width / 2.0f, 0.0f,  l.depth / 2.0f);
    glVertex3f(-l.width / 2.0f, 0.0f,  l.depth / 2.0f);
    glEnd();

    glPopMatrix();
}

// Renders a rain zone using falling point particles
static void renderRainZone(RainZone zone)
{
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_POINTS);
    for (int i = 0; i < NUM_RAIN; i++) {
        // Generate random X and Z positions inside the rain zone
        float x = zone.x + ((float)(rand() % 1000) / 1000.0f - 0.5f) * zone.width;
        float z = zone.z + ((float)(rand() % 1000) / 1000.0f - 0.5f) * zone.depth;
        float y = rain[i][1];

        glVertex3f(x, y, z);

        // Move raindrop downward
        rain[i][1] -= 0.2f;

        // Reset raindrop if it falls below the ground
        if (rain[i][1] < 0.0f) {
            rain[i][1] = (float)(rand() % 20 + 5);
        }
    }
    glEnd();
}

// Draws a road segment as a quad between two points
static void drawRoadSegment(float x1, float z1, float x2, float z2, float width)
{
    float dx = x2 - x1;
    float dz = z2 - z1;
    float len = sqrtf(dx * dx + dz * dz);

    if (len <= 0.0001f) return;

    // Compute perpendicular vector
    float nx = -dz / len;
    float nz =  dx / len;

    float hw = width / 2.0f;

    glBegin(GL_QUADS);
    glVertex3f(x1 + nx * hw, 0.0f, z1 + nz * hw);
    glVertex3f(x1 - nx * hw, 0.0f, z1 - nz * hw);
    glVertex3f(x2 - nx * hw, 0.0f, z2 - nz * hw);
    glVertex3f(x2 + nx * hw, 0.0f, z2 + nz * hw);
    glEnd();
}

// Draws a dashed line between two points
static void drawDashedLine(float x1, float z1, float x2, float z2)
{
    float dx = x2 - x1;
    float dz = z2 - z1;
    float len = sqrtf(dx * dx + dz * dz);

    if (len <= 0.0001f) return;

    float dirx = dx / len;
    float dirz = dz / len;

    float step = 2.0f;
    float dash = 1.0f;

    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINES);
    for (float i = 0.0f; i < len; i += step) {
        float sx = x1 + dirx * i;
        float sz = z1 + dirz * i;

        float ex = x1 + dirx * (i + dash);
        float ez = z1 + dirz * (i + dash);

        glVertex3f(sx, 0.02f, sz);
        glVertex3f(ex, 0.02f, ez);
    }
    glEnd();
}

// Draws the dashed center line along sampled spline points
static void drawCenterDashedLine(RoadPoint* samples, int sampleCount)
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
            // Determine how much remains in the current dash/gap section
            float needed = (draw ? dashLength : gapLength) - accumulated;
            float step = segLen - localPos;

            if (step > needed) step = needed;

            // Draw only when in dash mode
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

            // Switch between dash and gap
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

// Renders the road using Catmull-Rom spline sampling
static void renderRoad()
{
    if (currentMap.pointCount < 4) return;

    RoadPoint samples[MAX_SPLINE_SAMPLES];
    int sampleCount = 0;

    // 1. Sample the spline along all road control points
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

    // 2. Set road color
    float c = 0.2f * brightness;
    if (c > 1.0f) c = 1.0f;
    glColor3f(c, c, c);

    // 3. Draw the continuous road surface as a triangle strip
    float halfWidth = currentMap.roadWidth / 2.0f;

    glBegin(GL_TRIANGLE_STRIP);

    for (int i = 0; i < sampleCount; i++) {
        int prev = (i - 1 + sampleCount) % sampleCount;
        int next = (i + 1) % sampleCount;

        float dx = samples[next].x - samples[prev].x;
        float dz = samples[next].z - samples[prev].z;
        float len = sqrtf(dx * dx + dz * dz);

        if (len <= 0.0001f) continue;

        // Compute perpendicular normal to the road center line
        float nx = -dz / len;
        float nz =  dx / len;

        // Left edge of road
        float lx = samples[i].x + nx * halfWidth;
        float lz = samples[i].z + nz * halfWidth;

        // Right edge of road
        float rx = samples[i].x - nx * halfWidth;
        float rz = samples[i].z - nz * halfWidth;

        glVertex3f(lx, 0.0f, lz);
        glVertex3f(rx, 0.0f, rz);
    }

    // Close the strip by repeating the first pair of vertices
    {
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
    }

    glEnd();

    // 4. Draw dashed center line on top of the road
    drawCenterDashedLine(samples, sampleCount);
}

// Renders the whole map: road, obstacles, lakes, and rain zones
void renderMap()
{
    renderRoad();

    for (int i = 0; i < currentMap.obstacleCount; i++) {
        renderObstacle(currentMap.obstacles[i]);
    }

    for (int i = 0; i < currentMap.lakeCount; i++) {
        renderLake(currentMap.lakes[i]);
    }

    for (int i = 0; i < currentMap.rainZoneCount; i++) {
        renderRainZone(currentMap.rainZones[i]);
    }
}
