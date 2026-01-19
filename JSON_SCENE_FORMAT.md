# JSON Scene Format for SDLMM + Babylon3D

This implementation provides a JSON-based scene description format for both 2D and 3D rendering using SDLMM and Babylon3D engines.

## Overview

The JSON scene format allows you to:
- Define 2D scenes with shapes, images, text, and Mode7 effects
- Define 3D scenes with models, lights, and camera settings
- Save and load rendering configurations
- Easily switch between 2D and 3D rendering modes

## Features

### 2D Scene Support
- **Shapes**: Rectangle, Circle, RoundedRectangle, Line
- **Colors**: Border color, fill color with alpha support
- **Layers**: Z-ordering for drawing priority
- **Text**: Font file, font size, positioning
- **Images**: Image files with positioning
- **Mode7**: Pseudo-3D ground rendering

### 3D Scene Support
- **Camera**: Position and target vectors
- **Lights**: Position, intensity, and color
- **Models**: 3D meshes with position, rotation, and scale
- **Rendering State**: Scene-wide settings

## JSON Format

### 2D Scene Format

```json
{
  "format": 0,
  "width": 800,
  "height": 600,
  "backgroundColor": 16777215,
  "shapes": [
    {
      "type": 0,
      "layer": 0,
      "x": 100,
      "y": 100,
      "width": 200,
      "height": 150,
      "borderColor": 0,
      "fillColor": 16711680,
      "hasBorder": true,
      "hasFill": true
    }
  ]
}
```

#### Shape Types
- `0` - Rectangle
- `1` - Circle
- `2` - RoundedRectangle
- `3` - Line
- `4` - Text
- `5` - Image
- `6` - Mode7 Panel

#### 2D Shape Properties

**Common Properties:**
- `type` (int): Shape type (see above)
- `layer` (int): Z-order layer (higher values drawn on top)
- `x`, `y` (int): Position
- `width`, `height` (int): Dimensions
- `borderColor` (int): Border color in 0xRRGGBB format
- `fillColor` (int): Fill color in 0xRRGGBB format
- `hasBorder` (bool): Whether to draw border
- `hasFill` (bool): Whether to fill shape

**Circle/RoundedRectangle:**
- `radius` (float): Circle radius
- `cornerRadius` (int): Corner radius for rounded rectangles

**Line:**
- `x2`, `y2` (int): End point coordinates

**Text:**
- `text` (string): Text to display
- `fontFile` (string): Path to font file (e.g., "FreeMono.ttf")
- `fontSize` (int): Font size in points

**Image:**
- `imageFile` (string): Path to image file

**Mode7 Panel:**
- `angle` (float): Rotation angle
- `viewX`, `viewY` (int): Viewer position
- `backgroundImage` (string): Path to background texture

### 3D Scene Format

```json
{
  "format": 1,
  "width": 800,
  "height": 600,
  "backgroundColor": 0,
  "camera": {
    "position": [0, 0, 10],
    "target": [0, 0, 0]
  },
  "lights": [
    {
      "position": [5, 5, 5],
      "intensity": 1.0,
      "color": 16777215
    }
  ],
  "models": [
    {
      "modelFile": "cube.obj",
      "position": [0, 0, 0],
      "rotation": [0, 0, 0],
      "scale": [1, 1, 1]
    }
  ]
}
```

#### 3D Scene Properties

**Camera:**
- `position` (array[3]): Camera position [x, y, z]
- `target` (array[3]): Camera look-at target [x, y, z]

**Lights:**
- `position` (array[3]): Light position [x, y, z]
- `intensity` (float): Light intensity (0.0 to 1.0)
- `color` (int): Light color in 0xRRGGBB format

**Models:**
- `modelFile` (string): Path to 3D model file
- `position` (array[3]): Model position [x, y, z]
- `rotation` (array[3]): Model rotation [x, y, z] in radians
- `scale` (array[3]): Model scale [x, y, z]

## API Reference

### Header Files

```c
#include "scene_json.h"  // Scene format API
#include "sdlmm.h"       // 2D rendering
#include "babylon3D.h"   // 3D rendering
```

### 2D Scene API

#### Create/Destroy
```c
Scene2D* scene2d_create(int width, int height);
void scene2d_free(Scene2D* scene);
```

#### Add Shapes
```c
void scene2d_add_shape(Scene2D* scene, const Shape2D* shape);
```

#### Load/Save
```c
Scene2D* scene2d_load_from_json(const char* filename);
int scene2d_save_to_json(const Scene2D* scene, const char* filename);
```

#### Render
```c
void scene2d_render(const Scene2D* scene);
```

### 3D Scene API

#### Create/Destroy
```c
Scene3D* scene3d_create(int width, int height);
void scene3d_free(Scene3D* scene);
```

#### Add Elements
```c
void scene3d_add_model(Scene3D* scene, const Model3D* model);
void scene3d_add_light(Scene3D* scene, const Light3D* light);
```

#### Load/Save
```c
Scene3D* scene3d_load_from_json(const char* filename);
int scene3d_save_to_json(const Scene3D* scene, const char* filename);
```

#### Render
```c
void scene3d_render(const Scene3D* scene, Device* device);
```

### Utility Functions

```c
SceneFormat scene_get_format_from_json(const char* filename);
```

Returns `SCENE_FORMAT_2D` (0) or `SCENE_FORMAT_3D` (1).

## Usage Examples

### Loading and Rendering a 2D Scene

```c
#include "sdlmm.h"
#include "scene_json.h"

int main() {
    // Load scene from JSON
    Scene2D* scene = scene2d_load_from_json("scene.json");
    
    // Initialize screen
    screen(scene->width, scene->height);
    screentitle("2D Scene Viewer");
    
    // Render loop
    while (1) {
        scene2d_render(scene);
        flushscreen();
        delay(16);
    }
    
    scene2d_free(scene);
    return 0;
}
```

### Creating and Saving a 2D Scene

```c
#include "scene_json.h"

int main() {
    Scene2D* scene = scene2d_create(800, 600);
    
    // Add a red rectangle
    Shape2D rect = {0};
    rect.type = SHAPE_RECTANGLE;
    rect.x = 100;
    rect.y = 100;
    rect.width = 200;
    rect.height = 150;
    rect.fillColor = 0xFF0000;
    rect.hasFill = 1;
    scene2d_add_shape(scene, &rect);
    
    // Save to file
    scene2d_save_to_json(scene, "my_scene.json");
    
    scene2d_free(scene);
    return 0;
}
```

### Loading and Rendering a 3D Scene

```c
#include "sdlmm.h"
#include "babylon3D.h"
#include "scene_json.h"

int main() {
    // Load scene from JSON
    Scene3D* scene = scene3d_load_from_json("scene3d.json");
    
    // Create 3D device
    Device* device = device(scene->width, scene->height);
    
    // Initialize screen
    screen(scene->width, scene->height);
    screentitle("3D Scene Viewer");
    
    // Render loop
    while (1) {
        scene3d_render(scene, device);
        drawpixels(device->backbuffer, 0, 0, 
                   device->workingWidth, device->workingHeight);
        flushscreen();
        delay(16);
    }
    
    device_free(device);
    scene3d_free(scene);
    return 0;
}
```

### Auto-detecting Scene Format

```c
#include "scene_json.h"

int main() {
    const char* filename = "scene.json";
    SceneFormat format = scene_get_format_from_json(filename);
    
    if (format == SCENE_FORMAT_2D) {
        Scene2D* scene = scene2d_load_from_json(filename);
        // ... render 2D scene
        scene2d_free(scene);
    } else {
        Scene3D* scene = scene3d_load_from_json(filename);
        // ... render 3D scene
        scene3d_free(scene);
    }
    
    return 0;
}
```

## Building

### Prerequisites
- GCC or compatible C compiler
- cJSON library (included)
- SDL 1.2 with SDL_image and SDL_ttf (for actual rendering)
- FreeType2

### Compile Scene Viewer
```bash
make scene_viewer
```

### Run Tests
```bash
make test_scene_json
./test_scene_json
```

## Files

- `scene_json.h` - Header file with API declarations
- `scene_json.c` - Implementation of scene loading/saving/rendering
- `scene_viewer.c` - Example program for viewing JSON scenes
- `test_scene_json.c` - Test program for JSON functionality
- `test_stubs.c` - Stub implementations for testing without SDL
- `example_2d_scene.json` - Example 2D scene
- `example_3d_scene.json` - Example 3D scene
- `cJSON.h`, `cJSON.c` - JSON parsing library

## Color Format

Colors are specified as 32-bit integers in 0xRRGGBB format:
- `0xFF0000` - Red
- `0x00FF00` - Green
- `0x0000FF` - Blue
- `0xFFFFFF` - White
- `0x000000` - Black
- `0xFFFF00` - Yellow

## Layer Ordering

In 2D scenes, shapes are rendered in layer order:
- Lower layer numbers are drawn first (back)
- Higher layer numbers are drawn last (front)
- Shapes with the same layer are drawn in the order they appear in the JSON

## Future Enhancements

Potential improvements for this system:
- Animation keyframes in JSON
- Shader parameters for 3D rendering
- Joint/bone support for skeletal animation
- Particle system definitions
- Audio cue integration
- Scene transitions and effects

## License

This implementation follows the same BSD 3-Clause License as the SDLMM project.

## Contributing

To extend this format:
1. Add new shape types to the `ShapeType` enum
2. Add corresponding properties to `Shape2D` structure
3. Implement loading in `scene2d_load_from_json`
4. Implement saving in `scene2d_save_to_json`
5. Implement rendering in `scene2d_render`
6. Update this documentation

---

For questions or issues, please refer to the main SDLMM repository.
