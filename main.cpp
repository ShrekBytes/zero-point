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
// Display
// ============================================================================
void display() {
    drawSky();
    drawRiver();
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
    glutMainLoop();
    return 0;
}
