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



// ============================================================================
// ALGORITHM: Bresenham's Line Drawing
// ============================================================================



// ============================================================================
// Filled circle using triangle fan
// ============================================================================
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
// Environment Drawing
// ============================================================================



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

