# nbody3d NVIDIA Sample Enhancements

This document describes the enhancements made to nbody3d.c to match the NVIDIA CUDA nbody sample functionality.

## Overview

The nbody3d simulation has been enhanced with:
1. Mouse-based camera rotation control
2. Smooth camera center tracking with gradual transitions
3. UI-based particle size control
4. Improved user interface with visual sliders

## Changes Made

### 1. Mouse Control for Camera Rotation

**Implementation:**
- Added mouse drag support for camera rotation (similar to NVIDIA sample)
- Horizontal mouse movement rotates camera around Y-axis (yaw)
- Vertical mouse movement rotates camera around X-axis (pitch, inverted for natural feel)
- Vertical angle is clamped to prevent camera flipping (-1.5 to 1.5 radians)

**New Variables:**
```c
static int mouse_down = 0;
static int last_mouse_x = 0;
static int last_mouse_y = 0;
```

**Key Functions:**
- `mousefnc()`: Updated to detect mouse down/up and distinguish between slider clicks and camera rotation
- `mousemotion()`: New logic to handle camera rotation during drag
  - Sensitivity: 0.005 radians per pixel
  - Automatically disables camera tracking when user manually controls camera

### 2. Camera Center Tracking

**Implementation:**
- Pressing 'C' key toggles smooth camera tracking mode
- When enabled, camera gradually transitions to center view
- Uses exponential smoothing for smooth, natural transitions
- Target angles: (0.3, 0.0), distance: 50.0

**New Variables:**
```c
static int camera_tracking = 0;
static float target_cam_angle_x = 0.3f;
static float target_cam_angle_y = 0.0f;
static float target_cam_dist = 50.0f;
static const float cam_transition_speed = 0.1f;  /* Exponential smoothing: 10% of remaining distance per frame */
```

**Key Features:**
- **Gradual transition**: Camera angles and distance smoothly interpolate to target values
- **Auto-centralize**: When tracking is enabled, particles are centered around their center of mass
- **Manual override**: Any manual camera control (arrows, +/-, mouse) disables tracking
- **Visual feedback**: HUD shows "camera tracking: on/off [C]"

**Updated Functions:**
- `updateCamera()`: Now handles smooth transitions when tracking is enabled
- `updateParticlePositions()`: Links particle centering to camera tracking
- `kbfnc()`: Updated 'C' key behavior to toggle tracking instead of centralize

### 3. Particle Size Control

**Implementation:**
- Added UI slider for dynamic particle size adjustment
- Range: 5.0 to 40.0 (default: 15.0)
- Green slider bar for visual distinction from simulation speed slider

**New Variable:**
```c
static float particle_size = 15.0f;
```

**UI Elements:**
- Slider position: y: 65-80, x: 320-720
- Color: Green (0x00ff00) for filled portion, white (0xffffff) for outline
- Display: Shows current particle size value on HUD

**Updated Functions:**
- `draw3D()`: Changed hardcoded 15.0f to `particle_size` variable
- `mousefnc()`: Added slider handling for particle size adjustment

### 4. UI Improvements

**Layout Changes:**
- Reorganized HUD to accommodate new controls
- Moved simulation factor slider to y: 25-40 (was y: 60-80)
- Added particle size slider at y: 65-80
- Added camera tracking status indicator
- Updated help text to reflect new controls

**HUD Layout:**
```
Line  5: [loop/total] tm:xxx bodies:xxx
Line 25: simulate factor: x.xxxxx  [YELLOW SLIDER]
Line 45: random factor: on/off[r]
Line 65: particle size: xx.x        [GREEN SLIDER]
Line 85: cam dist:xx.x angle:(x.xx,x.xx)
Line 105: camera tracking: on/off[C]
Line 125: debug cube: on/off[d] (if enabled)
Line 145: [h]help [+/-]zoom [arrows/mouse]rotate [C]track
```

**Slider Design:**
- **Simulation Speed Slider**: Yellow (0xfdfd00), range 0-2
- **Particle Size Slider**: Green (0x00ff00), range 5-40
- Both sliders have white outlines and are draggable

## Controls Summary

### Keyboard Controls
- **0-3**: Change display mode (wireframe/solid/mixed)
- **C**: Toggle camera center tracking (with smooth transition)
- **r/R**: Toggle random simulation factor
- **h/H**: Toggle help overlay
- **+/-**: Zoom camera in/out (disables tracking)
- **Arrow keys**: Rotate camera (disables tracking)
- **d/D**: Toggle debug cube (if enabled)

### Mouse Controls
- **Mouse drag**: Rotate camera around scene (like NVIDIA sample)
  - Drag anywhere outside sliders to rotate
  - Automatically disables tracking
- **Click/drag on yellow slider**: Adjust simulation speed (0-2x)
- **Click/drag on green slider**: Adjust particle size (5-40)

## Technical Details

### Camera Transition Algorithm
```c
if (camera_tracking) {
    camAngleX += (target_cam_angle_x - camAngleX) * cam_transition_speed;
    camAngleY += (target_cam_angle_y - camAngleY) * cam_transition_speed;
    camDist += (target_cam_dist - camDist) * cam_transition_speed;
}
```
- Uses exponential smoothing with 10% transition speed (moves 10% of remaining distance each frame)
- Smooth, natural-looking camera movement
- Converges to target position over ~20-30 frames

### Particle Centering
When camera tracking is enabled:
- Particles are positioned relative to their center of mass
- Creates a consistent view regardless of where particles have drifted
- Helps visualize the gravitational system as a unified whole

When tracking is disabled:
- Particles are positioned relative to fixed origin (simulation space center)
- Allows particles to drift across the screen naturally

### Mouse Sensitivity
- Rotation: 0.005 radians per pixel
- Provides fine control while still allowing quick camera movements
- Vertical angle clamped to ±1.5 radians (±86 degrees) to prevent gimbal lock

## Compatibility with NVIDIA Sample

These changes bring nbody3d.c closer to the NVIDIA CUDA nbody sample:
- ✅ Mouse-based camera rotation
- ✅ Smooth camera transitions
- ✅ UI-based parameter control
- ✅ Particle size adjustment
- ✅ Similar visual appearance (cyan/turquoise/white particles with additive blending)
- ✅ **Same gravitational physics algorithm with softening parameter**

### Physics Algorithm (NVIDIA nbody sample)

The gravitational force calculation now matches the NVIDIA CUDA nbody sample:

**Formula:**
```
F = G * m_i * m_j * r / (r^2 + epsilon^2)^(3/2)
```

Where:
- `G` = Gravitational constant (`Gravity_Coef = 3.3f`)
- `m_i`, `m_j` = Masses of particles i and j
- `r` = Position difference vector
- `epsilon` = Softening parameter (`SOFTENING = 0.01f`)

**Key Implementation Details:**
```c
float distSqr = dx*dx + dy*dy + dz*dz + SOFTENING_SQUARED;
float invDist = 1.0f / sqrtf(distSqr);
float invDistCube = invDist * invDist * invDist;
float s = Gravity_Coef * Mass[j] * invDistCube;
```

**Benefits of Softening:**
1. **Prevents singularities**: When two particles occupy nearly the same position, the force remains finite
2. **Numerical stability**: Prevents extreme accelerations that cause integration errors
3. **Energy conservation**: Maintains better energy conservation over long simulations
4. **Realistic close encounters**: Models gravitational interactions more realistically when particles are very close

This is the standard approach used in astrophysical N-body simulations and matches the NVIDIA CUDA sample implementation.

## Build Instructions

```bash
cd /home/runner/work/sdlmm/sdlmm
make nbody3d
```

Or manually:
```bash
gcc -O2 -I/usr/include/SDL -I/usr/include/freetype2 -I../ -msse2 \
    exams/nbody3d.c ./sdlmm.c -lSDL -lm -lpthread -lSDL_ttf -lSDL_image \
    -lfreetype -fopenmp -o nbody3d
```

## Future Enhancements

Potential future improvements to further match NVIDIA sample:
- [ ] GPU acceleration using OpenCL/CUDA
- [ ] Additional camera modes (e.g., follow particles, orbit around center)
- [ ] Save/load camera positions
- [ ] Benchmark mode with performance metrics
- [ ] Different particle configurations (galaxy, shell, etc.)
