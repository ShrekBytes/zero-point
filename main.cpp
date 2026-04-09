// Zero Point — Where Forest Meets Village
// CSE422: Computer Graphics Lab
//
// Controls:
//   Spacebar — Toggle day/night
//   ESC / Q  — Quit

#include <GL/glut.h>
#include <cmath>
#include <cstdlib>

// =============================================================================
// Global State
// =============================================================================

enum TimeState { DAY, NIGHT };
TimeState timeState = DAY;

// World coordinate bounds
const float WORLD_LEFT   = -40.0f;
const float WORLD_RIGHT  =  40.0f;
const float WORLD_BOTTOM = -30.0f;
const float WORLD_TOP    =  30.0f;

// =============================================================================
// Animation Variables
// =============================================================================

float windmillAngle = 0.0f;
float globalAnimTime = 0.0f;

// Auto day/night cycle — toggles every ~10 seconds (625 ticks × 16 ms ≈ 10 s)
int dayNightCounter = 0;
const int DAY_NIGHT_INTERVAL = 625;

// Cat moves along the village fence (day only)
float catX   = -5.0f;
float catDir =  1.0f;   // +1 = moving right, -1 = moving left
const float CAT_SPEED = 0.04f;
const float CAT_X_MIN = -18.0f;
const float CAT_X_MAX =  14.0f;

// Deer moves along the forest bank (night only)
float deerX   = -5.0f;
float deerDir =  1.0f;
const float DEER_SPEED = 0.03f;
const float DEER_X_MIN = -18.0f;
const float DEER_X_MAX =  8.0f;

// Birds fly across the sky (day only)
const int BIRD_COUNT = 7;
float birdX[BIRD_COUNT] = { -45.0f, -55.0f, -62.0f, -70.0f, -78.0f, -85.0f, -92.0f };
float birdY[BIRD_COUNT] = {  22.0f,  18.5f,  25.0f,  15.0f,  23.0f,  19.5f,  21.0f };
const float BIRD_SPEED = 0.035f;

// Clouds drift across the sky at different speeds (day only)
const int CLOUD_COUNT = 7;
float cloudOffset[CLOUD_COUNT]      = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
const float CLOUD_SPEED[CLOUD_COUNT] = { 0.01f, 0.03f, 0.005f, 0.02f, 0.015f, 0.025f, 0.012f };

// Stars are only visible at night
const int STAR_COUNT = 80;
float starX[STAR_COUNT];
float starY[STAR_COUNT];
float starPhase[STAR_COUNT];

// =============================================================================
// New Details State (Smoke, Ripples, Flowers, Rocks, Leaves, Shooting Star)
// =============================================================================

struct SmokeParticle { float x, y, size, alpha; bool active; };
const int SMOKE_COUNT = 30;
SmokeParticle smokeParticles[SMOKE_COUNT];

struct Ripple { float x, y, radius, maxRadius, alpha; bool active; };
const int RIPPLE_COUNT = 8;
Ripple ripples[RIPPLE_COUNT];

struct StaticProp { float x, y; int type; };
const int FLOWER_COUNT = 40;
StaticProp flowers[FLOWER_COUNT];
const int ROCK_COUNT = 15;
StaticProp rocks[ROCK_COUNT];

struct Leaf { float x, y, drift, alpha; bool active; };
const int LEAF_COUNT = 25;
Leaf leaves[LEAF_COUNT];

struct ShootingStar { float x, y, len, alpha; bool active; };
ShootingStar shootingStar = {0,0,0,0,false};

// =============================================================================
// Grass & Firefly Data
// =============================================================================

// Each grass tuft has a position, height, spread, and lean direction
struct GrassTuft {
    float x, y;       // base position
    float h;          // height
    float spread;     // how wide the blades fan out
    float lean;       // slight horizontal lean
};

const int FOREST_BANK_GRASS_COUNT  = 60;
const int FOREST_PATCH_GRASS_COUNT = 120;
const int VILLAGE_GRASS_COUNT      = 80;
const int FIREFLY_COUNT            = 70;
const int FIREFLY_VILLAGE_COUNT    = 10;   // fireflies in both forest and village

GrassTuft forestBankGrass[FOREST_BANK_GRASS_COUNT];
GrassTuft forestPatchGrass[FOREST_PATCH_GRASS_COUNT];
GrassTuft villageGrass[VILLAGE_GRASS_COUNT];

float fireflyX[FIREFLY_COUNT];
float fireflyY[FIREFLY_COUNT];

// Firefly zone limits
const float FIREFLY_FOREST_X_MIN  = -38.0f;
const float FIREFLY_FOREST_X_MAX  =   8.0f;
const float FIREFLY_VILLAGE_X_MIN =  12.0f;
const float FIREFLY_VILLAGE_X_MAX =  36.0f;
const float FIREFLY_VILLAGE_Y_MIN = -20.0f;
const float FIREFLY_VILLAGE_Y_MAX =  -4.0f;
const float FOREST_Y_MIN          = -16.0f;
const float FOREST_Y_MAX          =   9.0f;

// =============================================================================
// Small Helper Functions
// =============================================================================

// Random float between 0 and 1
float frand01() {
    return (float)rand() / (float)RAND_MAX;
}

// Clamp a value between lo and hi
float clampf(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

// Y coordinate on the upper river bank at a given X
// The upper bank goes from (40, 10) to (-40, -20)
float riverUpperBankY(float x) {
    return 10.0f + (x - 40.0f) * 0.375f;
}

// Y coordinate on the lower river bank at a given X
// The lower bank goes from (40, 5) to (-40, -30)
float riverLowerBankY(float x) {
    return 5.0f + (x - 40.0f) * 0.4375f;
}

// Baseline for the fence and cat (just inside the village side of the river)
float fenceBase(float x) {
    return riverLowerBankY(x) - 1.2f;
}

// Baseline for the deer (above the upper river bank on the forest side)
float forestBase(float x) {
    return riverUpperBankY(x) + 4.5f;
}

// =============================================================================
// Initialise Stars, Grass, and Firefly Positions (called once at startup)
// =============================================================================

void initStars() {
    for (int i = 0; i < STAR_COUNT; i++) {
        starX[i] = -40.0f + frand01() * 80.0f;
        starY[i] =   6.0f + frand01() * 24.0f;
        starPhase[i] = frand01() * 2.0f * M_PI;
    }
}

void initDetails() {
    for (int i = 0; i < SMOKE_COUNT; i++) smokeParticles[i].active = false;
    for (int i = 0; i < RIPPLE_COUNT; i++) ripples[i].active = false;
    for (int i = 0; i < LEAF_COUNT; i++) leaves[i].active = false;
    shootingStar.active = false;

    for (int i = 0; i < ROCK_COUNT; i++) {
        rocks[i].x = -38.0f + frand01() * 76.0f;
        if (i % 2 == 0) rocks[i].y = riverUpperBankY(rocks[i].x) + 0.1f + frand01() * 1.5f;
        else            rocks[i].y = riverLowerBankY(rocks[i].x) - 0.1f - frand01() * 1.5f;
        rocks[i].type = rand() % 2;
    }
}

void initFireflies() {
    for (int i = 0; i < FIREFLY_COUNT; i++) {
        if (i < FIREFLY_VILLAGE_COUNT) {
            // A couple of fireflies appear in the village area
            fireflyX[i] = FIREFLY_VILLAGE_X_MIN + frand01() * (FIREFLY_VILLAGE_X_MAX - FIREFLY_VILLAGE_X_MIN);
            fireflyY[i] = FIREFLY_VILLAGE_Y_MIN + frand01() * (FIREFLY_VILLAGE_Y_MAX - FIREFLY_VILLAGE_Y_MIN);
        } else {
            // The rest are scattered through the forest
            float x    = FIREFLY_FOREST_X_MIN + frand01() * (FIREFLY_FOREST_X_MAX - FIREFLY_FOREST_X_MIN);
            float yMin = riverUpperBankY(x) + 0.8f;
            if (yMin < FOREST_Y_MIN) yMin = FOREST_Y_MIN;
            if (yMin > FOREST_Y_MAX) yMin = FOREST_Y_MAX;
            fireflyX[i] = x;
            fireflyY[i] = yMin + frand01() * (FOREST_Y_MAX - yMin);
        }
    }
}

void initGrass() {
    // Grass along the forest river bank
    for (int i = 0; i < FOREST_BANK_GRASS_COUNT; i++) {
        float x     = -37.5f + frand01() * 72.0f;
        float leftW = 1.0f - ((x + 40.0f) / 80.0f);  // taller on the left side
        forestBankGrass[i].x      = x;
        forestBankGrass[i].y      = riverUpperBankY(x) + 0.12f + frand01() * 0.40f;
        forestBankGrass[i].h      = 0.62f + 0.44f * leftW + frand01() * 0.26f;
        forestBankGrass[i].spread = 0.07f + frand01() * 0.06f;
        forestBankGrass[i].lean   = (frand01() - 0.5f) * 0.18f;
    }

    // Grass scattered through the forest interior
    for (int i = 0; i < FOREST_PATCH_GRASS_COUNT; i++) {
        float x = 0.0f, y = 0.0f;
        // Try to place the tuft above the river bank
        for (int tries = 0; tries < 60; tries++) {
            float tx = -37.0f + frand01() * 72.0f;
            float ty = -14.0f + frand01() * 23.0f;
            if (ty > riverUpperBankY(tx) + 1.05f) {
                x = tx;
                y = ty;
                break;
            }
        }
        // Fallback if no valid spot found
        if (x == 0.0f && y == 0.0f) {
            x = -37.0f + frand01() * 72.0f;
            y = riverUpperBankY(x) + 1.25f + frand01() * 5.5f;
        }
        float leftW = 1.0f - ((x + 40.0f) / 80.0f);
        forestPatchGrass[i].x      = x;
        forestPatchGrass[i].y      = y;
        forestPatchGrass[i].h      = 0.56f + 0.48f * leftW + frand01() * 0.28f;
        forestPatchGrass[i].spread = 0.08f + frand01() * 0.06f;
        forestPatchGrass[i].lean   = (frand01() - 0.5f) * 0.14f;
    }

    // Grass in the village ground area
    for (int i = 0; i < VILLAGE_GRASS_COUNT; i++) {
        float x = 0.0f, y = 0.0f;
        for (int tries = 0; tries < 60; tries++) {
            float tx = -37.0f + frand01() * 74.0f;
            float ty = -29.0f + frand01() * 20.0f;
            if (ty < riverLowerBankY(tx) - 1.0f) {
                x = tx;
                y = ty;
                break;
            }
        }
        // Fallback if no valid spot found
        if (x == 0.0f && y == 0.0f) {
            x = -37.0f + frand01() * 74.0f;
            y = -27.5f + frand01() * 12.0f;
        }
        villageGrass[i].x      = x;
        villageGrass[i].y      = y;
        villageGrass[i].h      = 0.76f + frand01() * 0.36f;
        villageGrass[i].spread = 0.40f + frand01() * 0.18f;
        villageGrass[i].lean   = (frand01() - 0.5f) * 0.12f;
    }
}

// =============================================================================
// ALGORITHM: DDA Line Drawing
// Computes intermediate points using floating-point increments each step.
// =============================================================================

void drawLineDDA(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;

    // In world-space coordinates, 1 unit can be visually large.
    // Use denser sampling so DDA point lines appear continuous.
    int steps = (int)ceilf((fabs(dx) > fabs(dy) ? fabs(dx) : fabs(dy)) * 7.0f);
    if (steps == 0) {
        glBegin(GL_POINTS);
        glVertex2f(x1, y1);
        glEnd();
        return;
    }

    float xInc = dx / steps;
    float yInc = dy / steps;
    float x = x1, y = y1;

    glBegin(GL_POINTS);
    for (int i = 0; i <= steps; i++) {
        glVertex2f(x, y);
        x += xInc;
        y += yInc;
    }
    glEnd();
}

// =============================================================================
// ALGORITHM: Bresenham Line Drawing
// Uses integer error accumulation — no floating-point needed.
//   err tracks how far we have drifted from the ideal line
//   e2  is the doubled error used to decide which axis to step
// =============================================================================

void drawLineBresenham(int x1, int y1, int x2, int y2) {
    int dx  =  abs(x2 - x1);
    int dy  =  abs(y2 - y1);
    int sx  = (x1 < x2) ? 1 : -1;
    int sy  = (y1 < y2) ? 1 : -1;
    int err =  dx - dy;

    glBegin(GL_POINTS);
    while (true) {
        glVertex2i(x1, y1);
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 <  dx) { err += dx; y1 += sy; }
    }
    glEnd();
}

// =============================================================================
// ALGORITHM: Midpoint Circle Drawing
// Uses integer decision variable to plot 8 symmetric points per step.
// =============================================================================

void drawMidpointCircle(float cx, float cy, float radius) {
    int r = (int)radius;
    int x = 0, y = r;
    int d = 1 - r;

    // Plot 8 symmetric points at once using reflections
    auto plot8 = [&](int px, int py) {
        glVertex2f(cx + px, cy + py); glVertex2f(cx - px, cy + py);
        glVertex2f(cx + px, cy - py); glVertex2f(cx - px, cy - py);
        glVertex2f(cx + py, cy + px); glVertex2f(cx - py, cy + px);
        glVertex2f(cx + py, cy - px); glVertex2f(cx - py, cy - px);
    };

    glBegin(GL_POINTS);
    while (x <= y) {
        plot8(x, y);
        if (d < 0) d += 2 * x + 3;
        else      { d += 2 * (x - y) + 5; y--; }
        x++;
    }
    glEnd();
}

// Filled version: draws horizontal spans derived from the circle equation
void fillMidpointCircle(float cx, float cy, float radius) {
    glBegin(GL_LINES);
    for (float dy = -radius; dy <= radius; dy += 0.05f) {
        float dx = sqrtf(radius * radius - dy * dy);
        glVertex2f(cx - dx, cy + dy);
        glVertex2f(cx + dx, cy + dy);
    }
    glEnd();
}

// =============================================================================
// Primitive Helpers
// =============================================================================

// Filled circle using a triangle fan
void fillCircle(float cx, float cy, float radius) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= 60; i++) {
        float angle = 2.0f * M_PI * i / 60.0f;
        glVertex2f(cx + radius * cosf(angle), cy + radius * sinf(angle));
    }
    glEnd();
}

// Filled rectangle
void fillRect(float x, float y, float w, float h) {
    glBegin(GL_QUADS);
    glVertex2f(x,     y    );
    glVertex2f(x + w, y    );
    glVertex2f(x + w, y + h);
    glVertex2f(x,     y + h);
    glEnd();
}

// Smooth line with settable width
void drawLine(float x1, float y1, float x2, float y2, float width = 1.0f) {
    glLineWidth(width);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glBegin(GL_LINES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glEnd();
    glDisable(GL_LINE_SMOOTH);
    glLineWidth(1.0f);
}

// =============================================================================
// Sky  —  background, clouds (day) or stars (night)
// =============================================================================

void drawSky() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glBegin(GL_QUADS);
    if (timeState == DAY) {
        glColor3f(0.25f, 0.55f, 0.95f);
        glVertex2f(-40.0f,  30.0f);
        glVertex2f( 40.0f,  30.0f);
        glColor3f(0.85f, 0.95f, 1.00f);
        glVertex2f( 40.0f, -30.0f);
        glVertex2f(-40.0f, -30.0f);
    } else {
        glColor3f(0.00f, 0.01f, 0.05f);
        glVertex2f(-40.0f,  30.0f);
        glVertex2f( 40.0f,  30.0f);
        glColor3f(0.08f, 0.15f, 0.35f);
        glVertex2f( 40.0f, -30.0f);
        glVertex2f(-40.0f, -30.0f);
    }
    glEnd();

    if (timeState == NIGHT) {
        // Stars Twinkling (Pronounced blinking)
        glPointSize(2.5f);
        glBegin(GL_POINTS);
        for (int i = 0; i < STAR_COUNT; i++) {
            // Faster oscillation and deeper dimming for a more noticeable blink
            float twinkle = (sinf(globalAnimTime * 5.0f + starPhase[i] * 10.0f) + 1.0f) * 0.5f;
            float brightness = 0.25f + 0.75f * twinkle; 
            glColor3f(brightness, brightness, brightness * 0.9f);
            glVertex2f(starX[i], starY[i]);
        }
        glEnd();
        glPointSize(1.0f);
        
        // Shooting star
        if (shootingStar.active) {
            glLineWidth(2.5f);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glBegin(GL_LINES);
            glColor4f(1.0f, 1.0f, 1.0f, 0.0f);
            glVertex2f(shootingStar.x + shootingStar.len, shootingStar.y + shootingStar.len);
            glColor4f(1.0f, 1.0f, 1.0f, shootingStar.alpha);
            glVertex2f(shootingStar.x, shootingStar.y);
            glEnd();
            glDisable(GL_BLEND);
            glLineWidth(1.0f);
        }
    }
}

void drawClouds() {
    if (timeState != DAY) return;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor3f(1.0f, 1.0f, 1.0f);

    float c1 = 45.0f - fmodf(28.0f + cloudOffset[0], 90.0f);
    fillMidpointCircle(c1,        24.0f, 3.0f);
    fillMidpointCircle(c1 + 3.5f, 25.0f, 4.0f);
    fillMidpointCircle(c1 + 7.0f, 24.0f, 3.0f);

    float c2 = 45.0f - fmodf(10.0f + cloudOffset[1], 90.0f);
    fillMidpointCircle(c2,        26.0f, 3.5f);
    fillMidpointCircle(c2 + 4.0f, 27.0f, 4.5f);
    fillMidpointCircle(c2 + 8.0f, 26.0f, 3.5f);

    float c3 = 45.0f - fmodf(65.0f + cloudOffset[2], 90.0f);
    fillMidpointCircle(c3,        22.0f, 2.5f);
    fillMidpointCircle(c3 + 3.0f, 23.0f, 3.0f);
    fillMidpointCircle(c3 + 6.0f, 22.0f, 2.5f);

    float c4 = 45.0f - fmodf(45.0f + cloudOffset[3], 90.0f);
    fillMidpointCircle(c4,        27.5f, 3.2f);
    fillMidpointCircle(c4 + 4.0f, 28.2f, 4.2f);
    fillMidpointCircle(c4 + 8.0f, 27.5f, 3.2f);

    float c5 = 45.0f - fmodf(80.0f + cloudOffset[4], 90.0f);
    fillMidpointCircle(c5,        20.0f, 2.8f);
    fillMidpointCircle(c5 + 3.5f, 21.0f, 3.8f);
    fillMidpointCircle(c5 + 7.0f, 20.0f, 2.8f);

    float c6 = 45.0f - fmodf(20.0f + cloudOffset[5], 90.0f);
    fillMidpointCircle(c6,        25.0f, 2.4f);
    fillMidpointCircle(c6 + 3.0f, 26.0f, 3.4f);
    fillMidpointCircle(c6 + 6.0f, 25.0f, 2.4f);

    float c7 = 45.0f - fmodf(55.0f + cloudOffset[6], 90.0f);
    fillMidpointCircle(c7,        21.5f, 3.0f);
    fillMidpointCircle(c7 + 4.2f, 22.8f, 4.0f);
    fillMidpointCircle(c7 + 8.4f, 21.5f, 3.0f);
    glDisable(GL_BLEND);
}

void drawSun() {
    if (timeState != DAY) return;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1.0f, 0.95f, 0.4f, 0.2f);
    fillCircle(25.0f, 20.0f, 6.0f);
    glColor4f(1.0f, 0.98f, 0.7f, 0.4f);
    fillCircle(25.0f, 20.0f, 4.8f);
    glDisable(GL_BLEND);
    
    glColor3f(1.0f, 0.95f, 0.0f);
    fillCircle(25.0f, 20.0f, 4.0f);
}

void drawMoon() {
    if (timeState != NIGHT) return;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.85f, 0.90f, 0.95f, 0.2f);
    fillCircle(-25.0f, 20.0f, 5.8f);
    glColor4f(0.95f, 0.95f, 1.00f, 0.4f);
    fillCircle(-25.0f, 20.0f, 4.6f);
    glDisable(GL_BLEND);

    glColor3f(0.98f, 0.98f, 0.92f);
    fillCircle(-25.0f, 20.0f, 4.0f); 
    
    glColor3f(0.85f, 0.85f, 0.80f);
    fillCircle(-24.0f, 21.0f, 0.8f);
    fillCircle(-26.5f, 19.5f, 1.2f);
    fillCircle(-24.5f, 18.0f, 0.6f);
}

// =============================================================================
// River  —  diagonal strip from top-right to bottom-left
// Upper bank: (40, 10) to (-40, -20)
// Lower bank: (40,  5) to (-40, -30)
// =============================================================================

void drawRiver() {
    // Main river body
    if (timeState == DAY) glColor3f(0.20f, 0.52f, 0.80f);
    else                  glColor3f(0.07f, 0.20f, 0.34f);
    glBegin(GL_POLYGON);
        glVertex2f( 40.0f,  10.0f);
        glVertex2f( 40.0f,   5.0f);
        glVertex2f(-40.0f, -30.0f);
        glVertex2f(-40.0f, -20.0f);
    glEnd();

    // Lighter shimmer strip running through the centre of the river
    const float iTopR  = 10.0f + (5.0f  - 10.0f) * 0.30f;  // 30% from top on the right
    const float iBotR  = 10.0f + (5.0f  - 10.0f) * 0.70f;  // 70% from top on the right
    const float iTopL  = -20.0f + (-30.0f - -20.0f) * 0.30f;
    const float iBotL  = -20.0f + (-30.0f - -20.0f) * 0.70f;
    if (timeState == DAY) glColor3f(0.38f, 0.68f, 0.92f);
    else                  glColor3f(0.13f, 0.30f, 0.46f);
    glBegin(GL_POLYGON);
        glVertex2f( 40.0f, iTopR); glVertex2f( 40.0f, iBotR);
        glVertex2f(-40.0f, iBotL); glVertex2f(-40.0f, iTopL);
    glEnd();

    // Animated flow streaks (shift driven by windmill angle for a shared animation)
    float flowShift = fmodf(windmillAngle * 0.085f, 10.0f);
    if (timeState == DAY) glColor3f(0.55f, 0.80f, 0.98f);
    else                  glColor3f(0.20f, 0.33f, 0.48f);
    glLineWidth(1.2f);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    float streakAnchors[] = { -37.0f, -28.0f, -19.0f, -10.0f, -1.0f, 8.0f, 17.0f, 26.0f, 35.0f };
    glBegin(GL_LINES);
    for (int i = 0; i < 9; i++) {
        float x1 = streakAnchors[i] - flowShift;
        if (x1 < -40.0f) x1 += 80.0f;
        float x2 = x1 + 6.5f;
        if (x2 > 40.0f) x2 = 40.0f;
        float y1 = (riverUpperBankY(x1) + riverLowerBankY(x1)) * 0.5f + 0.30f;
        float y2 = (riverUpperBankY(x2) + riverLowerBankY(x2)) * 0.5f + 0.30f;
        glVertex2f(x1, y1);
        glVertex2f(x2, y2);
    }
    glEnd();
    glDisable(GL_LINE_SMOOTH);
    glLineWidth(1.0f);

    // Muddy edge strips just inside the water at each bank
    if (timeState == DAY) glColor3f(0.46f, 0.34f, 0.21f);
    else                  glColor3f(0.28f, 0.20f, 0.13f);
    glBegin(GL_POLYGON);
        glVertex2f( 40.0f,  10.0f);   glVertex2f( 40.0f,   9.45f);
        glVertex2f(-40.0f, -20.55f);  glVertex2f(-40.0f, -20.0f);
    glEnd();
    glBegin(GL_POLYGON);
        glVertex2f( 40.0f,   5.0f);   glVertex2f( 40.0f,   5.55f);
        glVertex2f(-40.0f, -29.45f);  glVertex2f(-40.0f, -30.0f);
    glEnd();

    // Bank outlines drawn with Bresenham (required algorithm)
    glColor3f(0.36f, 0.24f, 0.11f);
    glPointSize(2.0f);
    drawLineBresenham( 40,  10, -40, -20);
    drawLineBresenham( 40,   5, -40, -30);
    glPointSize(1.0f);

    // Water reflections (Refined shimmering pillar)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    float tx = (timeState == DAY) ? 25.0f : -25.0f;
    float r, g, b, baseAlpha;
    if (timeState == DAY) { r = 1.0f; g = 0.95f; b = 0.5f; baseAlpha = 0.45f; glLineWidth(2.5f); }
    else                  { r = 0.85f; g = 0.95f; b = 1.0f; baseAlpha = 0.20f; glLineWidth(1.6f); }
    
    float yBot = riverLowerBankY(tx);
    float yTop = riverUpperBankY(tx);
    float yMid = (yBot + yTop) * 0.5f;
    float yR   = (yTop - yBot) * 0.48f;
    float step = (timeState == DAY) ? 0.35f : 0.25f; // denser lines at night to prevent "sticks"
    
    glBegin(GL_LINES);
    for (float ly = yMid - yR; ly <= yMid + yR; ly += step) {
        float f = fabsf(ly - yMid) / yR;
        float a = (1.0f - f) * baseAlpha;
        glColor4f(r, g, b, a);
        float wMult = (timeState == DAY) ? 3.2f : 1.8f;
        float wBase = (timeState == DAY) ? 1.0f : 0.6f;
        float wSin  = (timeState == DAY) ? 1.8f : 1.0f;
        float w = (1.0f - f) * wMult + wBase + sinf(ly * 32.0f) * wSin;
        if (w < 0.1f) w = 0.1f;
        glVertex2f(tx - w, ly);
        glVertex2f(tx + w, ly);
    }
    glEnd();
    glLineWidth(1.0f);
    glDisable(GL_BLEND);

    // Ripples
    glEnable(GL_LINE_SMOOTH);
    glLineWidth(2.5f);
    for (int i=0; i<RIPPLE_COUNT; i++) {
        if (ripples[i].active) {
            float alpha = ripples[i].alpha;
            if (timeState == DAY) glColor4f(0.75f, 0.95f, 1.0f, alpha);
            else                  glColor4f(0.4f, 0.6f, 0.8f, alpha * 0.7f);
            glBegin(GL_LINE_LOOP);
            float rx = ripples[i].radius;
            float ry = rx * 0.35f;
            for (int k = 0; k < 30; k++) {
                float a = 2.0f * M_PI * k / 30.0f;
                glVertex2f(ripples[i].x + rx * cosf(a), ripples[i].y + ry * sinf(a));
            }
            glEnd();
        }
    }
    glDisable(GL_LINE_SMOOTH);
    glLineWidth(1.0f);
    glDisable(GL_BLEND);

    // Rocks scattered on river banks
    for (int i=0; i<ROCK_COUNT; i++) {
        if (timeState == DAY) glColor3f(0.5f, 0.5f, 0.5f);
        else                  glColor3f(0.2f, 0.25f, 0.3f);
        float r = (rocks[i].type == 0) ? 0.3f : 0.45f;
        glPushMatrix();
        glTranslatef(rocks[i].x, rocks[i].y, 0.0f);
        glScalef(1.0f, 0.6f, 1.0f);
        fillCircle(0.0f, 0.0f, r);
        if (timeState == DAY) glColor3f(0.4f, 0.4f, 0.4f);
        else                  glColor3f(0.15f, 0.2f, 0.25f);
        fillCircle(-r*0.3f, -r*0.2f, r*0.6f);
        glPopMatrix();
    }
}

// =============================================================================
// Forest  —  upper-left triangle
// =============================================================================

// Draw a single tree at (x, y) with scale factor s
void drawTree(float x, float y, float s) {
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glScalef(s, s, 1.0f);   // scaling gives a sense of depth

    // Ground shadow
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    if (timeState == DAY) glColor4f(0.10f, 0.07f, 0.05f, 0.24f);
    else                  glColor4f(0.05f, 0.03f, 0.02f, 0.30f);
    float shX = (timeState == DAY) ? -0.5f : 0.5f;
    glPushMatrix();
    glTranslatef(shX, -4.5f, 0.0f);
    glScalef(1.1f, 0.35f, 1.0f);
    fillCircle(0.0f, 0.0f, 2.8f);
    glPopMatrix();
    glDisable(GL_BLEND);

    // Trunk
    glColor3f(0.42f, 0.24f, 0.07f);
    fillRect(-0.7f, -4.5f, 1.4f, 4.8f);
    glColor3f(0.28f, 0.14f, 0.03f);
    fillRect( 0.1f, -4.5f, 0.6f, 4.8f);   // shadow strip on right
    // Bark texture lines
    glColor3f(0.22f, 0.11f, 0.02f);
    drawLine(-0.45f, -4.0f, -0.45f, -1.8f, 1.0f);
    drawLine(-0.10f, -4.3f, -0.10f, -0.8f, 1.0f);
    drawLine( 0.35f, -3.8f,  0.35f, -2.2f, 1.0f);
    // Root flare bumps at base
    glColor3f(0.35f, 0.18f, 0.05f);
    fillCircle(-0.55f, -4.5f, 0.28f);
    fillCircle( 0.55f, -4.5f, 0.28f);

    // Sway offset based on tree X and global time
    float sway = sinf(globalAnimTime * 1.5f + x * 0.5f) * 0.15f;

    // Canopy (four overlapping circles — base dark layer)
    if (timeState == DAY) glColor3f(0.06f, 0.32f, 0.05f);
    else                  glColor3f(0.03f, 0.16f, 0.04f);
    fillCircle( sway + 0.0f,  0.2f, 2.4f);
    fillCircle( sway - 1.7f, -0.3f, 1.9f);
    fillCircle( sway + 1.6f, -0.3f, 1.9f);
    fillCircle( sway + 0.0f,  1.7f, 1.9f);

    // Highlight clusters (lighter green on upper-left of each lobe)
    if (timeState == DAY) glColor3f(0.14f, 0.46f, 0.12f);
    else                  glColor3f(0.05f, 0.22f, 0.06f);
    fillCircle( sway - 0.6f,  1.2f, 1.2f);
    fillCircle( sway - 2.1f,  0.4f, 0.9f);
    fillCircle( sway + 0.0f,  2.6f, 1.0f);

    // Small shadow blobs on the lower-right of canopy
    if (timeState == DAY) glColor3f(0.03f, 0.20f, 0.03f);
    else                  glColor3f(0.01f, 0.10f, 0.02f);
    fillCircle( sway + 1.3f, -0.8f, 1.1f);
    fillCircle( sway + 0.4f, -0.5f, 0.8f);

    glPopMatrix();
}

void drawForest() {
    // Ground wedge on the forest side
    if (timeState == DAY) glColor3f(0.16f, 0.52f, 0.14f);
    else                  glColor3f(0.06f, 0.22f, 0.08f);
    glBegin(GL_POLYGON);
        glVertex2f(-40.0f,  10.0f);
        glVertex2f( 40.0f,  10.0f);
        glVertex2f(-40.0f, -20.0f);
    glEnd();

    // Rolling hill silhouette along the top-left
    if (timeState == DAY) glColor3f(0.19f, 0.45f, 0.15f);
    else                  glColor3f(0.08f, 0.22f, 0.10f);
    glBegin(GL_POLYGON);
        glVertex2f(-40.0f, 10.0f);
        glVertex2f(-39.0f, 11.2f);
        glVertex2f(-36.5f, 12.8f);
        glVertex2f(-33.0f, 14.4f);
        glVertex2f(-29.5f, 15.0f);
        glVertex2f(-26.0f, 14.2f);
        glVertex2f(-22.5f, 12.6f);
        glVertex2f(-19.0f, 10.7f);
        glVertex2f(-16.0f, 10.0f);
    glEnd();

    // Grass tufts — three blades per tuft along the bank
    if (timeState == DAY) glColor3f(0.05f, 0.28f, 0.05f);
    else                  glColor3f(0.02f, 0.14f, 0.03f);
    glLineWidth(1.8f);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glBegin(GL_LINES);
    for (int i = 0; i < FOREST_BANK_GRASS_COUNT; i++) {
        float x = forestBankGrass[i].x,  y = forestBankGrass[i].y;
        float h = forestBankGrass[i].h,  s = forestBankGrass[i].spread;
        float l = forestBankGrass[i].lean;
        float sway = sinf(globalAnimTime * 2.0f + x * 0.2f) * 0.15f;
        glVertex2f(x, y); glVertex2f(x - s * 2.0f + l + sway, y + h * 0.92f);
        glVertex2f(x, y); glVertex2f(x + l * 0.35f + sway,    y + h * 1.10f);
        glVertex2f(x, y); glVertex2f(x + s * 2.0f + l + sway, y + h * 0.92f);
    }
    for (int i = 0; i < FOREST_PATCH_GRASS_COUNT; i++) {
        float x = forestPatchGrass[i].x,  y = forestPatchGrass[i].y;
        float h = forestPatchGrass[i].h,  s = forestPatchGrass[i].spread;
        float l = forestPatchGrass[i].lean;
        float sway = sinf(globalAnimTime * 2.0f + x * 0.2f) * 0.15f;
        glVertex2f(x, y); glVertex2f(x - s * 1.8f + l + sway, y + h * 0.84f);
        glVertex2f(x, y); glVertex2f(x + l * 0.35f + sway,    y + h * 1.06f);
        glVertex2f(x, y); glVertex2f(x + s * 1.8f + l + sway, y + h * 0.84f);
    }
    glEnd();
    glDisable(GL_LINE_SMOOTH);
    glLineWidth(1.0f);

    // Falling Leaves
    if (timeState == DAY) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glPointSize(4.0f);
        glBegin(GL_POINTS);
        for (int i = 0; i < LEAF_COUNT; i++) {
            if (leaves[i].active) {
                glColor4f(0.15f, 0.50f, 0.15f, leaves[i].alpha); // Leaf green
                glVertex2f(leaves[i].x, leaves[i].y);
            }
        }
        glEnd();
        glPointSize(1.0f);
        glDisable(GL_BLEND);
    }

    // Trees at different sizes to suggest depth (near = big, far = small)
    drawTree(-33.0f,  6.0f, 1.25f);
    drawTree(-23.0f,  6.5f, 0.95f);
    drawTree(-15.0f,  5.5f, 0.70f);
    drawTree(-30.0f, -1.0f, 0.90f);
    drawTree( -8.5f,  4.2f, 0.78f);
    drawTree(-20.5f, -2.8f, 0.72f);
    drawTree(-35.5f, -4.8f, 0.68f);
    // Extra Trees
    drawTree(-38.0f,  3.0f, 0.85f);
    drawTree(-28.0f,  7.5f, 1.10f);
    drawTree(-12.0f,  8.0f, 0.65f);
    drawTree(-35.0f, -10.0f, 0.75f);
    drawTree(-18.0f,  2.0f, 0.82f);
    drawTree( -5.0f,  12.0f, 0.60f);
}

// Fireflies — night only, rendered as glowing points
void drawFireflies() {
    if (timeState != NIGHT) return;

    // Glowing aura (pulsing)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for (int i = 0; i < FIREFLY_COUNT; i++) {
        // Different phase for each firefly using its index
        float pulse = (sinf(windmillAngle * 0.05f + i * 1.3f) + 1.0f) * 0.5f; // 0.0 to 1.0
        glColor4f(0.75f, 0.95f, 0.25f, 0.15f + 0.25f * pulse);
        fillCircle(fireflyX[i], fireflyY[i], 0.15f + 0.35f * pulse);
    }
    glDisable(GL_BLEND);

    // Bright core
    glColor3f(0.85f, 1.0f, 0.40f);
    glPointSize(3.0f);
    glBegin(GL_POINTS);
    for (int i = 0; i < FIREFLY_COUNT; i++) {
        glVertex2f(fireflyX[i], fireflyY[i]);
    }
    glEnd();
    glPointSize(1.0f);
}

// =============================================================================
// Fence  —  runs along the village side of the river with a gap in the middle
// =============================================================================

void drawFence() {
    const float xStart   = -34.0f;
    const float xEnd     =  38.0f;
    const float postHW   =   0.52f;        // half-width of each post
    const float postH    =   3.1f;         // height of each post
    const float spacing  =   3.6f;         // gap between post centres
    const float railHi   = postH * 0.74f;  // height of upper rail
    const float railLo   = postH * 0.40f;  // height of lower rail
    const float gapLeft  =  -4.5f;         // gate gap in the middle
    const float gapRight =   4.5f;

    // Draw post bodies
    for (float px = xStart; px <= xEnd + 0.1f; px += spacing) {
        if (px > gapLeft && px < gapRight) continue;   // skip the gate gap
        float py = fenceBase(px);

        // Ground shadow
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        if (timeState == DAY) glColor4f(0.10f, 0.07f, 0.05f, 0.24f);
        else                  glColor4f(0.05f, 0.03f, 0.02f, 0.30f);
        glPushMatrix();
        glTranslatef(px, py, 0.0f);
        glScalef(1.0f, 0.35f, 1.0f);
        fillCircle(0.0f, 0.0f, 0.85f);
        glPopMatrix();
        glDisable(GL_BLEND);

        glColor3f(0.42f, 0.24f, 0.07f);
        fillRect(px - postHW, py, postHW * 2.0f, postH);
        glColor3f(0.28f, 0.14f, 0.03f);
        fillRect(px + postHW * 0.42f, py, postHW * 0.58f, postH);  // right-side shadow
    }

    // Left rail segment (xStart to gapLeft)
    glColor3f(0.60f, 0.38f, 0.14f);
    drawLine(xStart, fenceBase(xStart) + railHi, gapLeft, fenceBase(gapLeft) + railHi, 6.0f);
    drawLine(xStart, fenceBase(xStart) + railLo, gapLeft, fenceBase(gapLeft) + railLo, 6.0f);
    glColor3f(0.78f, 0.58f, 0.28f);
    drawLine(xStart, fenceBase(xStart) + railHi + 0.22f, gapLeft, fenceBase(gapLeft) + railHi + 0.22f, 1.5f);
    drawLine(xStart, fenceBase(xStart) + railLo + 0.22f, gapLeft, fenceBase(gapLeft) + railLo + 0.22f, 1.5f);
    glColor3f(0.28f, 0.14f, 0.04f);
    drawLine(xStart, fenceBase(xStart) + railHi - 0.22f, gapLeft, fenceBase(gapLeft) + railHi - 0.22f, 1.5f);
    drawLine(xStart, fenceBase(xStart) + railLo - 0.22f, gapLeft, fenceBase(gapLeft) + railLo - 0.22f, 1.5f);

    // Right rail segment (gapRight to xEnd)
    glColor3f(0.60f, 0.38f, 0.14f);
    drawLine(gapRight, fenceBase(gapRight) + railHi, xEnd, fenceBase(xEnd) + railHi, 6.0f);
    drawLine(gapRight, fenceBase(gapRight) + railLo, xEnd, fenceBase(xEnd) + railLo, 6.0f);
    glColor3f(0.78f, 0.58f, 0.28f);
    drawLine(gapRight, fenceBase(gapRight) + railHi + 0.22f, xEnd, fenceBase(xEnd) + railHi + 0.22f, 1.5f);
    drawLine(gapRight, fenceBase(gapRight) + railLo + 0.22f, xEnd, fenceBase(xEnd) + railLo + 0.22f, 1.5f);
    glColor3f(0.28f, 0.14f, 0.04f);
    drawLine(gapRight, fenceBase(gapRight) + railHi - 0.22f, xEnd, fenceBase(xEnd) + railHi - 0.22f, 1.5f);
    drawLine(gapRight, fenceBase(gapRight) + railLo - 0.22f, xEnd, fenceBase(xEnd) + railLo - 0.22f, 1.5f);

    // Post top caps and left edge lines
    glColor3f(0.25f, 0.12f, 0.02f);
    for (float px = xStart; px <= xEnd + 0.1f; px += spacing) {
        if (px > gapLeft && px < gapRight) continue;
        float py  = fenceBase(px);
        float top = py + postH;
        drawLine(px - postHW, py,  px - postHW, top, 1.2f);  // left edge
        drawLine(px - postHW, top, px + postHW, top, 1.2f);  // top cap
    }
}

// =============================================================================
// Cat  —  day only, walks back and forth along the fence
// =============================================================================

void drawCat() {
    if (timeState != DAY) return;

    glPushMatrix();
    glTranslatef(catX, fenceBase(catX), 0.0f);   // move to current position

    // Ground shadow (subtle alpha-blended ovals under the cat)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.10f, 0.07f, 0.05f, 0.24f);
    glPushMatrix();
    glScalef(1.0f, 0.32f, 1.0f);
    fillCircle(-0.25f, -0.58f, 1.05f);
    fillCircle( 0.55f, -0.58f, 0.85f);
    glPopMatrix();
    glDisable(GL_BLEND);

    // Body (tall oval)
    glColor3f(0.92f, 0.50f, 0.10f);
    glPushMatrix();
    glScalef(1.0f, 1.35f, 1.0f);
    fillCircle(0.0f, 0.75f, 1.05f);
    glPopMatrix();

    // Tail (series of thick lines curling upward)
    glColor3f(0.78f, 0.36f, 0.05f);
    drawLine(-0.85f, 0.60f, -1.55f, 0.70f, 5.5f);
    drawLine(-1.55f, 0.70f, -2.00f, 1.30f, 5.5f);
    drawLine(-2.00f, 1.30f, -2.05f, 2.10f, 5.0f);
    drawLine(-2.05f, 2.10f, -1.70f, 2.70f, 4.5f);
    drawLine(-1.70f, 2.70f, -1.10f, 2.90f, 4.0f);
    glColor3f(0.97f, 0.62f, 0.18f);   // lighter highlight along centre of tail
    drawLine(-0.85f, 0.60f, -1.55f, 0.70f, 2.0f);
    drawLine(-1.55f, 0.70f, -2.00f, 1.30f, 2.0f);
    drawLine(-2.00f, 1.30f, -2.05f, 2.10f, 1.8f);
    drawLine(-2.05f, 2.10f, -1.70f, 2.70f, 1.6f);
    drawLine(-1.70f, 2.70f, -1.10f, 2.90f, 1.4f);

    // Belly patch (cream oval)
    glColor3f(0.97f, 0.87f, 0.68f);
    glPushMatrix();
    glScalef(0.62f, 0.80f, 1.0f);
    fillCircle(0.0f, 1.12f, 0.75f);
    glPopMatrix();

    // Head
    glColor3f(0.92f, 0.50f, 0.10f);
    glPushMatrix();
    glScalef(1.10f, 1.0f, 1.0f);
    fillCircle(0.0f, 2.55f, 0.78f);
    glPopMatrix();

    // Cheek puffs
    glColor3f(0.97f, 0.85f, 0.65f);
    fillCircle(-0.38f, 2.42f, 0.32f);
    fillCircle( 0.38f, 2.42f, 0.32f);

    // Ears (outer orange, inner pink)
    glColor3f(0.92f, 0.50f, 0.10f);
    glBegin(GL_TRIANGLES);
        glVertex2f(-0.62f, 3.05f); glVertex2f(-0.30f, 3.72f); glVertex2f( 0.05f, 3.08f);
        glVertex2f( 0.10f, 3.08f); glVertex2f( 0.42f, 3.72f); glVertex2f( 0.72f, 3.05f);
    glEnd();
    glColor3f(0.94f, 0.65f, 0.65f);
    glBegin(GL_TRIANGLES);
        glVertex2f(-0.52f, 3.10f); glVertex2f(-0.30f, 3.52f); glVertex2f(-0.02f, 3.12f);
        glVertex2f( 0.18f, 3.12f); glVertex2f( 0.42f, 3.52f); glVertex2f( 0.62f, 3.10f);
    glEnd();

    // Eyes (green iris, dark pupil, white shine dot)
    glColor3f(0.22f, 0.65f, 0.20f);
    fillCircle(-0.28f, 2.62f, 0.18f);
    fillCircle( 0.28f, 2.62f, 0.18f);
    glColor3f(0.05f, 0.05f, 0.05f);
    fillCircle(-0.28f, 2.62f, 0.10f);
    fillCircle( 0.28f, 2.62f, 0.10f);
    glColor3f(1.0f, 1.0f, 1.0f);
    fillCircle(-0.22f, 2.67f, 0.04f);
    fillCircle( 0.34f, 2.67f, 0.04f);

    // Nose
    glColor3f(0.90f, 0.38f, 0.38f);
    glBegin(GL_TRIANGLES);
        glVertex2f(-0.10f, 2.36f);
        glVertex2f( 0.10f, 2.36f);
        glVertex2f( 0.00f, 2.25f);
    glEnd();

    // Mouth
    glColor3f(0.30f, 0.08f, 0.05f);
    drawLine( 0.00f, 2.25f, -0.14f, 2.16f, 1.5f);
    drawLine( 0.00f, 2.25f,  0.14f, 2.16f, 1.5f);

    // Whiskers (3 per side)
    glColor3f(0.96f, 0.94f, 0.84f);
    drawLine(-0.10f, 2.38f, -1.10f, 2.52f, 1.2f);
    drawLine(-0.10f, 2.34f, -1.10f, 2.34f, 1.2f);
    drawLine(-0.10f, 2.30f, -1.10f, 2.16f, 1.2f);
    drawLine( 0.10f, 2.38f,  1.10f, 2.52f, 1.2f);
    drawLine( 0.10f, 2.34f,  1.10f, 2.34f, 1.2f);
    drawLine( 0.10f, 2.30f,  1.10f, 2.16f, 1.2f);

    // Front paws (flat ovals with toe lines)
    glColor3f(0.88f, 0.46f, 0.10f);
    glPushMatrix();
    glScalef(1.0f, 0.40f, 1.0f);
    fillCircle(-0.32f, 0.0f, 0.34f);
    fillCircle( 0.32f, 0.0f, 0.34f);
    glPopMatrix();
    glColor3f(0.68f, 0.28f, 0.04f);
    drawLine(-0.46f, 0.14f, -0.18f, 0.14f, 1.0f);
    drawLine( 0.18f, 0.14f,  0.46f, 0.14f, 1.0f);

    glPopMatrix();
}

// =============================================================================
// Deer  —  night only, walks along the forest river bank
// =============================================================================

void drawDeer() {
    if (timeState != NIGHT) return;

    glPushMatrix();
    glTranslatef(deerX, forestBase(deerX), 0.0f);
    glScalef(deerDir, 1.0f, 1.0f);   // flip horizontally when moving left

    // Ground shadow (subtle alpha-blended ovals under the deer)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.08f, 0.06f, 0.04f, 0.20f);
    glPushMatrix();
    glScalef(1.0f, 0.28f, 1.0f);
    fillCircle(0.10f, -9.85f, 1.85f);
    fillCircle(0.95f, -9.70f, 0.95f);
    glPopMatrix();
    glDisable(GL_BLEND);

    // Legs (four lines with darker hoof tips)
    glColor3f(0.55f, 0.27f, 0.07f);
    drawLine(-0.70f, 0.0f, -0.90f, -2.2f, 2.5f);
    drawLine(-0.30f, 0.0f, -0.40f, -2.2f, 2.5f);
    drawLine( 0.30f, 0.0f,  0.40f, -2.2f, 2.5f);
    drawLine( 0.80f, 0.0f,  0.95f, -2.2f, 2.5f);
    glColor3f(0.25f, 0.12f, 0.04f);
    drawLine(-0.90f, -2.2f, -0.90f, -2.6f, 2.5f);
    drawLine(-0.40f, -2.2f, -0.40f, -2.6f, 2.5f);
    drawLine( 0.40f, -2.2f,  0.40f, -2.6f, 2.5f);
    drawLine( 0.95f, -2.2f,  0.95f, -2.6f, 2.5f);

    // Body (wide horizontal oval)
    glColor3f(0.72f, 0.40f, 0.12f);
    glPushMatrix();
    glScalef(1.55f, 1.0f, 1.0f);
    fillCircle(0.0f, 0.85f, 1.10f);
    glPopMatrix();

    // Belly / chest patch
    glColor3f(0.88f, 0.74f, 0.52f);
    glPushMatrix();
    glScalef(0.70f, 0.85f, 1.0f);
    fillCircle(0.18f, 0.90f, 0.75f);
    glPopMatrix();

    // Neck
    glColor3f(0.72f, 0.40f, 0.12f);
    glPushMatrix();
    glTranslatef(0.85f, 1.55f, 0.0f);
    glScalef(0.55f, 1.0f, 1.0f);
    fillCircle(0.0f, 0.0f, 0.65f);
    glPopMatrix();

    // Head
    glPushMatrix();
    glTranslatef(1.30f, 2.60f, 0.0f);
    glScalef(1.15f, 0.90f, 1.0f);
    fillCircle(0.0f, 0.0f, 0.62f);
    glPopMatrix();

    // Snout
    glColor3f(0.82f, 0.60f, 0.40f);
    glPushMatrix();
    glTranslatef(1.80f, 2.42f, 0.0f);
    glScalef(1.10f, 0.65f, 1.0f);
    fillCircle(0.0f, 0.0f, 0.38f);
    glPopMatrix();

    // Nose and eye
    glColor3f(0.18f, 0.08f, 0.04f); fillCircle(2.14f, 2.38f, 0.13f);
    glColor3f(0.10f, 0.06f, 0.02f); fillCircle(1.52f, 2.72f, 0.15f);
    glColor3f(1.0f,  1.0f,  1.0f ); fillCircle(1.58f, 2.78f, 0.05f);

    // Ear (outer brown, inner pink)
    glColor3f(0.72f, 0.40f, 0.12f);
    glBegin(GL_TRIANGLES);
        glVertex2f(0.88f, 3.05f); glVertex2f(0.70f, 3.85f); glVertex2f(1.32f, 3.70f);
    glEnd();
    glColor3f(0.88f, 0.58f, 0.58f);
    glBegin(GL_TRIANGLES);
        glVertex2f(0.92f, 3.12f); glVertex2f(0.78f, 3.68f); glVertex2f(1.22f, 3.56f);
    glEnd();

    // White tail
    glColor3f(0.95f, 0.92f, 0.85f);
    fillCircle(-1.60f, 1.20f, 0.32f);

    // Antlers (branched lines from top of head)
    glColor3f(0.40f, 0.22f, 0.06f);
    drawLine( 0.90f, 3.18f,  0.50f, 4.30f, 2.0f);
    drawLine( 0.50f, 4.30f,  0.10f, 5.10f, 2.0f);
    drawLine( 0.50f, 4.30f,  0.70f, 5.00f, 2.0f);
    drawLine( 0.10f, 5.10f, -0.20f, 5.60f, 1.8f);
    drawLine( 0.10f, 5.10f,  0.30f, 5.55f, 1.8f);
    drawLine( 1.40f, 3.22f,  1.70f, 4.30f, 2.0f);
    drawLine( 1.70f, 4.30f,  1.40f, 5.10f, 2.0f);
    drawLine( 1.70f, 4.30f,  2.10f, 5.00f, 2.0f);
    drawLine( 1.40f, 5.10f,  1.20f, 5.60f, 1.8f);
    drawLine( 1.40f, 5.10f,  1.70f, 5.55f, 1.8f);

    glPopMatrix();
}

// =============================================================================
// Village  —  lower-right triangle
// =============================================================================

void drawVillage() {
    // Ground
    if (timeState == DAY) glColor3f(0.28f, 0.68f, 0.18f);
    else                  glColor3f(0.10f, 0.30f, 0.08f);
    glBegin(GL_TRIANGLES);
        glVertex2f( 40.0f,   5.0f);
        glVertex2f( 40.0f, -30.0f);
        glVertex2f(-40.0f, -30.0f);
    glEnd();

    // Dynamic Chickens roaming the village (behind houses layer)
    if (timeState == DAY) {
        float hX[3] = {10.0f, 22.0f, 32.0f};
        float hY[3] = {-15.0f, -14.0f, -10.0f}; // well below river, behind houses
        for(int i = 0; i < 3; i++) {
            float tOff = i * 2.3f;
            float mPhase = globalAnimTime * 0.4f + tOff;
            float cx = hX[i] + sinf(mPhase) * 2.5f;
            float cy = hY[i] + sinf(mPhase * 1.5f) * 0.3f;
            float dir = cosf(mPhase) > 0 ? 1.0f : -1.0f;
            
            bool isPecking = fmodf(globalAnimTime*1.5f + tOff, 4.0f) < 1.0f;
            float headAngle = isPecking ? -45.0f : 0.0f;

            glPushMatrix();
            glTranslatef(cx, cy, 0.0f);
            glScalef(dir, 0.8f, 1.0f); // slightly smaller since they are behind
            
            // shadow
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glColor4f(0.0f, 0.0f, 0.0f, 0.15f);
            glPushMatrix(); glTranslatef(0.0f, -0.4f, 0.0f); glScalef(1.0f, 0.3f, 1.0f); fillCircle(0.0f, 0.0f, 0.5f); glPopMatrix();
            glDisable(GL_BLEND);

            glColor3f(0.85f, 0.65f, 0.1f);
            drawLine(-0.2f, 0.0f, -0.2f, -0.4f, 1.5f);
            drawLine( 0.2f, 0.0f,  0.2f, -0.4f, 1.5f);

            glColor3f(0.95f, 0.95f, 0.95f);
            fillCircle(0.0f, 0.2f, 0.45f);
            glBegin(GL_TRIANGLES); glVertex2f(-0.3f, 0.2f); glVertex2f(-0.7f, 0.5f); glVertex2f(-0.2f, -0.1f); glEnd();
            
            glPushMatrix();
            glTranslatef(0.3f, 0.4f, 0.0f);
            glRotatef(headAngle, 0.0f, 0.0f, 1.0f);
            fillCircle(0.2f, 0.1f, 0.25f);
            glColor3f(0.9f, 0.6f, 0.1f);
            glBegin(GL_TRIANGLES); glVertex2f(0.4f, 0.15f); glVertex2f(0.6f, 0.05f); glVertex2f(0.35f, -0.05f); glEnd();
            glColor3f(0.1f, 0.1f, 0.1f);
            fillCircle(0.25f, 0.15f, 0.05f);
            glColor3f(0.8f, 0.1f, 0.1f);
            fillCircle(0.2f, 0.4f, 0.1f);
            fillCircle(0.1f, 0.35f, 0.08f);
            glPopMatrix();

            glPopMatrix();
        }
    }

    // Short grass (two blades per tuft)
    if (timeState == DAY) glColor3f(0.08f, 0.38f, 0.04f);
    else                  glColor3f(0.04f, 0.18f, 0.02f);
    glLineWidth(1.9f);
    glBegin(GL_LINES);
    for (int i = 0; i < VILLAGE_GRASS_COUNT; i++) {
        float x = villageGrass[i].x,  y = villageGrass[i].y;
        float h = villageGrass[i].h,  s = villageGrass[i].spread;
        float l = villageGrass[i].lean;
        float sway = sinf(globalAnimTime * 2.2f + x * 0.3f) * 0.1f;
        glVertex2f(x, y); glVertex2f(x - s + l + sway, y + h * 0.95f);
        glVertex2f(x, y); glVertex2f(x + s + l + sway, y + h * 0.95f);
    }
    glEnd();
    glLineWidth(1.0f);

    // Bush 1
    if (timeState == DAY) glColor3f(0.12f, 0.40f, 0.10f);
    else                  glColor3f(0.04f, 0.18f, 0.05f);
    fillCircle(6.0f, -26.0f, 1.5f);
    fillCircle(4.5f, -27.0f, 1.2f);
    fillCircle(7.5f, -27.5f, 1.3f);
    if (timeState == DAY) glColor3f(0.18f, 0.48f, 0.14f);
    else                  glColor3f(0.06f, 0.22f, 0.07f);
    fillCircle(5.8f, -25.2f, 0.8f);
    fillCircle(6.8f, -26.5f, 0.7f);

    // Bush 2 near House 2
    if (timeState == DAY) glColor3f(0.12f, 0.40f, 0.10f);
    else                  glColor3f(0.04f, 0.18f, 0.05f);
    fillCircle(30.0f, -10.0f, 1.6f);
    fillCircle(31.5f, -11.0f, 1.2f);
    if (timeState == DAY) glColor3f(0.18f, 0.48f, 0.14f);
    else                  glColor3f(0.06f, 0.22f, 0.07f);
    fillCircle(30.5f, -9.2f, 0.8f);

    // Stepping stones from House 1 to gap
    if (timeState == DAY) glColor3f(0.48f, 0.50f, 0.48f);
    else                  glColor3f(0.20f, 0.22f, 0.20f);
    float pathX[] = {12.0f, 9.0f, 6.0f, 2.5f, -1.0f};
    float pathY[] = {-28.5f, -29.0f, -28.2f, -27.0f, -25.0f};
    for (int i=0; i<5; i++) {
        glPushMatrix();
        glTranslatef(pathX[i], pathY[i], 0.0f);
        glScalef(1.2f, 0.5f, 1.0f);
        fillCircle(0.0f, 0.0f, 0.6f);
        glPopMatrix();
    }

    // Clothesline poles
    float poleBaseY = -27.8f;
    float poleH     =   7.4f;

    // Shadows for poles
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    if (timeState == DAY) glColor4f(0.10f, 0.07f, 0.05f, 0.24f);
    else                  glColor4f(0.05f, 0.03f, 0.02f, 0.30f);
    glPushMatrix(); glTranslatef(-8.2f, poleBaseY, 0.0f); glScalef(1.0f, 0.35f, 1.0f); fillCircle(0.0f, 0.0f, 0.85f); glPopMatrix();
    glPushMatrix(); glTranslatef( 3.8f, poleBaseY, 0.0f); glScalef(1.0f, 0.35f, 1.0f); fillCircle(0.0f, 0.0f, 0.85f); glPopMatrix();
    glDisable(GL_BLEND);

    if (timeState == DAY) glColor3f(0.44f, 0.27f, 0.11f);
    else                  glColor3f(0.27f, 0.18f, 0.08f);
    fillRect(-8.5f, poleBaseY, 0.60f, poleH);
    fillRect( 3.5f, poleBaseY, 0.60f, poleH);

    // Rope (slight sag from left pole to right)
    // Use DDA here so the algorithm is visibly demonstrated on-screen.
    glPointSize(2.0f);
    if (timeState == DAY) glColor3f(0.78f, 0.70f, 0.52f);
    else                  glColor3f(0.38f, 0.33f, 0.24f);
    drawLineDDA(-8.2f, poleBaseY + poleH - 0.45f,
                 3.8f, poleBaseY + poleH - 1.15f);
    glPointSize(1.0f);

    // Hanging clothes (4 pieces)
    if (timeState == DAY) glColor3f(0.72f, 0.25f, 0.22f);
    else                  glColor3f(0.48f, 0.16f, 0.16f);
    fillRect(-7.1f, -23.05f, 1.7f, 2.4f);

    if (timeState == DAY) glColor3f(0.24f, 0.50f, 0.72f);
    else                  glColor3f(0.15f, 0.29f, 0.42f);
    fillRect(-4.7f, -23.50f, 1.8f, 2.7f);

    if (timeState == DAY) glColor3f(0.86f, 0.78f, 0.62f);
    else                  glColor3f(0.58f, 0.52f, 0.42f);
    fillRect(-2.2f, -23.15f, 1.5f, 2.2f);

    if (timeState == DAY) glColor3f(0.66f, 0.34f, 0.62f);
    else                  glColor3f(0.40f, 0.22f, 0.36f);
    fillRect( 0.1f, -23.60f, 1.6f, 2.5f);

    // Clothes pegs
    glColor3f(0.24f, 0.14f, 0.07f);
    fillRect(-6.9f, -20.94f, 0.16f, 0.28f);
    fillRect(-5.7f, -20.96f, 0.16f, 0.28f);
    fillRect(-4.5f, -21.05f, 0.16f, 0.28f);
    fillRect(-3.3f, -21.07f, 0.16f, 0.28f);
    fillRect(-2.0f, -21.20f, 0.16f, 0.28f);
    fillRect(-0.9f, -21.22f, 0.16f, 0.28f);
    fillRect( 0.3f, -21.35f, 0.16f, 0.28f);
    fillRect( 1.4f, -21.37f, 0.16f, 0.28f);

    // House 1 (walls + roof triangle + door + window)
    // House 1 shadow
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    if (timeState == DAY) glColor4f(0.10f, 0.07f, 0.05f, 0.24f);
    else                  glColor4f(0.05f, 0.03f, 0.02f, 0.30f);
    glPushMatrix(); glTranslatef(14.5f, -28.0f, 0.0f); glScalef(1.0f, 0.25f, 1.0f); fillCircle(0.0f, 0.0f, 7.8f); glPopMatrix();
    glDisable(GL_BLEND);

    glColor3f(0.42f, 0.23f, 0.09f);
    fillRect(8.0f, -28.0f, 13.0f, 11.0f);

    glColor3f(0.28f, 0.13f, 0.04f);
    glBegin(GL_TRIANGLES);
        glVertex2f( 7.0f, -17.0f);
        glVertex2f(14.5f, -12.0f);
        glVertex2f(22.0f, -17.0f);
    glEnd();

    // House 1 Chimney
    glColor3f(0.35f, 0.20f, 0.10f); // dark brick color
    fillRect(10.5f, -15.0f, 1.2f, 3.5f);
    glColor3f(0.42f, 0.23f, 0.09f); // lighter top
    fillRect(10.3f, -11.5f, 1.6f, 0.4f);

    glColor3f(0.18f, 0.09f, 0.03f);
    fillRect(12.5f, -28.0f, 3.5f, 5.0f);

    if (timeState == DAY) glColor3f(0.82f, 0.76f, 0.58f);
    else                  glColor3f(0.95f, 0.85f, 0.30f);  // glowing yellow at night
    fillRect(9.5f, -22.0f, 2.5f, 2.5f);

    // Window light spill on ground at night
    if (timeState == NIGHT) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.95f, 0.85f, 0.30f, 0.20f);
        glBegin(GL_POLYGON);
            glVertex2f(9.5f, -22.0f);
            glVertex2f(12.0f, -22.0f);
            glVertex2f(13.5f, -28.0f);
            glVertex2f(8.5f, -28.0f);
        glEnd();
        glDisable(GL_BLEND);
    }

    // Inner glass shadow & wooden panes to give depth
    if (timeState == DAY) glColor3f(0.65f, 0.60f, 0.45f);
    else                  glColor3f(0.80f, 0.70f, 0.20f);
    fillRect(9.5f, -19.8f, 2.5f, 0.3f);  // top inner shadow
    fillRect(9.5f, -22.0f, 0.3f, 2.5f);  // left inner shadow

    glColor3f(0.20f, 0.10f, 0.04f);      // dark wood frame
    fillRect(10.6f, -22.0f, 0.3f, 2.5f); // vertical pane
    fillRect(9.5f, -21.0f, 2.5f, 0.3f);  // horizontal pane


    // House 2 (smaller, top-right corner)
    // House 2 shadow
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    if (timeState == DAY) glColor4f(0.10f, 0.07f, 0.05f, 0.24f);
    else                  glColor4f(0.05f, 0.03f, 0.02f, 0.30f);
    glPushMatrix(); glTranslatef(36.5f, -12.0f, 0.0f); glScalef(1.0f, 0.25f, 1.0f); fillCircle(0.0f, 0.0f, 3.2f); glPopMatrix();
    glDisable(GL_BLEND);

    glColor3f(0.55f, 0.28f, 0.10f);
    fillRect(34.0f, -12.0f, 5.0f, 6.0f);

    glColor3f(0.32f, 0.15f, 0.04f);
    glBegin(GL_TRIANGLES);
        glVertex2f(33.0f, -6.0f);
        glVertex2f(36.5f, -2.5f);
        glVertex2f(40.0f, -6.0f);
    glEnd();

    // House 2 Chimney
    glColor3f(0.40f, 0.22f, 0.12f);
    fillRect(34.2f, -4.8f, 0.9f, 2.5f);
    glColor3f(0.48f, 0.26f, 0.10f);
    fillRect(34.0f, -2.3f, 1.3f, 0.3f);

    if (timeState == DAY) glColor3f(0.82f, 0.76f, 0.58f);
    else                  glColor3f(0.95f, 0.85f, 0.30f);
    fillRect(34.5f, -10.0f, 1.8f, 1.8f);

    // Window light spill on ground at night
    if (timeState == NIGHT) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.95f, 0.85f, 0.30f, 0.20f);
        glBegin(GL_POLYGON);
            glVertex2f(34.5f, -10.0f);
            glVertex2f(36.3f, -10.0f);
            glVertex2f(37.3f, -12.0f);
            glVertex2f(33.5f, -12.0f);
        glEnd();
        glDisable(GL_BLEND);
    }

    // Inner shadow & wooden panes
    if (timeState == DAY) glColor3f(0.65f, 0.60f, 0.45f);
    else                  glColor3f(0.80f, 0.70f, 0.20f);
    fillRect(34.5f, -8.4f, 1.8f, 0.2f);
    fillRect(34.5f, -10.0f, 0.2f, 1.8f);

    glColor3f(0.20f, 0.10f, 0.04f);
    fillRect(35.3f, -10.0f, 0.2f, 1.8f);
    fillRect(34.5f, -9.2f, 1.8f, 0.2f);

    glColor3f(0.18f, 0.09f, 0.03f);
    fillRect(37.0f, -12.0f, 1.6f, 3.2f);

    // Windmill body (tapered tower)
    // Windmill shadow
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    if (timeState == DAY) glColor4f(0.10f, 0.07f, 0.05f, 0.24f);
    else                  glColor4f(0.05f, 0.03f, 0.02f, 0.30f);
    glPushMatrix(); glTranslatef(30.0f, -27.0f, 0.0f); glScalef(1.0f, 0.25f, 1.0f); fillCircle(0.0f, 0.0f, 5.5f); glPopMatrix();
    glDisable(GL_BLEND);

    glColor3f(0.52f, 0.38f, 0.22f);
    glBegin(GL_QUADS);
        glVertex2f(25.5f, -27.0f);
        glVertex2f(34.5f, -27.0f);
        glVertex2f(32.5f,  -8.0f);
        glVertex2f(27.5f,  -8.0f);
    glEnd();

    glColor3f(0.25f, 0.14f, 0.05f);
    fillRect(28.5f, -27.0f, 3.0f, 5.0f);  // door

    if (timeState == DAY) glColor3f(0.78f, 0.72f, 0.55f);
    else                  glColor3f(0.95f, 0.85f, 0.30f);
    fillRect(28.8f, -16.0f, 2.4f, 2.4f);  // window

    // Window light spill on windmill body at night
    if (timeState == NIGHT) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.95f, 0.85f, 0.30f, 0.15f);
        glBegin(GL_POLYGON);
            glVertex2f(28.8f, -16.0f);
            glVertex2f(31.2f, -16.0f);
            glVertex2f(33.5f, -28.0f);
            glVertex2f(26.5f, -28.0f);
        glEnd();
        glDisable(GL_BLEND);
    }

    // Inner shadow & wooden panes
    if (timeState == DAY) glColor3f(0.60f, 0.55f, 0.42f);
    else                  glColor3f(0.80f, 0.70f, 0.20f);
    fillRect(28.8f, -13.9f, 2.4f, 0.3f);
    fillRect(28.8f, -16.0f, 0.3f, 2.4f);

    glColor3f(0.20f, 0.10f, 0.04f);
    fillRect(29.85f, -16.0f, 0.3f, 2.4f);
    fillRect(28.8f, -14.95f, 2.4f, 0.3f);

    // Windmill blades — rotate around the pivot point using glRotatef
    float pivotX = 30.0f;
    float pivotY = -8.0f;
    glPushMatrix();
        glTranslatef(pivotX, pivotY, 0.0f);           // move origin to pivot
        glRotatef(windmillAngle, 0.0f, 0.0f, 1.0f);  // rotate blades
        glColor3f(0.82f, 0.79f, 0.68f);
        fillRect(-4.5f, -0.55f, 9.0f, 1.1f);    // horizontal blade
        fillRect(-0.55f, -4.5f, 1.1f, 9.0f);    // vertical blade
    glPopMatrix();

    // Pivot hub circle
    glColor3f(0.38f, 0.22f, 0.08f);
    fillCircle(pivotX, pivotY, 0.7f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for (int i=0; i<SMOKE_COUNT; i++) {
        if (smokeParticles[i].active) {
            float c = (timeState == DAY) ? 0.7f : 0.3f;
            glColor4f(c, c, c, smokeParticles[i].alpha);
            fillCircle(smokeParticles[i].x, smokeParticles[i].y, smokeParticles[i].size);
        }
    }
    glDisable(GL_BLEND);

    // Tree in front of houses / Left of windmill
    drawTree(22.0f, -25.5f, 1.25f);
}

// =============================================================================
// Birds  —  day only, fly left to right across the sky
// =============================================================================

void drawBirds() {
    if (timeState != DAY) return;
    glColor3f(0.1f, 0.1f, 0.1f); // Dark silhouettes
    glLineWidth(2.0f);
    for (int i = 0; i < BIRD_COUNT; i++) {
        float flap = sinf(globalAnimTime * 5.0f + i * 1.5f) * 0.4f;
        glBegin(GL_LINES);
        glVertex2f(birdX[i],       birdY[i]); glVertex2f(birdX[i] - 0.7f, birdY[i] + 0.3f + flap);
        glVertex2f(birdX[i] - 0.7f, birdY[i] + 0.3f + flap); glVertex2f(birdX[i] - 1.2f, birdY[i] + 0.2f + flap);
        glVertex2f(birdX[i],       birdY[i]); glVertex2f(birdX[i] + 0.7f, birdY[i] + 0.3f + flap);
        glVertex2f(birdX[i] + 0.7f, birdY[i] + 0.3f + flap); glVertex2f(birdX[i] + 1.2f, birdY[i] + 0.2f + flap);
        glEnd();
    }
    glLineWidth(1.0f);
}

// =============================================================================
// Timer  —  called every ~16ms, updates all animation state
// =============================================================================

void timer(int value) {
    globalAnimTime += 0.016f;

    // Auto day/night cycle every ~10 seconds
    dayNightCounter++;
    if (dayNightCounter >= DAY_NIGHT_INTERVAL) {
        dayNightCounter = 0;
        timeState = (timeState == DAY) ? NIGHT : DAY;
    }

    // Windmill rotates continuously
    windmillAngle += 1.2f;
    if (windmillAngle >= 360.0f) windmillAngle -= 360.0f;

    // Clouds drift across the sky (day only)
    for (int i = 0; i < CLOUD_COUNT; i++) {
        cloudOffset[i] += CLOUD_SPEED[i];
        if (cloudOffset[i] > 90.0f) cloudOffset[i] = 0.0f;
    }

    // Smoke particles update
    for (int i=0; i<SMOKE_COUNT; i++) {
        if (smokeParticles[i].active) {
            smokeParticles[i].y += 0.03f;
            smokeParticles[i].x += 0.015f + sinf(globalAnimTime * 2.0f + i) * 0.01f;
            smokeParticles[i].size += 0.012f;
            smokeParticles[i].alpha -= 0.003f;
            if (smokeParticles[i].alpha <= 0.0f) smokeParticles[i].active = false;
        } else {
            if (frand01() < 0.05f) { // spawn new
                smokeParticles[i].active = true;
                bool house1 = frand01() > 0.5f;
                smokeParticles[i].x = house1 ? 11.1f : 34.6f;
                smokeParticles[i].y = house1 ? -10.6f : -1.7f;
                smokeParticles[i].size = 0.3f;
                smokeParticles[i].alpha = 0.4f;
            }
        }
    }

    // Ripples update
    for (int i=0; i<RIPPLE_COUNT; i++) {
        if (ripples[i].active) {
            ripples[i].radius += 0.008f; // slowed down
            ripples[i].alpha -= 0.002f;  // slowed down
            if (ripples[i].alpha <= 0.0f) ripples[i].active = false;
        } else {
            if (frand01() < 0.001f) { // spawn new ripple
                ripples[i].active = true;
                ripples[i].x = -35.0f + frand01() * 70.0f;
                float yt = riverUpperBankY(ripples[i].x);
                float yb = riverLowerBankY(ripples[i].x);
                ripples[i].y = yb + frand01() * (yt - yb);
                ripples[i].radius = 0.1f;
                ripples[i].alpha = 0.6f;
            }
        }
    }

    // Leaves update
    if (timeState == DAY) {
        for (int i=0; i<LEAF_COUNT; i++) {
            if (leaves[i].active) {
                leaves[i].y -= 0.04f;
                leaves[i].x -= 0.02f + sinf(globalAnimTime * 3.0f + leaves[i].drift) * 0.03f;
                if (leaves[i].y < riverUpperBankY(leaves[i].x) + 2.0f) leaves[i].alpha -= 0.02f;
                if (leaves[i].alpha <= 0.0f) leaves[i].active = false;
            } else {
                if (frand01() < 0.03f) {
                    leaves[i].active = true;
                    float txs[] = {-33.0f, -23.0f, -15.0f, -30.0f, -8.5f, -20.5f, -35.5f};
                    float tys[] = { 6.0f,   6.5f,   5.5f,  -1.0f,  4.2f,  -2.8f,  -4.8f};
                    int ti = rand() % 7;
                    leaves[i].x = txs[ti] + (frand01()-0.5f)*3.0f;
                    leaves[i].y = tys[ti] + 1.0f + frand01()*2.0f;
                    leaves[i].alpha = 1.0f;
                    leaves[i].drift = frand01() * 10.0f;
                }
            }
        }
    } else {
        for (int i=0; i<LEAF_COUNT; i++) leaves[i].active = false;
    }

    // Shooting star update
    if (timeState == NIGHT) {
        if (shootingStar.active) {
            shootingStar.x -= 0.6f;
            shootingStar.y -= 0.3f;
            shootingStar.alpha -= 0.015f;
            if (shootingStar.alpha <= 0.0f || shootingStar.x < -50.0f) shootingStar.active = false;
        } else {
            if (frand01() < 0.003f) {
                shootingStar.active = true;
                shootingStar.x = 10.0f + frand01() * 30.0f;
                shootingStar.y = 20.0f + frand01() * 10.0f;
                shootingStar.len = 4.0f + frand01() * 4.0f;
                shootingStar.alpha = 1.0f;
            }
        }
    } else {
        shootingStar.active = false;
    }

    // Fireflies drift randomly within their zones (night only)
    if (timeState == NIGHT) {
        for (int i = 0; i < FIREFLY_COUNT; i++) {
            float dx = (frand01() * 2.0f - 1.0f) * 0.10f;
            float dy = (frand01() * 2.0f - 1.0f) * 0.10f;
            if (i < FIREFLY_VILLAGE_COUNT) {
                fireflyX[i] = clampf(fireflyX[i] + dx, FIREFLY_VILLAGE_X_MIN, FIREFLY_VILLAGE_X_MAX);
                fireflyY[i] = clampf(fireflyY[i] + dy, FIREFLY_VILLAGE_Y_MIN, FIREFLY_VILLAGE_Y_MAX);
            } else {
                float nx   = clampf(fireflyX[i] + dx, FIREFLY_FOREST_X_MIN, FIREFLY_FOREST_X_MAX);
                float minY = riverUpperBankY(nx) + 0.8f;
                if (minY < FOREST_Y_MIN) minY = FOREST_Y_MIN;
                fireflyX[i] = nx;
                fireflyY[i] = clampf(fireflyY[i] + dy, minY, FOREST_Y_MAX);
            }
        }
    }

    // Cat ping-pong translation (day only)
    if (timeState == DAY) {
        catX += catDir * CAT_SPEED;
        if (catX > CAT_X_MAX) { catX = CAT_X_MAX; catDir = -1.0f; }
        if (catX < CAT_X_MIN) { catX = CAT_X_MIN; catDir =  1.0f; }
    }

    // Deer ping-pong translation (night only)
    if (timeState == NIGHT) {
        deerX += deerDir * DEER_SPEED;
        if (deerX > DEER_X_MAX) { deerX = DEER_X_MAX; deerDir = -1.0f; }
        if (deerX < DEER_X_MIN) { deerX = DEER_X_MIN; deerDir =  1.0f; }
    }

    // Birds translation — moves right, resets off-screen to the left when it exits
    if (timeState == DAY) {
        for (int i = 0; i < BIRD_COUNT; i++) {
            birdX[i] += BIRD_SPEED;
            if (birdX[i] > 45.0f) birdX[i] = -50.0f;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

// =============================================================================
// Display  —  calls every draw function in the correct order
// =============================================================================

void display() {
    drawSky();        // background (must be first)
    drawSun();
    drawMoon();
    drawClouds();     // in front of sun/moon
    drawBirds();      // in front of clouds, behind everything else
    drawForest();     // drawn before river so river sits on top
    drawRiver();
    drawVillage();
    drawFence();
    drawCat();
    drawDeer();
    drawFireflies();  // drawn last so they appear on top
    glutSwapBuffers();
}

// =============================================================================
// Keyboard Handler
// =============================================================================

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case ' ':
            timeState = (timeState == DAY) ? NIGHT : DAY;
            glutPostRedisplay();
            break;
        case 27:   // ESC
        case 'q':
        case 'Q':
            exit(0);
            break;
    }
}

// =============================================================================
// Init & Main
// =============================================================================

void init() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(WORLD_LEFT, WORLD_RIGHT, WORLD_BOTTOM, WORLD_TOP);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    srand(1337);   // fixed seed so grass/firefly positions are the same every run
    initGrass();
    initFireflies();
    initStars();
    initDetails();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(1200, 900);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Zero Point — Forest Meets Village");

    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(16, timer, 0);
    glutMainLoop();
    return 0;
}

