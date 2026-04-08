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


// ============================================================================
// Bird globals
// ============================================================================
struct Bird {
    float x, y;
    float speed;
    float scale;
};

Bird birds[3] = {
    { -35.0f, 23.0f, 0.025f, 1.0f  },
    { -15.0f, 27.0f, 0.018f, 0.80f },
    {   5.0f, 21.0f, 0.030f, 0.90f }
};

float birdFlapAngle = 0.0f;          // oscillates 0–360 to drive sin()
const float BIRD_RESET_X = -42.0f;   // x position to reset to after exiting right

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
// ALGORITHM: Bresenham's Line Drawing
// ============================================================================
void drawLineBresenham(int x1, int y1, int x2, int y2) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    glBegin(GL_POINTS);
    while (true) {
        glVertex2i(x1, y1);
        if (x1 == x2 && y1 == y2) break;

        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 < dx)  { err += dx; y1 += sy; }
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
// CLOUDS – day only, three fluffy clouds built from overlapping circles
// ============================================================================
void drawCloud(float cx, float cy, float s) {
    // s = overall scale factor
    glColor3f(1.0f, 1.0f, 1.0f);
    fillCircle(cx,          cy,        s * 2.0f);
    fillCircle(cx + s*2.2f, cy,        s * 1.8f);
    fillCircle(cx - s*2.2f, cy,        s * 1.6f);
    fillCircle(cx + s*1.0f, cy + s*1.2f, s * 1.6f);
    fillCircle(cx - s*0.8f, cy + s*1.0f, s * 1.4f);
    fillCircle(cx + s*3.5f, cy + s*0.4f, s * 1.2f);
}

void drawClouds() {
    if (timeState != DAY) return;
    drawCloud(-28.0f, 18.0f, 1.3f);
    drawCloud(  2.0f, 25.0f, 1.0f);
    drawCloud(-10.0f, 14.0f, 0.85f);
}

// ============================================================================
// BIRDS – day only, V-shape with animated flapping wings
// TRANSFORM: Translation – each bird translates across the sky each frame
// ============================================================================
void drawSingleBird(float bx, float by, float s, float flapOffset) {
    // Wing tip Y offset: oscillates up and down via sin
    float wTipY = sinf(flapOffset) * s * 0.9f;

    // Wing span half-width
    float wSpan = s * 1.8f;
    // Wing mid-point (slight elbow)
    float wMidX = s * 0.9f;
    float wMidY = sinf(flapOffset) * s * 0.3f;  // mid follows flap slightly

    glColor3f(0.12f, 0.12f, 0.12f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
        // Left wing: centre -> elbow -> tip
        glVertex2f(bx,           by);
        glVertex2f(bx - wMidX,   by + wMidY);
        glVertex2f(bx - wMidX,   by + wMidY);
        glVertex2f(bx - wSpan,   by + wTipY);
        // Right wing: centre -> elbow -> tip
        glVertex2f(bx,           by);
        glVertex2f(bx + wMidX,   by + wMidY);
        glVertex2f(bx + wMidX,   by + wMidY);
        glVertex2f(bx + wSpan,   by + wTipY);
    glEnd();
    glLineWidth(1.0f);
}

void drawBirds() {
    if (timeState != DAY) return;

    // Each bird gets a slightly different phase so they don't all flap in sync
    float phases[3] = { birdFlapAngle, birdFlapAngle + 1.2f, birdFlapAngle + 2.4f };

    for (int i = 0; i < 3; i++) {
        drawSingleBird(birds[i].x, birds[i].y, birds[i].scale, phases[i]);
    }
}

// ============================================================================
// Stage 3B: River
// ============================================================================
void drawRiver() {
    // River body – diagonal from top-right to bottom-left
    glColor3f(0.0f, 0.5f, 0.8f);
    glBegin(GL_POLYGON);
        glVertex2f( 40.0f,  10.0f);
        glVertex2f( 40.0f,   5.0f);
        glVertex2f(-40.0f, -30.0f);
        glVertex2f(-40.0f, -20.0f);
    glEnd();

    // Sandy banks using DDA lines
    glColor3f(0.76f, 0.70f, 0.50f);
    drawLineDDA( 40.0f, 10.5f, -40.0f, -20.5f);   // ALGORITHM: DDA – sandy upper bank
    drawLineDDA( 40.0f,  4.5f, -40.0f, -29.5f);   // ALGORITHM: DDA – sandy lower bank

    // River banks using DDA lines
    glColor3f(0.0f, 0.3f, 0.5f);
    drawLineDDA( 40.0f, 11.0f, -40.0f, -21.0f);   // ALGORITHM: DDA – upper bank
    drawLineDDA( 40.0f,  4.0f, -40.0f, -29.0f);   // ALGORITHM: DDA – lower bank
}

// ============================================================================
// Helper: Y position on the village-side fence baseline
// ============================================================================
float fenceBase(float x) {
    return 4.0f + (x - 40.0f) * 0.4125f - 2.5f;
}

// ============================================================================
// Helper: Y position on the forest-side (upper) river bank
// ============================================================================
float forestBase(float x) {
    return 10.0f + (x - 40.0f) * 0.375f + 4.5f;
}

// ============================================================================
// Helper: GL_LINES wrapper with settable width
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
// Helper: filled rectangle
// ============================================================================
void fillRect(float x, float y, float w, float h) {
    glBegin(GL_QUADS);
    glVertex2f(x,   y);   glVertex2f(x+w, y);
    glVertex2f(x+w, y+h); glVertex2f(x,   y+h);
    glEnd();
}

// ============================================================================
// FENCE
// ============================================================================
void drawFence() {
    const float xStart      = -38.0f;
    const float xEnd        =  38.0f;
    const float postHW      =  0.52f;
    const float postHeight  =  3.1f;
    const float postSpacing =  3.6f;
    const float railHi      =  postHeight * 0.74f;
    const float railLo      =  postHeight * 0.40f;

    const float gapCentre = 0.0f;
    const float gapHalf   = 4.5f;
    const float gapLeft   = gapCentre - gapHalf;
    const float gapRight  = gapCentre + gapHalf;

    // 1. Filled post bodies
    for (float px = xStart; px <= xEnd + 0.1f; px += postSpacing) {
        if (px > gapLeft && px < gapRight) continue;
        float py = fenceBase(px);
        glColor3f(0.42f, 0.24f, 0.07f);
        fillRect(px - postHW, py, postHW * 2.0f, postHeight);
        glColor3f(0.28f, 0.14f, 0.03f);
        fillRect(px + postHW * 0.42f, py, postHW * 0.58f, postHeight);
    }

    // 2. Rails – left segment
    glColor3f(0.60f, 0.38f, 0.14f);
    drawLine(xStart, fenceBase(xStart)+railHi, gapLeft, fenceBase(gapLeft)+railHi, 6.0f);
    drawLine(xStart, fenceBase(xStart)+railLo, gapLeft, fenceBase(gapLeft)+railLo, 6.0f);
    glColor3f(0.78f, 0.58f, 0.28f);
    drawLine(xStart, fenceBase(xStart)+railHi+0.22f, gapLeft, fenceBase(gapLeft)+railHi+0.22f, 1.5f);
    drawLine(xStart, fenceBase(xStart)+railLo+0.22f, gapLeft, fenceBase(gapLeft)+railLo+0.22f, 1.5f);
    glColor3f(0.28f, 0.14f, 0.04f);
    drawLine(xStart, fenceBase(xStart)+railHi-0.22f, gapLeft, fenceBase(gapLeft)+railHi-0.22f, 1.5f);
    drawLine(xStart, fenceBase(xStart)+railLo-0.22f, gapLeft, fenceBase(gapLeft)+railLo-0.22f, 1.5f);

    // Rails – right segment
    glColor3f(0.60f, 0.38f, 0.14f);
    drawLine(gapRight, fenceBase(gapRight)+railHi, xEnd, fenceBase(xEnd)+railHi, 6.0f);
    drawLine(gapRight, fenceBase(gapRight)+railLo, xEnd, fenceBase(xEnd)+railLo, 6.0f);
    glColor3f(0.78f, 0.58f, 0.28f);
    drawLine(gapRight, fenceBase(gapRight)+railHi+0.22f, xEnd, fenceBase(xEnd)+railHi+0.22f, 1.5f);
    drawLine(gapRight, fenceBase(gapRight)+railLo+0.22f, xEnd, fenceBase(xEnd)+railLo+0.22f, 1.5f);
    glColor3f(0.28f, 0.14f, 0.04f);
    drawLine(gapRight, fenceBase(gapRight)+railHi-0.22f, xEnd, fenceBase(xEnd)+railHi-0.22f, 1.5f);
    drawLine(gapRight, fenceBase(gapRight)+railLo-0.22f, xEnd, fenceBase(xEnd)+railLo-0.22f, 1.5f);

    // 3. Post top caps and left edges
    glColor3f(0.25f, 0.12f, 0.02f);
    for (float px = xStart; px <= xEnd + 0.1f; px += postSpacing) {
        if (px > gapLeft && px < gapRight) continue;
        float py  = fenceBase(px);
        float top = py + postHeight;
        drawLine(px - postHW, py,  px - postHW, top,  1.2f);
        drawLine(px - postHW, top, px + postHW, top,  1.2f);
    }
}

// ============================================================================
// CAT  (day only)
// ============================================================================
void drawCat() {
    if (timeState != DAY) return;

    float baseY = fenceBase(catX);
    glPushMatrix();
    glTranslatef(catX, baseY, 0.0f);  // TRANSFORM: Translation

    glColor3f(0.92f, 0.50f, 0.10f);
    glPushMatrix();
    glScalef(1.0f, 1.35f, 1.0f);
    fillCircle(0.0f, 0.75f, 1.05f);
    glPopMatrix();

    glColor3f(0.78f, 0.36f, 0.05f);
    drawLine(-0.85f, 0.60f, -1.55f, 0.70f, 5.5f);
    drawLine(-1.55f, 0.70f, -2.00f, 1.30f, 5.5f);
    drawLine(-2.00f, 1.30f, -2.05f, 2.10f, 5.0f);
    drawLine(-2.05f, 2.10f, -1.70f, 2.70f, 4.5f);
    drawLine(-1.70f, 2.70f, -1.10f, 2.90f, 4.0f);
    glColor3f(0.97f, 0.62f, 0.18f);
    drawLine(-0.85f, 0.60f, -1.55f, 0.70f, 2.0f);
    drawLine(-1.55f, 0.70f, -2.00f, 1.30f, 2.0f);
    drawLine(-2.00f, 1.30f, -2.05f, 2.10f, 1.8f);
    drawLine(-2.05f, 2.10f, -1.70f, 2.70f, 1.6f);
    drawLine(-1.70f, 2.70f, -1.10f, 2.90f, 1.4f);

    glColor3f(0.97f, 0.87f, 0.68f);
    glPushMatrix();
    glScalef(0.62f, 0.80f, 1.0f);
    fillCircle(0.0f, 1.12f, 0.75f);
    glPopMatrix();

    glColor3f(0.92f, 0.50f, 0.10f);
    glPushMatrix();
    glScalef(1.10f, 1.0f, 1.0f);
    fillCircle(0.0f, 2.55f, 0.78f);
    glPopMatrix();

    glColor3f(0.97f, 0.85f, 0.65f);
    fillCircle(-0.38f, 2.42f, 0.32f);
    fillCircle( 0.38f, 2.42f, 0.32f);

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

    glColor3f(0.22f, 0.65f, 0.20f);
    fillCircle(-0.28f, 2.62f, 0.18f);
    fillCircle( 0.28f, 2.62f, 0.18f);
    glColor3f(0.05f, 0.05f, 0.05f);
    fillCircle(-0.28f, 2.62f, 0.10f);
    fillCircle( 0.28f, 2.62f, 0.10f);
    glColor3f(1.0f, 1.0f, 1.0f);
    fillCircle(-0.22f, 2.67f, 0.04f);
    fillCircle( 0.34f, 2.67f, 0.04f);

    glColor3f(0.90f, 0.38f, 0.38f);
    glBegin(GL_TRIANGLES);
        glVertex2f(-0.10f, 2.36f); glVertex2f( 0.10f, 2.36f); glVertex2f( 0.00f, 2.25f);
    glEnd();

    glColor3f(0.30f, 0.08f, 0.05f);
    drawLine( 0.00f, 2.25f, -0.14f, 2.16f, 1.5f);
    drawLine( 0.00f, 2.25f,  0.14f, 2.16f, 1.5f);

    glColor3f(0.96f, 0.94f, 0.84f);
    drawLine(-0.10f, 2.38f, -1.10f, 2.52f, 1.2f);
    drawLine(-0.10f, 2.34f, -1.10f, 2.34f, 1.2f);
    drawLine(-0.10f, 2.30f, -1.10f, 2.16f, 1.2f);
    drawLine( 0.10f, 2.38f,  1.10f, 2.52f, 1.2f);
    drawLine( 0.10f, 2.34f,  1.10f, 2.34f, 1.2f);
    drawLine( 0.10f, 2.30f,  1.10f, 2.16f, 1.2f);

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

// ============================================================================
// DEER  (night only)
// ============================================================================
void drawDeer() {
    if (timeState != NIGHT) return;

    float baseY = forestBase(deerX);
    glPushMatrix();
    glTranslatef(deerX, baseY, 0.0f);      // TRANSFORM: Translation
    glScalef(deerDir, 1.0f, 1.0f);         // TRANSFORM: Mirror

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

    glColor3f(0.72f, 0.40f, 0.12f);
    glPushMatrix();
    glScalef(1.55f, 1.0f, 1.0f);
    fillCircle(0.0f, 0.85f, 1.10f);
    glPopMatrix();

    glColor3f(0.88f, 0.74f, 0.52f);
    glPushMatrix();
    glScalef(0.70f, 0.85f, 1.0f);
    fillCircle(0.18f, 0.90f, 0.75f);
    glPopMatrix();

    glColor3f(0.72f, 0.40f, 0.12f);
    glPushMatrix();
    glTranslatef(0.85f, 1.55f, 0.0f);
    glScalef(0.55f, 1.0f, 1.0f);
    fillCircle(0.0f, 0.0f, 0.65f);
    glPopMatrix();

    glColor3f(0.72f, 0.40f, 0.12f);
    glPushMatrix();
    glTranslatef(1.30f, 2.60f, 0.0f);
    glScalef(1.15f, 0.90f, 1.0f);
    fillCircle(0.0f, 0.0f, 0.62f);
    glPopMatrix();

    glColor3f(0.82f, 0.60f, 0.40f);
    glPushMatrix();
    glTranslatef(1.80f, 2.42f, 0.0f);
    glScalef(1.10f, 0.65f, 1.0f);
    fillCircle(0.0f, 0.0f, 0.38f);
    glPopMatrix();

    glColor3f(0.18f, 0.08f, 0.04f);
    fillCircle(2.14f, 2.38f, 0.13f);

    glColor3f(0.10f, 0.06f, 0.02f);
    fillCircle(1.52f, 2.72f, 0.15f);
    glColor3f(1.0f, 1.0f, 1.0f);
    fillCircle(1.58f, 2.78f, 0.05f);

    glColor3f(0.72f, 0.40f, 0.12f);
    glBegin(GL_TRIANGLES);
        glVertex2f(0.88f, 3.05f); glVertex2f(0.70f, 3.85f); glVertex2f(1.32f, 3.70f);
    glEnd();
    glColor3f(0.88f, 0.58f, 0.58f);
    glBegin(GL_TRIANGLES);
        glVertex2f(0.92f, 3.12f); glVertex2f(0.78f, 3.68f); glVertex2f(1.22f, 3.56f);
    glEnd();

    glColor3f(0.95f, 0.92f, 0.85f);
    fillCircle(-1.60f, 1.20f, 0.32f);

    glColor3f(0.40f, 0.22f, 0.06f);
    drawLine(0.90f, 3.18f,  0.50f, 4.30f, 2.0f);
    drawLine(0.50f, 4.30f,  0.10f, 5.10f, 2.0f);
    drawLine(0.50f, 4.30f,  0.70f, 5.00f, 2.0f);
    drawLine(0.10f, 5.10f, -0.20f, 5.60f, 1.8f);
    drawLine(0.10f, 5.10f,  0.30f, 5.55f, 1.8f);
    drawLine(1.40f, 3.22f,  1.70f, 4.30f, 2.0f);
    drawLine(1.70f, 4.30f,  1.40f, 5.10f, 2.0f);
    drawLine(1.70f, 4.30f,  2.10f, 5.00f, 2.0f);
    drawLine(1.40f, 5.10f,  1.20f, 5.60f, 1.8f);
    drawLine(1.40f, 5.10f,  1.70f, 5.55f, 1.8f);

    glPopMatrix();
}

// ============================================================================
// Timer
// ============================================================================
void timer(int /*value*/) {

    if (timeState == DAY) {
        // Animate cat
        catX += catDir * CAT_SPEED;
        if (catX > CAT_X_MAX) { catX = CAT_X_MAX; catDir = -1.0f; }
        if (catX < CAT_X_MIN) { catX = CAT_X_MIN; catDir =  1.0f; }

        // Animate birds – TRANSFORM: Translation across sky
        for (int i = 0; i < 3; i++) {
            birds[i].x += birds[i].speed;
            if (birds[i].x > WORLD_RIGHT + 4.0f) {
                birds[i].x = BIRD_RESET_X;
            }
        }

        // Advance flap angle (drives sin() in drawSingleBird)
        birdFlapAngle += 0.10f;   // radians per frame – adjust for faster/slower flap

    } else {
        // Animate deer
        deerX += deerDir * DEER_SPEED;
        if (deerX > DEER_X_MAX) { deerX = DEER_X_MAX; deerDir = -1.0f; }
        if (deerX < DEER_X_MIN) { deerX = DEER_X_MIN; deerDir =  1.0f; }
    }

    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

// ============================================================================
// Display
// ============================================================================
void display() {
    drawSky();
    drawRiver();
    drawClouds();    // drawn after sky/ground, before fence/animals
    drawFence();
    drawCat();
    drawDeer();
    drawBirds();     // drawn last in sky so they appear in front of clouds
    drawSun();
    drawMoon();
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