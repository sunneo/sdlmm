#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "babylon3D.h"

// Stub implementations for testing without SDL

Vector3 vector3(float x, float y, float z) {
    Vector3 ret;
    ret.x = x;
    ret.y = y;
    ret.z = z;
    return ret;
}

Vector3 vector3_zero(void) {
    return vector3(0.0f, 0.0f, 0.0f);
}

Vector3 vector3_normalize_copy(const Vector3* vector) {
    float length = sqrtf(vector->x * vector->x + vector->y * vector->y + vector->z * vector->z);
    if (length == 0.0f) {
        return vector3_zero();
    }
    return vector3(vector->x / length, vector->y / length, vector->z / length);
}

Mesh* softengine_mesh(const char* name, int verticesCount, int facesCount) {
    Mesh* mesh = (Mesh*)malloc(sizeof(Mesh));
    if (!mesh) return NULL;
    
    snprintf(mesh->name, sizeof(mesh->name), "%s", name);
    mesh->verticesCount = verticesCount;
    mesh->faceCount = facesCount;
    mesh->Vertices = (Vertex*)calloc(verticesCount, sizeof(Vertex));
    mesh->faces = (Face*)calloc(facesCount, sizeof(Face));
    mesh->Rotation = vector3_zero();
    mesh->Position = vector3_zero();
    mesh->texture.width = 0;
    mesh->texture.height = 0;
    mesh->texture.internalBuffer = NULL;
    
    return mesh;
}

void mesh_free(Mesh* mesh) {
    if (!mesh) return;
    if (mesh->Vertices) free(mesh->Vertices);
    if (mesh->faces) free(mesh->faces);
    if (mesh->texture.internalBuffer) free(mesh->texture.internalBuffer);
    free(mesh);
}

Mesh* mesh_load_obj(const char* filename) {
    // Stub implementation - creates a simple cube for testing
    printf("Stub: Loading OBJ file '%s' (creating test cube)\n", filename);
    
    Mesh* mesh = softengine_mesh(filename, 8, 12);
    if (!mesh) return NULL;
    
    // Define cube vertices
    mesh->Vertices[0].Coordinates = vector3(-1.0f, -1.0f, -1.0f);
    mesh->Vertices[1].Coordinates = vector3(-1.0f, -1.0f,  1.0f);
    mesh->Vertices[2].Coordinates = vector3(-1.0f,  1.0f, -1.0f);
    mesh->Vertices[3].Coordinates = vector3(-1.0f,  1.0f,  1.0f);
    mesh->Vertices[4].Coordinates = vector3( 1.0f, -1.0f, -1.0f);
    mesh->Vertices[5].Coordinates = vector3( 1.0f, -1.0f,  1.0f);
    mesh->Vertices[6].Coordinates = vector3( 1.0f,  1.0f, -1.0f);
    mesh->Vertices[7].Coordinates = vector3( 1.0f,  1.0f,  1.0f);
    
    // Set default normals
    for (int i = 0; i < 8; i++) {
        mesh->Vertices[i].Normal = vector3(0.0f, 1.0f, 0.0f);
        mesh->Vertices[i].TextureCoordinates = vector3_zero();
    }
    
    // Define cube faces (2 triangles per face = 12 triangles)
    int faceIdx = 0;
    // Front face
    mesh->faces[faceIdx].A = 0; mesh->faces[faceIdx].B = 2; mesh->faces[faceIdx++].C = 3;
    mesh->faces[faceIdx].A = 0; mesh->faces[faceIdx].B = 3; mesh->faces[faceIdx++].C = 1;
    // Back face
    mesh->faces[faceIdx].A = 4; mesh->faces[faceIdx].B = 0; mesh->faces[faceIdx++].C = 1;
    mesh->faces[faceIdx].A = 4; mesh->faces[faceIdx].B = 1; mesh->faces[faceIdx++].C = 5;
    // Right face
    mesh->faces[faceIdx].A = 4; mesh->faces[faceIdx].B = 5; mesh->faces[faceIdx++].C = 7;
    mesh->faces[faceIdx].A = 4; mesh->faces[faceIdx].B = 7; mesh->faces[faceIdx++].C = 6;
    // Left face
    mesh->faces[faceIdx].A = 6; mesh->faces[faceIdx].B = 7; mesh->faces[faceIdx++].C = 3;
    mesh->faces[faceIdx].A = 6; mesh->faces[faceIdx].B = 3; mesh->faces[faceIdx++].C = 2;
    // Top face
    mesh->faces[faceIdx].A = 1; mesh->faces[faceIdx].B = 3; mesh->faces[faceIdx++].C = 7;
    mesh->faces[faceIdx].A = 1; mesh->faces[faceIdx].B = 7; mesh->faces[faceIdx++].C = 5;
    // Bottom face
    mesh->faces[faceIdx].A = 6; mesh->faces[faceIdx].B = 2; mesh->faces[faceIdx++].C = 0;
    mesh->faces[faceIdx].A = 6; mesh->faces[faceIdx].B = 0; mesh->faces[faceIdx++].C = 4;
    
    return mesh;
}

Device* device(int width, int height) {
    Device* dev = (Device*)malloc(sizeof(Device));
    if (!dev) return NULL;
    
    dev->workingWidth = width;
    dev->workingHeight = height;
    dev->backbuffer = (int*)calloc(width * height, sizeof(int));
    dev->depthbuffer = (int*)calloc(width * height, sizeof(int));
    
    return dev;
}

void device_free(Device* dev) {
    if (!dev) return;
    if (dev->backbuffer) free(dev->backbuffer);
    if (dev->depthbuffer) free(dev->depthbuffer);
    free(dev);
}

void device_clear(Device* dev) {
    if (!dev) return;
    memset(dev->backbuffer, 0, dev->workingWidth * dev->workingHeight * sizeof(int));
    memset(dev->depthbuffer, 0, dev->workingWidth * dev->workingHeight * sizeof(int));
}

void device_render(Device* dev, const Camera* camera, const Mesh* meshes, int meshesLength) {
    // Stub implementation - just clear the buffer
    if (dev) {
        device_clear(dev);
    }
}

Texture* texture_load(const char* filename) {
    Texture* tex = (Texture*)malloc(sizeof(Texture));
    if (!tex) return NULL;
    
    tex->width = 0;
    tex->height = 0;
    tex->internalBuffer = NULL;
    
    return tex;
}

void texture_unload(Texture* tex) {
    if (!tex) return;
    if (tex->internalBuffer) free(tex->internalBuffer);
    free(tex);
}

// Stub SDLMM functions for testing
void fillrect(int x, int y, int w, int h, int color) {
    // Stub - does nothing
}

void fillcircle(int x, int y, double r, int color) {
    // Stub - does nothing
}

void drawcircle(int x, int y, double r, int color) {
    // Stub - does nothing
}

void drawrect(int x, int y, int w, int h, int color) {
    // Stub - does nothing
}

void drawline(int x1, int y1, int x2, int y2, int color) {
    // Stub - does nothing
}

void drawtext(const char* str, int x, int y, int color) {
    // Stub - does nothing
}

void settextfont(const char* font, int fontsize) {
    // Stub - does nothing
}

void loadimage(const char* filename, int** ret, int* w, int* h) {
    // Stub - return empty image
    *ret = NULL;
    *w = 0;
    *h = 0;
}

void drawpixels(int* pixels, int x, int y, int w, int h) {
    // Stub - does nothing
}

void mode7render(float angle, int vx, int vy, int* bg, int bw, int bh, int tx, int ty, int w, int h) {
    // Stub - does nothing
}
