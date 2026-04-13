#ifndef MODEL_H
#define MODEL_H

#include <GL/gl.h>

typedef struct {
    float x, y, z;
} Vertex;

typedef struct {
    float u, v;
} TexCoord;

typedef struct {
    int v[3];
    int vt[3];
    int materialIndex;
} Face;

typedef struct {
    char name[64];
    GLuint textureID;
    float kd[3];
} Material;

typedef struct {
    Vertex* vertices;
    TexCoord* texcoords;
    Face* faces;
    Material* materials;

    int vertexCount;
    int texcoordCount;
    int faceCount;
    int materialCount;
} Model;

int loadOBJ(const char* filename, Model* model);
void renderModel(const Model* model);
void renderShadowModel(const Model* model, float lightX, float lightY, float lightZ);
void freeModel(Model* model);

#endif
