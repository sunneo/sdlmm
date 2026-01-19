#include "scene_json.h"
#include "sdlmm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helper function to read file contents
static char* read_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* content = (char*)malloc(length + 1);
    if (content) {
        size_t read = fread(content, 1, length, file);
        if (read < (size_t)length) {
            // If we read less than expected, adjust the string termination
            content[read] = '\0';
        } else {
            content[length] = '\0';
        }
    }
    
    fclose(file);
    return content;
}

// Helper function to write file contents
static int write_file(const char* filename, const char* content) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        return 0;
    }
    
    fprintf(file, "%s", content);
    fclose(file);
    return 1;
}

// 2D Scene functions

Scene2D* scene2d_create(int width, int height) {
    Scene2D* scene = (Scene2D*)malloc(sizeof(Scene2D));
    if (!scene) return NULL;
    
    scene->format = SCENE_FORMAT_2D;
    scene->width = width;
    scene->height = height;
    scene->backgroundColor = 0xFFFFFF;
    scene->shapeCount = 0;
    scene->shapes = NULL;
    
    return scene;
}

void scene2d_free(Scene2D* scene) {
    if (!scene) return;
    
    for (int i = 0; i < scene->shapeCount; i++) {
        if (scene->shapes[i].text) free(scene->shapes[i].text);
        if (scene->shapes[i].fontFile) free(scene->shapes[i].fontFile);
        if (scene->shapes[i].imageFile) free(scene->shapes[i].imageFile);
        if (scene->shapes[i].backgroundImage) free(scene->shapes[i].backgroundImage);
    }
    
    if (scene->shapes) free(scene->shapes);
    free(scene);
}

void scene2d_add_shape(Scene2D* scene, const Shape2D* shape) {
    if (!scene || !shape) return;
    
    scene->shapes = (Shape2D*)realloc(scene->shapes, sizeof(Shape2D) * (scene->shapeCount + 1));
    if (!scene->shapes) return;
    
    // Deep copy the shape
    scene->shapes[scene->shapeCount] = *shape;
    
    // Deep copy strings
    if (shape->text) {
        scene->shapes[scene->shapeCount].text = strdup(shape->text);
    }
    if (shape->fontFile) {
        scene->shapes[scene->shapeCount].fontFile = strdup(shape->fontFile);
    }
    if (shape->imageFile) {
        scene->shapes[scene->shapeCount].imageFile = strdup(shape->imageFile);
    }
    if (shape->backgroundImage) {
        scene->shapes[scene->shapeCount].backgroundImage = strdup(shape->backgroundImage);
    }
    
    scene->shapeCount++;
}

Scene2D* scene2d_load_from_json(const char* filename) {
    char* content = read_file(filename);
    if (!content) return NULL;
    
    cJSON* root = cJSON_Parse(content);
    free(content);
    
    if (!root) return NULL;
    
    cJSON* format = cJSON_GetObjectItem(root, "format");
    if (!format || cJSON_GetNumberValue(format) != SCENE_FORMAT_2D) {
        cJSON_Delete(root);
        return NULL;
    }
    
    cJSON* width = cJSON_GetObjectItem(root, "width");
    cJSON* height = cJSON_GetObjectItem(root, "height");
    
    Scene2D* scene = scene2d_create(
        width ? (int)cJSON_GetNumberValue(width) : 800,
        height ? (int)cJSON_GetNumberValue(height) : 600
    );
    
    cJSON* bgColor = cJSON_GetObjectItem(root, "backgroundColor");
    if (bgColor) {
        scene->backgroundColor = (int)cJSON_GetNumberValue(bgColor);
    }
    
    cJSON* shapes = cJSON_GetObjectItem(root, "shapes");
    if (shapes && cJSON_IsArray(shapes)) {
        cJSON* shapeItem = NULL;
        cJSON_ArrayForEach(shapeItem, shapes) {
            Shape2D shape = {0};
            
            cJSON* type = cJSON_GetObjectItem(shapeItem, "type");
            shape.type = type ? (ShapeType)(int)cJSON_GetNumberValue(type) : SHAPE_RECTANGLE;
            
            cJSON* layer = cJSON_GetObjectItem(shapeItem, "layer");
            shape.layer = layer ? (int)cJSON_GetNumberValue(layer) : 0;
            
            cJSON* x = cJSON_GetObjectItem(shapeItem, "x");
            shape.x = x ? (int)cJSON_GetNumberValue(x) : 0;
            
            cJSON* y = cJSON_GetObjectItem(shapeItem, "y");
            shape.y = y ? (int)cJSON_GetNumberValue(y) : 0;
            
            cJSON* w = cJSON_GetObjectItem(shapeItem, "width");
            shape.width = w ? (int)cJSON_GetNumberValue(w) : 0;
            
            cJSON* h = cJSON_GetObjectItem(shapeItem, "height");
            shape.height = h ? (int)cJSON_GetNumberValue(h) : 0;
            
            cJSON* borderColor = cJSON_GetObjectItem(shapeItem, "borderColor");
            shape.borderColor = borderColor ? (int)cJSON_GetNumberValue(borderColor) : 0x000000;
            
            cJSON* fillColor = cJSON_GetObjectItem(shapeItem, "fillColor");
            shape.fillColor = fillColor ? (int)cJSON_GetNumberValue(fillColor) : 0xFFFFFF;
            
            cJSON* hasBorder = cJSON_GetObjectItem(shapeItem, "hasBorder");
            shape.hasBorder = hasBorder ? cJSON_IsTrue(hasBorder) : 1;
            
            cJSON* hasFill = cJSON_GetObjectItem(shapeItem, "hasFill");
            shape.hasFill = hasFill ? cJSON_IsTrue(hasFill) : 1;
            
            cJSON* radius = cJSON_GetObjectItem(shapeItem, "radius");
            shape.radius = radius ? cJSON_GetNumberValue(radius) : 0.0;
            
            cJSON* cornerRadius = cJSON_GetObjectItem(shapeItem, "cornerRadius");
            shape.cornerRadius = cornerRadius ? (int)cJSON_GetNumberValue(cornerRadius) : 0;
            
            cJSON* x2 = cJSON_GetObjectItem(shapeItem, "x2");
            shape.x2 = x2 ? (int)cJSON_GetNumberValue(x2) : 0;
            
            cJSON* y2 = cJSON_GetObjectItem(shapeItem, "y2");
            shape.y2 = y2 ? (int)cJSON_GetNumberValue(y2) : 0;
            
            cJSON* text = cJSON_GetObjectItem(shapeItem, "text");
            shape.text = text ? strdup(cJSON_GetStringValue(text)) : NULL;
            
            cJSON* fontFile = cJSON_GetObjectItem(shapeItem, "fontFile");
            shape.fontFile = fontFile ? strdup(cJSON_GetStringValue(fontFile)) : NULL;
            
            cJSON* fontSize = cJSON_GetObjectItem(shapeItem, "fontSize");
            shape.fontSize = fontSize ? (int)cJSON_GetNumberValue(fontSize) : 12;
            
            cJSON* imageFile = cJSON_GetObjectItem(shapeItem, "imageFile");
            shape.imageFile = imageFile ? strdup(cJSON_GetStringValue(imageFile)) : NULL;
            
            cJSON* angle = cJSON_GetObjectItem(shapeItem, "angle");
            shape.angle = angle ? (float)cJSON_GetNumberValue(angle) : 0.0f;
            
            cJSON* viewX = cJSON_GetObjectItem(shapeItem, "viewX");
            shape.viewX = viewX ? (int)cJSON_GetNumberValue(viewX) : 0;
            
            cJSON* viewY = cJSON_GetObjectItem(shapeItem, "viewY");
            shape.viewY = viewY ? (int)cJSON_GetNumberValue(viewY) : 0;
            
            cJSON* backgroundImage = cJSON_GetObjectItem(shapeItem, "backgroundImage");
            shape.backgroundImage = backgroundImage ? strdup(cJSON_GetStringValue(backgroundImage)) : NULL;
            
            // Add deep copy of shape to scene
            scene2d_add_shape(scene, &shape);
            
            // Free temporary strings (they were deep-copied by scene2d_add_shape)
            if (shape.text) free(shape.text);
            if (shape.fontFile) free(shape.fontFile);
            if (shape.imageFile) free(shape.imageFile);
            if (shape.backgroundImage) free(shape.backgroundImage);
        }
    }
    
    cJSON_Delete(root);
    return scene;
}

int scene2d_save_to_json(const Scene2D* scene, const char* filename) {
    if (!scene) return 0;
    
    cJSON* root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "format", SCENE_FORMAT_2D);
    cJSON_AddNumberToObject(root, "width", scene->width);
    cJSON_AddNumberToObject(root, "height", scene->height);
    cJSON_AddNumberToObject(root, "backgroundColor", scene->backgroundColor);
    
    cJSON* shapes = cJSON_CreateArray();
    for (int i = 0; i < scene->shapeCount; i++) {
        const Shape2D* shape = &scene->shapes[i];
        cJSON* shapeObj = cJSON_CreateObject();
        
        cJSON_AddNumberToObject(shapeObj, "type", shape->type);
        cJSON_AddNumberToObject(shapeObj, "layer", shape->layer);
        cJSON_AddNumberToObject(shapeObj, "x", shape->x);
        cJSON_AddNumberToObject(shapeObj, "y", shape->y);
        cJSON_AddNumberToObject(shapeObj, "width", shape->width);
        cJSON_AddNumberToObject(shapeObj, "height", shape->height);
        cJSON_AddNumberToObject(shapeObj, "borderColor", shape->borderColor);
        cJSON_AddNumberToObject(shapeObj, "fillColor", shape->fillColor);
        cJSON_AddBoolToObject(shapeObj, "hasBorder", shape->hasBorder);
        cJSON_AddBoolToObject(shapeObj, "hasFill", shape->hasFill);
        
        if (shape->type == SHAPE_CIRCLE || shape->type == SHAPE_ROUNDED_RECTANGLE) {
            cJSON_AddNumberToObject(shapeObj, "radius", shape->radius);
        }
        
        if (shape->type == SHAPE_ROUNDED_RECTANGLE) {
            cJSON_AddNumberToObject(shapeObj, "cornerRadius", shape->cornerRadius);
        }
        
        if (shape->type == SHAPE_LINE) {
            cJSON_AddNumberToObject(shapeObj, "x2", shape->x2);
            cJSON_AddNumberToObject(shapeObj, "y2", shape->y2);
        }
        
        if (shape->type == SHAPE_TEXT) {
            if (shape->text) cJSON_AddStringToObject(shapeObj, "text", shape->text);
            if (shape->fontFile) cJSON_AddStringToObject(shapeObj, "fontFile", shape->fontFile);
            cJSON_AddNumberToObject(shapeObj, "fontSize", shape->fontSize);
        }
        
        if (shape->type == SHAPE_IMAGE) {
            if (shape->imageFile) cJSON_AddStringToObject(shapeObj, "imageFile", shape->imageFile);
        }
        
        if (shape->type == SHAPE_MODE7_PANEL) {
            cJSON_AddNumberToObject(shapeObj, "angle", shape->angle);
            cJSON_AddNumberToObject(shapeObj, "viewX", shape->viewX);
            cJSON_AddNumberToObject(shapeObj, "viewY", shape->viewY);
            if (shape->backgroundImage) cJSON_AddStringToObject(shapeObj, "backgroundImage", shape->backgroundImage);
        }
        
        cJSON_AddItemToArray(shapes, shapeObj);
    }
    cJSON_AddItemToObject(root, "shapes", shapes);
    
    char* json_str = cJSON_Print(root);
    int result = write_file(filename, json_str);
    
    free(json_str);
    cJSON_Delete(root);
    
    return result;
}

// Comparison function for qsort (sort by layer)
static int compare_shapes_by_layer(const void* a, const void* b) {
    const Shape2D* shapeA = (const Shape2D*)a;
    const Shape2D* shapeB = (const Shape2D*)b;
    return shapeA->layer - shapeB->layer;
}

void scene2d_render(const Scene2D* scene) {
    if (!scene) return;
    
    // Sort shapes by layer (create a copy to avoid modifying original)
    Shape2D* sortedShapes = (Shape2D*)malloc(sizeof(Shape2D) * scene->shapeCount);
    if (!sortedShapes) return;
    
    // Deep copy shapes to avoid pointer aliasing issues
    for (int i = 0; i < scene->shapeCount; i++) {
        sortedShapes[i] = scene->shapes[i];
        // Note: We don't deep copy strings here because we're only reading them
        // and this copy is temporary (freed at end of function)
    }
    
    qsort(sortedShapes, scene->shapeCount, sizeof(Shape2D), compare_shapes_by_layer);
    
    // Clear background
    fillrect(0, 0, scene->width, scene->height, scene->backgroundColor);
    
    // Render each shape
    for (int i = 0; i < scene->shapeCount; i++) {
        const Shape2D* shape = &sortedShapes[i];
        
        switch (shape->type) {
            case SHAPE_RECTANGLE:
                if (shape->hasFill) {
                    fillrect(shape->x, shape->y, shape->width, shape->height, shape->fillColor);
                }
                if (shape->hasBorder) {
                    drawrect(shape->x, shape->y, shape->width, shape->height, shape->borderColor);
                }
                break;
                
            case SHAPE_CIRCLE:
                if (shape->hasFill) {
                    fillcircle(shape->x, shape->y, shape->radius, shape->fillColor);
                }
                if (shape->hasBorder) {
                    drawcircle(shape->x, shape->y, shape->radius, shape->borderColor);
                }
                break;
                
            case SHAPE_ROUNDED_RECTANGLE:
                // Approximate rounded rectangle with filled rect and circles at corners
                if (shape->hasFill) {
                    fillrect(shape->x + shape->cornerRadius, shape->y, 
                            shape->width - 2 * shape->cornerRadius, shape->height, shape->fillColor);
                    fillrect(shape->x, shape->y + shape->cornerRadius, 
                            shape->width, shape->height - 2 * shape->cornerRadius, shape->fillColor);
                    
                    // Corner circles
                    fillcircle(shape->x + shape->cornerRadius, shape->y + shape->cornerRadius, 
                              shape->cornerRadius, shape->fillColor);
                    fillcircle(shape->x + shape->width - shape->cornerRadius, shape->y + shape->cornerRadius, 
                              shape->cornerRadius, shape->fillColor);
                    fillcircle(shape->x + shape->cornerRadius, shape->y + shape->height - shape->cornerRadius, 
                              shape->cornerRadius, shape->fillColor);
                    fillcircle(shape->x + shape->width - shape->cornerRadius, shape->y + shape->height - shape->cornerRadius, 
                              shape->cornerRadius, shape->fillColor);
                }
                if (shape->hasBorder) {
                    drawrect(shape->x, shape->y, shape->width, shape->height, shape->borderColor);
                }
                break;
                
            case SHAPE_LINE:
                drawline(shape->x, shape->y, shape->x2, shape->y2, shape->borderColor);
                break;
                
            case SHAPE_TEXT:
                if (shape->text) {
                    if (shape->fontFile) {
                        settextfont(shape->fontFile, shape->fontSize);
                    }
                    drawtext(shape->text, shape->x, shape->y, shape->fillColor);
                }
                break;
                
            case SHAPE_IMAGE:
                if (shape->imageFile) {
                    int* pixels = NULL;
                    int w = 0, h = 0;
                    loadimage(shape->imageFile, &pixels, &w, &h);
                    if (pixels) {
                        drawpixels(pixels, shape->x, shape->y, w, h);
                        free(pixels);
                    }
                }
                break;
                
            case SHAPE_MODE7_PANEL:
                if (shape->backgroundImage) {
                    int* bgPixels = NULL;
                    int bgW = 0, bgH = 0;
                    loadimage(shape->backgroundImage, &bgPixels, &bgW, &bgH);
                    if (bgPixels) {
                        mode7render(shape->angle, shape->viewX, shape->viewY, 
                                   bgPixels, bgW, bgH,
                                   shape->x, shape->y, shape->width, shape->height);
                        free(bgPixels);
                    }
                }
                break;
        }
    }
    
    free(sortedShapes);
}

// 3D Scene functions

Scene3D* scene3d_create(int width, int height) {
    Scene3D* scene = (Scene3D*)malloc(sizeof(Scene3D));
    if (!scene) return NULL;
    
    scene->format = SCENE_FORMAT_3D;
    scene->width = width;
    scene->height = height;
    scene->backgroundColor = 0x000000;
    scene->camera.Position = vector3(0.0f, 0.0f, 10.0f);
    scene->camera.Target = vector3_zero();
    scene->lightCount = 0;
    scene->lights = NULL;
    scene->modelCount = 0;
    scene->models = NULL;
    
    return scene;
}

void scene3d_free(Scene3D* scene) {
    if (!scene) return;
    
    for (int i = 0; i < scene->modelCount; i++) {
        if (scene->models[i].modelFile) free(scene->models[i].modelFile);
        if (scene->models[i].mesh) mesh_free(scene->models[i].mesh);
    }
    
    if (scene->models) free(scene->models);
    if (scene->lights) free(scene->lights);
    free(scene);
}

void scene3d_add_model(Scene3D* scene, const Model3D* model) {
    if (!scene || !model) return;
    
    scene->models = (Model3D*)realloc(scene->models, sizeof(Model3D) * (scene->modelCount + 1));
    if (!scene->models) return;
    
    scene->models[scene->modelCount] = *model;
    if (model->modelFile) {
        scene->models[scene->modelCount].modelFile = strdup(model->modelFile);
    }
    
    scene->modelCount++;
}

void scene3d_add_light(Scene3D* scene, const Light3D* light) {
    if (!scene || !light) return;
    
    scene->lights = (Light3D*)realloc(scene->lights, sizeof(Light3D) * (scene->lightCount + 1));
    if (!scene->lights) return;
    
    scene->lights[scene->lightCount] = *light;
    scene->lightCount++;
}

Scene3D* scene3d_load_from_json(const char* filename) {
    char* content = read_file(filename);
    if (!content) return NULL;
    
    cJSON* root = cJSON_Parse(content);
    free(content);
    
    if (!root) return NULL;
    
    cJSON* format = cJSON_GetObjectItem(root, "format");
    if (!format || cJSON_GetNumberValue(format) != SCENE_FORMAT_3D) {
        cJSON_Delete(root);
        return NULL;
    }
    
    cJSON* width = cJSON_GetObjectItem(root, "width");
    cJSON* height = cJSON_GetObjectItem(root, "height");
    
    Scene3D* scene = scene3d_create(
        width ? (int)cJSON_GetNumberValue(width) : 800,
        height ? (int)cJSON_GetNumberValue(height) : 600
    );
    
    cJSON* bgColor = cJSON_GetObjectItem(root, "backgroundColor");
    if (bgColor) {
        scene->backgroundColor = (int)cJSON_GetNumberValue(bgColor);
    }
    
    // Load camera
    cJSON* camera = cJSON_GetObjectItem(root, "camera");
    if (camera) {
        cJSON* position = cJSON_GetObjectItem(camera, "position");
        if (position && cJSON_IsArray(position) && cJSON_GetArraySize(position) >= 3) {
            scene->camera.Position.x = (float)cJSON_GetNumberValue(cJSON_GetArrayItem(position, 0));
            scene->camera.Position.y = (float)cJSON_GetNumberValue(cJSON_GetArrayItem(position, 1));
            scene->camera.Position.z = (float)cJSON_GetNumberValue(cJSON_GetArrayItem(position, 2));
        }
        
        cJSON* target = cJSON_GetObjectItem(camera, "target");
        if (target && cJSON_IsArray(target) && cJSON_GetArraySize(target) >= 3) {
            scene->camera.Target.x = (float)cJSON_GetNumberValue(cJSON_GetArrayItem(target, 0));
            scene->camera.Target.y = (float)cJSON_GetNumberValue(cJSON_GetArrayItem(target, 1));
            scene->camera.Target.z = (float)cJSON_GetNumberValue(cJSON_GetArrayItem(target, 2));
        }
    }
    
    // Load lights
    cJSON* lights = cJSON_GetObjectItem(root, "lights");
    if (lights && cJSON_IsArray(lights)) {
        cJSON* lightItem = NULL;
        cJSON_ArrayForEach(lightItem, lights) {
            Light3D light = {0};
            
            cJSON* position = cJSON_GetObjectItem(lightItem, "position");
            if (position && cJSON_IsArray(position) && cJSON_GetArraySize(position) >= 3) {
                light.position.x = (float)cJSON_GetNumberValue(cJSON_GetArrayItem(position, 0));
                light.position.y = (float)cJSON_GetNumberValue(cJSON_GetArrayItem(position, 1));
                light.position.z = (float)cJSON_GetNumberValue(cJSON_GetArrayItem(position, 2));
            }
            
            cJSON* intensity = cJSON_GetObjectItem(lightItem, "intensity");
            light.intensity = intensity ? (float)cJSON_GetNumberValue(intensity) : 1.0f;
            
            cJSON* color = cJSON_GetObjectItem(lightItem, "color");
            light.color = color ? (int)cJSON_GetNumberValue(color) : 0xFFFFFF;
            
            scene3d_add_light(scene, &light);
        }
    }
    
    // Load models
    cJSON* models = cJSON_GetObjectItem(root, "models");
    if (models && cJSON_IsArray(models)) {
        cJSON* modelItem = NULL;
        cJSON_ArrayForEach(modelItem, models) {
            Model3D model = {0};
            
            cJSON* modelFile = cJSON_GetObjectItem(modelItem, "modelFile");
            model.modelFile = modelFile ? strdup(cJSON_GetStringValue(modelFile)) : NULL;
            
            cJSON* position = cJSON_GetObjectItem(modelItem, "position");
            if (position && cJSON_IsArray(position) && cJSON_GetArraySize(position) >= 3) {
                model.position.x = (float)cJSON_GetNumberValue(cJSON_GetArrayItem(position, 0));
                model.position.y = (float)cJSON_GetNumberValue(cJSON_GetArrayItem(position, 1));
                model.position.z = (float)cJSON_GetNumberValue(cJSON_GetArrayItem(position, 2));
            }
            
            cJSON* rotation = cJSON_GetObjectItem(modelItem, "rotation");
            if (rotation && cJSON_IsArray(rotation) && cJSON_GetArraySize(rotation) >= 3) {
                model.rotation.x = (float)cJSON_GetNumberValue(cJSON_GetArrayItem(rotation, 0));
                model.rotation.y = (float)cJSON_GetNumberValue(cJSON_GetArrayItem(rotation, 1));
                model.rotation.z = (float)cJSON_GetNumberValue(cJSON_GetArrayItem(rotation, 2));
            }
            
            cJSON* scale = cJSON_GetObjectItem(modelItem, "scale");
            if (scale && cJSON_IsArray(scale) && cJSON_GetArraySize(scale) >= 3) {
                model.scale.x = (float)cJSON_GetNumberValue(cJSON_GetArrayItem(scale, 0));
                model.scale.y = (float)cJSON_GetNumberValue(cJSON_GetArrayItem(scale, 1));
                model.scale.z = (float)cJSON_GetNumberValue(cJSON_GetArrayItem(scale, 2));
            } else {
                model.scale = vector3(1.0f, 1.0f, 1.0f);
            }
            
            // For now, create a simple cube mesh as placeholder
            // In a real implementation, you would load the model from file
            // Using constants for cube vertices (8 vertices, 12 triangular faces)
            #define CUBE_VERTICES 8
            #define CUBE_FACES 12
            model.mesh = softengine_mesh("placeholder", CUBE_VERTICES, CUBE_FACES);
            #undef CUBE_VERTICES
            #undef CUBE_FACES
            if (model.mesh) {
                model.mesh->Position = model.position;
                model.mesh->Rotation = model.rotation;
            }
            
            scene3d_add_model(scene, &model);
            
            if (model.modelFile) free(model.modelFile);
        }
    }
    
    cJSON_Delete(root);
    return scene;
}

int scene3d_save_to_json(const Scene3D* scene, const char* filename) {
    if (!scene) return 0;
    
    cJSON* root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "format", SCENE_FORMAT_3D);
    cJSON_AddNumberToObject(root, "width", scene->width);
    cJSON_AddNumberToObject(root, "height", scene->height);
    cJSON_AddNumberToObject(root, "backgroundColor", scene->backgroundColor);
    
    // Save camera
    cJSON* camera = cJSON_CreateObject();
    cJSON* camPosition = cJSON_CreateArray();
    cJSON_AddItemToArray(camPosition, cJSON_CreateNumber(scene->camera.Position.x));
    cJSON_AddItemToArray(camPosition, cJSON_CreateNumber(scene->camera.Position.y));
    cJSON_AddItemToArray(camPosition, cJSON_CreateNumber(scene->camera.Position.z));
    cJSON_AddItemToObject(camera, "position", camPosition);
    
    cJSON* camTarget = cJSON_CreateArray();
    cJSON_AddItemToArray(camTarget, cJSON_CreateNumber(scene->camera.Target.x));
    cJSON_AddItemToArray(camTarget, cJSON_CreateNumber(scene->camera.Target.y));
    cJSON_AddItemToArray(camTarget, cJSON_CreateNumber(scene->camera.Target.z));
    cJSON_AddItemToObject(camera, "target", camTarget);
    cJSON_AddItemToObject(root, "camera", camera);
    
    // Save lights
    cJSON* lights = cJSON_CreateArray();
    for (int i = 0; i < scene->lightCount; i++) {
        const Light3D* light = &scene->lights[i];
        cJSON* lightObj = cJSON_CreateObject();
        
        cJSON* position = cJSON_CreateArray();
        cJSON_AddItemToArray(position, cJSON_CreateNumber(light->position.x));
        cJSON_AddItemToArray(position, cJSON_CreateNumber(light->position.y));
        cJSON_AddItemToArray(position, cJSON_CreateNumber(light->position.z));
        cJSON_AddItemToObject(lightObj, "position", position);
        
        cJSON_AddNumberToObject(lightObj, "intensity", light->intensity);
        cJSON_AddNumberToObject(lightObj, "color", light->color);
        
        cJSON_AddItemToArray(lights, lightObj);
    }
    cJSON_AddItemToObject(root, "lights", lights);
    
    // Save models
    cJSON* models = cJSON_CreateArray();
    for (int i = 0; i < scene->modelCount; i++) {
        const Model3D* model = &scene->models[i];
        cJSON* modelObj = cJSON_CreateObject();
        
        if (model->modelFile) {
            cJSON_AddStringToObject(modelObj, "modelFile", model->modelFile);
        }
        
        cJSON* position = cJSON_CreateArray();
        cJSON_AddItemToArray(position, cJSON_CreateNumber(model->position.x));
        cJSON_AddItemToArray(position, cJSON_CreateNumber(model->position.y));
        cJSON_AddItemToArray(position, cJSON_CreateNumber(model->position.z));
        cJSON_AddItemToObject(modelObj, "position", position);
        
        cJSON* rotation = cJSON_CreateArray();
        cJSON_AddItemToArray(rotation, cJSON_CreateNumber(model->rotation.x));
        cJSON_AddItemToArray(rotation, cJSON_CreateNumber(model->rotation.y));
        cJSON_AddItemToArray(rotation, cJSON_CreateNumber(model->rotation.z));
        cJSON_AddItemToObject(modelObj, "rotation", rotation);
        
        cJSON* scale = cJSON_CreateArray();
        cJSON_AddItemToArray(scale, cJSON_CreateNumber(model->scale.x));
        cJSON_AddItemToArray(scale, cJSON_CreateNumber(model->scale.y));
        cJSON_AddItemToArray(scale, cJSON_CreateNumber(model->scale.z));
        cJSON_AddItemToObject(modelObj, "scale", scale);
        
        cJSON_AddItemToArray(models, modelObj);
    }
    cJSON_AddItemToObject(root, "models", models);
    
    char* json_str = cJSON_Print(root);
    int result = write_file(filename, json_str);
    
    free(json_str);
    cJSON_Delete(root);
    
    return result;
}

void scene3d_render(const Scene3D* scene, Device* device) {
    if (!scene || !device) return;
    
    device_clear(device);
    
    // Prepare mesh array for rendering
    // Note: device_render expects an array of Mesh structures, not pointers
    Mesh* meshArray = (Mesh*)malloc(sizeof(Mesh) * scene->modelCount);
    if (!meshArray) return;
    
    for (int i = 0; i < scene->modelCount; i++) {
        if (scene->models[i].mesh) {
            meshArray[i] = *(scene->models[i].mesh);
        }
    }
    
    // Render all meshes
    device_render(device, &scene->camera, (const Mesh*)meshArray, scene->modelCount);
    
    free(meshArray);
}

// Generic scene functions

SceneFormat scene_get_format_from_json(const char* filename) {
    char* content = read_file(filename);
    if (!content) return SCENE_FORMAT_2D;
    
    cJSON* root = cJSON_Parse(content);
    free(content);
    
    if (!root) return SCENE_FORMAT_2D;
    
    cJSON* format = cJSON_GetObjectItem(root, "format");
    SceneFormat result = format ? (SceneFormat)(int)cJSON_GetNumberValue(format) : SCENE_FORMAT_2D;
    
    cJSON_Delete(root);
    return result;
}
