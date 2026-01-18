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
    // A cube has 8 vertices and 12 faces (2 triangles per side, 6 sides)
    cubeMesh = softengine_mesh("Cube", 8, 12);
    
    if(!cubeMesh) {
        fprintf(stderr, "Failed to create cube mesh\n");
        return;
    }
    
    // Define the 8 vertices of a cube
    cubeMesh->Vertices[0].Coordinates = vector3(-1, 1, 1);
    cubeMesh->Vertices[1].Coordinates = vector3(1, 1, 1);
    cubeMesh->Vertices[2].Coordinates = vector3(-1, -1, 1);
    cubeMesh->Vertices[3].Coordinates = vector3(1, -1, 1);
    cubeMesh->Vertices[4].Coordinates = vector3(-1, 1, -1);
    cubeMesh->Vertices[5].Coordinates = vector3(1, 1, -1);
    cubeMesh->Vertices[6].Coordinates = vector3(-1, -1, -1);
    cubeMesh->Vertices[7].Coordinates = vector3(1, -1, -1);
    
    // Set normals and texture coordinates
    int i;
    for(i = 0; i < 8; i++) {
        cubeMesh->Vertices[i].Normal = vector3_normalize_copy(&cubeMesh->Vertices[i].Coordinates);
        cubeMesh->Vertices[i].WorldCoordinates = vector3_zero();
    }
    
    // Set texture coordinates for each vertex (UV mapping)
    // Vertex 0: (-1, 1, 1) -> (0, 0)
    cubeMesh->Vertices[0].TextureCoordinates = vector3(0, 0, 0);
    // Vertex 1: (1, 1, 1) -> (1, 0)
    cubeMesh->Vertices[1].TextureCoordinates = vector3(1, 0, 0);
    // Vertex 2: (-1, -1, 1) -> (0, 1)
    cubeMesh->Vertices[2].TextureCoordinates = vector3(0, 1, 0);
    // Vertex 3: (1, -1, 1) -> (1, 1)
    cubeMesh->Vertices[3].TextureCoordinates = vector3(1, 1, 0);
    // Vertex 4: (-1, 1, -1) -> (0, 0)
    cubeMesh->Vertices[4].TextureCoordinates = vector3(0, 0, 0);
    // Vertex 5: (1, 1, -1) -> (1, 0)
    cubeMesh->Vertices[5].TextureCoordinates = vector3(1, 0, 0);
    // Vertex 6: (-1, -1, -1) -> (0, 1)
    cubeMesh->Vertices[6].TextureCoordinates = vector3(0, 1, 0);
    // Vertex 7: (1, -1, -1) -> (1, 1)
    cubeMesh->Vertices[7].TextureCoordinates = vector3(1, 1, 0);
    
    // Define the 12 faces (2 triangles per side)
    // Front face
    cubeMesh->faces[0].A = 0; cubeMesh->faces[0].B = 1; cubeMesh->faces[0].C = 2;
    cubeMesh->faces[1].A = 1; cubeMesh->faces[1].B = 3; cubeMesh->faces[1].C = 2;
    
    // Back face
    cubeMesh->faces[2].A = 5; cubeMesh->faces[2].B = 4; cubeMesh->faces[2].C = 6;
    cubeMesh->faces[3].A = 5; cubeMesh->faces[3].B = 6; cubeMesh->faces[3].C = 7;
    
    // Top face
    cubeMesh->faces[4].A = 4; cubeMesh->faces[4].B = 5; cubeMesh->faces[4].C = 0;
    cubeMesh->faces[5].A = 5; cubeMesh->faces[5].B = 1; cubeMesh->faces[5].C = 0;
    
    // Bottom face
    cubeMesh->faces[6].A = 2; cubeMesh->faces[6].B = 3; cubeMesh->faces[6].C = 6;
    cubeMesh->faces[7].A = 3; cubeMesh->faces[7].B = 7; cubeMesh->faces[7].C = 6;
    
    // Left face
    cubeMesh->faces[8].A = 4; cubeMesh->faces[8].B = 0; cubeMesh->faces[8].C = 2;
    cubeMesh->faces[9].A = 4; cubeMesh->faces[9].B = 2; cubeMesh->faces[9].C = 6;
    
    // Right face
    cubeMesh->faces[10].A = 1; cubeMesh->faces[10].B = 5; cubeMesh->faces[10].C = 7;
    cubeMesh->faces[11].A = 1; cubeMesh->faces[11].B = 7; cubeMesh->faces[11].C = 3;
    
    // Set initial position and rotation
    cubeMesh->Position = vector3(0, 0, 10);
    cubeMesh->Rotation = vector3_zero();
    
    // Load texture
    cubeMesh->texture = *texture_load("texture.png");
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
