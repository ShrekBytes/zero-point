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

// Three birds fly across the sky (day only)
float birdX[3] = { -45.0f, -55.0f, -62.0f };
float birdY[3] = {  22.0f,  18.5f,  25.0f };
const float BIRD_SPEED = 0.035f;

// Four clouds drift across the sky at different speeds (day only)
float cloudOffset[4]      = { 0.0f, 0.0f, 0.0f, 0.0f };
const float CLOUD_SPEED[4] = { 0.01f, 0.03f, 0.005f, 0.02f };

// Stars are only visible at night
const int STAR_COUNT = 60;
float starX[STAR_COUNT];
float starY[STAR_COUNT];

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

const int FOREST_BANK_GRASS_COUNT  = 42;
const int FOREST_PATCH_GRASS_COUNT = 84;
const int VILLAGE_GRASS_COUNT      = 74;
const int FIREFLY_COUNT            = 28;
const int FIREFLY_VILLAGE_COUNT    = 2;   // most fireflies stay in the forest

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

    int steps = (int)(fabs(dx) > fabs(dy) ? fabs(dx) : fabs(dy));
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
        glVertex2f(roundf(x), roundf(y));
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
    if (timeState == DAY) glClearColor(0.53f, 0.81f, 0.92f, 1.0f);  // light blue
    else                  glClearColor(0.04f, 0.10f, 0.16f, 1.0f);  // dark navy
    glClear(GL_COLOR_BUFFER_BIT);

    if (timeState == DAY) {
        // Four clouds drifting right to left at different speeds
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
        fillMidpointCircle(c4,        25.0f, 3.0f);
        fillMidpointCircle(c4 + 3.5f, 26.0f, 3.5f);
        fillMidpointCircle(c4 + 7.0f, 25.0f, 3.0f);
    } else {
        // Stars rendered as bright points across the upper sky
        glColor3f(0.95f, 0.95f, 0.85f);
        glPointSize(2.5f);
        glBegin(GL_POINTS);
        for (int i = 0; i < STAR_COUNT; i++)
            glVertex2f(starX[i], starY[i]);
        glEnd();
        glPointSize(1.0f);
    }
}

void drawSun() {
    if (timeState != DAY) return;
    glColor3f(1.0f, 0.95f, 0.0f);
    fillMidpointCircle(25.0f, 20.0f, 4.0f);
}

void drawMoon() {
    if (timeState != NIGHT) return;
    // Full circle, then mask off a portion to create a crescent shape
    glColor3f(0.94f, 0.94f, 0.82f);
    fillMidpointCircle(-25.0f, 20.0f, 4.0f);
    glColor3f(0.04f, 0.10f, 0.16f);   // same colour as sky to cut out the crescent
    fillMidpointCircle(-23.5f, 21.0f, 3.5f);
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
    glLineWidth(1.35f);
    glBegin(GL_LINES);
        glVertex2f( 40.0f, 10.0f); glVertex2f(-40.0f, -20.0f);
        glVertex2f( 40.0f,  5.0f); glVertex2f(-40.0f, -30.0f);
    glEnd();
    glLineWidth(1.0f);
}

// =============================================================================
// Forest  —  upper-left triangle
// =============================================================================

// Draw a single tree at (x, y) with scale factor s
void drawTree(float x, float y, float s) {
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glScalef(s, s, 1.0f);   // scaling gives a sense of depth

    // Trunk
    glColor3f(0.42f, 0.24f, 0.07f);
    fillRect(-0.7f, -4.5f, 1.4f, 4.8f);
    glColor3f(0.28f, 0.14f, 0.03f);
    fillRect( 0.1f, -4.5f, 0.6f, 4.8f);   // shadow strip on right

    // Canopy (four overlapping circles)
    if (timeState == DAY) glColor3f(0.12f, 0.48f, 0.10f);
    else                  glColor3f(0.06f, 0.26f, 0.07f);
    fillCircle( 0.0f,  0.2f, 2.4f);
    fillCircle(-1.7f, -0.3f, 1.9f);
    fillCircle( 1.6f, -0.3f, 1.9f);
    fillCircle( 0.0f,  1.7f, 1.9f);

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
        glVertex2f(x, y); glVertex2f(x - s * 2.0f + l, y + h * 0.92f);  // left blade
        glVertex2f(x, y); glVertex2f(x + l * 0.35f,    y + h * 1.10f);  // centre blade
        glVertex2f(x, y); glVertex2f(x + s * 2.0f + l, y + h * 0.92f);  // right blade
    }
    // Patch grass scattered through the forest interior
    for (int i = 0; i < FOREST_PATCH_GRASS_COUNT; i++) {
        float x = forestPatchGrass[i].x,  y = forestPatchGrass[i].y;
        float h = forestPatchGrass[i].h,  s = forestPatchGrass[i].spread;
        float l = forestPatchGrass[i].lean;
        glVertex2f(x, y); glVertex2f(x - s * 1.8f + l, y + h * 0.84f);
        glVertex2f(x, y); glVertex2f(x + l * 0.35f,    y + h * 1.06f);
        glVertex2f(x, y); glVertex2f(x + s * 1.8f + l, y + h * 0.84f);
    }
    glEnd();
    glDisable(GL_LINE_SMOOTH);
    glLineWidth(1.0f);

    // Trees at different sizes to suggest depth (near = big, far = small)
    drawTree(-33.0f,  6.0f, 1.25f);
    drawTree(-23.0f,  6.5f, 0.95f);
    drawTree(-15.0f,  5.5f, 0.70f);
    drawTree(-30.0f, -1.0f, 0.90f);
    drawTree( -8.5f,  4.2f, 0.78f);
    drawTree(-20.5f, -2.8f, 0.72f);
    drawTree(-35.5f, -4.8f, 0.68f);
}

// Fireflies — night only, rendered as glowing points
void drawFireflies() {
    if (timeState != NIGHT) return;
    glColor3f(0.75f, 0.95f, 0.25f);
    glPointSize(4.0f);
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

    // Short grass (two blades per tuft)
    if (timeState == DAY) glColor3f(0.08f, 0.38f, 0.04f);
    else                  glColor3f(0.04f, 0.18f, 0.02f);
    glLineWidth(1.9f);
    glBegin(GL_LINES);
    for (int i = 0; i < VILLAGE_GRASS_COUNT; i++) {
        float x = villageGrass[i].x,  y = villageGrass[i].y;
        float h = villageGrass[i].h,  s = villageGrass[i].spread;
        float l = villageGrass[i].lean;
        glVertex2f(x, y); glVertex2f(x - s + l, y + h * 0.95f);
        glVertex2f(x, y); glVertex2f(x + s + l, y + h * 0.95f);
    }
    glEnd();
    glLineWidth(1.0f);

    // Clothesline poles
    float poleBaseY = -27.8f;
    float poleH     =   7.4f;
    if (timeState == DAY) glColor3f(0.44f, 0.27f, 0.11f);
    else                  glColor3f(0.27f, 0.18f, 0.08f);
    fillRect(-8.5f, poleBaseY, 0.60f, poleH);
    fillRect( 3.5f, poleBaseY, 0.60f, poleH);

    // Rope (slight sag from left pole to right)
    if (timeState == DAY) glColor3f(0.70f, 0.62f, 0.46f);
    else                  glColor3f(0.44f, 0.40f, 0.30f);
    drawLine(-8.2f, poleBaseY + poleH - 0.2f,
              3.8f, poleBaseY + poleH - 0.9f, 1.8f);

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
    glColor3f(0.42f, 0.23f, 0.09f);
    fillRect(8.0f, -28.0f, 13.0f, 11.0f);

    glColor3f(0.28f, 0.13f, 0.04f);
    glBegin(GL_TRIANGLES);
        glVertex2f( 7.0f, -17.0f);
        glVertex2f(14.5f, -12.0f);
        glVertex2f(22.0f, -17.0f);
    glEnd();

    glColor3f(0.18f, 0.09f, 0.03f);
    fillRect(12.5f, -28.0f, 3.5f, 5.0f);

    if (timeState == DAY) glColor3f(0.82f, 0.76f, 0.58f);
    else                  glColor3f(0.95f, 0.85f, 0.30f);  // glowing yellow at night
    fillRect(9.5f, -22.0f, 2.5f, 2.5f);

    // House 2 (smaller, top-right corner)
    glColor3f(0.55f, 0.28f, 0.10f);
    fillRect(34.0f, -12.0f, 5.0f, 6.0f);

    glColor3f(0.32f, 0.15f, 0.04f);
    glBegin(GL_TRIANGLES);
        glVertex2f(33.0f, -6.0f);
        glVertex2f(36.5f, -2.5f);
        glVertex2f(40.0f, -6.0f);
    glEnd();

    if (timeState == DAY) glColor3f(0.82f, 0.76f, 0.58f);
    else                  glColor3f(0.95f, 0.85f, 0.30f);
    fillRect(34.5f, -10.0f, 1.8f, 1.8f);

    glColor3f(0.18f, 0.09f, 0.03f);
    fillRect(37.0f, -12.0f, 1.6f, 3.2f);

    // Windmill body (tapered tower)
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
}

// =============================================================================
// Birds  —  day only, fly left to right across the sky
// =============================================================================

void drawBirds() {
    if (timeState != DAY) return;

    glColor3f(0.08f, 0.08f, 0.08f);
    glLineWidth(1.8f);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    for (int i = 0; i < 3; i++) {
        // Each bird is a simple V-shape: left tip → centre → right tip
        glBegin(GL_LINE_STRIP);
            glVertex2f(birdX[i] - 1.3f, birdY[i] + 0.5f);  // left wing tip
            glVertex2f(birdX[i],        birdY[i]        );  // body centre
            glVertex2f(birdX[i] + 1.3f, birdY[i] + 0.5f);  // right wing tip
        glEnd();
    }

    glDisable(GL_LINE_SMOOTH);
    glLineWidth(1.0f);
}

// =============================================================================
// Timer  —  called every ~16ms, updates all animation state
// =============================================================================

void timer(int value) {
    // Windmill rotates continuously
    windmillAngle += 1.2f;
    if (windmillAngle >= 360.0f) windmillAngle -= 360.0f;

    // Clouds drift across the sky (day only)
    if (timeState == DAY) {
        for (int i = 0; i < 4; i++) {
            cloudOffset[i] += CLOUD_SPEED[i];
            if (cloudOffset[i] > 90.0f) cloudOffset[i] = 0.0f;
        }
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

    // Bird translation — moves right, resets off-screen to the left when it exits
    if (timeState == DAY) {
        for (int i = 0; i < 3; i++) {
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
    drawBirds();      // in front of sky, behind everything else
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

