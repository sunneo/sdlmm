# Babylon 3D Examples

This directory contains examples demonstrating 3D rendering using the babylon3D library with sdlmm.

## babylon3D_cube.c

A simple example that demonstrates:
- Creating a 3D device
- Setting up a camera
- Creating a cube mesh with vertices and faces
- Rendering a rotating cube in real-time
- Proper memory cleanup

### How to compile:

```bash
gcc -O2 -I/usr/include/SDL -I/usr/include/freetype2 -I../ \
    babylon3D_cube.c ../sdlmm.c \
    -lSDL -lm -lpthread -lSDL_ttf -lSDL_image -lfreetype -fopenmp \
    -o babylon3D_cube
```

### How to run:

```bash
./babylon3D_cube
```

### Features demonstrated:

1. **3D Mesh Creation**: Shows how to create a cube with 8 vertices and 12 faces (triangles)
2. **Camera Setup**: Demonstrates positioning the camera in 3D space
3. **Rotation Animation**: The cube rotates continuously on X and Y axes
4. **Lighting**: Basic lighting calculation with normal vectors
5. **Depth Buffering**: Proper depth-based rendering for 3D objects
6. **Memory Management**: Shows proper cleanup with mesh_free() and device_free()

### Understanding the Code:

The example follows the Babylon.js pattern:

1. **Device** - The rendering context that manages framebuffers
2. **Camera** - Defines the viewpoint and projection
3. **Mesh** - Contains geometry (vertices) and topology (faces)
4. **Rendering Loop** - Clears, transforms, renders, and presents each frame

### Key Functions:

- `device()` - Create a rendering device with specified dimensions
- `softengine_mesh()` - Create a mesh with vertices and faces
- `device_clear()` - Clear the depth and color buffers
- `device_render()` - Transform and render meshes with lighting
- `mesh_free()` / `device_free()` - Cleanup allocated resources

### Next Steps:

Try modifying the example to:
- Add more geometric shapes (pyramid, sphere, etc.)
- Load a texture and apply it to the cube
- Add multiple light sources
- Implement camera controls with keyboard/mouse input
