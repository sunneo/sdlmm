/**
 * nbody3d.c - 3D N-Body Gravitational Simulation using Babylon3D
 *
 * Renders gravitational bodies as 3D spheres in a true 3D scene
 * with camera controls and real-time physics simulation.
 *
 * Controls:
 *   0-3 : Change display mode (wireframe/solid/mixed)
 *   c/C : Toggle centralize view
 *   r/R : Toggle random simulation factor
 *   h/H : Toggle help overlay
 *   +/- : Zoom camera in/out
 *   Arrow keys: Rotate camera
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
#define SPHERE_SEGMENTS 6
#define SPHERE_RINGS 4
#define VERTS_PER_SPHERE ((SPHERE_SEGMENTS + 1) * (SPHERE_RINGS + 1))
#define FACES_PER_SPHERE (SPHERE_SEGMENTS * SPHERE_RINGS * 2)

/* Global state */
static int showhelp = 1;
static int showmode = 3;
static float simulatetime_factor = 0.01f;
static int random_simulatefactor = 1;
static int centralize = 0;
static int SZ = NUM_BODY;
/* Camera */
static float camDist = 30.0f;
static float camAngleX = 0.3f;
static float camAngleY = 0.0f;

/* Physics arrays */
static float *X_axis, *Y_axis, *Z_axis;
static float *X_Velocity, *Y_Velocity, *Z_Velocity;
static float *newX_velocity, *newY_velocity, *newZ_velocity;
static float *Mass;

/* 3D rendering */
static Device* m_device = NULL;
static Camera camera;
static Mesh* bodyMeshes = NULL;  /* array of meshes, one per body */
static int meshCount = 0;

/* Glow colors for body textures */
static const int glowColors[] = {
    0xff4040, 0x40ff40, 0x4040ff, 0xff8020, 0xffff40,
    0xff40ff, 0x40ffff, 0xff6060, 0x60ff60, 0x6060ff
};
#define NUM_GLOW_COLORS 10

static void generate_glow_texture(Texture* tex, int baseColor) {
    int size = 32;
    int* buf = (int*)malloc(sizeof(int) * size * size);
    tex->width = size;
    tex->height = size;
    tex->internalBuffer = buf;

    int br = (baseColor >> 16) & 0xff;
    int bg = (baseColor >> 8) & 0xff;
    int bb = baseColor & 0xff;

    float cx = size / 2.0f, cy = size / 2.0f;
    float maxR = size / 2.0f;
    int x, y;
    for (y = 0; y < size; y++) {
        for (x = 0; x < size; x++) {
            float dx = x - cx, dy = y - cy;
            float dist = sqrtf(dx*dx + dy*dy) / maxR;
            if (dist > 1.0f) dist = 1.0f;
            float intensity;
            int r, g, b;
            if (dist < 0.3f) {
                /* White-hot core */
                float t = dist / 0.3f;
                r = 255 - (int)((255 - br) * t);
                g = 255 - (int)((255 - bg) * t);
                b = 255 - (int)((255 - bb) * t);
            } else {
                /* Color to dark */
                float t = (dist - 0.3f) / 0.7f;
                intensity = 1.0f - t * t;
                r = (int)(br * intensity);
                g = (int)(bg * intensity);
                b = (int)(bb * intensity);
            }
            if (r > 255) r = 255; if (r < 0) r = 0;
            if (g > 255) g = 255; if (g < 0) g = 0;
            if (b > 255) b = 255; if (b < 0) b = 0;
            buf[y * size + x] = (r << 16) | (g << 8) | b;
        }
    }
}

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
 * Create a sphere mesh with given segments/rings.
 * The sphere is unit-sized; we scale it via Position/offset in the render.
 */
static void init_sphere_mesh(Mesh* mesh, float radius, int bodyIndex) {
    int seg, ring, idx, fidx;
    int vCount = VERTS_PER_SPHERE;
    int fCount = FACES_PER_SPHERE;
    mesh->Vertices = (Vertex*)malloc(sizeof(Vertex) * vCount);
    mesh->faces = (Face*)malloc(sizeof(Face) * fCount);
    mesh->verticesCount = vCount;
    mesh->faceCount = fCount;
    mesh->Rotation = vector3_zero();
    mesh->Position = vector3_zero();
    mesh->texture.internalBuffer = NULL;
    mesh->texture.width = 0;
    mesh->texture.height = 0;
    generate_glow_texture(&mesh->texture, glowColors[bodyIndex % NUM_GLOW_COLORS]);
    strcpy(mesh->name, "body");

    /* Generate vertices */
    idx = 0;
    for (ring = 0; ring <= SPHERE_RINGS; ring++) {
        float phi = (float)ring / SPHERE_RINGS * 3.14159265f;
        float y = cosf(phi) * radius;
        float ringRadius = sinf(phi) * radius;
        for (seg = 0; seg <= SPHERE_SEGMENTS; seg++) {
            float theta = (float)seg / SPHERE_SEGMENTS * 2.0f * 3.14159265f;
            float x = cosf(theta) * ringRadius;
            float z = sinf(theta) * ringRadius;
            mesh->Vertices[idx].Coordinates = vector3(x, y, z);
            /* Normal is just the normalized position for a sphere */
            float len = sqrtf(x * x + y * y + z * z);
            if (len > 0) {
                mesh->Vertices[idx].Normal = vector3(x / len, y / len, z / len);
            } else {
                mesh->Vertices[idx].Normal = vector3(0, 1, 0);
            }
            mesh->Vertices[idx].WorldCoordinates = vector3_zero();
            mesh->Vertices[idx].TextureCoordinates = vector3(
                (float)seg / SPHERE_SEGMENTS,
                (float)ring / SPHERE_RINGS, 0);
            idx++;
        }
    }

    /* Generate faces (triangles) */
    fidx = 0;
    for (ring = 0; ring < SPHERE_RINGS; ring++) {
        for (seg = 0; seg < SPHERE_SEGMENTS; seg++) {
            int a = ring * (SPHERE_SEGMENTS + 1) + seg;
            int b = a + SPHERE_SEGMENTS + 1;
            mesh->faces[fidx].A = a;
            mesh->faces[fidx].B = b;
            mesh->faces[fidx].C = a + 1;
            fidx++;
            mesh->faces[fidx].A = b;
            mesh->faces[fidx].B = b + 1;
            mesh->faces[fidx].C = a + 1;
            fidx++;
        }
    }
}

/**
 * Initialize all 3D meshes for the bodies.
 * We use a shared pool of meshes to reduce allocations.
 */
static void initMeshes() {
    int i;
    /* Limit rendered bodies for performance */
    meshCount = SZ;
    if (meshCount > 300) meshCount = 300;
    bodyMeshes = (Mesh*)malloc(sizeof(Mesh) * meshCount);
    for (i = 0; i < meshCount; i++) {
        float r = 0.3f + (Mass[i] / MAX_Mass) * 0.5f;
        init_sphere_mesh(&bodyMeshes[i], r, i);
    }
}

static void freeMeshes() {
    int i;
    if (bodyMeshes) {
        for (i = 0; i < meshCount; i++) {
            if (bodyMeshes[i].Vertices) free(bodyMeshes[i].Vertices);
            if (bodyMeshes[i].faces) free(bodyMeshes[i].faces);
            if (bodyMeshes[i].texture.internalBuffer) free(bodyMeshes[i].texture.internalBuffer);
        }
        free(bodyMeshes);
        bodyMeshes = NULL;
    }
}

/**
 * Update mesh positions from physics simulation.
 * Maps simulation coordinates to 3D world space.
 */
static void updateMeshPositions(float avgX, float avgY, float avgZ) {
    int i;
    float scale = 0.05f;
    for (i = 0; i < meshCount; i++) {
        bodyMeshes[i].Position = vector3(
            (X_axis[i] - avgX) * scale,
            (Y_axis[i] - avgY) * scale,
            (Z_axis[i] - avgZ) * scale + 15.0f  /* offset so bodies are in front of camera */
        );
    }
}

/**
 * Update camera position based on angles and distance.
 */
static void updateCamera() {
    camera.Position = vector3(
        camDist * sinf(camAngleY) * cosf(camAngleX),
        camDist * sinf(camAngleX),
        -camDist * cosf(camAngleY) * cosf(camAngleX)
    );
    camera.Target = vector3(0, 0, 15.0f);
}

/**
 * Draw the 3D scene: clear, render meshes, draw HUD text overlay.
 */
static void draw3D(int loop, int totalLoop, double tm, float avgX, float avgY, float avgZ) {
    char buf[256];
    double rendert1, rendert2;
    Vector3 lightPos;

    rendert1 = getDoubleTime();

    /* Update mesh positions from simulation */
    updateMeshPositions(avgX, avgY, avgZ);

    /* Update camera */
    updateCamera();

    /* Clear device */
    device_clear(m_device);

    /* Light from above-right */
    lightPos = vector3(10, 20, -5);

    /* Render all body meshes */
    device_render(m_device, &camera, bodyMeshes, meshCount, &lightPos);

    /* Draw HUD overlay */
    if (showhelp) {
        sprintf(buf, "[%-3d/%-3d] tm:%-3.3f bodies:%d", loop, totalLoop, tm, SZ);
        drawtext(buf, 5, 5, 0xffffff);
        sprintf(buf, "simulate factor: %-3.5f", simulatetime_factor);
        drawtext(buf, 5, 25, 0xffffff);
        sprintf(buf, "random factor: %s[r]", random_simulatefactor ? "on" : "off");
        drawtext(buf, 5, 45, 0xffffff);
        sprintf(buf, "cam dist:%.1f angle:(%.2f,%.2f)", camDist, camAngleX, camAngleY);
        drawtext(buf, 5, 65, 0xffffff);
        drawtext("[h]help [+/-]zoom [arrows]rotate", 5, 85, 0xaaaaaa);
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
            case 'c': case 'C': centralize = !centralize; break;
            case 'r': case 'R': random_simulatefactor = !random_simulatefactor; break;
            case 'h': case 'H': showhelp = !showhelp; break;
            case '+': case '=': if (camDist > 5.0f) camDist -= 3.0f; break;
            case '-': case '_': camDist += 3.0f; break;
        }
        /* Arrow keys - SDLK values */
        if (k == 273) camAngleX += 0.1f;       /* Up */
        else if (k == 274) camAngleX -= 0.1f;  /* Down */
        else if (k == 276) camAngleY -= 0.1f;  /* Left */
        else if (k == 275) camAngleY += 0.1f;  /* Right */
    }
}

/* Mouse handler for slider */
static void mousefnc(int x, int y, int on, int btn) {
    if (on) {
        if (y > 60 && y < 80 && x >= 320 && x <= 320 + 400) {
            float value = 2.0f * ((float)(x - 320)) / 400;
            simulatetime_factor = value;
        }
    }
}

static void mousemotion(int x, int y, int on) {
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
    camera.Position = vector3(0, 5, -30);
    camera.Target = vector3(0, 0, 15);

    /* Initialize bodies first so we know masses */
    Init_AllBody();

    /* Create sphere meshes for rendering */
    initMeshes();

    for (i = 0; i < 20; ++i) {
        main_run(argc, argv);
    }

    freeMeshes();
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
