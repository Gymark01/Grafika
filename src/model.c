#include "../includes/model.h"
#include "../includes/lighting.h"
#include <GL/gl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#include "../includes/stb_image.h"

static void getDirectoryFromPath(const char* path, char* outDir)
{
    strcpy(outDir, path);

    char* lastSlash1 = strrchr(outDir, '/');
    char* lastSlash2 = strrchr(outDir, '\\');
    char* lastSlash = lastSlash1 > lastSlash2 ? lastSlash1 : lastSlash2;

    if (lastSlash) {
        *(lastSlash + 1) = '\0';
    } else {
        outDir[0] = '\0';
    }
}

static void buildPath(const char* dir, const char* file, char* outPath)
{
    sprintf(outPath, "%s%s", dir, file);
}

static GLuint loadTexture(const char* filename)
{
    int width, height, channels;

    stbi_set_flip_vertically_on_load(1);
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 0);

    if (!data) {
        printf("Texture load error: %s\n", filename);
        return 0;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    GLenum format = GL_RGB;
    if (channels == 4) format = GL_RGBA;
    else if (channels == 3) format = GL_RGB;
    else if (channels == 1) format = GL_LUMINANCE;

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0,
                 format, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    stbi_image_free(data);
    return textureID;
}

static int findMaterialIndex(const Model* model, const char* name)
{
    for (int i = 0; i < model->materialCount; i++) {
        if (strcmp(model->materials[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

static void parseFaceVertexToken(const char* token, int* vIndex, int* vtIndex)
{
    *vIndex = -1;
    *vtIndex = -1;

    int v = -1, vt = -1;

    if (strstr(token, "//")) {
        sscanf(token, "%d//", &v);
        *vIndex = v - 1;
        *vtIndex = -1;
    } else {
        int count = sscanf(token, "%d/%d", &v, &vt);

        if (count >= 1) {
            *vIndex = v - 1;
        }
        if (count >= 2) {
            *vtIndex = vt - 1;
        }
    }
}

static void loadMTL(const char* mtlPath, Model* model)
{
    FILE* file = fopen(mtlPath, "r");
    if (!file) {
        printf("Failed to open the MTL: %s\n", mtlPath);
        return;
    }

    char baseDir[512];
    getDirectoryFromPath(mtlPath, baseDir);

    int materialCapacity = 0;
    Material currentMaterial;
    int hasCurrent = 0;

    char line[512];

    while (fgets(line, sizeof(line), file)) {
        char* ptr = line;
        while (*ptr == ' ' || *ptr == '\t') {
            ptr++;
        }

        if (strncmp(ptr, "newmtl ", 7) == 0) {
            if (hasCurrent) {
                if (model->materialCount >= materialCapacity) {
                    materialCapacity = (materialCapacity == 0) ? 8 : materialCapacity * 2;
                    model->materials = (Material*)realloc(model->materials, materialCapacity * sizeof(Material));
                }

                printf("MTL: %s  Kd=(%f, %f, %f)  tex=%u\n",
                       currentMaterial.name,
                       currentMaterial.kd[0],
                       currentMaterial.kd[1],
                       currentMaterial.kd[2],
                       currentMaterial.textureID);

                model->materials[model->materialCount++] = currentMaterial;
            }

            memset(&currentMaterial, 0, sizeof(Material));
            sscanf(ptr, "newmtl %63s", currentMaterial.name);

            currentMaterial.textureID = 0;
            currentMaterial.kd[0] = 1.0f;
            currentMaterial.kd[1] = 1.0f;
            currentMaterial.kd[2] = 1.0f;

            hasCurrent = 1;
        }
        else if (strncmp(ptr, "Kd ", 3) == 0) {
            sscanf(ptr, "Kd %f %f %f",
                   &currentMaterial.kd[0],
                   &currentMaterial.kd[1],
                   &currentMaterial.kd[2]);
        }
        else if (strncmp(ptr, "map_Kd ", 7) == 0) {
            char texFile[256];
            char texPath[512];

            sscanf(ptr, "map_Kd %255s", texFile);
            buildPath(baseDir, texFile, texPath);

            currentMaterial.textureID = loadTexture(texPath);
        }
    }

    if (hasCurrent) {
        if (model->materialCount >= materialCapacity) {
            materialCapacity = (materialCapacity == 0) ? 8 : materialCapacity * 2;
            model->materials = (Material*)realloc(model->materials, materialCapacity * sizeof(Material));
        }

        printf("MTL: %s  Kd=(%f, %f, %f)  tex=%u\n",
               currentMaterial.name,
               currentMaterial.kd[0],
               currentMaterial.kd[1],
               currentMaterial.kd[2],
               currentMaterial.textureID);

        model->materials[model->materialCount++] = currentMaterial;
    }

    fclose(file);
}

int loadOBJ(const char* filename, Model* model)
{
    model->fallbackTextureID = 0;

    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Failed to open: %s\n", filename);
        return 0;
    }

    model->vertices = NULL;
    model->texcoords = NULL;
    model->faces = NULL;
    model->materials = NULL;

    model->vertexCount = 0;
    model->texcoordCount = 0;
    model->faceCount = 0;
    model->materialCount = 0;

    int vertexCapacity = 0;
    int texcoordCapacity = 0;
    int faceCapacity = 0;

    int currentMaterialIndex = -1;

    char baseDir[512];
    getDirectoryFromPath(filename, baseDir);

    char line[512];

    while (fgets(line, sizeof(line), file)) {
        char* ptr = line;
        while (*ptr == ' ' || *ptr == '\t') {
            ptr++;
        }

        if (strncmp(ptr, "mtllib ", 7) == 0) {
            char mtlFile[256];
            char mtlPath[512];

            strcpy(mtlFile, ptr + 7);
            mtlFile[strcspn(mtlFile, "\r\n")] = '\0';

            buildPath(baseDir, mtlFile, mtlPath);
            loadMTL(mtlPath, model);
        }
        else if (strncmp(ptr, "usemtl ", 7) == 0) {
            char materialName[64];
            sscanf(ptr, "usemtl %63s", materialName);
            currentMaterialIndex = findMaterialIndex(model, materialName);

            printf("usemtl: %s -> index = %d\n", materialName, currentMaterialIndex);
        }
        else if (strncmp(ptr, "v ", 2) == 0) {
            if (model->vertexCount >= vertexCapacity) {
                vertexCapacity = (vertexCapacity == 0) ? 128 : vertexCapacity * 2;
                model->vertices = (Vertex*)realloc(model->vertices, vertexCapacity * sizeof(Vertex));
            }

            Vertex v;
            sscanf(ptr, "v %f %f %f", &v.x, &v.y, &v.z);
            model->vertices[model->vertexCount++] = v;
        }
        else if (strncmp(ptr, "vt ", 3) == 0) {
            if (model->texcoordCount >= texcoordCapacity) {
                texcoordCapacity = (texcoordCapacity == 0) ? 128 : texcoordCapacity * 2;
                model->texcoords = (TexCoord*)realloc(model->texcoords, texcoordCapacity * sizeof(TexCoord));
            }

            TexCoord t;
            sscanf(ptr, "vt %f %f", &t.u, &t.v);
            model->texcoords[model->texcoordCount++] = t;
        }
        else if (strncmp(ptr, "f ", 2) == 0) {
            char buffer[512];
            strcpy(buffer, ptr);

            char* tokens[32];
            int tokenCount = 0;

            char* tok = strtok(buffer, " \t\r\n");

            tok = strtok(NULL, " \t\r\n");

            while (tok && tokenCount < 32) {
                tokens[tokenCount++] = tok;
                tok = strtok(NULL, " \t\r\n");
            }

            if (tokenCount >= 3) {
                int vIdx[32];
                int vtIdx[32];

                for (int i = 0; i < tokenCount; i++) {
                    parseFaceVertexToken(tokens[i], &vIdx[i], &vtIdx[i]);
                }

                for (int i = 1; i < tokenCount - 1; i++) {
                    Face face;
                    memset(&face, 0, sizeof(Face));

                    face.materialIndex = currentMaterialIndex;

                    face.v[0] = vIdx[0];
                    face.v[1] = vIdx[i];
                    face.v[2] = vIdx[i + 1];

                    face.vt[0] = vtIdx[0];
                    face.vt[1] = vtIdx[i];
                    face.vt[2] = vtIdx[i + 1];

                    if (model->faceCount >= faceCapacity) {
                        faceCapacity = (faceCapacity == 0) ? 128 : faceCapacity * 2;
                        model->faces = (Face*)realloc(model->faces, faceCapacity * sizeof(Face));
                    }

                    model->faces[model->faceCount++] = face;
                }
            }
        }
    }

    fclose(file);

    printf("%s -> vertices: %d texcoords: %d faces: %d materials: %d\n",
           filename,
           model->vertexCount,
           model->texcoordCount,
           model->faceCount,
           model->materialCount);

    return 1;
}

GLuint loadModelTexture(const char* filename)
{
    return loadTexture(filename);

}

void renderModel(const Model* model)
{
    static int printed = 0;

    if (!printed) {
        int counts[64] = {0};

        for (int i = 0; i < model->faceCount; i++) {
            int mi = model->faces[i].materialIndex;
            if (mi >= 0 && mi < 64) {
                counts[mi]++;
            }
        }

        for (int i = 0; i < model->materialCount; i++) {
            printf("material[%d] = %s, faces = %d, Kd=(%f,%f,%f)\n",
                   i,
                   model->materials[i].name,
                   counts[i],
                   model->materials[i].kd[0],
                   model->materials[i].kd[1],
                   model->materials[i].kd[2]);
        }

        printed = 1;
    }

    for (int i = 0; i < model->faceCount; i++) {
        Face f = model->faces[i];

        if (f.materialIndex >= 0 && f.materialIndex < model->materialCount) {
            Material m = model->materials[f.materialIndex];

            float r = m.kd[0] * brightness;
            float g = m.kd[1] * brightness;
            float b = m.kd[2] * brightness;

            if (r > 1.0f) r = 1.0f;
            if (g > 1.0f) g = 1.0f;
            if (b > 1.0f) b = 1.0f;

            glColor3f(r, g, b);

            if (m.textureID != 0) {
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, m.textureID);
            } else {
                glDisable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, 0);
            }
        } else {
            float c = 1.0f * brightness;
            if (c > 1.0f) c = 1.0f;

            glColor3f(c, c, c);

            if (model->fallbackTextureID != 0) {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, model->fallbackTextureID);

            } else {
            glDisable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, 0);

            }
        }

        glBegin(GL_TRIANGLES);

        for (int j = 0; j < 3; j++) {
            if (f.v[j] < 0 || f.v[j] >= model->vertexCount) {
                continue;
            }

            Vertex v = model->vertices[f.v[j]];

            if (f.vt[j] >= 0 && f.vt[j] < model->texcoordCount) {
                TexCoord t = model->texcoords[f.vt[j]];
                glTexCoord2f(t.u, t.v);
            }

            glVertex3f(v.x, v.y, v.z);
        }

        glEnd();
    }

    glDisable(GL_TEXTURE_2D);
    glColor3f(1.0f, 1.0f, 1.0f);
}

void renderShadowModel(const Model* model, float lightX, float lightY, float lightZ)
{
    if (lightY == 0.0f) {
        return;
    }

    GLboolean lightingWasEnabled = glIsEnabled(GL_LIGHTING);
    GLboolean textureWasEnabled = glIsEnabled(GL_TEXTURE_2D);
    GLboolean blendWasEnabled = glIsEnabled(GL_BLEND);

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDepthMask(GL_FALSE);

    glColor4f(0.0f, 0.0f, 0.0f, 0.45f);

    glBegin(GL_TRIANGLES);

    for (int i = 0; i < model->faceCount; i++) {
        Face f = model->faces[i];

        for (int j = 0; j < 3; j++) {
            if (f.v[j] < 0 || f.v[j] >= model->vertexCount) {
                continue;
            }

            Vertex v = model->vertices[f.v[j]];

            float sx = v.x - (v.y * lightX / lightY);
            float sz = v.z - (v.y * lightZ / lightY);
            float sy = 0.02f;

            glVertex3f(sx, sy, sz);
        }
    }

    glEnd();

    glDepthMask(GL_TRUE);

    if (!blendWasEnabled) {
        glDisable(GL_BLEND);
    }

    if (textureWasEnabled) {
        glEnable(GL_TEXTURE_2D);
    } else {
        glDisable(GL_TEXTURE_2D);
    }

    if (lightingWasEnabled) {
        glEnable(GL_LIGHTING);
    } else {
        glDisable(GL_LIGHTING);
    }

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void freeModel(Model* model)
{
    if (model->materials) {
        for (int i = 0; i < model->materialCount; i++) {
            if (model->materials[i].textureID != 0) {
                glDeleteTextures(1, &model->materials[i].textureID);
            }
        }
    }

    free(model->vertices);
    free(model->texcoords);
    free(model->faces);
    free(model->materials);

    model->vertices = NULL;
    model->texcoords = NULL;
    model->faces = NULL;
    model->materials = NULL;

    model->vertexCount = 0;
    model->texcoordCount = 0;
    model->faceCount = 0;
    model->materialCount = 0;
}
