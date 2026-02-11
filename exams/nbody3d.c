/**
 * nbody3d.c - 3D N-Body Gravitational Simulation using Babylon3D
 *
 * Renders gravitational bodies as 3D spheres in a true 3D scene
 * with camera controls and real-time physics simulation.
 * Based on NVIDIA CUDA nbody sample.
 *
 * Controls:
 *   0-3 : Change display mode (wireframe/solid/mixed)
 *   C   : Toggle camera center tracking (with smooth transition)
 *   r/R : Toggle random simulation factor
 *   h/H : Toggle help overlay
 *   +/- : Zoom camera in/out
 *   Arrow keys: Rotate camera
 *   Mouse drag: Rotate camera (like NVIDIA sample)
 *   Mouse sliders: Adjust simulation speed and particle size
 *
 * Compile:
 *   gcc -O2 -I/usr/include/SDL -I/usr/include/freetype2 -I../ -msse2 \
 *       nbody3d.c ../sdlmm.c -lSDL -lm -lpthread -lSDL_ttf -lSDL_image \
 *       -lfreetype -fopenmp -o nbody3d
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <omp.h>
#include "../sdlmm.h"
#include "../babylon3D.c"

#define SCREENX 800
#define SCREENY 600
#define NUM_BODY 500
#define LOOP 9999
#define MAX_X_axis 500
#define MIN_X_axis 0
#define MAX_Y_axis 500
#define MIN_Y_axis 0
#define MAX_Z_axis 500
#define MIN_Z_axis 0
#define MAX_Velocity 200
#define MIN_velocity -200
#define MAX_Mass 150
#define MIN_Mass 3
#define Gravity_Coef 3.3f

/* UI slider constants */
#define SLIDER_X_START 320
#define SLIDER_WIDTH 400
#define SLIDER_SIM_Y 25
#define SLIDER_SIM_HEIGHT 15
#define SLIDER_PARTICLE_Y 65
#define SLIDER_PARTICLE_HEIGHT 15

/* Particle size range */
#define PARTICLE_SIZE_MIN 5.0f
#define PARTICLE_SIZE_MAX 40.0f
#define PARTICLE_SIZE_DEFAULT 15.0f

/* Mouse control constants */
#define MOUSE_ROTATION_SENSITIVITY 0.005f
#define CAM_ANGLE_X_MAX 1.5f
#define CAM_ANGLE_X_MIN -1.5f

/* Global state */
static int showhelp = 1;
static int showmode = 3;
static float simulatetime_factor = 0.01f;
static int random_simulatefactor = 1;
static int SZ = NUM_BODY;
/* Camera */
static float camDist = 50.0f;
static float camAngleX = 0.3f;
static float camAngleY = 0.0f;
/* Camera tracking */
static int camera_tracking = 0;  /* Toggle camera center tracking */
static float target_cam_angle_x = 0.3f;  /* Target angles for smooth transition */
static float target_cam_angle_y = 0.0f;
static float target_cam_dist = 50.0f;
static const float cam_transition_speed = 0.1f;  /* Speed of camera transitions */
/* Mouse control */
static int mouse_down = 0;
static int last_mouse_x = 0;
static int last_mouse_y = 0;
/* Particle size control */
static float particle_size = PARTICLE_SIZE_DEFAULT;  /* Default particle size */

/* Physics arrays */
static float *X_axis, *Y_axis, *Z_axis;
static float *X_Velocity, *Y_Velocity, *Z_Velocity;
static float *newX_velocity, *newY_velocity, *newZ_velocity;
static float *Mass;

/* 3D rendering */
static Device* m_device = NULL;
static Camera camera;
static Texture* particleTexture = NULL;  /* Gaussian texture for particles */
static Vector3* particlePositions = NULL;  /* World positions for particles */
static int* particleColors = NULL;  /* Colors for each particle */
static Mesh* debugCube = NULL;  /* Debug cube to verify rendering */
static const int hasDebugCube=0;
static int showDebugCube = 1;  /* Toggle for debug cube visibility */

/* Glow colors for particles - cyan/turquoise/white like NVIDIA nbody demo */
static const int glowColors[] = {
    0x00FFFF,  /* cyan */
    0x40FFFF,  /* light cyan */
    0x80FFFF,  /* lighter cyan */
    0xC0FFFF,  /* very light cyan */
    0xFFFFFF,  /* white */
    0x00E0E0,  /* darker cyan */
    0x00C0C0,  /* dark cyan */
    0x60FFFF,  /* cyan variant */
    0xA0FFFF,  /* cyan variant 2 */
    0xE0FFFF   /* almost white */
};
#define NUM_GLOW_COLORS 10

#ifdef __linux__
#include <sys/time.h>
#else
#include <time.h>
#endif

static double getDoubleTime() {
#ifdef __linux__
    struct timeval tm_tv;
    gettimeofday(&tm_tv, 0);
    return (double)tm_tv.tv_sec + (1e-6) * tm_tv.tv_usec;
#else
    return ((double)clock()) / CLOCKS_PER_SEC;
#endif
}

static float clampf(float v, float minv, float maxv) {
    if (v > maxv) v = (v + maxv) / 2;
    if (v < minv) v = (v + minv) / 2;
    return v;
}

static float* allocateBody() {
    float* ret = (float*)malloc(sizeof(float) * SZ);
    memset(ret, 0, sizeof(float) * SZ);
    return ret;
}

static void freeBody(void* p) { free(p); }

/* Initialize body positions and velocities */
static void Init_AllBody() {
    int i;
    for (i = 0; i < SZ; i++) {
        X_axis[i] = rand() % (MAX_X_axis - MIN_X_axis) + MIN_X_axis;
        Y_axis[i] = rand() % (MAX_Y_axis - MIN_Y_axis) + MIN_Y_axis;
        Z_axis[i] = rand() % (MAX_Z_axis - MIN_Z_axis) + MIN_Z_axis;
        X_Velocity[i] = newX_velocity[i] = 0;
        Y_Velocity[i] = newY_velocity[i] = 0;
        Z_Velocity[i] = newZ_velocity[i] = 0;
        Mass[i] = rand() % (MAX_Mass - MIN_Mass) + MIN_Mass;
    }
}

/* N-body gravitational calculation for body i */
static void Nbody(int i, int sz) {
    int j;
    float sumX = 0, sumY = 0, sumZ = 0;
    for (j = 0; j < sz; j++) {
        float X_position, Y_position, Z_position;
        float Distance, Force;
        if (j == i) continue;
        X_position = X_axis[j] - X_axis[i];
        Y_position = Y_axis[j] - Y_axis[i];
        Z_position = Z_axis[j] - Z_axis[i];
        Distance = sqrtf(X_position * X_position + Y_position * Y_position + Z_position * Z_position);
        if (Distance == 0) continue;
        Force = Gravity_Coef * Mass[i] / (Distance * Distance);
        sumX += Force * X_position;
        sumY += Force * Y_position;
        sumZ += Force * Z_position;
    }
    newX_velocity[i] += sumX * simulatetime_factor;
    newY_velocity[i] += sumY * simulatetime_factor;
    newZ_velocity[i] += sumZ * simulatetime_factor;
    X_axis[i] += clampf(newX_velocity[i], MIN_velocity, MAX_Velocity) * simulatetime_factor;
    Y_axis[i] += clampf(newY_velocity[i], MIN_velocity, MAX_Velocity) * simulatetime_factor;
    Z_axis[i] += clampf(newZ_velocity[i], MIN_velocity, MAX_Velocity) * simulatetime_factor;
    X_Velocity[i] = newX_velocity[i];
    Y_Velocity[i] = newY_velocity[i];
    Z_Velocity[i] = newZ_velocity[i];
}

/**
 * Create a simple debug cube for visual confirmation
 */
static void initDebugCube() {
    if (!hasDebugCube) return;
    debugCube = softengine_mesh("DebugCube", 8, 12);
    if (!debugCube) return;
    
    // Simple cube vertices
    debugCube->Vertices[0].Coordinates = vector3(-2, -2, -2);
    debugCube->Vertices[1].Coordinates = vector3(2, -2, -2);
    debugCube->Vertices[2].Coordinates = vector3(2, 2, -2);
    debugCube->Vertices[3].Coordinates = vector3(-2, 2, -2);
    debugCube->Vertices[4].Coordinates = vector3(-2, -2, 2);
    debugCube->Vertices[5].Coordinates = vector3(2, -2, 2);
    debugCube->Vertices[6].Coordinates = vector3(2, 2, 2);
    debugCube->Vertices[7].Coordinates = vector3(-2, 2, 2);
    
    // Set normals (simple outward normals)
    for (int i = 0; i < 8; i++) {
        Vector3 n = debugCube->Vertices[i].Coordinates;
        vector3_normalize(&n);
        debugCube->Vertices[i].Normal = n;
        debugCube->Vertices[i].WorldCoordinates = vector3_zero();
        debugCube->Vertices[i].TextureCoordinates = vector3_zero();
    }
    
    // 12 faces (2 per side)
    // Front face
    debugCube->faces[0].A = 0; debugCube->faces[0].B = 1; debugCube->faces[0].C = 2;
    debugCube->faces[1].A = 0; debugCube->faces[1].B = 2; debugCube->faces[1].C = 3;
    // Back face
    debugCube->faces[2].A = 5; debugCube->faces[2].B = 4; debugCube->faces[2].C = 7;
    debugCube->faces[3].A = 5; debugCube->faces[3].B = 7; debugCube->faces[3].C = 6;
    // Top face
    debugCube->faces[4].A = 3; debugCube->faces[4].B = 2; debugCube->faces[4].C = 6;
    debugCube->faces[5].A = 3; debugCube->faces[5].B = 6; debugCube->faces[5].C = 7;
    // Bottom face
    debugCube->faces[6].A = 4; debugCube->faces[6].B = 5; debugCube->faces[6].C = 1;
    debugCube->faces[7].A = 4; debugCube->faces[7].B = 1; debugCube->faces[7].C = 0;
    // Left face
    debugCube->faces[8].A = 4; debugCube->faces[8].B = 0; debugCube->faces[8].C = 3;
    debugCube->faces[9].A = 4; debugCube->faces[9].B = 3; debugCube->faces[9].C = 7;
    // Right face
    debugCube->faces[10].A = 1; debugCube->faces[10].B = 5; debugCube->faces[10].C = 6;
    debugCube->faces[11].A = 1; debugCube->faces[11].B = 6; debugCube->faces[11].C = 2;
    
    debugCube->Position = vector3(0, 0, 0);  // At origin
    debugCube->Rotation = vector3_zero();
    debugCube->texture.internalBuffer = NULL;
    debugCube->texture.width = 0;
    debugCube->texture.height = 0;
}

static void freeDebugCube() {
    if (debugCube) {
        mesh_free(debugCube);
        debugCube = NULL;
    }
}

/**
 * Initialize particle rendering structures.
 */
static void initParticles() {
    int i;
    particlePositions = (Vector3*)malloc(sizeof(Vector3) * SZ);
    particleColors = (int*)malloc(sizeof(int) * SZ);
    
    /* Create Gaussian texture for particle sprites */
    particleTexture = texture_create_gaussian(64);
    
    /* Assign colors to particles based on their index */
    for (i = 0; i < SZ; i++) {
        particleColors[i] = glowColors[i % NUM_GLOW_COLORS];
    }
}

static void freeParticles() {
    if (particlePositions) {
        free(particlePositions);
        particlePositions = NULL;
    }
    if (particleColors) {
        free(particleColors);
        particleColors = NULL;
    }
    if (particleTexture) {
        if (particleTexture->internalBuffer) {
            free(particleTexture->internalBuffer);
        }
        free(particleTexture);
        particleTexture = NULL;
    }
}

/**
 * Update particle positions from physics simulation.
 * Maps simulation coordinates to 3D world space.
 * When camera tracking is enabled, centers particles around their center of mass.
 */
static void updateParticlePositions(float avgX, float avgY, float avgZ) {
    int i;
    float scale = 0.1f;  /* Scale factor for world coordinates */
    
    /* When camera tracking is enabled, centralize particles around center of mass */
    int should_centralize = camera_tracking;
    
    for (i = 0; i < SZ; i++) {
        if (should_centralize) {
            particlePositions[i] = vector3(
                (X_axis[i] - avgX) * scale,
                (Y_axis[i] - avgY) * scale,
                (Z_axis[i] - avgZ) * scale
            );
        } else {
            particlePositions[i] = vector3(
                (X_axis[i] - MAX_X_axis/2) * scale,
                (Y_axis[i] - MAX_Y_axis/2) * scale,
                (Z_axis[i] - MAX_Z_axis/2) * scale
            );
        }
    }
}

/**
 * Update camera position based on angles and distance.
 * Smoothly transitions camera angles and distance when tracking is enabled.
 */
static void updateCamera() {
    /* Smooth camera transition */
    if (camera_tracking) {
        /* Gradually move towards target angles */
        camAngleX += (target_cam_angle_x - camAngleX) * cam_transition_speed;
        camAngleY += (target_cam_angle_y - camAngleY) * cam_transition_speed;
        camDist += (target_cam_dist - camDist) * cam_transition_speed;
    } else {
        /* Update targets to match current position when not tracking */
        target_cam_angle_x = camAngleX;
        target_cam_angle_y = camAngleY;
        target_cam_dist = camDist;
    }
    
    camera.Position = vector3(
        camDist * sinf(camAngleY) * cosf(camAngleX),
        camDist * sinf(camAngleX),
        -camDist * cosf(camAngleY) * cosf(camAngleX)
    );
    camera.Target = vector3(0, 0, 0);  /* Look at center */
}

/**
 * Draw the 3D scene: clear, render particles, draw HUD text overlay.
 */
static void draw3D(int loop, int totalLoop, double tm, float avgX, float avgY, float avgZ) {
    char buf[256];
    double rendert1, rendert2;

    rendert1 = getDoubleTime();

    /* Update particle positions from simulation */
    updateParticlePositions(avgX, avgY, avgZ);

    /* Update camera */
    updateCamera();

    /* Clear device to black */
    device_clear(m_device);

    /* Render debug cube if enabled (for visual confirmation) */
    if (hasDebugCube && showDebugCube && debugCube) {
        Vector3 lightPos = vector3(10, 10, -10);
        device_render(m_device, &camera, debugCube, 1, &lightPos);
    }

    /* Render all particles with additive blending */
    device_render_particles(m_device, &camera, particlePositions, particleColors, 
                           SZ, particle_size, particleTexture, 1);  /* 1 = additive blending */

    /* Draw HUD overlay */
    if (showhelp) {
        sprintf(buf, "[%-3d/%-3d] tm:%-3.3f bodies:%d", loop, totalLoop, tm, SZ);
        drawtext(buf, 5, 5, 0xffffff);
        sprintf(buf, "simulate factor: %-3.5f", simulatetime_factor);
        drawtext(buf, 5, 25, 0xffffff);
        sprintf(buf, "random factor: %s[r]", random_simulatefactor ? "on" : "off");
        drawtext(buf, 5, 45, 0xffffff);
        /* Draw simulation factor slider */
        fillrect(SLIDER_X_START, SLIDER_SIM_Y, SLIDER_WIDTH * (simulatetime_factor / 2.0f), SLIDER_SIM_HEIGHT, 0xfdfd00);
        drawrect(SLIDER_X_START, SLIDER_SIM_Y, SLIDER_WIDTH, SLIDER_SIM_HEIGHT, 0xffffff);
        
        sprintf(buf, "particle size: %-3.1f", particle_size);
        drawtext(buf, 5, 65, 0xffffff);
        /* Draw particle size slider */
        fillrect(SLIDER_X_START, SLIDER_PARTICLE_Y, SLIDER_WIDTH * ((particle_size - PARTICLE_SIZE_MIN) / (PARTICLE_SIZE_MAX - PARTICLE_SIZE_MIN)), SLIDER_PARTICLE_HEIGHT, 0x00ff00);
        drawrect(SLIDER_X_START, SLIDER_PARTICLE_Y, SLIDER_WIDTH, SLIDER_PARTICLE_HEIGHT, 0xffffff);
        
        sprintf(buf, "cam dist:%.1f angle:(%.2f,%.2f)", camDist, camAngleX, camAngleY);
        drawtext(buf, 5, 85, 0xffffff);
        sprintf(buf, "camera tracking: %s[C]", camera_tracking ? "on" : "off");
        drawtext(buf, 5, 105, 0xffffff);
	if(hasDebugCube){
           sprintf(buf, "debug cube: %s[d]", showDebugCube ? "on" : "off");
           drawtext(buf, 5, 125, 0xffffff);
        }
        drawtext("[h]help [+/-]zoom [arrows/mouse]rotate [C]track", 5, 145, 0xaaaaaa);
    }

    flushscreen();

    rendert2 = getDoubleTime();
    tm += (rendert2 - rendert1);
    if (tm < 1.0 / 60) {
        delay((int)((1.0 / 60) * 1000 - tm * 1000));
    }
}

/* Main simulation loop */
static int main_run(int argc, char** argv) {
    int loop;
    double tmstart, tmend;
    double fps_time_1, fps_time_2;
    float avgX = 0, avgY = 0, avgZ = 0;
    tmstart = getDoubleTime();
    Init_AllBody();
    for (loop = 0; loop < LOOP; loop++) {
        int i;
        avgX = 0; avgY = 0; avgZ = 0;
        fps_time_1 = getDoubleTime();
#pragma omp parallel for
        for (i = 0; i < SZ; i++) {
            Nbody(i, SZ);
        }
        for (i = 0; i < SZ; i++) {
            avgX += X_axis[i];
            avgY += Y_axis[i];
            avgZ += Z_axis[i];
        }
        avgX /= SZ;
        avgY /= SZ;
        avgZ /= SZ;
        fps_time_2 = getDoubleTime();
        draw3D(loop, LOOP, fps_time_2 - fps_time_1, avgX, avgY, avgZ);
    }
    tmend = getDoubleTime();
    printf("%d %lf\n", SZ, tmend - tmstart);
    if (random_simulatefactor) {
        simulatetime_factor = (((float)rand()) / RAND_MAX);
    }
    return 0;
}

/* Keyboard handler */
static void kbfnc(int k, int ctrl, int on) {
    if (on) {
        switch (k) {
            case '0': case '1': case '2': case '3':
                showmode = k - '0'; break;
            case 'c': case 'C': 
                /* Toggle camera tracking mode */
                camera_tracking = !camera_tracking;
                if (camera_tracking) {
                    /* Set target to center view (looking down slightly) */
                    target_cam_angle_x = 0.3f;
                    target_cam_angle_y = 0.0f;
                    target_cam_dist = 50.0f;
                }
                break;
            case 'r': case 'R': random_simulatefactor = !random_simulatefactor; break;
            case 'h': case 'H': showhelp = !showhelp; break;
            case 'd': case 'D': showDebugCube = !showDebugCube; break;  /* Toggle debug cube */
            case '+': case '=': 
                if (camDist > 5.0f) camDist -= 3.0f; 
                camera_tracking = 0;  /* Disable tracking on manual control */
                break;
            case '-': case '_': 
                camDist += 3.0f; 
                camera_tracking = 0;  /* Disable tracking on manual control */
                break;
        }
        /* Arrow keys - SDLK values */
        if (k == 273) {
            camAngleX += 0.1f;       /* Up */
            camera_tracking = 0;  /* Disable tracking on manual control */
        }
        else if (k == 274) {
            camAngleX -= 0.1f;  /* Down */
            camera_tracking = 0;  /* Disable tracking on manual control */
        }
        else if (k == 276) {
            camAngleY -= 0.1f;  /* Left */
            camera_tracking = 0;  /* Disable tracking on manual control */
        }
        else if (k == 275) {
            camAngleY += 0.1f;  /* Right */
            camera_tracking = 0;  /* Disable tracking on manual control */
        }
    }
}

/* Mouse handler for sliders and camera rotation */
static void mousefnc(int x, int y, int on, int btn) {
    if (on) {
        /* Check for simulation factor slider */
        if (y > SLIDER_SIM_Y && y < SLIDER_SIM_Y + SLIDER_SIM_HEIGHT && x >= SLIDER_X_START && x <= SLIDER_X_START + SLIDER_WIDTH) {
            float value = 2.0f * ((float)(x - SLIDER_X_START)) / SLIDER_WIDTH;
            if (value >= 0.0f && value <= 2.0f) {
                simulatetime_factor = value;
            }
        }
        /* Check for particle size slider */
        else if (y > SLIDER_PARTICLE_Y && y < SLIDER_PARTICLE_Y + SLIDER_PARTICLE_HEIGHT && x >= SLIDER_X_START && x <= SLIDER_X_START + SLIDER_WIDTH) {
            float value = PARTICLE_SIZE_MIN + (PARTICLE_SIZE_MAX - PARTICLE_SIZE_MIN) * ((float)(x - SLIDER_X_START)) / SLIDER_WIDTH;
            if (value >= PARTICLE_SIZE_MIN && value <= PARTICLE_SIZE_MAX) {
                particle_size = value;
            }
        }
        /* Camera rotation - mouse drag outside slider areas */
        else {
            if (!mouse_down) {
                mouse_down = 1;
                last_mouse_x = x;
                last_mouse_y = y;
            }
        }
    } else {
        mouse_down = 0;
    }
}

static void mousemotion(int x, int y, int on) {
    /* Handle mouse dragging for camera rotation */
    if (mouse_down && on) {
        int dx = x - last_mouse_x;
        int dy = y - last_mouse_y;
        
        /* Only rotate if not clicking on sliders */
        int slider_max_y = (SLIDER_SIM_Y > SLIDER_PARTICLE_Y ? SLIDER_SIM_Y : SLIDER_PARTICLE_Y) + SLIDER_PARTICLE_HEIGHT;
        if (!(y > SLIDER_SIM_Y && y < slider_max_y && x >= SLIDER_X_START && x <= SLIDER_X_START + SLIDER_WIDTH)) {
            /* Rotate camera based on mouse movement */
            camAngleY += (float)dx * MOUSE_ROTATION_SENSITIVITY;  /* Horizontal rotation */
            camAngleX -= (float)dy * MOUSE_ROTATION_SENSITIVITY;  /* Vertical rotation (inverted) */
            
            /* Clamp vertical angle to prevent flipping */
            if (camAngleX > CAM_ANGLE_X_MAX) camAngleX = CAM_ANGLE_X_MAX;
            if (camAngleX < CAM_ANGLE_X_MIN) camAngleX = CAM_ANGLE_X_MIN;
            
            camera_tracking = 0;  /* Disable tracking on manual control */
        }
        
        last_mouse_x = x;
        last_mouse_y = y;
    }
    /* Also handle slider dragging */
    mousefnc(x, y, on, 0);
}

int main(int argc, char** argv) {
    int i;
    if (argc > 1) {
        SZ = atoi(argv[1]);
    }
    X_axis = allocateBody();
    Y_axis = allocateBody();
    Z_axis = allocateBody();
    X_Velocity = allocateBody();
    Y_Velocity = allocateBody();
    Z_Velocity = allocateBody();
    Mass = allocateBody();
    newX_velocity = allocateBody();
    newY_velocity = allocateBody();
    newZ_velocity = allocateBody();

    screen(SCREENX, SCREENY);
    screentitle("[3D] NBody-Simulation (Babylon3D)");
    settextfont("FreeMono.ttf", 16);
    setonkey(kbfnc);
    setonmouse(mousefnc);
    setonmotion(mousemotion);

    /* Initialize 3D device */
    m_device = device(SCREENX, SCREENY);
    if (!m_device) {
        fprintf(stderr, "Failed to create 3D device\n");
        return 1;
    }

    /* Initialize camera */
    camera.Position = vector3(0, 0, -camDist);
    camera.Target = vector3(0, 0, 0);

    /* Initialize bodies first so we know masses */
    Init_AllBody();

    /* Create particle rendering structures */
    initParticles();
    
    /* Create debug cube for visual confirmation */
    initDebugCube();

    for (i = 0; i < 20; ++i) {
        main_run(argc, argv);
    }

    freeParticles();
    freeDebugCube();
    device_free(m_device);
    freeBody(X_axis);
    freeBody(Y_axis);
    freeBody(Z_axis);
    freeBody(X_Velocity);
    freeBody(Y_Velocity);
    freeBody(Z_Velocity);
    freeBody(Mass);
    freeBody(newX_velocity);
    freeBody(newY_velocity);
    freeBody(newZ_velocity);
    return 0;
}
