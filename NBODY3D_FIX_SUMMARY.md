# NBody3D Rendering Fix Summary

## Problem
The nbody3d simulation was running but showing a completely black screen with no visible particles, despite reporting 5000 bodies in the status overlay.

## Root Cause Analysis
The issue was in the `device_render_particles` function in `babylon3D.c`. Several problems were identified:

1. **Incorrect perspective divide handling**: The code was attempting to manually divide by w, but `vector3_transform_coordinates` already performs perspective division automatically.

2. **Too strict culling**: The original code had tight screen bounds checking that could reject particles with large sprites.

3. **Improper sprite size scaling**: The distance-to-size calculation was using `1.0f / viewPos.z * 100.0f` which could result in extremely large or small values.

## Changes Made

### babylon3D.c - device_render_particles function

1. **Fixed view space check**:
   - Changed from `if (viewPos.z < 0.1f)` to `if (viewPos.z <= 0.1f)`
   - Added comment explaining LH coordinate system behavior

2. **Improved clip space culling**:
   - Changed from tight margin-based screen bounds to NDC space check: `clipPos.x/y < -2.0 or > 2.0`
   - This allows large sprites near screen edges to render properly

3. **Fixed sprite size calculation**:
   - Changed from `spriteSize * (1.0f / viewPos.z) * 100.0f` 
   - To: `spriteSize * (50.0f / viewPos.z)`
   - This gives more predictable sizing: base size at distance 50 units

4. **Adjusted size clamping**:
   - Minimum: 3.0f (was 2.0f) - ensures particles are visible
   - Maximum: 80.0f (was 100.0f) - better performance

## Expected Behavior After Fix

With the camera at distance 50 and particles scaled to 0.1 * [0-500]:
- Particles should be positioned in a cube from (-25, -25, -25) to (25, 25, 25)
- Camera looking at origin from (~0, 15, -47) with angles (0.3, 0.0)
- Particles should appear as cyan/white glowing sprites with additive blending
- Sprite sizes should scale naturally with distance from camera

## Testing Required

To verify the fix works:

1. Build nbody3d: `make nbody3d` or use provided build command
2. Run the simulation with default 500 bodies
3. Verify particles are visible as cyan glowing points
4. Test camera controls (arrows, +/-, etc.)
5. Compare with CUDA sample visual appearance

## Related Files

- `/home/runner/work/sdlmm/sdlmm/babylon3D.c` - Rendering engine
- `/home/runner/work/sdlmm/sdlmm/exams/nbody3d.c` - Simulation code
- `/home/runner/work/sdlmm/sdlmm/babylon3D.h` - API declarations

## Additional Notes

The coordinate system used is **Left-Handed (LH)**:
- X: right
- Y: up  
- Z: forward (positive Z is away from camera)

When `matrix_LookAtLH` creates the view matrix:
- Z-axis points from eye to target
- Particles in front of camera have positive viewPos.z
- The near plane check `viewPos.z <= 0.1f` filters out anything behind or very close to camera
