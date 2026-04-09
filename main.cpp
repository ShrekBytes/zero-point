// ============================================================================
// main.cpp – Zero Point
// ============================================================================
// "Where Forest Meets Village: Time-Driven Environmental Simulation"
//
// Controls:
//   Spacebar  – Toggle day/night
//   ESC / q   – Quit
// ============================================================================

#include <GL/glut.h>
#include <cmath>
#include <cstdlib>

// ============================================================================
// Globals
// ============================================================================
enum TimeState { DAY, NIGHT };
TimeState timeState = DAY;

const float WORLD_LEFT   = -40.0f;
const float WORLD_RIGHT  =  40.0f;
const float WORLD_BOTTOM = -30.0f;
const float WORLD_TOP    =  30.0f;

// Cat animation globals
float catX   = -5.0f;
float catDir =  1.0f;
const float CAT_SPEED = 0.04f;
const float CAT_X_MIN = -18.0f;
const float CAT_X_MAX =  14.0f;

// Deer animation globals
float deerX   = -5.0f;
float deerDir =  1.0f;
const float DEER_SPEED = 0.03f;
const float DEER_X_MIN = -18.0f;
const float DEER_X_MAX =  8.0f;


float windmillAngle = 0.0f;

// Bird animation globals (Person 1)
float birdX[3]        = { -45.0f, -55.0f, -62.0f };  // start off-screen left
float birdY[3]        = {  22.0f,  18.5f,  25.0f };  // different sky heights
const float BIRD_SPEED = 0.035f;

// ============================================================================
// Forest + fireflies (night only)
// ============================================================================
static constexpr int FIREFLY_COUNT = 28;
static constexpr int FIREFLY_VILLAGE_COUNT = 2;
static float fireflyX[FIREFLY_COUNT];
static float fireflyY[FIREFLY_COUNT];

static constexpr float FIREFLY_FOREST_X_MIN = -38.0f;
static constexpr float FIREFLY_FOREST_X_MAX = 8.0f;
static constexpr float FIREFLY_VILLAGE_X_MIN = 12.0f;
static constexpr float FIREFLY_VILLAGE_X_MAX = 36.0f;
static constexpr float FIREFLY_VILLAGE_Y_MIN = -20.0f;
static constexpr float FIREFLY_VILLAGE_Y_MAX = -4.0f;

struct GrassTuft {
    float x;
    float y;
    float h;
    float spread;
    float lean;
};

static constexpr int FOREST_BANK_GRASS_COUNT = 42;
static constexpr int FOREST_PATCH_GRASS_COUNT = 84;
static constexpr int VILLAGE_GRASS_COUNT = 74;

static GrassTuft forestBankGrass[FOREST_BANK_GRASS_COUNT];
static GrassTuft forestPatchGrass[FOREST_PATCH_GRASS_COUNT];
static GrassTuft villageGrass[VILLAGE_GRASS_COUNT];

// Forest bounding box (upper-left triangle area, tuned to match sketch)
static constexpr float FOREST_X_MIN = -38.0f;
static constexpr float FOREST_X_MAX =  34.0f;
static constexpr float FOREST_Y_MIN = -16.0f;
static constexpr float FOREST_Y_MAX =  9.0f;

static float frand01() {
    return (float)rand() / (float)RAND_MAX;
}

static float clampf(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static float fireflyForestBankY(float x) {
    // Matches the river upper bank: (40,10) -> (-40,-20)
    return 10.0f + (x - 40.0f) * 0.375f;
}

static float villageRiverBankY(float x) {
    // Matches the river lower bank: (40,5) -> (-40,-30)
    return 5.0f + (x - 40.0f) * 0.4375f;
}

// Forward declaration for helpers used before their definitions.
void fillRect(float x, float y, float w, float h);

static void initFireflies() {
    // Keep almost all fireflies in the forest; allow only a couple in village.
    for (int i = 0; i < FIREFLY_COUNT; i++) {
        if (i < FIREFLY_VILLAGE_COUNT) {
            fireflyX[i] = FIREFLY_VILLAGE_X_MIN + frand01() * (FIREFLY_VILLAGE_X_MAX - FIREFLY_VILLAGE_X_MIN);
            fireflyY[i] = FIREFLY_VILLAGE_Y_MIN + frand01() * (FIREFLY_VILLAGE_Y_MAX - FIREFLY_VILLAGE_Y_MIN);
        } else {
            float x = FIREFLY_FOREST_X_MIN + frand01() * (FIREFLY_FOREST_X_MAX - FIREFLY_FOREST_X_MIN);
            float yMin = fireflyForestBankY(x) + 0.8f;
            if (yMin < FOREST_Y_MIN) yMin = FOREST_Y_MIN;
            if (yMin > FOREST_Y_MAX - 0.2f) yMin = FOREST_Y_MAX - 0.2f;
            fireflyX[i] = x;
            fireflyY[i] = yMin + frand01() * (FOREST_Y_MAX - yMin);
        }
    }
}

static void initGrass() {
    // Forest bank grass: random x + varied offsets to avoid visible straight rows.
    for (int i = 0; i < FOREST_BANK_GRASS_COUNT; i++) {
        float x = -37.5f + frand01() * 72.0f;
        float baseY = fireflyForestBankY(x) + 0.12f + frand01() * 0.40f;
        float leftWeight = 1.0f - ((x + 40.0f) / 80.0f);
        forestBankGrass[i] = {
            x,
            baseY,
            0.62f + 0.44f * leftWeight + frand01() * 0.26f,
            0.07f + frand01() * 0.06f,
            (frand01() - 0.5f) * 0.18f
        };
    }

    // Forest interior grass: rejection sample points above upper bank to keep natural scatter.
    for (int i = 0; i < FOREST_PATCH_GRASS_COUNT; i++) {
        float x = 0.0f;
        float y = 0.0f;
        for (int tries = 0; tries < 60; tries++) {
            float tx = -37.0f + frand01() * 72.0f;
            float ty = -14.0f + frand01() * 23.0f;
            if (ty > fireflyForestBankY(tx) + 1.05f) {
                x = tx;
                y = ty;
                break;
            }
        }
        if (x == 0.0f && y == 0.0f) {
            x = -37.0f + frand01() * 72.0f;
            y = fireflyForestBankY(x) + 1.25f + frand01() * 5.5f;
        }

        float leftWeight = 1.0f - ((x + 40.0f) / 80.0f);
        forestPatchGrass[i] = {
            x,
            y,
            0.56f + 0.48f * leftWeight + frand01() * 0.28f,
            0.08f + frand01() * 0.06f,
            (frand01() - 0.5f) * 0.14f
        };
    }

    // Village grass: random in village triangle under lower river bank.
    for (int i = 0; i < VILLAGE_GRASS_COUNT; i++) {
        float x = 0.0f;
        float y = 0.0f;
        for (int tries = 0; tries < 60; tries++) {
            float tx = -37.0f + frand01() * 74.0f;
            float upper = villageRiverBankY(tx) - 1.0f;
            float ty = -29.0f + frand01() * 20.0f;
            if (ty < upper) {
                x = tx;
                y = ty;
                break;
            }
        }
        if (x == 0.0f && y == 0.0f) {
            x = -37.0f + frand01() * 74.0f;
            y = -27.5f + frand01() * 12.0f;
        }

        villageGrass[i] = {
            x,
            y,
            0.76f + frand01() * 0.36f,
            0.40f + frand01() * 0.18f,
            (frand01() - 0.5f) * 0.12f
        };
    }
}

// ============================================================================
// ALGORITHM: DDA Line Drawing
// ============================================================================
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

    float xInc = dx / (float)steps;
    float yInc = dy / (float)steps;

    float x = x1;
    float y = y1;

    glBegin(GL_POINTS);
    for (int i = 0; i <= steps; i++) {
        glVertex2f(round(x), round(y));
        x += xInc;
        y += yInc;
    }
    glEnd();
}

// ============================================================================
// ALGORITHM: Bresenham's Line Drawing  [Person 1 — owns this]
//
// Works by tracking a cumulative error term (err) that decides whether
// to step in the minor axis each iteration — integer-only arithmetic,
// no floating-point needed.
//
//   dx, dy  = absolute delta in each axis
//   sx, sy  = step direction (+1 or -1) for each axis
//   err     = current accumulated error
//   e2      = doubled error used for the two threshold comparisons
//
// Used for: both edges of the diagonal river
// ============================================================================
void drawLineBresenham(int x1, int y1, int x2, int y2) {
    int dx  =  abs(x2 - x1);
    int dy  =  abs(y2 - y1);
    int sx  = (x1 < x2) ? 1 : -1;   // horizontal step direction
    int sy  = (y1 < y2) ? 1 : -1;   // vertical step direction
    int err =  dx - dy;              // initial error

    glBegin(GL_POINTS);
    while (true) {
        glVertex2i(x1, y1);
        if (x1 == x2 && y1 == y2) break;   // reached destination

        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; }   // step horizontally
        if (e2 <  dx) { err += dx; y1 += sy; }   // step vertically
    }
    glEnd();
}

// ============================================================================
// Filled circle using triangle fan
// ============================================================================
void fillCircle(float xc, float yc, float radius) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(xc, yc);
    int segments = 60;
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * M_PI * (float)i / (float)segments;
        float x = xc + radius * cos(angle);
        float y = yc + radius * sin(angle);
        glVertex2f(x, y);
    }
    glEnd();
}

// ============================================================================
// Environment Drawing
// ============================================================================

void drawSky() {
    if (timeState == DAY) {
        glClearColor(0.53f, 0.81f, 0.92f, 1.0f);   // light blue
    } else {
        glClearColor(0.04f, 0.10f, 0.16f, 1.0f);   // dark blue
    }
    glClear(GL_COLOR_BUFFER_BIT);
}

void drawSun() {
    if (timeState != DAY) return;
    glColor3f(1.0f, 1.0f, 0.0f);
    fillCircle(25.0f, 20.0f, 3.5f);
}

void drawMoon() {
    if (timeState != NIGHT) return;
    glColor3f(0.94f, 0.94f, 0.82f);
    fillCircle(-25.0f, 20.0f, 3.5f);
}

// ============================================================================
// RIVER  [Person 1 — owns this]
//
// The diagonal river runs from top-right to bottom-left across the scene.
// Two parallel Bresenham edges define the river boundaries; the body is
// a filled GL_POLYGON between them, with a shimmer strip in the centre.
//
// Upper edge: (40,10) -> (-40,-20)   ALGORITHM: Bresenham
// Lower edge: (40, 5) -> (-40,-30)   ALGORITHM: Bresenham
// ============================================================================
void drawRiver() {
    const float riverUpperRight = 10.0f;
    const float riverLowerRight = 5.0f;
    const float riverUpperLeft  = -20.0f;
    const float riverLowerLeft  = -30.0f;

    // Interpolate directly from the outer river edges so inner strip always follows.
    const float innerTopRatio = 0.30f;
    const float innerBotRatio = 0.70f;

    const float innerUpperRight = riverUpperRight + (riverLowerRight - riverUpperRight) * innerTopRatio;
    const float innerLowerRight = riverUpperRight + (riverLowerRight - riverUpperRight) * innerBotRatio;
    const float innerUpperLeft  = riverUpperLeft  + (riverLowerLeft  - riverUpperLeft)  * innerTopRatio;
    const float innerLowerLeft  = riverUpperLeft  + (riverLowerLeft  - riverUpperLeft)  * innerBotRatio;

    // --- Main river body: filled polygon between the two Bresenham edges ---
    if (timeState == DAY) glColor3f(0.20f, 0.52f, 0.80f);
    else                 glColor3f(0.07f, 0.20f, 0.34f);
    glBegin(GL_POLYGON);
        glVertex2f( 40.0f,  riverUpperRight);   // upper-right
        glVertex2f( 40.0f,  riverLowerRight);   // lower-right touches village boundary
        glVertex2f(-40.0f,  riverLowerLeft);    // lower-left touches village boundary
        glVertex2f(-40.0f,  riverUpperLeft);    // upper-left
    glEnd();

    // --- Centre shimmer strip: lighter blue highlight ---
    // Keep margins proportional to river width on both ends.
    if (timeState == DAY) glColor3f(0.38f, 0.68f, 0.92f);
    else                 glColor3f(0.13f, 0.30f, 0.46f);
    glBegin(GL_POLYGON);
        glVertex2f( 40.0f, innerUpperRight);
        glVertex2f( 40.0f, innerLowerRight);
        glVertex2f(-40.0f, innerLowerLeft);
        glVertex2f(-40.0f, innerUpperLeft);
    glEnd();

    // --- Lively streaks flowing along the river direction ---
    float flowShift = fmodf(windmillAngle * 0.085f, 10.0f);
    if (timeState == DAY) glColor3f(0.55f, 0.80f, 0.98f);
    else                 glColor3f(0.20f, 0.33f, 0.48f);
    glLineWidth(1.2f);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glBegin(GL_LINES);
    const float streakAnchors[] = {-37.0f, -28.0f, -19.0f, -10.0f, -1.0f, 8.0f, 17.0f, 26.0f, 35.0f};
    for (float a : streakAnchors) {
        float x1 = a - flowShift;
        if (x1 < -40.0f) x1 += 80.0f;
        float x2 = x1 + 6.5f;
        if (x2 > 40.0f) x2 = 40.0f;

        float up1 = 10.0f + (x1 - 40.0f) * 0.375f;
        float lo1 =  5.0f + (x1 - 40.0f) * 0.4375f;
        float up2 = 10.0f + (x2 - 40.0f) * 0.375f;
        float lo2 =  5.0f + (x2 - 40.0f) * 0.4375f;

        float y1 = (up1 + lo1) * 0.5f + 0.30f;
        float y2 = (up2 + lo2) * 0.5f + 0.30f;
        glVertex2f(x1, y1);
        glVertex2f(x2, y2);
    }
    glEnd();

    // Thin muddy strips at both river edges (inside water) to break the green-line look.
    if (timeState == DAY) glColor3f(0.46f, 0.34f, 0.21f);
    else                 glColor3f(0.28f, 0.20f, 0.13f);

    glBegin(GL_POLYGON);
        glVertex2f( 40.0f, 10.0f);
        glVertex2f( 40.0f,  9.45f);
        glVertex2f(-40.0f,-20.55f);
        glVertex2f(-40.0f,-20.0f);
    glEnd();

    glBegin(GL_POLYGON);
        glVertex2f( 40.0f, 5.0f);
        glVertex2f( 40.0f, 5.55f);
        glVertex2f(-40.0f,-29.45f);
        glVertex2f(-40.0f,-30.0f);
    glEnd();

    // --- River banks: smooth continuous lines (no dotted points) ---
    glColor3f(0.36f, 0.24f, 0.11f);
    glLineWidth(1.35f);
    glBegin(GL_LINES);
        glVertex2f( 40.0f, 10.0f); glVertex2f(-40.0f, -20.0f);
        glVertex2f( 40.0f,  5.0f); glVertex2f(-40.0f, -30.0f);
    glEnd();
    glDisable(GL_LINE_SMOOTH);
    glLineWidth(1.0f);
}

// ============================================================================
// Helper: Y position on the village-side fence baseline
// Village top boundary passes through (40,5) and (-40,-30), slope = 0.4375
// Offset -1.2 so fence stays safely inside village ground.
// ============================================================================
float fenceBase(float x) {
    return 5.0f + (x - 40.0f) * 0.4375f - 1.2f;
}

// ============================================================================
// Helper: Y position on the forest-side (upper) river bank
// Upper river bank passes through (40,10) and (-40,-20), slope = 0.375
// Offset +4.5 upward so deer stands on dry ground well above the bank
// ============================================================================
float forestBase(float x) {
    return 10.0f + (x - 40.0f) * 0.375f + 4.5f;
}

// ============================================================================
// FOREST ZONE – upper-left side of the river
// Includes: ground wedge, 3–4 scaled trees, and tall dense grass
// ============================================================================
static float riverUpperBankY(float x) {
    // Upper river edge passes through (40,10) and (-40,-20), slope = 0.375
    return 10.0f + (x - 40.0f) * 0.375f;
}

static void drawTree(float x, float y, float s) {
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glScalef(s, s, 1.0f); // TRANSFORM: Scaling (depth)

    // trunk
    glColor3f(0.42f, 0.24f, 0.07f);
    fillRect(-0.7f, -4.5f, 1.4f, 4.8f);
    glColor3f(0.28f, 0.14f, 0.03f);
    fillRect(0.10f, -4.5f, 0.60f, 4.8f);

    // canopy (rounded clusters)
    if (timeState == DAY) glColor3f(0.12f, 0.48f, 0.10f);
    else                 glColor3f(0.06f, 0.26f, 0.07f);
    fillCircle(0.0f, 0.2f, 2.4f);
    fillCircle(-1.7f, -0.3f, 1.9f);
    fillCircle( 1.6f, -0.3f, 1.9f);
    fillCircle(0.0f, 1.7f, 1.9f);

    glPopMatrix();
}

void drawForest() {
    // Ground wedge (forest side) — draw before the river so the river sits on top
    if (timeState == DAY) glColor3f(0.16f, 0.52f, 0.14f);
    else                 glColor3f(0.06f, 0.22f, 0.08f);

    glBegin(GL_POLYGON);
        glVertex2f(-40.0f, 10.0f);
        glVertex2f( 40.0f, 10.0f);
        glVertex2f(-40.0f,-20.0f);
    glEnd();

    // Simple hill silhouette on the top-left forest side.
    if (timeState == DAY) glColor3f(0.19f, 0.45f, 0.15f);
    else                 glColor3f(0.08f, 0.22f, 0.10f);
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

    // Tall, dense grass tufts in village style: left + center + right blades
    if (timeState == DAY) glColor3f(0.05f, 0.28f, 0.05f);
    else                 glColor3f(0.02f, 0.14f, 0.03f);

    glLineWidth(1.8f);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glBegin(GL_LINES);
    for (const auto& g : forestBankGrass) {
        float x = g.x;
        float baseY = g.y;
        float h = g.h;
        float spread = g.spread;
        float lean = g.lean;
        glVertex2f(x, baseY); glVertex2f(x - spread * 2.0f + lean, baseY + h * 0.92f);
        glVertex2f(x, baseY); glVertex2f(x + lean * 0.35f,        baseY + h * 1.10f);
        glVertex2f(x, baseY); glVertex2f(x + spread * 2.0f + lean, baseY + h * 0.92f);
    }
    for (const auto& g : forestPatchGrass) {
        float x = g.x;
        float y = g.y;
        float h = g.h;
        float spread = g.spread;
        float lean = g.lean;
        glVertex2f(x, y); glVertex2f(x - spread * 1.8f + lean, y + h * 0.84f);
        glVertex2f(x, y); glVertex2f(x + lean * 0.35f,          y + h * 1.06f);
        glVertex2f(x, y); glVertex2f(x + spread * 1.8f + lean, y + h * 0.84f);
    }
    glEnd();
    glDisable(GL_LINE_SMOOTH);
    glLineWidth(1.0f);

    // Trees (different sizes for depth) drawn after grass so grass stays behind trunks.
    drawTree(-33.0f,  6.0f, 1.25f);  // near/big
    drawTree(-23.0f,  6.5f, 0.95f);  // mid
    drawTree(-15.0f,  5.5f, 0.70f);  // far/small
    drawTree(-30.0f, -1.0f, 0.90f);  // mid
    drawTree(-8.5f,   4.2f, 0.78f);  // extra
    drawTree(-20.5f, -2.8f, 0.72f);  // extra
    drawTree(-35.5f, -4.8f, 0.68f);  // extra
}

// ============================================================================
// FIREFLIES – night only particle points drifting in forest bounding box
// ============================================================================
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

// ============================================================================
// Helper: GL_LINES wrapper with settable width (fence rails & cat details)
// ============================================================================
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

// ============================================================================
// Helper: filled rectangle (fence post bodies)
// ============================================================================
void fillRect(float x, float y, float w, float h) {
    glBegin(GL_QUADS);
    glVertex2f(x,   y);   glVertex2f(x+w, y);
    glVertex2f(x+w, y+h); glVertex2f(x,   y+h);
    glEnd();
}

// ============================================================================
// FENCE – village side of the river
//
//  Post bodies     → fillRect (GL_QUADS)
//  Post shading    → fillRect (GL_QUADS, darker strip for depth)
//  Rails           → drawLine (GL_LINES + glLineWidth) – crisp & thick
//  Rail highlight  → drawLine (lighter, thin)
//  Rail shadow     → drawLine (darker, thin)
//  Post caps/edges → drawLine (GL_LINES)
//
//  GAP: a gap is left in the middle of the fence (around x=0)
// ============================================================================
void drawFence() {
    const float xStart      = -34.0f;
    const float xEnd        =  38.0f;
    const float postHW      =  0.52f;   // half-width of each post
    const float postHeight  =  3.1f;    // height of post above baseline
    const float postSpacing =  3.6f;    // centre-to-centre spacing
    const float railHi      =  postHeight * 0.74f;
    const float railLo      =  postHeight * 0.40f;

    // Gap parameters – centred around x = 0
    const float gapCentre = 0.0f;
    const float gapHalf   = 4.5f;   // half-width of gap (total gap ~9 units)
    const float gapLeft   = gapCentre - gapHalf;   // -4.5
    const float gapRight  = gapCentre + gapHalf;   //  4.5

    // 1. Filled post bodies – skip posts inside the gap
    for (float px = xStart; px <= xEnd + 0.1f; px += postSpacing) {
        // Skip posts that fall inside the gap
        if (px > gapLeft && px < gapRight) continue;

        float py = fenceBase(px);
        // main body – medium dark brown
        glColor3f(0.42f, 0.24f, 0.07f);
        fillRect(px - postHW, py, postHW * 2.0f, postHeight);
        // right-side shadow strip for depth
        glColor3f(0.28f, 0.14f, 0.03f);
        fillRect(px + postHW * 0.42f, py, postHW * 0.58f, postHeight);
    }

    // 2. Rails – split into left segment and right segment to create gap
    // Left rail segment: xStart to gapLeft
    glColor3f(0.60f, 0.38f, 0.14f);
    drawLine(xStart, fenceBase(xStart)+railHi, gapLeft, fenceBase(gapLeft)+railHi, 6.0f);
    drawLine(xStart, fenceBase(xStart)+railLo, gapLeft, fenceBase(gapLeft)+railLo, 6.0f);
    // Left rail highlight
    glColor3f(0.78f, 0.58f, 0.28f);
    drawLine(xStart, fenceBase(xStart)+railHi+0.22f, gapLeft, fenceBase(gapLeft)+railHi+0.22f, 1.5f);
    drawLine(xStart, fenceBase(xStart)+railLo+0.22f, gapLeft, fenceBase(gapLeft)+railLo+0.22f, 1.5f);
    // Left rail shadow
    glColor3f(0.28f, 0.14f, 0.04f);
    drawLine(xStart, fenceBase(xStart)+railHi-0.22f, gapLeft, fenceBase(gapLeft)+railHi-0.22f, 1.5f);
    drawLine(xStart, fenceBase(xStart)+railLo-0.22f, gapLeft, fenceBase(gapLeft)+railLo-0.22f, 1.5f);

    // Right rail segment: gapRight to xEnd
    glColor3f(0.60f, 0.38f, 0.14f);
    drawLine(gapRight, fenceBase(gapRight)+railHi, xEnd, fenceBase(xEnd)+railHi, 6.0f);
    drawLine(gapRight, fenceBase(gapRight)+railLo, xEnd, fenceBase(xEnd)+railLo, 6.0f);
    // Right rail highlight
    glColor3f(0.78f, 0.58f, 0.28f);
    drawLine(gapRight, fenceBase(gapRight)+railHi+0.22f, xEnd, fenceBase(xEnd)+railHi+0.22f, 1.5f);
    drawLine(gapRight, fenceBase(gapRight)+railLo+0.22f, xEnd, fenceBase(xEnd)+railLo+0.22f, 1.5f);
    // Right rail shadow
    glColor3f(0.28f, 0.14f, 0.04f);
    drawLine(gapRight, fenceBase(gapRight)+railHi-0.22f, xEnd, fenceBase(xEnd)+railHi-0.22f, 1.5f);
    drawLine(gapRight, fenceBase(gapRight)+railLo-0.22f, xEnd, fenceBase(xEnd)+railLo-0.22f, 1.5f);

    // 3. Post top caps and left edges – skip posts inside the gap
    glColor3f(0.25f, 0.12f, 0.02f);
    for (float px = xStart; px <= xEnd + 0.1f; px += postSpacing) {
        if (px > gapLeft && px < gapRight) continue;

        float py  = fenceBase(px);
        float top = py + postHeight;
        drawLine(px - postHW, py,  px - postHW, top,  1.2f);  // left edge
        drawLine(px - postHW, top, px + postHW, top,  1.2f);  // top cap
    }
}

// ============================================================================
// CAT  (day only) – translated along fence baseline
//
//  Filled shapes   → fillCircle / GL_TRIANGLES
//  Tail            → drawLine (GL_LINES, thick) – curves up behind body
//  Whiskers/Mouth  → drawLine (GL_LINES, thin)
//  TRANSFORM       → glTranslatef (translation along fence)
// ============================================================================
void drawCat() {
    if (timeState != DAY) return;

    float baseY = fenceBase(catX);
    glPushMatrix();
    glTranslatef(catX, baseY, 0.0f);  // TRANSFORM: Translation

    // --- Body: tall oval for sitting posture (drawn first so tail overlaps at base) ---
    glColor3f(0.92f, 0.50f, 0.10f);
    glPushMatrix();
    glScalef(1.0f, 1.35f, 1.0f);
    fillCircle(0.0f, 0.75f, 1.05f);
    glPopMatrix();

    // --- Tail: emerges from left side of body, sweeps up and curls over ---
    // Draw as a series of thick filled circles along the arc path for a smooth, attached look
    // Arc: base at body-left (-0.85, 0.6), sweeps left-down then up then curls right
    glColor3f(0.78f, 0.36f, 0.05f);
    drawLine(-0.85f, 0.60f, -1.55f, 0.70f, 5.5f);   // base – emerges left from body
    drawLine(-1.55f, 0.70f, -2.00f, 1.30f, 5.5f);   // curves upward
    drawLine(-2.00f, 1.30f, -2.05f, 2.10f, 5.0f);   // rises up
    drawLine(-2.05f, 2.10f, -1.70f, 2.70f, 4.5f);   // curls inward at top
    drawLine(-1.70f, 2.70f, -1.10f, 2.90f, 4.0f);   // tip hooks right over body
    // Lighter highlight stripe along tail centre
    glColor3f(0.97f, 0.62f, 0.18f);
    drawLine(-0.85f, 0.60f, -1.55f, 0.70f, 2.0f);
    drawLine(-1.55f, 0.70f, -2.00f, 1.30f, 2.0f);
    drawLine(-2.00f, 1.30f, -2.05f, 2.10f, 1.8f);
    drawLine(-2.05f, 2.10f, -1.70f, 2.70f, 1.6f);
    drawLine(-1.70f, 2.70f, -1.10f, 2.90f, 1.4f);

    // --- Cream belly patch ---
    glColor3f(0.97f, 0.87f, 0.68f);
    glPushMatrix();
    glScalef(0.62f, 0.80f, 1.0f);
    fillCircle(0.0f, 1.12f, 0.75f);
    glPopMatrix();

    // --- Head: slightly wider than tall ---
    glColor3f(0.92f, 0.50f, 0.10f);
    glPushMatrix();
    glScalef(1.10f, 1.0f, 1.0f);
    fillCircle(0.0f, 2.55f, 0.78f);
    glPopMatrix();

    // --- Cheek puffs (cream) ---
    glColor3f(0.97f, 0.85f, 0.65f);
    fillCircle(-0.38f, 2.42f, 0.32f);
    fillCircle( 0.38f, 2.42f, 0.32f);

    // --- Ears: tall pointed triangles ---
    glColor3f(0.92f, 0.50f, 0.10f);
    glBegin(GL_TRIANGLES);
        glVertex2f(-0.62f, 3.05f);
        glVertex2f(-0.30f, 3.72f);
        glVertex2f( 0.05f, 3.08f);

        glVertex2f( 0.10f, 3.08f);
        glVertex2f( 0.42f, 3.72f);
        glVertex2f( 0.72f, 3.05f);
    glEnd();
    // pink inner ears
    glColor3f(0.94f, 0.65f, 0.65f);
    glBegin(GL_TRIANGLES);
        glVertex2f(-0.52f, 3.10f);
        glVertex2f(-0.30f, 3.52f);
        glVertex2f(-0.02f, 3.12f);

        glVertex2f( 0.18f, 3.12f);
        glVertex2f( 0.42f, 3.52f);
        glVertex2f( 0.62f, 3.10f);
    glEnd();

    // --- Eyes: green iris, dark pupil, white shine ---
    glColor3f(0.22f, 0.65f, 0.20f);
    fillCircle(-0.28f, 2.62f, 0.18f);
    fillCircle( 0.28f, 2.62f, 0.18f);
    glColor3f(0.05f, 0.05f, 0.05f);
    fillCircle(-0.28f, 2.62f, 0.10f);
    fillCircle( 0.28f, 2.62f, 0.10f);
    glColor3f(1.0f, 1.0f, 1.0f);
    fillCircle(-0.22f, 2.67f, 0.04f);
    fillCircle( 0.34f, 2.67f, 0.04f);

    // --- Nose: small pink triangle ---
    glColor3f(0.90f, 0.38f, 0.38f);
    glBegin(GL_TRIANGLES);
        glVertex2f(-0.10f, 2.36f);
        glVertex2f( 0.10f, 2.36f);
        glVertex2f( 0.00f, 2.25f);
    glEnd();

    // --- Mouth: two short lines from nose tip ---
    glColor3f(0.30f, 0.08f, 0.05f);
    drawLine( 0.00f, 2.25f,  -0.14f, 2.16f,  1.5f);
    drawLine( 0.00f, 2.25f,   0.14f, 2.16f,  1.5f);

    // --- Whiskers: 3 per side, fanning from cheeks ---
    glColor3f(0.96f, 0.94f, 0.84f);
    drawLine(-0.10f, 2.38f,  -1.10f, 2.52f,  1.2f);
    drawLine(-0.10f, 2.34f,  -1.10f, 2.34f,  1.2f);
    drawLine(-0.10f, 2.30f,  -1.10f, 2.16f,  1.2f);
    drawLine( 0.10f, 2.38f,   1.10f, 2.52f,  1.2f);
    drawLine( 0.10f, 2.34f,   1.10f, 2.34f,  1.2f);
    drawLine( 0.10f, 2.30f,   1.10f, 2.16f,  1.2f);

    // --- Front paws: two flat ovals ---
    glColor3f(0.88f, 0.46f, 0.10f);
    glPushMatrix();
    glScalef(1.0f, 0.40f, 1.0f);
    fillCircle(-0.32f, 0.0f, 0.34f);
    fillCircle( 0.32f, 0.0f, 0.34f);
    glPopMatrix();
    // toe lines
    glColor3f(0.68f, 0.28f, 0.04f);
    drawLine(-0.46f, 0.14f,  -0.18f, 0.14f,  1.0f);
    drawLine( 0.18f, 0.14f,   0.46f, 0.14f,  1.0f);

    glPopMatrix();
}

// ============================================================================
// DEER  (night only) – translated along forest-side bank baseline
//
//  Body/Head       → fillCircle (scaled ovals)
//  Legs            → drawLine (GL_LINES, thick)
//  Antlers         → drawLine (GL_LINES, medium)
//  Belly patch     → fillCircle (cream/white)
//  Tail            → fillCircle (white)
//  Eyes/Nose       → fillCircle (small)
//  TRANSFORM       → glTranslatef (translation along bank)
// ============================================================================
void drawDeer() {
    if (timeState != NIGHT) return;

    float baseY = forestBase(deerX);
    glPushMatrix();
    glTranslatef(deerX, baseY, 0.0f);      // TRANSFORM: Translation
    glScalef(deerDir, 1.0f, 1.0f);         // TRANSFORM: Mirror when moving left (deerDir = -1)

    // --- Legs: four legs, slightly angled ---
    glColor3f(0.55f, 0.27f, 0.07f);
    // back legs
    drawLine(-0.70f, 0.0f, -0.90f, -2.2f, 2.5f);
    drawLine(-0.30f, 0.0f, -0.40f, -2.2f, 2.5f);
    // front legs
    drawLine( 0.30f, 0.0f,  0.40f, -2.2f, 2.5f);
    drawLine( 0.80f, 0.0f,  0.95f, -2.2f, 2.5f);
    // hooves (darker tips)
    glColor3f(0.25f, 0.12f, 0.04f);
    drawLine(-0.90f, -2.2f, -0.90f, -2.6f, 2.5f);
    drawLine(-0.40f, -2.2f, -0.40f, -2.6f, 2.5f);
    drawLine( 0.40f, -2.2f,  0.40f, -2.6f, 2.5f);
    drawLine( 0.95f, -2.2f,  0.95f, -2.6f, 2.5f);

    // --- Body: wide horizontal oval ---
    glColor3f(0.72f, 0.40f, 0.12f);
    glPushMatrix();
    glScalef(1.55f, 1.0f, 1.0f);
    fillCircle(0.0f, 0.85f, 1.10f);
    glPopMatrix();

    // --- Belly/chest patch: lighter cream ---
    glColor3f(0.88f, 0.74f, 0.52f);
    glPushMatrix();
    glScalef(0.70f, 0.85f, 1.0f);
    fillCircle(0.18f, 0.90f, 0.75f);
    glPopMatrix();

    // --- Neck: thick angled oval connecting body to head ---
    glColor3f(0.72f, 0.40f, 0.12f);
    glPushMatrix();
    glTranslatef(0.85f, 1.55f, 0.0f);
    glScalef(0.55f, 1.0f, 1.0f);
    fillCircle(0.0f, 0.0f, 0.65f);
    glPopMatrix();

    // --- Head: rounded oval, slightly tilted forward ---
    glColor3f(0.72f, 0.40f, 0.12f);
    glPushMatrix();
    glTranslatef(1.30f, 2.60f, 0.0f);
    glScalef(1.15f, 0.90f, 1.0f);
    fillCircle(0.0f, 0.0f, 0.62f);
    glPopMatrix();

    // --- Snout: slightly lighter protruding muzzle ---
    glColor3f(0.82f, 0.60f, 0.40f);
    glPushMatrix();
    glTranslatef(1.80f, 2.42f, 0.0f);
    glScalef(1.10f, 0.65f, 1.0f);
    fillCircle(0.0f, 0.0f, 0.38f);
    glPopMatrix();

    // --- Nose: dark oval at tip of snout ---
    glColor3f(0.18f, 0.08f, 0.04f);
    fillCircle(2.14f, 2.38f, 0.13f);

    // --- Eye: dark with white shine ---
    glColor3f(0.10f, 0.06f, 0.02f);
    fillCircle(1.52f, 2.72f, 0.15f);
    glColor3f(1.0f, 1.0f, 1.0f);
    fillCircle(1.58f, 2.78f, 0.05f);

    // --- Ear: rounded triangle shape ---
    glColor3f(0.72f, 0.40f, 0.12f);
    glBegin(GL_TRIANGLES);
        glVertex2f(0.88f, 3.05f);
        glVertex2f(0.70f, 3.85f);
        glVertex2f(1.32f, 3.70f);
    glEnd();
    // inner ear (pink)
    glColor3f(0.88f, 0.58f, 0.58f);
    glBegin(GL_TRIANGLES);
        glVertex2f(0.92f, 3.12f);
        glVertex2f(0.78f, 3.68f);
        glVertex2f(1.22f, 3.56f);
    glEnd();

    // --- White tail ---
    glColor3f(0.95f, 0.92f, 0.85f);
    fillCircle(-1.60f, 1.20f, 0.32f);

    // --- Antlers: branched lines from top of head ---
    glColor3f(0.40f, 0.22f, 0.06f);
    // left antler base
    drawLine(0.90f, 3.18f,  0.50f, 4.30f, 2.0f);
    // left antler branch 1
    drawLine(0.50f, 4.30f,  0.10f, 5.10f, 2.0f);
    // left antler branch 2
    drawLine(0.50f, 4.30f,  0.70f, 5.00f, 2.0f);
    // left antler branch 3 (top tine)
    drawLine(0.10f, 5.10f, -0.20f, 5.60f, 1.8f);
    drawLine(0.10f, 5.10f,  0.30f, 5.55f, 1.8f);

    // right antler base
    drawLine(1.40f, 3.22f,  1.70f, 4.30f, 2.0f);
    // right antler branch 1
    drawLine(1.70f, 4.30f,  1.40f, 5.10f, 2.0f);
    // right antler branch 2
    drawLine(1.70f, 4.30f,  2.10f, 5.00f, 2.0f);
    // right antler branch 3 (top tine)
    drawLine(1.40f, 5.10f,  1.20f, 5.60f, 1.8f);
    drawLine(1.40f, 5.10f,  1.70f, 5.55f, 1.8f);

    glPopMatrix();
}

// ============================================================================
// VILLAGE
// ============================================================================
void drawVillage() {

    // Village ground
    if (timeState == DAY)
        glColor3f(0.28f, 0.68f, 0.18f);
    else
        glColor3f(0.10f, 0.30f, 0.08f);

    glBegin(GL_TRIANGLES);
        glVertex2f( 40.0f,  5.0f);
        glVertex2f( 40.0f, -30.0f);
        glVertex2f(-40.0f, -30.0f);
    glEnd();

    // Grass
    if (timeState == DAY)
        glColor3f(0.08f, 0.38f, 0.04f);
    else
        glColor3f(0.04f, 0.18f, 0.02f);

    glLineWidth(1.9f);
    glBegin(GL_LINES);
    for (const auto& g : villageGrass) {
        float x = g.x;
        float y = g.y;
        float h = g.h;
        float s = g.spread;
        float lean = g.lean;
        glVertex2f(x, y); glVertex2f(x - s + lean, y + h * 0.95f);
        glVertex2f(x, y); glVertex2f(x + s + lean, y + h * 0.95f);
    }
    glEnd();
    glLineWidth(1.0f);

    // Clothesline setup: two poles + rope + hanging clothes.
    float poleLeftX  = -8.5f;
    float poleRightX =  3.5f;
    float poleBaseY  = -27.8f;
    float poleH      =  7.4f;

    if (timeState == DAY) glColor3f(0.44f, 0.27f, 0.11f);
    else                 glColor3f(0.27f, 0.18f, 0.08f);
    fillRect(poleLeftX,  poleBaseY, 0.60f, poleH);
    fillRect(poleRightX, poleBaseY, 0.60f, poleH);

    // Rope with a slight sag in the middle.
    if (timeState == DAY) glColor3f(0.70f, 0.62f, 0.46f);
    else                 glColor3f(0.44f, 0.40f, 0.30f);
    drawLine(poleLeftX + 0.30f,  poleBaseY + poleH - 0.2f,
             poleRightX + 0.30f, poleBaseY + poleH - 0.9f, 1.8f);

    // Hanging cloth pieces.
    if (timeState == DAY) glColor3f(0.72f, 0.25f, 0.22f);
    else                 glColor3f(0.48f, 0.16f, 0.16f);
    fillRect(-7.1f, -23.05f, 1.7f, 2.4f);

    if (timeState == DAY) glColor3f(0.24f, 0.50f, 0.72f);
    else                 glColor3f(0.15f, 0.29f, 0.42f);
    fillRect(-4.7f, -23.50f, 1.8f, 2.7f);

    if (timeState == DAY) glColor3f(0.86f, 0.78f, 0.62f);
    else                 glColor3f(0.58f, 0.52f, 0.42f);
    fillRect(-2.2f, -23.15f, 1.5f, 2.2f);

    if (timeState == DAY) glColor3f(0.66f, 0.34f, 0.62f);
    else                 glColor3f(0.40f, 0.22f, 0.36f);
    fillRect(0.1f, -23.60f, 1.6f, 2.5f);

    // Cloth clips/pegs for small details.
    glColor3f(0.24f, 0.14f, 0.07f);
    fillRect(-6.9f, -20.94f, 0.16f, 0.28f);
    fillRect(-5.7f, -20.96f, 0.16f, 0.28f);
    fillRect(-4.5f, -21.05f, 0.16f, 0.28f);
    fillRect(-3.3f, -21.07f, 0.16f, 0.28f);
    fillRect(-2.0f, -21.20f, 0.16f, 0.28f);
    fillRect(-0.9f, -21.22f, 0.16f, 0.28f);
    fillRect(0.3f,  -21.35f, 0.16f, 0.28f);
    fillRect(1.4f,  -21.37f, 0.16f, 0.28f);

    // House 1
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

    if (timeState == DAY)
        glColor3f(0.82f, 0.76f, 0.58f);
    else
        glColor3f(0.95f, 0.85f, 0.30f);
    fillRect(9.5f, -22.0f, 2.5f, 2.5f);

    // House 2
    glColor3f(0.55f, 0.28f, 0.10f);
    fillRect(34.0f, -12.0f, 5.0f, 6.0f);

    glColor3f(0.32f, 0.15f, 0.04f);
    glBegin(GL_TRIANGLES);
        glVertex2f(33.0f, -6.0f);
        glVertex2f(36.5f, -2.5f);
        glVertex2f(40.0f, -6.0f);
    glEnd();

    if (timeState == DAY)
        glColor3f(0.82f, 0.76f, 0.58f);
    else
        glColor3f(0.95f, 0.85f, 0.30f);
    fillRect(34.5f, -10.0f, 1.8f, 1.8f);

    glColor3f(0.18f, 0.09f, 0.03f);
    fillRect(37.0f, -12.0f, 1.6f, 3.2f);

    // Windmill body
    glColor3f(0.52f, 0.38f, 0.22f);
    glBegin(GL_QUADS);
        glVertex2f(25.5f, -27.0f);
        glVertex2f(34.5f, -27.0f);
        glVertex2f(32.5f,  -8.0f);
        glVertex2f(27.5f,  -8.0f);
    glEnd();

    glColor3f(0.25f, 0.14f, 0.05f);
    fillRect(28.5f, -27.0f, 3.0f, 5.0f);

    if (timeState == DAY)
        glColor3f(0.78f, 0.72f, 0.55f);
    else
        glColor3f(0.95f, 0.85f, 0.30f);
    fillRect(28.8f, -16.0f, 2.4f, 2.4f);

    // Windmill blades
    float pivotX = 30.0f;
    float pivotY = -8.0f;

    glPushMatrix();
        glTranslatef(pivotX, pivotY, 0.0f);
        glRotatef(windmillAngle, 0.0f, 0.0f, 1.0f);

        glColor3f(0.82f, 0.79f, 0.68f);
        float bw = 0.55f;
        float bl = 4.5f;

        fillRect(-bl, -bw, bl * 2.0f, bw * 2.0f);
        fillRect(-bw, -bl, bw * 2.0f, bl * 2.0f);
    glPopMatrix();

    glColor3f(0.38f, 0.22f, 0.08f);
    fillCircle(pivotX, pivotY, 0.7f);
}

// ============================================================================
// BIRDS  [Person 1 -- owns this]
//
// Three birds drawn as V-shapes using GL_LINE_STRIP, visible day only.
// Each bird is a left-wing tip -> centre -> right-wing tip polyline.
//
// TRANSFORM: Translation -- birdX[] increments every frame in timer().
//            When a bird drifts past the right edge it resets off the
//            left edge so it loops continuously across the sky.
// ============================================================================
void drawBirds() {
    if (timeState != DAY) return;

    glColor3f(0.08f, 0.08f, 0.08f);
    glLineWidth(1.8f);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    for (int i = 0; i < 3; i++) {
        float bx = birdX[i];
        float by = birdY[i];
        float ws = 1.3f;   // half wing-span
        float wh = 0.5f;   // wing dip height

        // V-shape: left tip -> body centre -> right tip
        glBegin(GL_LINE_STRIP);
            glVertex2f(bx - ws, by + wh);   // left wing tip
            glVertex2f(bx,      by);         // centre (body)
            glVertex2f(bx + ws, by + wh);   // right wing tip
        glEnd();
    }

    glDisable(GL_LINE_SMOOTH);
    glLineWidth(1.0f);
}

// ============================================================================
// Timer – animates cat back and forth (day only)
//        animates deer back and forth (night only)
// ============================================================================
void timer(int /*value*/) {
    windmillAngle += 1.2f;
    if (windmillAngle >= 360.0f) windmillAngle -= 360.0f;

    // Fireflies drift (night only) — constrained by side-specific zones.
    if (timeState == NIGHT) {
        for (int i = 0; i < FIREFLY_COUNT; i++) {
            float dx = (frand01() * 2.0f - 1.0f) * 0.10f; // ~1–2 pixels in this world scale
            float dy = (frand01() * 2.0f - 1.0f) * 0.10f;
            if (i < FIREFLY_VILLAGE_COUNT) {
                fireflyX[i] = clampf(fireflyX[i] + dx, FIREFLY_VILLAGE_X_MIN, FIREFLY_VILLAGE_X_MAX);
                fireflyY[i] = clampf(fireflyY[i] + dy, FIREFLY_VILLAGE_Y_MIN, FIREFLY_VILLAGE_Y_MAX);
            } else {
                float nx = clampf(fireflyX[i] + dx, FIREFLY_FOREST_X_MIN, FIREFLY_FOREST_X_MAX);
                float ny = fireflyY[i] + dy;
                float minY = fireflyForestBankY(nx) + 0.8f;
                if (minY < FOREST_Y_MIN) minY = FOREST_Y_MIN;
                fireflyX[i] = nx;
                fireflyY[i] = clampf(ny, minY, FOREST_Y_MAX);
            }
        }
    }

    if (timeState == DAY) {
        catX += catDir * CAT_SPEED;
        if (catX > CAT_X_MAX) { catX = CAT_X_MAX; catDir = -1.0f; }
        if (catX < CAT_X_MIN) { catX = CAT_X_MIN; catDir =  1.0f; }
    } else {
        deerX += deerDir * DEER_SPEED;
        if (deerX > DEER_X_MAX) { deerX = DEER_X_MAX; deerDir = -1.0f; }
        if (deerX < DEER_X_MIN) { deerX = DEER_X_MIN; deerDir =  1.0f; }
    }
    // TRANSFORM: Translation -- move birds left to right (Person 1)
    if (timeState == DAY) {
        for (int i = 0; i < 3; i++) {
            birdX[i] += BIRD_SPEED;
            if (birdX[i] > 45.0f)    // flew off right edge
                birdX[i] = -50.0f;   // reset off-screen to the left
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

// ============================================================================
// Display
// ============================================================================
void display() {
    drawSky();
    drawSun();
    drawMoon();
    drawBirds();       // Draw after sun/moon so birds appear in front
    drawForest();
    drawRiver();
    drawVillage();
    drawFence();
    drawCat();
    drawDeer();
    drawFireflies();
    glutSwapBuffers();
}

// ============================================================================
// Keyboard
// ============================================================================
void keyboard(unsigned char key, int /*x*/, int /*y*/) {
    switch (key) {
        case ' ':
            timeState = (timeState == DAY) ? NIGHT : DAY;
            glutPostRedisplay();
            break;
        case 27:
        case 'q':
        case 'Q':
            exit(0);
            break;
    }
}

// ============================================================================
// Init & Main
// ============================================================================
void init() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(WORLD_LEFT, WORLD_RIGHT, WORLD_BOTTOM, WORLD_TOP);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    srand(1337);
    initGrass();
    initFireflies();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(1200, 900);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Zero Point - Forest Meets Village");

    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(16, timer, 0);
    glutMainLoop();
    return 0;
}