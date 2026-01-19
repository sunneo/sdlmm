#ifndef SCENE_JSON_H
#define SCENE_JSON_H

#include "cJSON.h"
#include "babylon3D.h"

// Scene format types
typedef enum {
    SCENE_FORMAT_2D = 0,
    SCENE_FORMAT_3D = 1
} SceneFormat;

// 2D Shape types
typedef enum {
    SHAPE_RECTANGLE = 0,
    SHAPE_CIRCLE = 1,
    SHAPE_ROUNDED_RECTANGLE = 2,
    SHAPE_LINE = 3,
    SHAPE_TEXT = 4,
    SHAPE_IMAGE = 5,
    SHAPE_MODE7_PANEL = 6
} ShapeType;

// 2D Shape structure
typedef struct {
    ShapeType type;
    int layer;
    
    // Common properties
    int x, y;
    int width, height;
    int borderColor;
    int fillColor;
    int hasBorder;
    int hasFill;
    
    // For Circle and RoundedRectangle
    double radius;
    int cornerRadius;
    
    // For Line
    int x2, y2;
    
    // For Text
    char* text;
    char* fontFile;
    int fontSize;
    
    // For Image
    char* imageFile;
    
    // For Mode7 panel
    float angle;
    int viewX, viewY;
    char* backgroundImage;
} Shape2D;

// 2D Scene structure
typedef struct {
    SceneFormat format;
    int shapeCount;
    Shape2D* shapes;
    int width;
    int height;
    int backgroundColor;
} Scene2D;

// 3D Light structure
typedef struct {
    Vector3 position;
    float intensity;
    int color;
} Light3D;

// 3D Model structure (extends Mesh)
typedef struct {
    Mesh* mesh;
    char* modelFile;
    char* textureFile;
    Vector3 position;
    Vector3 rotation;
    Vector3 scale;
} Model3D;

// 3D Scene structure
typedef struct {
    SceneFormat format;
    Camera camera;
    int lightCount;
    Light3D* lights;
    int modelCount;
    Model3D* models;
    int width;
    int height;
    int backgroundColor;
} Scene3D;

// Function declarations

// 2D Scene functions
Scene2D* scene2d_create(int width, int height);
void scene2d_free(Scene2D* scene);
void scene2d_add_shape(Scene2D* scene, const Shape2D* shape);
Scene2D* scene2d_load_from_json(const char* filename);
int scene2d_save_to_json(const Scene2D* scene, const char* filename);
void scene2d_render(const Scene2D* scene);

// 3D Scene functions
Scene3D* scene3d_create(int width, int height);
void scene3d_free(Scene3D* scene);
void scene3d_add_model(Scene3D* scene, const Model3D* model);
void scene3d_add_light(Scene3D* scene, const Light3D* light);
Scene3D* scene3d_load_from_json(const char* filename);
int scene3d_save_to_json(const Scene3D* scene, const char* filename);
void scene3d_render(const Scene3D* scene, Device* device);

// Generic scene functions
SceneFormat scene_get_format_from_json(const char* filename);

#endif // SCENE_JSON_H
