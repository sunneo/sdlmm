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

/* Debug cube (same as babylon3D_cube) to verify rendering pipeline */
static Mesh* debugCubeMesh = NULL;
static float debugCubeRotX = 0, debugCubeRotY = 0;

static void createDebugCube() {
    int i;
    debugCubeMesh = softengine_mesh("DebugCube", 24, 12);
    if(!debugCubeMesh) return;
    /* Front face (Z = 1) */
    debugCubeMesh->Vertices[0].Coordinates = vector3(-1, 1, 1);
    debugCubeMesh->Vertices[1].Coordinates = vector3(1, 1, 1);
    debugCubeMesh->Vertices[2].Coordinates = vector3(-1, -1, 1);
    debugCubeMesh->Vertices[3].Coordinates = vector3(1, -1, 1);
    /* Back face (Z = -1) */
    debugCubeMesh->Vertices[4].Coordinates = vector3(1, 1, -1);
    debugCubeMesh->Vertices[5].Coordinates = vector3(-1, 1, -1);
    debugCubeMesh->Vertices[6].Coordinates = vector3(1, -1, -1);
    debugCubeMesh->Vertices[7].Coordinates = vector3(-1, -1, -1);
    /* Top face (Y = 1) */
    debugCubeMesh->Vertices[8].Coordinates = vector3(-1, 1, -1);
    debugCubeMesh->Vertices[9].Coordinates = vector3(1, 1, -1);
    debugCubeMesh->Vertices[10].Coordinates = vector3(-1, 1, 1);
    debugCubeMesh->Vertices[11].Coordinates = vector3(1, 1, 1);
    /* Bottom face (Y = -1) */
    debugCubeMesh->Vertices[12].Coordinates = vector3(-1, -1, 1);
    debugCubeMesh->Vertices[13].Coordinates = vector3(1, -1, 1);
    debugCubeMesh->Vertices[14].Coordinates = vector3(-1, -1, -1);
    debugCubeMesh->Vertices[15].Coordinates = vector3(1, -1, -1);
    /* Left face (X = -1) */
    debugCubeMesh->Vertices[16].Coordinates = vector3(-1, 1, -1);
    debugCubeMesh->Vertices[17].Coordinates = vector3(-1, 1, 1);
    debugCubeMesh->Vertices[18].Coordinates = vector3(-1, -1, -1);
    debugCubeMesh->Vertices[19].Coordinates = vector3(-1, -1, 1);
    /* Right face (X = 1) */
    debugCubeMesh->Vertices[20].Coordinates = vector3(1, 1, 1);
    debugCubeMesh->Vertices[21].Coordinates = vector3(1, 1, -1);
    debugCubeMesh->Vertices[22].Coordinates = vector3(1, -1, 1);
    debugCubeMesh->Vertices[23].Coordinates = vector3(1, -1, -1);
    /* Normals */
    for(i = 0; i < 4; i++) debugCubeMesh->Vertices[i].Normal = vector3(0, 0, 1);
    for(i = 4; i < 8; i++) debugCubeMesh->Vertices[i].Normal = vector3(0, 0, -1);
    for(i = 8; i < 12; i++) debugCubeMesh->Vertices[i].Normal = vector3(0, 1, 0);
    for(i = 12; i < 16; i++) debugCubeMesh->Vertices[i].Normal = vector3(0, -1, 0);
    for(i = 16; i < 20; i++) debugCubeMesh->Vertices[i].Normal = vector3(-1, 0, 0);
    for(i = 20; i < 24; i++) debugCubeMesh->Vertices[i].Normal = vector3(1, 0, 0);
    for(i = 0; i < 24; i++) debugCubeMesh->Vertices[i].WorldCoordinates = vector3_zero();
    for(i = 0; i < 6; i++) {
        int base = i * 4;
        debugCubeMesh->Vertices[base + 0].TextureCoordinates = vector3(0, 0, 0);
        debugCubeMesh->Vertices[base + 1].TextureCoordinates = vector3(1, 0, 0);
        debugCubeMesh->Vertices[base + 2].TextureCoordinates = vector3(0, 1, 0);
        debugCubeMesh->Vertices[base + 3].TextureCoordinates = vector3(1, 1, 0);
    }
    /* Faces */
    debugCubeMesh->faces[0].A=0;debugCubeMesh->faces[0].B=1;debugCubeMesh->faces[0].C=2;
    debugCubeMesh->faces[1].A=1;debugCubeMesh->faces[1].B=3;debugCubeMesh->faces[1].C=2;
    debugCubeMesh->faces[2].A=4;debugCubeMesh->faces[2].B=5;debugCubeMesh->faces[2].C=6;
    debugCubeMesh->faces[3].A=5;debugCubeMesh->faces[3].B=7;debugCubeMesh->faces[3].C=6;
    debugCubeMesh->faces[4].A=8;debugCubeMesh->faces[4].B=9;debugCubeMesh->faces[4].C=10;
    debugCubeMesh->faces[5].A=9;debugCubeMesh->faces[5].B=11;debugCubeMesh->faces[5].C=10;
    debugCubeMesh->faces[6].A=12;debugCubeMesh->faces[6].B=13;debugCubeMesh->faces[6].C=14;
    debugCubeMesh->faces[7].A=13;debugCubeMesh->faces[7].B=15;debugCubeMesh->faces[7].C=14;
    debugCubeMesh->faces[8].A=16;debugCubeMesh->faces[8].B=17;debugCubeMesh->faces[8].C=18;
    debugCubeMesh->faces[9].A=17;debugCubeMesh->faces[9].B=19;debugCubeMesh->faces[9].C=18;
    debugCubeMesh->faces[10].A=20;debugCubeMesh->faces[10].B=21;debugCubeMesh->faces[10].C=22;
    debugCubeMesh->faces[11].A=21;debugCubeMesh->faces[11].B=23;debugCubeMesh->faces[11].C=22;
    /* Position at (0,0,10) - same as babylon3D_cube */
    debugCubeMesh->Position = vector3(0, 0, 10);
    debugCubeMesh->Rotation = vector3_zero();
    /* Load texture - same as babylon3D_cube */
    {
        Texture* loadedTexture = texture_load("texture.png");
        if(loadedTexture && loadedTexture->internalBuffer) {
            debugCubeMesh->texture = *loadedTexture;
            free(loadedTexture);
            fprintf(stderr, "Debug cube: texture.png loaded OK\n");
        } else {
            if(loadedTexture) free(loadedTexture);
            fprintf(stderr, "Debug cube: WARNING texture.png not found\n");
            debugCubeMesh->texture.internalBuffer = NULL;
            debugCubeMesh->texture.width = 0;
            debugCubeMesh->texture.height = 0;
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
static void init_sphere_mesh(Mesh* mesh, float radius) {
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
        init_sphere_mesh(&bodyMeshes[i], r);
    }
}

static void freeMeshes() {
    int i;
    if (bodyMeshes) {
        for (i = 0; i < meshCount; i++) {
            if (bodyMeshes[i].Vertices) free(bodyMeshes[i].Vertices);
            if (bodyMeshes[i].faces) free(bodyMeshes[i].faces);
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

    /* Render debug cube using babylon3D_cube's exact camera setup */
    if (debugCubeMesh) {
        Camera debugCam;
        debugCam.Position = vector3(0, 0, -10);
        debugCam.Target = vector3(0, 0, 0);
        debugCubeRotX += 0.01f;
        debugCubeRotY += 0.01f;
        debugCubeMesh->Rotation = vector3(debugCubeRotX, debugCubeRotY, 0);
        device_render(m_device, &debugCam, debugCubeMesh, 1, NULL);
    }

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

    /* Create debug cube (same as babylon3D_cube) for testing */
    createDebugCube();

    for (i = 0; i < 20; ++i) {
        main_run(argc, argv);
    }

    freeMeshes();
    if (debugCubeMesh) { mesh_free(debugCubeMesh); debugCubeMesh = NULL; }
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
