/**
 * missilecmd3d.c - 3D Missile Command using Babylon3D
 *
 * A 3D version of the classic Missile Command game rendered with
 * the Babylon3D software rendering engine.
 *
 * Buildings are rendered as 3D cubes, missiles as elongated shapes,
 * and explosions as expanding spheres in 3D space.
 *
 * Controls:
 *   Mouse click : Launch interceptor missile toward cursor position
 *   +/- : Zoom camera
 *   Arrow keys : Rotate camera view
 *   h/H : Toggle help overlay
 *
 * Compile:
 *   gcc -O2 -I/usr/include/SDL -I/usr/include/freetype2 -I../ -msse2 \
 *       missilecmd3d.c ../sdlmm.c -lSDL -lm -lpthread -lSDL_ttf -lSDL_image \
 *       -lfreetype -fopenmp -o missilecmd3d
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "../sdlmm.h"
#include "../babylon3D.c"

#define SCREENX 800
#define SCREENY 600
#define MAX_BUILD 5
#define MAX_ENEMY 15
#define MAX_OUR_MISSILE 12
#define MAX_EXPL_R 2.5f
#define ENEMY_MAX_EXPL_R 3.0f
#define GROUND_Y (-3.0f)
#define WORLD_WIDTH 20.0f
#define WORLD_DEPTH 5.0f

/* Sphere mesh parameters (low-poly for performance) */
#define SPH_SEG 5
#define SPH_RING 3
#define SPH_VERTS ((SPH_SEG + 1) * (SPH_RING + 1))
#define SPH_FACES (SPH_SEG * SPH_RING * 2)

/* --- Data structures --- */
typedef struct {
    Vector3 pos;
    float alive;  /* 1=alive, 0=dead */
    int isbuild;  /* 1=building, 0=launcher */
} Build3D;

typedef struct {
    Vector3 from, to, pos, vel;
    int alive, expl, ishit, targetBuild;
    float r;
} EnemyMissile3D;

typedef struct {
    Vector3 target, pos, vel;
    int active, expl;
    float r;
} OurMissile3D;

/* --- Global state --- */
static int score = 0;
static int remainMissile = 45;
static int remainGenEnemy = 40;
static int remainEnemy = 40;
static int showhelp = 1;
static volatile int mx = SCREENX / 2, my = SCREENY / 2;

/* Camera */
static float camDist = 25.0f;
static float camAngleX = 0.4f;
static float camAngleY = 0.0f;

/* 3D rendering */
static Device* m_device = NULL;
static Camera camera;

/* Game objects */
static Build3D builds[MAX_BUILD];
static EnemyMissile3D enemies[MAX_ENEMY];
static OurMissile3D ourMissiles[MAX_OUR_MISSILE];

/* Shared meshes (templates) */
static Mesh buildingMesh;
static Mesh destroyedMesh;
static Mesh launcherMesh;

/* Pre-allocated mesh arrays for rendering */
static Mesh* renderMeshes = NULL;
static int renderMeshCount = 0;
static int renderMeshCap = 0;

/* Shared singleton meshes for scene objects */
static Mesh groundMesh;
static Mesh explSphere;
static Mesh missileSphere;
static Mesh ourExplSphere;
static Mesh ourMissileSphere;

static float frandf() { return ((float)rand()) / RAND_MAX; }

/* --- Sphere mesh helper (fills pre-allocated mesh) --- */
static void fill_sphere(Mesh* mesh, float radius, int segments, int rings) {
    int seg, ring, idx, fidx;
    int vCount = (segments + 1) * (rings + 1);
    int fCount = segments * rings * 2;
    mesh->Vertices = (Vertex*)malloc(sizeof(Vertex) * vCount);
    mesh->faces = (Face*)malloc(sizeof(Face) * fCount);
    mesh->verticesCount = vCount;
    mesh->faceCount = fCount;
    mesh->Rotation = vector3_zero();
    mesh->Position = vector3_zero();
    mesh->texture.internalBuffer = NULL;
    mesh->texture.width = 0;
    mesh->texture.height = 0;
    strcpy(mesh->name, "sphere");

    idx = 0;
    for (ring = 0; ring <= rings; ring++) {
        float phi = (float)ring / rings * 3.14159265f;
        float y = cosf(phi) * radius;
        float rr = sinf(phi) * radius;
        for (seg = 0; seg <= segments; seg++) {
            float theta = (float)seg / segments * 2.0f * 3.14159265f;
            float x = cosf(theta) * rr;
            float z = sinf(theta) * rr;
            mesh->Vertices[idx].Coordinates = vector3(x, y, z);
            float len = sqrtf(x * x + y * y + z * z);
            if (len > 0)
                mesh->Vertices[idx].Normal = vector3(x / len, y / len, z / len);
            else
                mesh->Vertices[idx].Normal = vector3(0, 1, 0);
            mesh->Vertices[idx].WorldCoordinates = vector3_zero();
            mesh->Vertices[idx].TextureCoordinates = vector3(
                (float)seg / segments, (float)ring / rings, 0);
            idx++;
        }
    }
    fidx = 0;
    for (ring = 0; ring < rings; ring++) {
        for (seg = 0; seg < segments; seg++) {
            int a = ring * (segments + 1) + seg;
            int b = a + segments + 1;
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

/* --- Cube mesh helper --- */
static void fill_cube(Mesh* mesh, float sx, float sy, float sz) {
    int i;
    mesh->Vertices = (Vertex*)malloc(sizeof(Vertex) * 24);
    mesh->faces = (Face*)malloc(sizeof(Face) * 12);
    mesh->verticesCount = 24;
    mesh->faceCount = 12;
    mesh->Rotation = vector3_zero();
    mesh->Position = vector3_zero();
    mesh->texture.internalBuffer = NULL;
    mesh->texture.width = 0;
    mesh->texture.height = 0;
    strcpy(mesh->name, "cube");

    float hx = sx / 2, hy = sy / 2, hz = sz / 2;
    /* Front */
    mesh->Vertices[0].Coordinates = vector3(-hx, hy, hz);
    mesh->Vertices[1].Coordinates = vector3(hx, hy, hz);
    mesh->Vertices[2].Coordinates = vector3(-hx, -hy, hz);
    mesh->Vertices[3].Coordinates = vector3(hx, -hy, hz);
    /* Back */
    mesh->Vertices[4].Coordinates = vector3(hx, hy, -hz);
    mesh->Vertices[5].Coordinates = vector3(-hx, hy, -hz);
    mesh->Vertices[6].Coordinates = vector3(hx, -hy, -hz);
    mesh->Vertices[7].Coordinates = vector3(-hx, -hy, -hz);
    /* Top */
    mesh->Vertices[8].Coordinates = vector3(-hx, hy, -hz);
    mesh->Vertices[9].Coordinates = vector3(hx, hy, -hz);
    mesh->Vertices[10].Coordinates = vector3(-hx, hy, hz);
    mesh->Vertices[11].Coordinates = vector3(hx, hy, hz);
    /* Bottom */
    mesh->Vertices[12].Coordinates = vector3(-hx, -hy, hz);
    mesh->Vertices[13].Coordinates = vector3(hx, -hy, hz);
    mesh->Vertices[14].Coordinates = vector3(-hx, -hy, -hz);
    mesh->Vertices[15].Coordinates = vector3(hx, -hy, -hz);
    /* Left */
    mesh->Vertices[16].Coordinates = vector3(-hx, hy, -hz);
    mesh->Vertices[17].Coordinates = vector3(-hx, hy, hz);
    mesh->Vertices[18].Coordinates = vector3(-hx, -hy, -hz);
    mesh->Vertices[19].Coordinates = vector3(-hx, -hy, hz);
    /* Right */
    mesh->Vertices[20].Coordinates = vector3(hx, hy, hz);
    mesh->Vertices[21].Coordinates = vector3(hx, hy, -hz);
    mesh->Vertices[22].Coordinates = vector3(hx, -hy, hz);
    mesh->Vertices[23].Coordinates = vector3(hx, -hy, -hz);

    /* Normals */
    for (i = 0; i < 4; i++)  mesh->Vertices[i].Normal = vector3(0, 0, 1);
    for (i = 4; i < 8; i++)  mesh->Vertices[i].Normal = vector3(0, 0, -1);
    for (i = 8; i < 12; i++) mesh->Vertices[i].Normal = vector3(0, 1, 0);
    for (i = 12; i < 16; i++) mesh->Vertices[i].Normal = vector3(0, -1, 0);
    for (i = 16; i < 20; i++) mesh->Vertices[i].Normal = vector3(-1, 0, 0);
    for (i = 20; i < 24; i++) mesh->Vertices[i].Normal = vector3(1, 0, 0);
    for (i = 0; i < 24; i++) {
        mesh->Vertices[i].WorldCoordinates = vector3_zero();
        int local = i % 4;
        float u = (local == 1 || local == 3) ? 1.0f : 0.0f;
        float v = (local >= 2) ? 1.0f : 0.0f;
        mesh->Vertices[i].TextureCoordinates = vector3(u, v, 0);
    }

    /* Faces */
    int faceIdx = 0;
    for (i = 0; i < 6; i++) {
        int base = i * 4;
        mesh->faces[faceIdx].A = base; mesh->faces[faceIdx].B = base + 1; mesh->faces[faceIdx].C = base + 2; faceIdx++;
        mesh->faces[faceIdx].A = base + 1; mesh->faces[faceIdx].B = base + 3; mesh->faces[faceIdx].C = base + 2; faceIdx++;
    }
}

/* --- Ensure render mesh array capacity --- */
static void ensureRenderCap(int needed) {
    if (needed > renderMeshCap) {
        int newCap = needed + 32;
        renderMeshes = (Mesh*)realloc(renderMeshes, sizeof(Mesh) * newCap);
        renderMeshCap = newCap;
    }
}

/* Copy a template mesh and set position */
static Mesh meshAt(const Mesh* tpl, Vector3 pos) {
    Mesh m = *tpl;
    m.Position = pos;
    return m;
}

/* --- Initialize all scene template meshes --- */
static void init_scene_meshes() {
    fill_cube(&groundMesh, WORLD_WIDTH + 4, 0.3f, WORLD_DEPTH + 4);
    fill_sphere(&explSphere, 1.0f, SPH_SEG, SPH_RING);
    fill_sphere(&missileSphere, 0.2f, 4, 3);
    fill_sphere(&ourExplSphere, 0.8f, SPH_SEG, SPH_RING);
    fill_sphere(&ourMissileSphere, 0.15f, 4, 3);
}

/* --- Initialize buildings --- */
static void init_builds() {
    int i;
    float spacing = WORLD_WIDTH / MAX_BUILD;
    float startX = -WORLD_WIDTH / 2 + spacing / 2;
    for (i = 0; i < MAX_BUILD; i++) {
        builds[i].pos = vector3(startX + i * spacing, GROUND_Y + 1.0f, 0);
        builds[i].alive = 1;
        builds[i].isbuild = 1;
    }
    /* Middle one is the launcher */
    builds[MAX_BUILD / 2].isbuild = 0;
    builds[MAX_BUILD / 2].pos.y = GROUND_Y + 0.6f;
}

/* --- Generate enemy missiles --- */
static void generate_enemy() {
    int i;
    if (remainGenEnemy <= 0) return;
    for (i = 0; i < MAX_ENEMY; i++) {
        if (enemies[i].alive) continue;
        if (remainGenEnemy <= 0) return;
        {
            int targetIdx = rand() % MAX_BUILD;
            float sx = (frandf() - 0.5f) * WORLD_WIDTH;
            float sy = 15.0f;  /* from the sky */
            float sz = (frandf() - 0.5f) * WORLD_DEPTH;
            float tx = builds[targetIdx].pos.x;
            float ty = builds[targetIdx].pos.y;
            float tz = builds[targetIdx].pos.z;
            float speed = 0.03f + frandf() * 0.04f;
            float dx = tx - sx, dy = ty - sy, dz = tz - sz;
            float len = sqrtf(dx * dx + dy * dy + dz * dz);
            if (len < 0.001f) len = 1.0f;
            enemies[i].from = vector3(sx, sy, sz);
            enemies[i].to = vector3(tx, ty, tz);
            enemies[i].pos = vector3(sx, sy, sz);
            enemies[i].vel = vector3(dx / len * speed, dy / len * speed, dz / len * speed);
            enemies[i].alive = 1;
            enemies[i].expl = 0;
            enemies[i].ishit = 0;
            enemies[i].r = 0.2f;
            enemies[i].targetBuild = targetIdx;
            remainGenEnemy--;
        }
    }
}

/* --- Update enemy missiles --- */
static void update_enemies() {
    int i, j;
    for (i = 0; i < MAX_ENEMY; i++) {
        if (!enemies[i].alive) continue;
        if (enemies[i].expl) {
            enemies[i].r += 0.15f;
            if (enemies[i].r >= ENEMY_MAX_EXPL_R) {
                enemies[i].alive = 0;
                enemies[i].expl = 0;
                enemies[i].ishit = 0;
                if (remainEnemy > 0) remainEnemy--;
            }
            continue;
        }
        /* Check collision with our explosions */
        for (j = 0; j < MAX_OUR_MISSILE; j++) {
            if (!ourMissiles[j].active || !ourMissiles[j].expl) continue;
            float dx = enemies[i].pos.x - ourMissiles[j].pos.x;
            float dy = enemies[i].pos.y - ourMissiles[j].pos.y;
            float dz = enemies[i].pos.z - ourMissiles[j].pos.z;
            if (dx * dx + dy * dy + dz * dz < ourMissiles[j].r * ourMissiles[j].r) {
                enemies[i].expl = 1;
                enemies[i].ishit = 1;
                score += 100;
                break;
            }
        }
        if (enemies[i].expl) continue;
        /* Check chain-reaction with other exploding enemies */
        for (j = 0; j < MAX_ENEMY; j++) {
            if (j == i || !enemies[j].alive || !enemies[j].expl || !enemies[j].ishit) continue;
            float dx = enemies[i].pos.x - enemies[j].pos.x;
            float dy = enemies[i].pos.y - enemies[j].pos.y;
            float dz = enemies[i].pos.z - enemies[j].pos.z;
            if (dx * dx + dy * dy + dz * dz < enemies[j].r * enemies[j].r) {
                enemies[i].expl = 1;
                enemies[i].ishit = 1;
                score += 100;
                break;
            }
        }
        if (enemies[i].expl) continue;
        /* Move */
        enemies[i].pos.x += enemies[i].vel.x;
        enemies[i].pos.y += enemies[i].vel.y;
        enemies[i].pos.z += enemies[i].vel.z;
        /* Check if reached target */
        float dx = enemies[i].to.x - enemies[i].pos.x;
        float dy = enemies[i].to.y - enemies[i].pos.y;
        float dz = enemies[i].to.z - enemies[i].pos.z;
        if (dx * dx + dy * dy + dz * dz < 0.5f) {
            enemies[i].expl = 1;
            builds[enemies[i].targetBuild].alive = 0;
        }
    }
}

/* --- Update our missiles --- */
static void update_our_missiles() {
    int i;
    for (i = 0; i < MAX_OUR_MISSILE; i++) {
        if (!ourMissiles[i].active) continue;
        if (!ourMissiles[i].expl) {
            float dx = ourMissiles[i].target.x - ourMissiles[i].pos.x;
            float dy = ourMissiles[i].target.y - ourMissiles[i].pos.y;
            float dz = ourMissiles[i].target.z - ourMissiles[i].pos.z;
            if (dx * dx + dy * dy + dz * dz < 0.5f) {
                ourMissiles[i].expl = 1;
                ourMissiles[i].r = 0.3f;
            }
            ourMissiles[i].pos.x += ourMissiles[i].vel.x;
            ourMissiles[i].pos.y += ourMissiles[i].vel.y;
            ourMissiles[i].pos.z += ourMissiles[i].vel.z;
        } else {
            if (ourMissiles[i].r < MAX_EXPL_R) {
                ourMissiles[i].r += 0.08f;
            } else {
                ourMissiles[i].active = 0;
                ourMissiles[i].expl = 0;
            }
        }
    }
}

/* --- Launch an interceptor missile --- */
static void launch_missile(int screenX, int screenY) {
    int i;
    if (remainMissile <= 0) return;
    for (i = 0; i < MAX_OUR_MISSILE; i++) {
        if (!ourMissiles[i].active) {
            /* Map screen coordinates to 3D world space (approximate) */
            float worldX = (((float)screenX / SCREENX) - 0.5f) * WORLD_WIDTH;
            float worldY = -(((float)screenY / SCREENY) - 0.5f) * 15.0f;
            float worldZ = 0.0f;
            /* Launch from the launcher building position */
            Vector3 launcherPos = builds[MAX_BUILD / 2].pos;
            float tx = worldX, ty = worldY, tz = worldZ;
            float dx = tx - launcherPos.x;
            float dy = ty - launcherPos.y;
            float dz = tz - launcherPos.z;
            float len = sqrtf(dx * dx + dy * dy + dz * dz);
            if (len < 0.001f) len = 1.0f;
            float speed = 0.15f;
            ourMissiles[i].active = 1;
            ourMissiles[i].expl = 0;
            ourMissiles[i].pos = launcherPos;
            ourMissiles[i].target = vector3(tx, ty, tz);
            ourMissiles[i].vel = vector3(dx / len * speed, dy / len * speed, dz / len * speed);
            ourMissiles[i].r = 0.15f;
            remainMissile--;
            break;
        }
    }
}

/* --- Check for round reset --- */
static void check_reinit() {
    if (remainEnemy <= 0 && remainGenEnemy <= 0) {
        init_builds();
        remainEnemy = 40;
        remainGenEnemy = 40;
        remainMissile = 45;
        memset(enemies, 0, sizeof(enemies));
        memset(ourMissiles, 0, sizeof(ourMissiles));
    }
}

/* --- Build the render mesh list and draw --- */
static void drawScene() {
    int i;
    Vector3 lightPos = vector3(5, 15, -10);

    check_reinit();
    update_enemies();
    update_our_missiles();
    if (rand() % 100 < 15) generate_enemy();

    /* Count needed meshes */
    int needed = 0;
    /* Ground plane (1 flat cube) */
    needed += 1;
    /* Buildings */
    needed += MAX_BUILD;
    /* Enemy missiles (non-exploding + explosions) */
    for (i = 0; i < MAX_ENEMY; i++) {
        if (enemies[i].alive) needed++;
    }
    /* Our missiles */
    for (i = 0; i < MAX_OUR_MISSILE; i++) {
        if (ourMissiles[i].active) needed++;
    }
    ensureRenderCap(needed);
    renderMeshCount = 0;

    /* Ground: a flat wide cube */
    renderMeshes[renderMeshCount++] = meshAt(&groundMesh, vector3(0, GROUND_Y - 0.15f, 0));

    /* Buildings and launcher */
    for (i = 0; i < MAX_BUILD; i++) {
        if (builds[i].isbuild) {
            if (builds[i].alive) {
                renderMeshes[renderMeshCount++] = meshAt(&buildingMesh, builds[i].pos);
            } else {
                renderMeshes[renderMeshCount++] = meshAt(&destroyedMesh, builds[i].pos);
            }
        } else {
            renderMeshes[renderMeshCount++] = meshAt(&launcherMesh, builds[i].pos);
        }
    }

    /* Enemy missiles and explosions */
    for (i = 0; i < MAX_ENEMY; i++) {
        if (!enemies[i].alive) continue;
        if (enemies[i].expl) {
            renderMeshes[renderMeshCount++] = meshAt(&explSphere, enemies[i].pos);
        } else {
            renderMeshes[renderMeshCount++] = meshAt(&missileSphere, enemies[i].pos);
        }
    }

    /* Our missiles and explosions */
    for (i = 0; i < MAX_OUR_MISSILE; i++) {
        if (!ourMissiles[i].active) continue;
        if (ourMissiles[i].expl) {
            renderMeshes[renderMeshCount++] = meshAt(&ourExplSphere, ourMissiles[i].pos);
        } else {
            renderMeshes[renderMeshCount++] = meshAt(&ourMissileSphere, ourMissiles[i].pos);
        }
    }

    /* Update camera */
    camera.Position = vector3(
        camDist * sinf(camAngleY) * cosf(camAngleX),
        camDist * sinf(camAngleX),
        -camDist * cosf(camAngleY) * cosf(camAngleX)
    );
    camera.Target = vector3(0, 2, 0);

    /* Render all meshes */
    device_clear(m_device);
    device_render(m_device, &camera, renderMeshes, renderMeshCount, &lightPos);

    /* HUD overlay (drawn after 3D render) */
    {
        char buf[256];
        sprintf(buf, "Score:%04d", score);
        drawtext(buf, 5, 5, 0xffffff);
        sprintf(buf, "Missiles:%03d", remainMissile);
        drawtext(buf, 5, 25, 0xffffff);
        sprintf(buf, "Enemy:%03d/%03d", remainEnemy, remainGenEnemy);
        drawtext(buf, SCREENX - 200, 5, 0xffffff);
        if (showhelp) {
            drawtext("[click]fire [+/-]zoom [arrows]rotate [h]help", 5, SCREENY - 25, 0xaaaaaa);
        }
    }

    /* Draw crosshair at mouse position */
    drawline(mx - 8, my, mx + 8, my, 0x00ff00);
    drawline(mx, my - 8, mx, my + 8, 0x00ff00);

    flushscreen();
    delay(16);
}

/* --- Event handlers --- */
static void onmouse(int x, int y, int on, int btn) {
    mx = x; my = y;
    if (on) {
        launch_missile(mx, my);
    }
}

static void onmotion(int x, int y, int on) {
    mx = x; my = y;
}

static void onkey(int k, int ctrl, int on) {
    if (!on) return;
    switch (k) {
        case 'h': case 'H': showhelp = !showhelp; break;
        case '+': case '=': if (camDist > 8.0f) camDist -= 2.0f; break;
        case '-': case '_': camDist += 2.0f; break;
    }
    if (k == 273) camAngleX += 0.08f;       /* Up */
    else if (k == 274) camAngleX -= 0.08f;  /* Down */
    else if (k == 276) camAngleY -= 0.08f;  /* Left */
    else if (k == 275) camAngleY += 0.08f;  /* Right */
}

/* Helper to free mesh internals */
static void freeMeshInternals(Mesh* m) {
    if (m->Vertices) { free(m->Vertices); m->Vertices = NULL; }
    if (m->faces) { free(m->faces); m->faces = NULL; }
}

/* --- Cleanup --- */
static void cleanup() {
    if (m_device) { device_free(m_device); m_device = NULL; }
    freeMeshInternals(&buildingMesh);
    freeMeshInternals(&destroyedMesh);
    freeMeshInternals(&launcherMesh);
    freeMeshInternals(&groundMesh);
    freeMeshInternals(&explSphere);
    freeMeshInternals(&missileSphere);
    freeMeshInternals(&ourExplSphere);
    freeMeshInternals(&ourMissileSphere);
    if (renderMeshes) { free(renderMeshes); renderMeshes = NULL; }
}

int main(int argc, char** argv) {
    screen(SCREENX, SCREENY);
    screentitle("Missile Command 3D (Babylon3D)");
    settextfont("FreeMono.ttf", 18);
    setonmouse(onmouse);
    setonmotion(onmotion);
    setonkey(onkey);

    /* Initialize 3D device */
    m_device = device(SCREENX, SCREENY);
    if (!m_device) {
        fprintf(stderr, "Failed to create 3D device\n");
        return 1;
    }

    /* Camera setup */
    camera.Position = vector3(0, 10, -25);
    camera.Target = vector3(0, 2, 0);

    /* Create template meshes */
    fill_cube(&buildingMesh, 1.5f, 2.0f, 1.5f);      /* Alive building */
    fill_cube(&destroyedMesh, 1.5f, 0.5f, 1.5f);     /* Destroyed building (short) */
    fill_cube(&launcherMesh, 1.0f, 1.2f, 1.0f);       /* Launcher (smaller) */
    init_scene_meshes();

    /* Initialize game */
    init_builds();
    memset(enemies, 0, sizeof(enemies));
    memset(ourMissiles, 0, sizeof(ourMissiles));

    atexit(cleanup);

    /* Main game loop */
    while (1) {
        drawScene();
    }

    return 0;
}
