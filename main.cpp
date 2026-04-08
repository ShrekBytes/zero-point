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
// Stage 3B: River
// ============================================================================
void drawRiver() {
    // River body – diagonal from top-right to bottom-left
    // Starts below sun/moon (y=15), touches left/right/bottom edges
    glColor3f(0.0f, 0.5f, 0.8f);
    glBegin(GL_POLYGON);
        glVertex2f( 40.0f,  10.0f);   // top-right corner (right edge)
        glVertex2f( 40.0f,   5.0f);   // lower right of river band
        glVertex2f(-40.0f, -30.0f);   // bottom-left corner (left + bottom edge)
        glVertex2f(-40.0f, -20.0f);   // upper left of river band
    glEnd();

    // Sandy banks (outer edge) using DDA lines – offset outward from river
    glColor3f(0.76f, 0.70f, 0.50f);
    drawLineDDA( 40.0f, 10.5f, -40.0f, -20.5f);   // ALGORITHM: DDA – sandy upper bank
    drawLineDDA( 40.0f,  4.5f, -40.0f, -29.5f);   // ALGORITHM: DDA – sandy lower bank

    // River banks (inner edge) using DDA lines – right at the water's edge
    glColor3f(0.0f, 0.3f, 0.5f);
    drawLineDDA( 40.0f, 11.0f, -40.0f, -21.0f);   // ALGORITHM: DDA – upper bank
    drawLineDDA( 40.0f,  4.0f, -40.0f, -29.0f);   // ALGORITHM: DDA – lower bank
}

// ============================================================================
// Helper: Y position on the village-side fence baseline
// Lower river bank passes through (40,4) and (-40,-29), slope = 0.4125
// Offset -2.5 downward so fence stands on dry ground below the bank
// ============================================================================
float fenceBase(float x) {
    return 4.0f + (x - 40.0f) * 0.4125f - 2.5f;
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
// ============================================================================
void drawFence() {
    const float xStart      = -38.0f;
    const float xEnd        =  38.0f;
    const float postHW      =  0.52f;   // half-width of each post
    const float postHeight  =  3.1f;    // height of post above baseline
    const float postSpacing =  3.6f;    // centre-to-centre spacing
    const float railHi      =  postHeight * 0.74f;
    const float railLo      =  postHeight * 0.40f;

    // 1. Filled post bodies
    for (float px = xStart; px <= xEnd + 0.1f; px += postSpacing) {
        float py = fenceBase(px);
        // main body – medium dark brown
        glColor3f(0.42f, 0.24f, 0.07f);
        fillRect(px - postHW, py, postHW * 2.0f, postHeight);
        // right-side shadow strip for depth
        glColor3f(0.28f, 0.14f, 0.03f);
        fillRect(px + postHW * 0.42f, py, postHW * 0.58f, postHeight);
    }

    // 2. Rails – GL_LINES (thick, smooth)
    glColor3f(0.60f, 0.38f, 0.14f);
    drawLine(xStart, fenceBase(xStart)+railHi, xEnd, fenceBase(xEnd)+railHi, 6.0f);
    drawLine(xStart, fenceBase(xStart)+railLo, xEnd, fenceBase(xEnd)+railLo, 6.0f);
    // rail top highlight
    glColor3f(0.78f, 0.58f, 0.28f);
    drawLine(xStart, fenceBase(xStart)+railHi+0.22f, xEnd, fenceBase(xEnd)+railHi+0.22f, 1.5f);
    drawLine(xStart, fenceBase(xStart)+railLo+0.22f, xEnd, fenceBase(xEnd)+railLo+0.22f, 1.5f);
    // rail bottom shadow
    glColor3f(0.28f, 0.14f, 0.04f);
    drawLine(xStart, fenceBase(xStart)+railHi-0.22f, xEnd, fenceBase(xEnd)+railHi-0.22f, 1.5f);
    drawLine(xStart, fenceBase(xStart)+railLo-0.22f, xEnd, fenceBase(xEnd)+railLo-0.22f, 1.5f);

    // 3. Post top caps and left edges – GL_LINES
    glColor3f(0.25f, 0.12f, 0.02f);
    for (float px = xStart; px <= xEnd + 0.1f; px += postSpacing) {
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

    // --- Tail: sweeps up and curves to the right ---
    glColor3f(0.78f, 0.36f, 0.05f);
    drawLine( 1.5f, 0.3f,  2.4f, 0.8f,  4.0f);
    drawLine( 2.4f, 0.8f,  2.6f, 1.8f,  4.0f);
    drawLine( 2.6f, 1.8f,  2.1f, 2.4f,  4.0f);
    // tail tip – lighter highlight
    glColor3f(0.97f, 0.62f, 0.18f);
    drawLine( 1.5f, 0.3f,  2.4f, 0.8f,  1.5f);
    drawLine( 2.4f, 0.8f,  2.6f, 1.8f,  1.5f);
    drawLine( 2.6f, 1.8f,  2.1f, 2.4f,  1.5f);

    // --- Body: tall oval for sitting posture ---
    glColor3f(0.92f, 0.50f, 0.10f);
    glPushMatrix();
    glScalef(1.0f, 1.35f, 1.0f);
    fillCircle(0.0f, 0.75f, 1.05f);
    glPopMatrix();

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
// Timer – animates cat back and forth (day only)
// ============================================================================
void timer(int /*value*/) {
    if (timeState == DAY) {
        catX += catDir * CAT_SPEED;
        if (catX > CAT_X_MAX) { catX = CAT_X_MAX; catDir = -1.0f; }
        if (catX < CAT_X_MIN) { catX = CAT_X_MIN; catDir =  1.0f; }
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
    drawFence();
    drawCat();
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
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Zero Point - Forest Meets Village");

    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(16, timer, 0);
    glutMainLoop();
    return 0;
}