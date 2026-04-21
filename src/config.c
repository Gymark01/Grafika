#include "../includes/config.h"
#include "../includes/cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Config config;

static char* readFile(const char* filename)
{
    FILE* f = fopen(filename, "rb");
    if (!f) {
        printf("Could not open config file: %s\n", filename);
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    rewind(f);

    if (length < 0) {
        fclose(f);
        return NULL;
    }

    char* data = (char*)malloc((size_t)length + 1);
    if (!data) {
        fclose(f);
        return NULL;
    }

    size_t bytesRead = fread(data, 1, (size_t)length, f);
    data[bytesRead] = '\0';

    fclose(f);
    return data;
}

int loadConfig(const char* filename)
{
    memset(&config, 0, sizeof(Config));

    char* jsonText = readFile(filename);
    if (!jsonText) return 0;

    cJSON* root = cJSON_Parse(jsonText);
    free(jsonText);

    if (!root) {
        printf("Invalid JSON format\n");
        return 0;
    }

    cJSON* car = cJSON_GetObjectItem(root, "car");
    if (car) {
        config.car.maxSpeed = (float)cJSON_GetObjectItem(car, "maxSpeed")->valuedouble;
        config.car.acceleration = (float)cJSON_GetObjectItem(car, "acceleration")->valuedouble;
        config.car.friction = (float)cJSON_GetObjectItem(car, "friction")->valuedouble;
        config.car.carRadius = (float)cJSON_GetObjectItem(car, "carRadius")->valuedouble;
    }

    cJSON* game = cJSON_GetObjectItem(root, "game");
    if (game) {
        config.game.lives = cJSON_GetObjectItem(game, "lives")->valueint;
        config.game.coinValue = cJSON_GetObjectItem(game, "coinValue")->valueint;
    }

    cJSON* world = cJSON_GetObjectItem(root, "world");
    if (world) {
        config.world.width = (float)cJSON_GetObjectItem(world, "width")->valuedouble;
        config.world.depth = (float)cJSON_GetObjectItem(world, "depth")->valuedouble;
    }

    cJSON* graphics = cJSON_GetObjectItem(root, "graphics");
    if (graphics) {
        config.graphics.brightness = (float)cJSON_GetObjectItem(graphics, "brightness")->valuedouble;
    }

    cJSON* cam = cJSON_GetObjectItem(root, "camera");
    if (cam) {
        config.camera.baseDistance = (float)cJSON_GetObjectItem(cam, "baseDistance")->valuedouble;
        config.camera.maxDistance = (float)cJSON_GetObjectItem(cam, "maxDistance")->valuedouble;
        config.camera.distanceSpeedFactor = (float)cJSON_GetObjectItem(cam, "distanceSpeedFactor")->valuedouble;
        config.camera.baseHeight = (float)cJSON_GetObjectItem(cam, "baseHeight")->valuedouble;
        config.camera.heightSpeedFactor = (float)cJSON_GetObjectItem(cam, "heightSpeedFactor")->valuedouble;
        config.camera.directionSmooth = (float)cJSON_GetObjectItem(cam, "directionSmooth")->valuedouble;
        config.camera.baseSmooth = (float)cJSON_GetObjectItem(cam, "baseSmooth")->valuedouble;
        config.camera.smoothSpeedFactor = (float)cJSON_GetObjectItem(cam, "smoothSpeedFactor")->valuedouble;
        config.camera.maxSmooth = (float)cJSON_GetObjectItem(cam, "maxSmooth")->valuedouble;
        config.camera.reverseThreshold = (float)cJSON_GetObjectItem(cam, "reverseThreshold")->valuedouble;
        config.camera.minVertical = (float)cJSON_GetObjectItem(cam, "minVertical")->valuedouble;
        config.camera.maxVertical = (float)cJSON_GetObjectItem(cam, "maxVertical")->valuedouble;
    }

    cJSON* timing = cJSON_GetObjectItem(root, "timing");
    if (timing) {
        config.timing.hitFlashDuration = cJSON_GetObjectItem(timing, "hitFlashDuration")->valueint;
        config.timing.hitCooldown = cJSON_GetObjectItem(timing, "hitCooldown")->valueint;
        config.timing.oilSlipDuration = cJSON_GetObjectItem(timing, "oilSlipDuration")->valueint;
    }

    cJSON* effects = cJSON_GetObjectItem(root, "effects");
    if (effects) {
        config.effects.rainTurnMultiplier = (float)cJSON_GetObjectItem(effects, "rainTurnMultiplier")->valuedouble;
        config.effects.rainFriction = (float)cJSON_GetObjectItem(effects, "rainFriction")->valuedouble;
        config.effects.rainAccelerationMultiplier = (float)cJSON_GetObjectItem(effects, "rainAccelerationMultiplier")->valuedouble;
        config.effects.oilTurnMultiplier = (float)cJSON_GetObjectItem(effects, "oilTurnMultiplier")->valuedouble;
        config.effects.oilFriction = (float)cJSON_GetObjectItem(effects, "oilFriction")->valuedouble;
    }

    cJSON* obj = cJSON_GetObjectItem(root, "objects");
    if (obj) {
        config.objects.trees = cJSON_GetObjectItem(obj, "trees")->valueint;
        config.objects.clouds = cJSON_GetObjectItem(obj, "clouds")->valueint;
        config.objects.coins = cJSON_GetObjectItem(obj, "coins")->valueint;
        config.objects.oilPatches = cJSON_GetObjectItem(obj, "oilPatches")->valueint;
        config.objects.rainZones = cJSON_GetObjectItem(obj, "rainZones")->valueint;
        config.objects.lakes = cJSON_GetObjectItem(obj, "lakes")->valueint;
    }

    cJSON* road = cJSON_GetObjectItem(root, "road");
    if (road) {
        config.road.pointCount = cJSON_GetObjectItem(road, "pointCount")->valueint;
        config.road.radius = (float)cJSON_GetObjectItem(road, "radius")->valuedouble;
        config.road.width = (float)cJSON_GetObjectItem(road, "width")->valuedouble;
        config.road.closed = cJSON_GetObjectItem(road, "closed")->valueint;
        config.road.randomOffsetMin = (float)cJSON_GetObjectItem(road, "randomOffsetMin")->valuedouble;
        config.road.randomOffsetMax = (float)cJSON_GetObjectItem(road, "randomOffsetMax")->valuedouble;
    }

    cJSON* sky = cJSON_GetObjectItem(root, "sky");
    if (sky) {
        config.sky.r = (float)cJSON_GetObjectItem(sky, "r")->valuedouble;
        config.sky.g = (float)cJSON_GetObjectItem(sky, "g")->valuedouble;
        config.sky.b = (float)cJSON_GetObjectItem(sky, "b")->valuedouble;
    }

    cJSON* ground = cJSON_GetObjectItem(root, "ground");
    if (ground) {
        config.ground.r = (float)cJSON_GetObjectItem(ground, "r")->valuedouble;
        config.ground.g = (float)cJSON_GetObjectItem(ground, "g")->valuedouble;
        config.ground.b = (float)cJSON_GetObjectItem(ground, "b")->valuedouble;
    }

    cJSON* cloudsArray = cJSON_GetObjectItem(root, "cloudsData");
    config.cloudsDataCount = 0;

    cJSON* cloud = cJSON_GetObjectItem(root, "cloud");

    if (cloud) {
        config.cloud.heightMin = (float)cJSON_GetObjectItem(cloud, "heightMin")->valuedouble;
        config.cloud.heightMax = (float)cJSON_GetObjectItem(cloud, "heightMax")->valuedouble;
        config.cloud.scaleMin = (float)cJSON_GetObjectItem(cloud, "scaleMin")->valuedouble;
        config.cloud.scaleMax = (float)cJSON_GetObjectItem(cloud, "scaleMax")->valuedouble;
        config.cloud.driftSpeed = (float)cJSON_GetObjectItem(cloud, "driftSpeed")->valuedouble;
    }

    cJSON* lake = cJSON_GetObjectItem(root, "lake");
    if (lake) {
        config.lake.baseRadiusMin = (float)cJSON_GetObjectItem(lake, "baseRadiusMin")->valuedouble;
        config.lake.baseRadiusMax = (float)cJSON_GetObjectItem(lake, "baseRadiusMax")->valuedouble;
        config.lake.shorePadding = (float)cJSON_GetObjectItem(lake, "shorePadding")->valuedouble;
    }

    cJSON* tree = cJSON_GetObjectItem(root, "tree");
    if (tree) {
        config.tree.scaleMin = (float)cJSON_GetObjectItem(tree, "scaleMin")->valuedouble;
        config.tree.scaleMax = (float)cJSON_GetObjectItem(tree, "scaleMax")->valuedouble;
        config.tree.roadPadding = (float)cJSON_GetObjectItem(tree, "roadPadding")->valuedouble;
        config.tree.collisionRadiusFactor = (float)cJSON_GetObjectItem(tree, "collisionRadiusFactor")->valuedouble;
    }

    cJSON* oilPatch = cJSON_GetObjectItem(root, "oilPatch");
    if (oilPatch) {
        config.oilPatch.radiusMin = (float)cJSON_GetObjectItem(oilPatch, "radiusMin")->valuedouble;
        config.oilPatch.radiusMax = (float)cJSON_GetObjectItem(oilPatch, "radiusMax")->valuedouble;
    }

    if (cJSON_IsArray(cloudsArray)) {
        int count = cJSON_GetArraySize(cloudsArray);
        if (count > MAX_CONFIG_CLOUDS) count = MAX_CONFIG_CLOUDS;

        config.cloudsDataCount = count;

        for (int i = 0; i < count; i++) {
            cJSON* c = cJSON_GetArrayItem(cloudsArray, i);

            config.cloudsData[i].x = (float)cJSON_GetObjectItem(c, "x")->valuedouble;
            config.cloudsData[i].y = (float)cJSON_GetObjectItem(c, "y")->valuedouble;
            config.cloudsData[i].z = (float)cJSON_GetObjectItem(c, "z")->valuedouble;
            config.cloudsData[i].scale = (float)cJSON_GetObjectItem(c, "scale")->valuedouble;
        }
    }

    cJSON_Delete(root);

    printf("Config loaded successfully\n");
    return 1;
}
