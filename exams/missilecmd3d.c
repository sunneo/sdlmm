/**
 * missilecmd3d.c - 3D Missile Command using Babylon3D
 *
 * A 3D version of the classic Missile Command game rendered with
 * the Babylon3D software rendering engine.
 *
 * Buildings are rendered as 3D cubes, missiles as elongated shapes,
 * and explosions as particle effects in 3D space.
 *
 * Controls:
 *   Mouse click : Launch interceptor missile toward cursor position
 *   Mouse wheel : Zoom camera in/out
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

/* Trajectory line rendering constants */
#define TRAJECTORY_MIN_ALPHA 50
#define TRAJECTORY_ALPHA_RANGE 50

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
    Vector3 launchPos;  /* Launch position for smoke trail */
    int smokeTick;      /* Smoke particle timer */
    int smokeParticleIds[5];  /* IDs of the 5 smoke particles for this missile */
    int smokeCount;     /* Number of smoke particles spawned (up to 5) */
    int smokeHead;      /* Head index for circular buffer (0-4) */
} OurMissile3D;

/* Particle system for smoke and explosions */
#define MAX_SMOKE_PARTICLES 200
#define MAX_EXPLOSION_PARTICLES 500

typedef struct {
    Vector3 pos, vel;
    int color;
    float life;      /* 0.0 to 1.0 */
    float size;
    int active;
} Particle;

static Particle smokeParticles[MAX_SMOKE_PARTICLES];
static Particle explosionParticles[MAX_EXPLOSION_PARTICLES];
static int nextSmokeIdx = 0;
static int nextExplosionIdx = 0;

/* --- Global state --- */
static int score = 0;
static int remainMissile = 45;
static int remainGenEnemy = 40;
static int remainEnemy = 40;
static int showhelp = 1;
static volatile int mx = SCREENX / 2, my = SCREENY / 2;

/* Camera */
static float camDist = 25.0f;
static float camAngleX = 0.4f;  /* Fixed angle */
static float camAngleY = 0.0f;  /* Fixed angle */

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

/* Particle textures */
static Texture* smokeTexture = NULL;
static Texture* explosionTexture = NULL;

static void generate_glow_texture(Texture* tex, int baseColor) {
    int size = 16;
    int* buf = (int*)malloc(sizeof(int) * size * size);
    if (!buf) {
        tex->internalBuffer = NULL;
        tex->width = 0;
        tex->height = 0;
        return;
    }
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
            int r, g, b;
            if (dist < 0.3f) {
                float t = dist / 0.3f;
                r = 255 - (int)((255 - br) * t);
                g = 255 - (int)((255 - bg) * t);
                b = 255 - (int)((255 - bb) * t);
            } else {
                float t = (dist - 0.3f) / 0.7f;
                float intensity = 1.0f - t * t;
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
    generate_glow_texture(&groundMesh.texture, 0x403020);
    fill_sphere(&explSphere, 1.0f, SPH_SEG, SPH_RING);
    generate_glow_texture(&explSphere.texture, 0xff4010);
    fill_sphere(&missileSphere, 0.2f, 4, 3);
    generate_glow_texture(&missileSphere.texture, 0x40ff40);
    fill_sphere(&ourExplSphere, 0.8f, SPH_SEG, SPH_RING);
    generate_glow_texture(&ourExplSphere.texture, 0x40c0ff);
    fill_sphere(&ourMissileSphere, 0.15f, 4, 3);
    generate_glow_texture(&ourMissileSphere.texture, 0xe0e0ff);
    
    /* Create particle textures */
    smokeTexture = texture_create_gaussian(32);
    explosionTexture = texture_create_gaussian(32);
    
    if (!smokeTexture || !explosionTexture) {
        fprintf(stderr, "Failed to create particle textures\n");
    }
    
    /* Initialize particle systems */
    memset(smokeParticles, 0, sizeof(smokeParticles));
    memset(explosionParticles, 0, sizeof(explosionParticles));
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

/* --- Particle system helpers --- */
static int spawn_smoke_particle(Vector3 pos) {
    Particle* p = &smokeParticles[nextSmokeIdx];
    int particleId = nextSmokeIdx;
    nextSmokeIdx = (nextSmokeIdx + 1) % MAX_SMOKE_PARTICLES;
    
    p->pos = pos;
    /* Small random velocity for spread */
    p->vel = vector3((frandf() - 0.5f) * 0.02f, 
                     (frandf() - 0.5f) * 0.02f, 
                     (frandf() - 0.5f) * 0.02f);
    p->life = 1.0f;
    p->size = 0.3f + frandf() * 0.2f;
    p->color = 0xc0c0c0;  /* Light gray */
    p->active = 1;
    return particleId;
}

/* Glow colors for explosion particles - cyan/white like nbody */
static const int explosionGlowColors[] = {0x00FFFF, 0x80FFFF, 0xC0FFFF, 0xFFFFFF};
#define NUM_EXPLOSION_GLOW_COLORS 4

static void spawn_explosion_particles(Vector3 pos, int count) {
    int i;
    for (i = 0; i < count; i++) {
        Particle* p = &explosionParticles[nextExplosionIdx];
        nextExplosionIdx = (nextExplosionIdx + 1) % MAX_EXPLOSION_PARTICLES;
        
        /* Random direction */
        float theta = frandf() * 2.0f * 3.14159265f;
        float phi = frandf() * 3.14159265f;
        float speed = 0.05f + frandf() * 0.15f;
        
        p->pos = pos;
        p->vel = vector3(
            sinf(phi) * cosf(theta) * speed,
            sinf(phi) * sinf(theta) * speed,
            cosf(phi) * speed
        );
        p->life = 1.0f;
        p->size = 0.2f + frandf() * 0.3f;
        /* Glow colors like nbody - cyan/white */
        int colorIdx = rand() % NUM_EXPLOSION_GLOW_COLORS;
        p->color = explosionGlowColors[colorIdx];
        p->active = 1;
    }
}

static void update_particles() {
    int i;
    /* Update smoke particles */
    for (i = 0; i < MAX_SMOKE_PARTICLES; i++) {
        if (!smokeParticles[i].active) continue;
        
        smokeParticles[i].pos.x += smokeParticles[i].vel.x;
        smokeParticles[i].pos.y += smokeParticles[i].vel.y;
        smokeParticles[i].pos.z += smokeParticles[i].vel.z;
        
        /* Fade out */
        smokeParticles[i].life -= 0.02f;
        if (smokeParticles[i].life <= 0) {
            smokeParticles[i].active = 0;
        }
    }
    
    /* Update explosion particles */
    for (i = 0; i < MAX_EXPLOSION_PARTICLES; i++) {
        if (!explosionParticles[i].active) continue;
        
        explosionParticles[i].pos.x += explosionParticles[i].vel.x;
        explosionParticles[i].pos.y += explosionParticles[i].vel.y;
        explosionParticles[i].pos.z += explosionParticles[i].vel.z;
        
        /* Expand and fade */
        explosionParticles[i].size += 0.03f;
        explosionParticles[i].life -= 0.015f;
        
        if (explosionParticles[i].life <= 0) {
            explosionParticles[i].active = 0;
        }
    }
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
            /* Spawn explosion particles for enemy explosions */
            if (enemies[i].r < ENEMY_MAX_EXPL_R && rand() % 3 == 0) {
                spawn_explosion_particles(enemies[i].pos, 3);
            }
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
                /* Spawn initial explosion particles */
                spawn_explosion_particles(enemies[i].pos, 50);
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
                /* Spawn initial explosion particles */
                spawn_explosion_particles(enemies[i].pos, 50);
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
            /* Spawn initial explosion particles */
            spawn_explosion_particles(enemies[i].pos, 50);
        }
    }
}

/* --- Update our missiles --- */
static void update_our_missiles() {
    int i, j;
    for (i = 0; i < MAX_OUR_MISSILE; i++) {
        if (!ourMissiles[i].active) continue;
        if (!ourMissiles[i].expl) {
            /* Spawn smoke particles periodically - limit to 5 particles */
            ourMissiles[i].smokeTick++;
            if (ourMissiles[i].smokeTick % 2 == 0) {
                int particleId = spawn_smoke_particle(ourMissiles[i].pos);
                /* If we already have 5 smoke particles, replace the oldest one using circular buffer */
                if (ourMissiles[i].smokeCount >= 5) {
                    /* Deactivate the oldest particle at the head position */
                    smokeParticles[ourMissiles[i].smokeParticleIds[ourMissiles[i].smokeHead]].active = 0;
                    /* Replace with new particle */
                    ourMissiles[i].smokeParticleIds[ourMissiles[i].smokeHead] = particleId;
                    /* Move head to next position in circular buffer */
                    ourMissiles[i].smokeHead = (ourMissiles[i].smokeHead + 1) % 5;
                } else {
                    /* Still filling up the initial 5 particles */
                    ourMissiles[i].smokeParticleIds[ourMissiles[i].smokeCount] = particleId;
                    ourMissiles[i].smokeCount++;
                }
            }
            
            float dx = ourMissiles[i].target.x - ourMissiles[i].pos.x;
            float dy = ourMissiles[i].target.y - ourMissiles[i].pos.y;
            float dz = ourMissiles[i].target.z - ourMissiles[i].pos.z;
            if (dx * dx + dy * dy + dz * dz < 0.5f) {
                ourMissiles[i].expl = 1;
                ourMissiles[i].r = 0.3f;
                /* Spawn explosion particles */
                spawn_explosion_particles(ourMissiles[i].pos, 50);
            }
            ourMissiles[i].pos.x += ourMissiles[i].vel.x;
            ourMissiles[i].pos.y += ourMissiles[i].vel.y;
            ourMissiles[i].pos.z += ourMissiles[i].vel.z;
        } else {
            if (ourMissiles[i].r < MAX_EXPL_R) {
                ourMissiles[i].r += 0.08f;
                /* Continue spawning explosion particles */
                if (rand() % 3 == 0) {
                    spawn_explosion_particles(ourMissiles[i].pos, 3);
                }
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
            ourMissiles[i].launchPos = launcherPos;
            ourMissiles[i].smokeTick = 0;
            ourMissiles[i].smokeCount = 0;  /* Initialize smoke count */
            ourMissiles[i].smokeHead = 0;   /* Initialize circular buffer head */
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

/* --- Draw trajectory lines for enemy missiles --- */
static void draw_trajectory_lines(const Camera* camera) {
    int i, j;
    Vector3 up = vector3_up();
    Matrix viewMatrix = matrix_LookAtLH(&camera->Position, &camera->Target, &up);
    Matrix projectionMatrix = matrix_PerspectiveFovLH(0.78f, 
        (float)SCREENX / (float)SCREENY, 0.01f, 1000.0f);
    Matrix viewProj = matrix_multiply(&viewMatrix, &projectionMatrix);
    
    /* Draw trajectory lines for non-exploding enemy missiles */
    for (i = 0; i < MAX_ENEMY; i++) {
        if (!enemies[i].alive || enemies[i].expl) continue;
        
        /* Draw line from starting position to current position (laser from sky) */
        int segments = 20;
        for (j = 0; j < segments; j++) {
            float t1 = (float)j / segments;
            float t2 = (float)(j + 1) / segments;
            
            Vector3 p1 = vector3(
                enemies[i].from.x + (enemies[i].pos.x - enemies[i].from.x) * t1,
                enemies[i].from.y + (enemies[i].pos.y - enemies[i].from.y) * t1,
                enemies[i].from.z + (enemies[i].pos.z - enemies[i].from.z) * t1
            );
            Vector3 p2 = vector3(
                enemies[i].from.x + (enemies[i].pos.x - enemies[i].from.x) * t2,
                enemies[i].from.y + (enemies[i].pos.y - enemies[i].from.y) * t2,
                enemies[i].from.z + (enemies[i].pos.z - enemies[i].from.z) * t2
            );
            
            /* Transform to screen space */
            Vector3 clip1 = vector3_transform_coordinates(&p1, &viewProj);
            Vector3 clip2 = vector3_transform_coordinates(&p2, &viewProj);
            
            /* Project to screen */
            float screenX1 = (clip1.x + 1.0f) * 0.5f * SCREENX;
            float screenY1 = (1.0f - clip1.y) * 0.5f * SCREENY;
            float screenX2 = (clip2.x + 1.0f) * 0.5f * SCREENX;
            float screenY2 = (1.0f - clip2.y) * 0.5f * SCREENY;
            
            /* Check if on screen */
            if (screenX1 >= 0 && screenX1 < SCREENX && screenY1 >= 0 && screenY1 < SCREENY &&
                screenX2 >= 0 && screenX2 < SCREENX && screenY2 >= 0 && screenY2 < SCREENY) {
                /* Draw with transparency (using alpha value in color) */
                int alpha = (int)(TRAJECTORY_MIN_ALPHA + TRAJECTORY_ALPHA_RANGE * (1.0f - t1));  /* Fade along trajectory */
                int color = (alpha << 24) | 0x40ff40;  /* Green with alpha */
                drawline((int)screenX1, (int)screenY1, (int)screenX2, (int)screenY2, color);
            }
        }
    }
}

/* --- Build the render mesh list and draw --- */
static void drawScene() {
    int i;
    Vector3 lightPos = vector3(5, 15, -10);

    check_reinit();
    update_enemies();
    update_our_missiles();
    update_particles();
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
        /* Only render missile mesh when not exploding - explosions use particles only */
        if (!enemies[i].expl) {
            renderMeshes[renderMeshCount++] = meshAt(&missileSphere, enemies[i].pos);
        }
    }

    /* Our missiles and explosions */
    for (i = 0; i < MAX_OUR_MISSILE; i++) {
        if (!ourMissiles[i].active) continue;
        /* Only render missile mesh when not exploding - explosions use particles only */
        if (!ourMissiles[i].expl) {
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

    /* Purple gradient sky background */
    {
        int bx, by;
        for (by = 0; by < SCREENY; by++) {
            float t = (float)by / SCREENY;
            int r = (int)(20 + t * 40);
            int g = (int)(0 + t * 15);
            int b = (int)(60 + t * 50);
            int color = (r << 16) | (g << 8) | b;
            for (bx = 0; bx < SCREENX; bx++) {
                m_device->backbuffer[by * SCREENX + bx] = color;
            }
        }
    }

    device_render(m_device, &camera, renderMeshes, renderMeshCount, &lightPos);

    /* Draw trajectory lines after 3D meshes */
    draw_trajectory_lines(&camera);
    
    /* Render smoke particles */
    {
        Vector3* particlePositions;
        int* particleColors;
        int particleCount = 0;
        int i;
        
        /* Count active smoke particles */
        for (i = 0; i < MAX_SMOKE_PARTICLES; i++) {
            if (smokeParticles[i].active) particleCount++;
        }
        
        if (particleCount > 0 && smokeTexture) {
            particlePositions = (Vector3*)malloc(sizeof(Vector3) * particleCount);
            particleColors = (int*)malloc(sizeof(int) * particleCount);
            int idx = 0;
            
            /* Add smoke particles */
            for (i = 0; i < MAX_SMOKE_PARTICLES; i++) {
                if (smokeParticles[i].active) {
                    particlePositions[idx] = smokeParticles[i].pos;
                    /* Apply alpha based on life */
                    int alpha = (int)(smokeParticles[i].life * 128);
                    particleColors[idx] = (alpha << 24) | smokeParticles[i].color;
                    idx++;
                }
            }
            
            /* Render smoke with transparency */
            device_render_particles(m_device, &camera, particlePositions, particleColors,
                                   particleCount, 8.0f, smokeTexture, 0);
            
            free(particlePositions);
            free(particleColors);
        }
    }
    
    /* Render explosion particles */
    {
        Vector3* particlePositions;
        int* particleColors;
        int particleCount = 0;
        int i;
        
        /* Count active explosion particles */
        for (i = 0; i < MAX_EXPLOSION_PARTICLES; i++) {
            if (explosionParticles[i].active) particleCount++;
        }
        
        if (particleCount > 0 && explosionTexture) {
            particlePositions = (Vector3*)malloc(sizeof(Vector3) * particleCount);
            particleColors = (int*)malloc(sizeof(int) * particleCount);
            int idx = 0;
            
            /* Add explosion particles */
            for (i = 0; i < MAX_EXPLOSION_PARTICLES; i++) {
                if (explosionParticles[i].active) {
                    particlePositions[idx] = explosionParticles[i].pos;
                    /* Apply alpha based on life */
                    int alpha = (int)(explosionParticles[i].life * 255);
                    particleColors[idx] = (alpha << 24) | explosionParticles[i].color;
                    idx++;
                }
            }
            
            /* Render explosions with additive blending for glow */
            device_render_particles(m_device, &camera, particlePositions, particleColors,
                                   particleCount, 10.0f, explosionTexture, 1);
            
            free(particlePositions);
            free(particleColors);
        }
    }

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
            drawtext("[click]fire [wheel]zoom [h]help", 5, SCREENY - 25, 0xaaaaaa);
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
    }
}

/* Mouse wheel handler for zoom */
static void onwheel(int delta) {
    if (delta > 0) {
        /* Zoom in */
        if (camDist > 8.0f) camDist -= 2.0f;
    } else {
        /* Zoom out */
        if (camDist < 50.0f) camDist += 2.0f;
    }
}

/* Helper to free mesh internals */
static void freeMeshInternals(Mesh* m) {
    if (m->Vertices) { free(m->Vertices); m->Vertices = NULL; }
    if (m->faces) { free(m->faces); m->faces = NULL; }
    if (m->texture.internalBuffer) { free(m->texture.internalBuffer); m->texture.internalBuffer = NULL; }
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
    
    /* Free particle textures */
    if (smokeTexture) {
        if (smokeTexture->internalBuffer) free(smokeTexture->internalBuffer);
        free(smokeTexture);
        smokeTexture = NULL;
    }
    if (explosionTexture) {
        if (explosionTexture->internalBuffer) free(explosionTexture->internalBuffer);
        free(explosionTexture);
        explosionTexture = NULL;
    }
}

int main(int argc, char** argv) {
    screen(SCREENX, SCREENY);
    screentitle("Missile Command 3D (Babylon3D)");
    settextfont("FreeMono.ttf", 18);
    setonmouse(onmouse);
    setonmotion(onmotion);
    setonkey(onkey);
    setonwheel(onwheel);

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
    generate_glow_texture(&buildingMesh.texture, 0x6080a0);
    fill_cube(&destroyedMesh, 1.5f, 0.5f, 1.5f);     /* Destroyed building (short) */
    generate_glow_texture(&destroyedMesh.texture, 0x804020);
    fill_cube(&launcherMesh, 1.0f, 1.2f, 1.0f);       /* Launcher (smaller) */
    generate_glow_texture(&launcherMesh.texture, 0x60a060);
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
