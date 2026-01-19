#include <stdio.h>
#include <stdlib.h>
#include "scene_json.h"

int main(int argc, char* argv[]) {
    printf("=== Testing JSON Scene Format ===\n\n");
    
    // Test 1: Create and save a 2D scene
    printf("Test 1: Creating 2D scene...\n");
    Scene2D* scene2d = scene2d_create(800, 600);
    
    // Add a rectangle
    Shape2D rect = {0};
    rect.type = SHAPE_RECTANGLE;
    rect.layer = 0;
    rect.x = 100;
    rect.y = 100;
    rect.width = 200;
    rect.height = 150;
    rect.borderColor = 0x000000;
    rect.fillColor = 0xFF0000;
    rect.hasBorder = 1;
    rect.hasFill = 1;
    scene2d_add_shape(scene2d, &rect);
    
    // Add a circle
    Shape2D circle = {0};
    circle.type = SHAPE_CIRCLE;
    circle.layer = 1;
    circle.x = 400;
    circle.y = 300;
    circle.radius = 80.0;
    circle.borderColor = 0x0000FF;
    circle.fillColor = 0x00FF00;
    circle.hasBorder = 1;
    circle.hasFill = 1;
    scene2d_add_shape(scene2d, &circle);
    
    // Add text
    Shape2D text = {0};
    text.type = SHAPE_TEXT;
    text.layer = 2;
    text.x = 300;
    text.y = 50;
    text.text = "Hello JSON!";
    text.fontFile = "FreeMono.ttf";
    text.fontSize = 24;
    text.fillColor = 0x000000;
    text.hasFill = 1;
    scene2d_add_shape(scene2d, &text);
    
    printf("  Added %d shapes\n", scene2d->shapeCount);
    
    // Save to JSON
    printf("  Saving to test_2d_scene.json...\n");
    if (scene2d_save_to_json(scene2d, "test_2d_scene.json")) {
        printf("  ✓ Saved successfully\n");
    } else {
        printf("  ✗ Failed to save\n");
    }
    
    scene2d_free(scene2d);
    
    // Test 2: Load the 2D scene back
    printf("\nTest 2: Loading 2D scene from test_2d_scene.json...\n");
    Scene2D* loaded2d = scene2d_load_from_json("test_2d_scene.json");
    if (loaded2d) {
        printf("  ✓ Loaded successfully\n");
        printf("  Width: %d, Height: %d\n", loaded2d->width, loaded2d->height);
        printf("  Shapes: %d\n", loaded2d->shapeCount);
        for (int i = 0; i < loaded2d->shapeCount; i++) {
            printf("    Shape %d: type=%d, layer=%d, x=%d, y=%d\n",
                   i, loaded2d->shapes[i].type, loaded2d->shapes[i].layer,
                   loaded2d->shapes[i].x, loaded2d->shapes[i].y);
        }
        scene2d_free(loaded2d);
    } else {
        printf("  ✗ Failed to load\n");
    }
    
    // Test 3: Create and save a 3D scene
    printf("\nTest 3: Creating 3D scene...\n");
    Scene3D* scene3d = scene3d_create(800, 600);
    
    // Set camera
    scene3d->camera.Position = vector3(5.0f, 5.0f, 10.0f);
    scene3d->camera.Target = vector3_zero();
    
    // Add a light
    Light3D light = {0};
    light.position = vector3(5.0f, 5.0f, 5.0f);
    light.intensity = 1.0f;
    light.color = 0xFFFFFF;
    scene3d_add_light(scene3d, &light);
    
    // Add a model
    Model3D model = {0};
    model.modelFile = "cube.obj";
    model.position = vector3(0.0f, 0.0f, 0.0f);
    model.rotation = vector3(0.0f, 0.0f, 0.0f);
    model.scale = vector3(1.0f, 1.0f, 1.0f);
    model.mesh = NULL;  // Would be loaded in actual use
    scene3d_add_model(scene3d, &model);
    
    printf("  Added %d models and %d lights\n", scene3d->modelCount, scene3d->lightCount);
    
    // Save to JSON
    printf("  Saving to test_3d_scene.json...\n");
    if (scene3d_save_to_json(scene3d, "test_3d_scene.json")) {
        printf("  ✓ Saved successfully\n");
    } else {
        printf("  ✗ Failed to save\n");
    }
    
    scene3d_free(scene3d);
    
    // Test 4: Load the 3D scene back
    printf("\nTest 4: Loading 3D scene from test_3d_scene.json...\n");
    Scene3D* loaded3d = scene3d_load_from_json("test_3d_scene.json");
    if (loaded3d) {
        printf("  ✓ Loaded successfully\n");
        printf("  Width: %d, Height: %d\n", loaded3d->width, loaded3d->height);
        printf("  Camera Position: (%.2f, %.2f, %.2f)\n",
               loaded3d->camera.Position.x,
               loaded3d->camera.Position.y,
               loaded3d->camera.Position.z);
        printf("  Camera Target: (%.2f, %.2f, %.2f)\n",
               loaded3d->camera.Target.x,
               loaded3d->camera.Target.y,
               loaded3d->camera.Target.z);
        printf("  Models: %d\n", loaded3d->modelCount);
        printf("  Lights: %d\n", loaded3d->lightCount);
        scene3d_free(loaded3d);
    } else {
        printf("  ✗ Failed to load\n");
    }
    
    // Test 5: Test format detection
    printf("\nTest 5: Testing format detection...\n");
    SceneFormat format2d = scene_get_format_from_json("test_2d_scene.json");
    SceneFormat format3d = scene_get_format_from_json("test_3d_scene.json");
    printf("  test_2d_scene.json format: %d (expected 0)\n", format2d);
    printf("  test_3d_scene.json format: %d (expected 1)\n", format3d);
    
    if (format2d == SCENE_FORMAT_2D && format3d == SCENE_FORMAT_3D) {
        printf("  ✓ Format detection working correctly\n");
    } else {
        printf("  ✗ Format detection failed\n");
    }
    
    // Test 6: Load example JSON files
    printf("\nTest 6: Loading example JSON files...\n");
    Scene2D* example2d = scene2d_load_from_json("example_2d_scene.json");
    if (example2d) {
        printf("  ✓ Loaded example_2d_scene.json: %d shapes\n", example2d->shapeCount);
        scene2d_free(example2d);
    } else {
        printf("  ✗ Failed to load example_2d_scene.json\n");
    }
    
    Scene3D* example3d = scene3d_load_from_json("example_3d_scene.json");
    if (example3d) {
        printf("  ✓ Loaded example_3d_scene.json: %d models, %d lights\n",
               example3d->modelCount, example3d->lightCount);
        scene3d_free(example3d);
    } else {
        printf("  ✗ Failed to load example_3d_scene.json\n");
    }
    
    printf("\n=== All tests completed ===\n");
    return 0;
}
