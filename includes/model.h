#ifndef MODEL_H
#define MODEL_H

#include <GL/gl.h>

// Represents a single 3D vertex position.
typedef struct {
    float x, y, z;
} Vertex;

// Represents a texture coordinate (UV mapping).
typedef struct {
    float u, v;
} TexCoord;

// Represents a triangle face with vertex, texture, and material indices.
typedef struct {
    int v[3];
    int vt[3];
    int materialIndex;
} Face;

// Represents a material loaded from an MTL file.
typedef struct {
    char name[64];
    GLuint textureID;
    float kd[3];
} Material;

// Main model structure containing geometry and materials.
typedef struct {
    Vertex* vertices;
    TexCoord* texcoords;
    Face* faces;
    Material* materials;

    int vertexCount;
    int texcoordCount;
    int faceCount;
    int materialCount;

    GLuint fallbackTextureID;
} Model;

// Loads an OBJ file (and its MTL if present) into the model.
int loadOBJ(const char* filename, Model* model);

// Renders the model using its materials and textures.
void renderModel(const Model* model);

// Renders a projected shadow of the model onto the ground plane.
void renderShadowModel(const Model* model, float lightX, float lightY, float lightZ);

// Frees all allocated memory and OpenGL textures used by the model.
void freeModel(Model* model);

// Loads a texture image manually and returns its OpenGL texture ID.
GLuint loadModelTexture(const char* filename);

#endif
