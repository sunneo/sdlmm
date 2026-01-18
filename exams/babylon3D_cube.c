#include "../sdlmm.h"
#include "../babylon3D.c"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Screen dimensions
static const int width = 800;
static const int height = 600;

// 3D objects
static Device* m_device = NULL;
static Mesh* cubeMesh = NULL;
static Camera camera;

// Create a simple cube mesh
static void createCube() {
    // A cube needs 24 vertices (4 per face) for proper texture mapping
    // This is because each vertex can have different UV coordinates per face
    cubeMesh = softengine_mesh("Cube", 24, 12);
    
    if(!cubeMesh) {
        fprintf(stderr, "Failed to create cube mesh\n");
        return;
    }
    
    // Front face (Z = 1)
    cubeMesh->Vertices[0].Coordinates = vector3(-1, 1, 1);   // Top-left
    cubeMesh->Vertices[1].Coordinates = vector3(1, 1, 1);    // Top-right
    cubeMesh->Vertices[2].Coordinates = vector3(-1, -1, 1);  // Bottom-left
    cubeMesh->Vertices[3].Coordinates = vector3(1, -1, 1);   // Bottom-right
    
    // Back face (Z = -1)
    cubeMesh->Vertices[4].Coordinates = vector3(1, 1, -1);   // Top-left (from back)
    cubeMesh->Vertices[5].Coordinates = vector3(-1, 1, -1);  // Top-right (from back)
    cubeMesh->Vertices[6].Coordinates = vector3(1, -1, -1);  // Bottom-left (from back)
    cubeMesh->Vertices[7].Coordinates = vector3(-1, -1, -1); // Bottom-right (from back)
    
    // Top face (Y = 1)
    cubeMesh->Vertices[8].Coordinates = vector3(-1, 1, -1);  // Top-left
    cubeMesh->Vertices[9].Coordinates = vector3(1, 1, -1);   // Top-right
    cubeMesh->Vertices[10].Coordinates = vector3(-1, 1, 1);  // Bottom-left
    cubeMesh->Vertices[11].Coordinates = vector3(1, 1, 1);   // Bottom-right
    
    // Bottom face (Y = -1)
    cubeMesh->Vertices[12].Coordinates = vector3(-1, -1, 1); // Top-left
    cubeMesh->Vertices[13].Coordinates = vector3(1, -1, 1);  // Top-right
    cubeMesh->Vertices[14].Coordinates = vector3(-1, -1, -1);// Bottom-left
    cubeMesh->Vertices[15].Coordinates = vector3(1, -1, -1); // Bottom-right
    
    // Left face (X = -1)
    cubeMesh->Vertices[16].Coordinates = vector3(-1, 1, -1); // Top-left
    cubeMesh->Vertices[17].Coordinates = vector3(-1, 1, 1);  // Top-right
    cubeMesh->Vertices[18].Coordinates = vector3(-1, -1, -1);// Bottom-left
    cubeMesh->Vertices[19].Coordinates = vector3(-1, -1, 1); // Bottom-right
    
    // Right face (X = 1)
    cubeMesh->Vertices[20].Coordinates = vector3(1, 1, 1);   // Top-left
    cubeMesh->Vertices[21].Coordinates = vector3(1, 1, -1);  // Top-right
    cubeMesh->Vertices[22].Coordinates = vector3(1, -1, 1);  // Bottom-left
    cubeMesh->Vertices[23].Coordinates = vector3(1, -1, -1); // Bottom-right
    
    // Set normals for each face
    int i;
    // Front face normals (pointing forward)
    for(i = 0; i < 4; i++) cubeMesh->Vertices[i].Normal = vector3(0, 0, 1);
    // Back face normals (pointing backward)
    for(i = 4; i < 8; i++) cubeMesh->Vertices[i].Normal = vector3(0, 0, -1);
    // Top face normals (pointing up)
    for(i = 8; i < 12; i++) cubeMesh->Vertices[i].Normal = vector3(0, 1, 0);
    // Bottom face normals (pointing down)
    for(i = 12; i < 16; i++) cubeMesh->Vertices[i].Normal = vector3(0, -1, 0);
    // Left face normals (pointing left)
    for(i = 16; i < 20; i++) cubeMesh->Vertices[i].Normal = vector3(-1, 0, 0);
    // Right face normals (pointing right)
    for(i = 20; i < 24; i++) cubeMesh->Vertices[i].Normal = vector3(1, 0, 0);
    
    // Initialize WorldCoordinates
    for(i = 0; i < 24; i++) {
        cubeMesh->Vertices[i].WorldCoordinates = vector3_zero();
    }
    
    // Set texture coordinates - each face gets the full texture (0,0) to (1,1)
    // This allows the texture to be mapped properly on each face
    for(i = 0; i < 6; i++) {
        int base = i * 4;
        cubeMesh->Vertices[base + 0].TextureCoordinates = vector3(0, 0, 0); // Top-left
        cubeMesh->Vertices[base + 1].TextureCoordinates = vector3(1, 0, 0); // Top-right
        cubeMesh->Vertices[base + 2].TextureCoordinates = vector3(0, 1, 0); // Bottom-left
        cubeMesh->Vertices[base + 3].TextureCoordinates = vector3(1, 1, 0); // Bottom-right
    }
    
    // Define the 12 faces (2 triangles per side, using the new vertex indices)
    // Front face
    cubeMesh->faces[0].A = 0; cubeMesh->faces[0].B = 1; cubeMesh->faces[0].C = 2;
    cubeMesh->faces[1].A = 1; cubeMesh->faces[1].B = 3; cubeMesh->faces[1].C = 2;
    
    // Back face
    cubeMesh->faces[2].A = 4; cubeMesh->faces[2].B = 5; cubeMesh->faces[2].C = 6;
    cubeMesh->faces[3].A = 5; cubeMesh->faces[3].B = 7; cubeMesh->faces[3].C = 6;
    
    // Top face
    cubeMesh->faces[4].A = 8; cubeMesh->faces[4].B = 9; cubeMesh->faces[4].C = 10;
    cubeMesh->faces[5].A = 9; cubeMesh->faces[5].B = 11; cubeMesh->faces[5].C = 10;
    
    // Bottom face
    cubeMesh->faces[6].A = 12; cubeMesh->faces[6].B = 13; cubeMesh->faces[6].C = 14;
    cubeMesh->faces[7].A = 13; cubeMesh->faces[7].B = 15; cubeMesh->faces[7].C = 14;
    
    // Left face
    cubeMesh->faces[8].A = 16; cubeMesh->faces[8].B = 17; cubeMesh->faces[8].C = 18;
    cubeMesh->faces[9].A = 17; cubeMesh->faces[9].B = 19; cubeMesh->faces[9].C = 18;
    
    // Right face
    cubeMesh->faces[10].A = 20; cubeMesh->faces[10].B = 21; cubeMesh->faces[10].C = 22;
    cubeMesh->faces[11].A = 21; cubeMesh->faces[11].B = 23; cubeMesh->faces[11].C = 22;
    
    // Set initial position and rotation
    cubeMesh->Position = vector3(0, 0, 10);
    cubeMesh->Rotation = vector3_zero();
    
    // Load texture - Note: we keep the loaded texture pointer structure
    // but the internalBuffer is owned by the mesh now
    Texture* loadedTexture = texture_load("texture.png");
    if(loadedTexture && loadedTexture->internalBuffer) {
        cubeMesh->texture = *loadedTexture;
        // Don't call texture_unload here as it would free the internalBuffer
        // Just free the Texture structure wrapper
        free(loadedTexture);
    } else {
        if(loadedTexture) free(loadedTexture);
        fprintf(stderr, "Warning: Failed to load texture.png, cube will render without texture\n");
        cubeMesh->texture.internalBuffer = NULL;
        cubeMesh->texture.width = 0;
        cubeMesh->texture.height = 0;
    }
}

static void drawfnc() {
    // Clear the m_device buffer
    device_clear(m_device);
    
    // Rotate the cube
    cubeMesh->Rotation.x += 0.01;
    cubeMesh->Rotation.y += 0.01;
    
    // Render the mesh
    device_render(m_device, &camera, cubeMesh, 1);
    
    // Present to screen
    flushscreen();
    
    delay(16); // ~60 FPS
}

static void cleanup() {
    if(cubeMesh) {
        mesh_free(cubeMesh);
        cubeMesh = NULL;
    }
    if(m_device) {
        device_free(m_device);
        m_device = NULL;
    }
}

int main(int argc, char** argv) {
    // Initialize SDL screen
    screen(width, height);
    screentitle("Babylon 3D - Rotating Cube Demo");
    
    // Create the rendering m_device
    m_device = device(width, height);
    if(!m_device) {
        fprintf(stderr, "Failed to create m_device\n");
        return 1;
    }
    
    // Setup camera
    camera.Position = vector3(0, 0, -10);
    camera.Target = vector3_zero();
    
    // Create the cube mesh
    createCube();
    
    // Register cleanup
    atexit(cleanup);
    
    // Main render loop
    while(1) {
        drawfnc();
    }
    
    return 0;
}
