#ifndef CONFIG_H
#define CONFIG_H

#define MAX_CONFIG_CLOUDS 100

typedef struct {
    float maxSpeed;
    float acceleration;
    float friction;
    float carRadius;
} CarConfig;

typedef struct {
    int lives;
    int coinValue;
} GameConfig;

typedef struct {
    float width;
    float depth;
} WorldConfig;

typedef struct {
    float brightness;
} GraphicsConfig;

typedef struct {
    float baseDistance;
    float maxDistance;
    float distanceSpeedFactor;
    float baseHeight;
    float heightSpeedFactor;
    float directionSmooth;
    float baseSmooth;
    float smoothSpeedFactor;
    float maxSmooth;
    float reverseThreshold;
    float minVertical;
    float maxVertical;
} CameraConfig;

typedef struct {
    int hitFlashDuration;
    int hitCooldown;
    int oilSlipDuration;
} TimingConfig;

typedef struct {
    float rainTurnMultiplier;
    float rainFriction;
    float rainAccelerationMultiplier;
    float oilTurnMultiplier;
    float oilFriction;
} EffectsConfig;

typedef struct {
    int trees;
    int clouds;
    int coins;
    int oilPatches;
    int rainZones;
    int lakes;
} ObjectsConfig;

typedef struct {
    int pointCount;
    float radius;
    float width;
    int closed;
    float randomOffsetMin;
    float randomOffsetMax;
} RoadConfig;

typedef struct {
    float r, g, b;
} ColorConfig;

typedef struct {
    float x, y, z;
    float scale;
} CloudConfig;

typedef struct {
    float heightMin;
    float heightMax;
    float scaleMin;
    float scaleMax;
    float driftSpeed;
} CloudVisualConfig;

typedef struct {
    float baseRadiusMin;
    float baseRadiusMax;
    float shorePadding;
} LakeConfig;

typedef struct {
    float scaleMin;
    float scaleMax;
    float roadPadding;
    float collisionRadiusFactor;
} TreeConfig;

typedef struct {
    float radiusMin;
    float radiusMax;
} OilPatchConfig;

typedef struct {
    CarConfig car;
    GameConfig game;
    WorldConfig world;
    GraphicsConfig graphics;
    CameraConfig camera;
    TimingConfig timing;
    EffectsConfig effects;
    ObjectsConfig objects;
    RoadConfig road;
    ColorConfig sky;
    ColorConfig ground;
    LakeConfig lake;
    TreeConfig tree;
    CloudVisualConfig cloud;
    OilPatchConfig oilPatch;

    CloudConfig cloudsData[MAX_CONFIG_CLOUDS];
    int cloudsDataCount;
} Config;

extern Config config;

int loadConfig(const char* filename);

#endif
