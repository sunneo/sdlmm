# OBJ Model Loader and JSON Model Support

This document describes the 3D model loading capabilities of the babylon3D engine.

## Overview

The babylon3D engine now supports two methods for loading 3D models:

1. **External OBJ Files** - Load models from Wavefront OBJ format files
2. **Inline JSON Models** - Define simple models directly in JSON scene files

## OBJ File Loader

### Features

- Full support for Wavefront OBJ format (.obj files)
- Parses vertices (v), normals (vn), texture coordinates (vt), and faces (f)
- Supports multiple face formats:
  - `f v1 v2 v3` (vertices only)
  - `f v1/vt1 v2/vt2 v3/vt3` (vertices and texture coordinates)
  - `f v1//vn1 v2//vn2 v3//vn3` (vertices and normals)
  - `f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3` (vertices, texture coords, and normals)
- Automatic conversion of OBJ 1-based indices to 0-based
- Default normals assigned if not provided
- Error handling for malformed files

### API

```c
#include "babylon3D.h"

// Load an OBJ file into a Mesh structure
Mesh* mesh_load_obj(const char* filename);

// Free mesh memory when done
void mesh_free(Mesh* mesh);
```

### Example Usage

```c
#include "babylon3D.h"

int main() {
    // Load cube model
    Mesh* cube = mesh_load_obj("cube.obj");
    if (!cube) {
        printf("Failed to load cube.obj\n");
        return 1;
    }
    
    printf("Loaded: %d vertices, %d faces\n", 
           cube->verticesCount, cube->faceCount);
    
    // Use mesh for rendering...
    
    // Clean up
    mesh_free(cube);
    return 0;
}
```

## Sample Model: cube.obj

A sample cube model is included (`cube.obj`):
- 8 vertices defining the corners of a unit cube
- 12 triangular faces (2 per cube face)
- Includes vertex normals for proper lighting
- UV texture coordinates for texture mapping

## JSON Model Support

### Using External OBJ Files

Reference an OBJ file in your JSON scene:

```json
{
  "format": 1,
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

### Using Inline Mesh Data

Define simple models directly in JSON:

```json
{
  "format": 1,
  "models": [
    {
      "position": [0, 0, 0],
      "rotation": [0, 0, 0],
      "scale": [1, 1, 1],
      "mesh": {
        "vertices": [
          {
            "coordinates": [-1, -1, -1],
            "normal": [0, 0, -1],
            "texCoord": [0, 0]
          },
          {
            "coordinates": [1, -1, -1],
            "normal": [0, 0, -1],
            "texCoord": [1, 0]
          },
          {
            "coordinates": [1, 1, -1],
            "normal": [0, 0, -1],
            "texCoord": [1, 1]
          },
          {
            "coordinates": [-1, 1, -1],
            "normal": [0, 0, -1],
            "texCoord": [0, 1]
          }
        ],
        "faces": [
          [0, 1, 2],
          [0, 2, 3]
        ]
      }
    }
  ]
}
```

### Vertex Properties

Each vertex in an inline mesh can have:

- **coordinates** (required): `[x, y, z]` position in 3D space
- **normal** (optional): `[x, y, z]` surface normal for lighting (default: `[0, 1, 0]`)
- **texCoord** (optional): `[u, v]` texture coordinates (default: `[0, 0]`)

### Face Definition

Faces are defined as triangles using vertex indices:
- `[0, 1, 2]` defines a triangle using vertices 0, 1, and 2
- Indices are 0-based
- Faces must reference valid vertex indices

## Implementation Details

### OBJ Loader (`mesh_load_obj`)

1. **First Pass**: Counts vertices, normals, texture coordinates, and faces
2. **Second Pass**: Reads and stores vertex data
3. **Third Pass**: Reads faces and creates unique vertices for each face vertex
4. **Memory Management**: Allocates mesh with proper vertex and face counts

### JSON Mesh Loader (`mesh_load_from_json`)

1. Parses vertex array with coordinates, normals, and texture coordinates
2. Parses face array with vertex indices
3. Creates mesh structure with all vertex and face data
4. Validates data and provides default values for missing properties

## Error Handling

Both loaders include comprehensive error checking:

- File not found errors
- Invalid format detection
- Empty or malformed data
- Out-of-bounds indices
- Memory allocation failures

Errors are reported to stdout with descriptive messages.

## Performance Considerations

### OBJ Files
- Suitable for complex models with many vertices
- File I/O overhead on load
- Efficient for models reused across scenes
- Recommended for models > 100 vertices

### Inline JSON Models
- Best for simple models (< 100 vertices)
- No file I/O overhead
- Larger JSON file size
- Good for procedural or scene-specific geometry

## Creating Your Own Models

### Using 3D Software

1. Create model in Blender, Maya, 3DS Max, etc.
2. Export as Wavefront OBJ format
3. Reference in JSON scene: `"modelFile": "mymodel.obj"`

### Manual OBJ Creation

Simple models can be hand-coded:

```obj
# Tetrahedron example
v 0.0 1.0 0.0
v -1.0 -1.0 1.0
v 1.0 -1.0 1.0
v 0.0 -1.0 -1.0

vn 0.0 1.0 0.0
vn -0.816 -0.333 0.471
vn 0.816 -0.333 0.471
vn 0.0 -0.333 -0.943

f 1//1 2//2 3//3
f 1//1 3//3 4//4
f 1//1 4//4 2//2
f 2//2 4//4 3//3
```

### Inline JSON Creation

For simple shapes, define directly in JSON (see examples above).

## Examples

See the provided example files:
- `example_3d_scene.json` - Uses external `cube.obj` file
- `example_3d_scene_inline.json` - Uses inline mesh data
- `cube.obj` - Sample cube model

## Building and Testing

```bash
# Build test suite
make test_scene_json

# Run tests (includes model loading tests)
./test_scene_json

# Build scene viewer (requires SDL)
make scene_viewer

# View a 3D scene
./scene_viewer example_3d_scene.json
```

## Integration with Scene System

The model loading is fully integrated with the JSON scene system:

```c
#include "scene_json.h"

// Load scene (automatically loads models)
Scene3D* scene = scene3d_load_from_json("scene.json");

// Models are ready to render
scene3d_render(scene, device);

// Cleanup (also frees all loaded models)
scene3d_free(scene);
```

## Future Enhancements

Potential improvements:
- Support for other model formats (PLY, STL, etc.)
- Model caching to avoid reloading
- Texture loading from OBJ material files (MTL)
- Skeletal animation support
- Level of detail (LOD) systems

## License

This implementation follows the same BSD 3-Clause License as the SDLMM project.
