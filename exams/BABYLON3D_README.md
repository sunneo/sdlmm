# Babylon 3D Examples

This directory contains examples demonstrating 3D rendering using the babylon3D library with sdlmm.

## missilecmd3d.c

A 3D version of the classic Missile Command game with advanced visual effects.

### Recent Enhancements (2026-02)

The game now features enhanced visual effects:

1. **Fixed Camera with Mouse Wheel Zoom**
   - Camera position is locked at a fixed viewing angle
   - Use mouse wheel to zoom in/out (distance: 8-50 units)
   - Provides stable viewing experience while maintaining gameplay flexibility

2. **Transparent 3D Trajectory Lines**
   - Enemy missiles display semi-transparent trajectory lines showing their flight path
   - Lines are drawn from current position to target
   - Fades along the trajectory for depth perception
   - Uses 3D-to-2D projection for accurate screen rendering

3. **Semi-Transparent Smoke Trail Effects**
   - Our missiles leave a smoke trail as they travel
   - Particles fade out gradually (alpha blending)
   - Maximum 200 smoke particles system
   - Spawned periodically during missile flight

4. **N-Body Style Glowing Explosion Particles**
   - Explosions spawn 50+ glowing particles
   - Particles expand and fade like N-body simulations
   - Cyan/turquoise/white glow colors
   - Maximum 500 explosion particles system
   - Uses additive blending for glow effects

### Controls:
- **Mouse Click**: Launch interceptor missile toward cursor position
- **Mouse Wheel**: Zoom camera in/out
- **h/H**: Toggle help overlay

### How to compile:

```bash
make missilecmd3d
# Or manually:
gcc -O2 -I/usr/include/SDL -I/usr/include/freetype2 -I../ -msse2 \
    missilecmd3d.c ../sdlmm.c -lSDL -lm -lpthread -lSDL_ttf -lSDL_image \
    -lfreetype -fopenmp -o missilecmd3d
```

### Technical Implementation:

**Particle Systems:**
- Two separate particle systems: smoke (200 max) and explosions (500 max)
- Each particle has position, velocity, color, life, and size
- Particles update position and fade based on lifetime
- Uses device_render_particles() for efficient batch rendering

**Trajectory Lines:**
- Computed in 3D space from missile position to target
- Segmented into 20 parts for smooth curves
- Projected to screen space using camera matrices
- Alpha blending for transparency effect

**Mouse Wheel Support:**
- New event handler added to sdlmm library (setonwheel)
- SDL 1.2 mouse wheel events (button 4=up, 5=down)
- Smooth zoom with distance limits

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
