#include <stdio.h>
#include <stdlib.h>
#include "sdlmm.h"
#include "babylon3D.h"
#include "scene_json.h"

// Global scene pointers
Scene2D* scene2d = NULL;
Scene3D* scene3d = NULL;
Device* device3d = NULL;
SceneFormat currentFormat = SCENE_FORMAT_2D;

void render_frame() {
    if (currentFormat == SCENE_FORMAT_2D && scene2d) {
        scene2d_render(scene2d);
    } else if (currentFormat == SCENE_FORMAT_3D && scene3d && device3d) {
        scene3d_render(scene3d, device3d);
        // Copy device backbuffer to screen
        drawpixels(device3d->backbuffer, 0, 0, device3d->workingWidth, device3d->workingHeight);
    }
    flushscreen();
}

void cleanup() {
    if (scene2d) {
        scene2d_free(scene2d);
        scene2d = NULL;
    }
    if (scene3d) {
        scene3d_free(scene3d);
        scene3d = NULL;
    }
    if (device3d) {
        device_free(device3d);
        device3d = NULL;
    }
}

int main(int argc, char* argv[]) {
    const char* filename = "scene.json";
    int width = 800;
    int height = 600;
    
    // Parse command line arguments
    if (argc > 1) {
        filename = argv[1];
    }
    
    printf("Loading scene from: %s\n", filename);
    
    // Detect scene format
    currentFormat = scene_get_format_from_json(filename);
    
    if (currentFormat == SCENE_FORMAT_2D) {
        printf("Loading 2D scene...\n");
        scene2d = scene2d_load_from_json(filename);
        if (!scene2d) {
            printf("Failed to load 2D scene from %s\n", filename);
            return 1;
        }
        width = scene2d->width;
        height = scene2d->height;
        printf("2D scene loaded successfully: %dx%d, %d shapes\n", 
               width, height, scene2d->shapeCount);
    } else {
        printf("Loading 3D scene...\n");
        scene3d = scene3d_load_from_json(filename);
        if (!scene3d) {
            printf("Failed to load 3D scene from %s\n", filename);
            return 1;
        }
        width = scene3d->width;
        height = scene3d->height;
        device3d = device(width, height);
        if (!device3d) {
            printf("Failed to create 3D device\n");
            cleanup();
            return 1;
        }
        printf("3D scene loaded successfully: %dx%d, %d models, %d lights\n", 
               width, height, scene3d->modelCount, scene3d->lightCount);
    }
    
    // Initialize SDL screen
    screen(width, height);
    screentitle("SDLMM+Babylon3D JSON Scene Viewer");
    
    // Main rendering loop
    int running = 1;
    int frameCount = 0;
    
    printf("Rendering scene... Press Ctrl+C to exit\n");
    
    while (running && frameCount < 300) {  // Run for 300 frames for testing
        render_frame();
        delay(16);  // ~60 FPS
        frameCount++;
        
        // For 3D scenes, rotate the camera for demo
        if (currentFormat == SCENE_FORMAT_3D && scene3d) {
            float angle = frameCount * 0.01f;
            scene3d->camera.Position.x = 10.0f * cosf(angle);
            scene3d->camera.Position.z = 10.0f * sinf(angle);
        }
    }
    
    printf("Rendered %d frames\n", frameCount);
    
    cleanup();
    return 0;
}
