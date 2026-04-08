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

// Cat animation
float catX   = -5.0f;
float catDir =  1.0f;
const float CAT_SPEED = 0.04f;
const float CAT_X_MIN = -18.0f;
const float CAT_X_MAX =  14.0f;

// ============================================================================
// Lower river bank baseline (village / fence side)
// Passes through (40, 4) and (-40, -29), slope = 0.4125
// ============================================================================
float fenceBase(float x) {
    return 4.0f + (x - 40.0f) * 0.4125f - 2.5f;
}

// ============================================================================
// ALGORITHM: DDA Line Drawing  (retained for river banks)
// ============================================================================
void drawLineDDA(float x1, float y1, float x2, float y2) {
    float dx = x2-x1, dy = y2-y1;
    int steps = (int)(fabs(dx)>fabs(dy) ? fabs(dx) : fabs(dy));
    if (steps==0){ glBegin(GL_POINTS); glVertex2f(x1,y1); glEnd(); return; }
    float xInc=dx/steps, yInc=dy/steps, x=x1, y=y1;
    glBegin(GL_POINTS);
    for(int i=0;i<=steps;i++){ glVertex2f(roundf(x),roundf(y)); x+=xInc; y+=yInc; }
    glEnd();
}

// ============================================================================
// ALGORITHM: Bresenham Line Drawing
// ============================================================================
void drawLineBresenham(int x1,int y1,int x2,int y2){
    int dx=abs(x2-x1),dy=abs(y2-y1),sx=(x1<x2)?1:-1,sy=(y1<y2)?1:-1,err=dx-dy;
    glBegin(GL_POINTS);
    while(true){
        glVertex2i(x1,y1);
        if(x1==x2&&y1==y2)break;
        int e2=2*err;
        if(e2>-dy){err-=dy;x1+=sx;}
        if(e2< dx){err+=dx;y1+=sy;}
    }
    glEnd();
}

// ============================================================================
// GL_LINES wrapper with settable width
// Used for fence rails, cat details – crisp GPU-rendered lines
// ============================================================================
void drawLine(float x1,float y1,float x2,float y2,float width=1.0f){
    glLineWidth(width);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glBegin(GL_LINES);
        glVertex2f(x1,y1); glVertex2f(x2,y2);
    glEnd();
    glDisable(GL_LINE_SMOOTH);
    glLineWidth(1.0f);
}

// ============================================================================
// Filled primitives
// ============================================================================
void fillCircle(float xc,float yc,float r,int segs=60){
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(xc,yc);
    for(int i=0;i<=segs;i++){
        float a=2.0f*M_PI*i/segs;
        glVertex2f(xc+r*cosf(a),yc+r*sinf(a));
    }
    glEnd();
}

void fillRect(float x,float y,float w,float h){
    glBegin(GL_QUADS);
    glVertex2f(x,y); glVertex2f(x+w,y); glVertex2f(x+w,y+h); glVertex2f(x,y+h);
    glEnd();
}

// ============================================================================
// Sky
// ============================================================================
void drawSky(){
    if(timeState==DAY) glClearColor(0.53f,0.81f,0.92f,1.0f);
    else               glClearColor(0.04f,0.10f,0.16f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void drawSun(){
    if(timeState!=DAY)return;
    glColor3f(1.0f,1.0f,0.0f);
    fillCircle(25.0f,20.0f,3.5f);
}

void drawMoon(){
    if(timeState!=NIGHT)return;
    glColor3f(0.94f,0.94f,0.82f);
    fillCircle(-25.0f,20.0f,3.5f);
}

// ============================================================================
// River  (banks use DDA as originally specified)
// ============================================================================
void drawRiver(){
    glColor3f(0.0f,0.5f,0.8f);
    glBegin(GL_POLYGON);
        glVertex2f( 40.0f, 10.0f); glVertex2f( 40.0f,  5.0f);
        glVertex2f(-40.0f,-30.0f); glVertex2f(-40.0f,-20.0f);
    glEnd();
    // Sandy banks – ALGORITHM: DDA
    glColor3f(0.76f,0.70f,0.50f);
    drawLineDDA( 40.0f,10.5f,-40.0f,-20.5f);
    drawLineDDA( 40.0f, 4.5f,-40.0f,-29.5f);
    // Inner bank edges – ALGORITHM: DDA
    glColor3f(0.0f,0.3f,0.5f);
    drawLineDDA( 40.0f,11.0f,-40.0f,-21.0f);
    drawLineDDA( 40.0f, 4.0f,-40.0f,-29.0f);
}

// ============================================================================
// FENCE – village side of the river
//
//  Post bodies     → fillRect (GL_QUADS)
//  Post shading    → fillRect (GL_QUADS, darker strip for depth)
//  Rails           → drawLine (GL_LINES + glLineWidth) – CRISP & THICK
//  Rail highlight  → drawLine (lighter, thin)
//  Rail shadow     → drawLine (darker, thin)
//  Post caps/edges → drawLine (GL_LINES)
// ============================================================================
void drawFence(){
    const float xStart      = -38.0f;
    const float xEnd        =  38.0f;
    const float postHW      =  0.52f;
    const float postHeight  =  3.1f;
    const float postSpacing =  3.6f;
    const float railHi      =  postHeight * 0.74f;
    const float railLo      =  postHeight * 0.40f;

    // 1. Filled post bodies
    for(float px=xStart; px<=xEnd+0.1f; px+=postSpacing){
        float py=fenceBase(px);
        // main body
        glColor3f(0.42f,0.24f,0.07f);
        fillRect(px-postHW, py, postHW*2.0f, postHeight);
        // right-side shadow strip for depth
        glColor3f(0.28f,0.14f,0.03f);
        fillRect(px+postHW*0.42f, py, postHW*0.58f, postHeight);
    }

    // 2. Rails – GL_LINES (thick, smooth)
    // Rail body
    glColor3f(0.60f,0.38f,0.14f);
    drawLine(xStart,fenceBase(xStart)+railHi, xEnd,fenceBase(xEnd)+railHi, 6.0f);
    drawLine(xStart,fenceBase(xStart)+railLo, xEnd,fenceBase(xEnd)+railLo, 6.0f);
    // Rail top highlight
    glColor3f(0.78f,0.58f,0.28f);
    drawLine(xStart,fenceBase(xStart)+railHi+0.22f, xEnd,fenceBase(xEnd)+railHi+0.22f, 1.5f);
    drawLine(xStart,fenceBase(xStart)+railLo+0.22f, xEnd,fenceBase(xEnd)+railLo+0.22f, 1.5f);
    // Rail bottom shadow
    glColor3f(0.28f,0.14f,0.04f);
    drawLine(xStart,fenceBase(xStart)+railHi-0.22f, xEnd,fenceBase(xEnd)+railHi-0.22f, 1.5f);
    drawLine(xStart,fenceBase(xStart)+railLo-0.22f, xEnd,fenceBase(xEnd)+railLo-0.22f, 1.5f);

    // 3. Post top caps and left edges – GL_LINES
    glColor3f(0.25f,0.12f,0.02f);
    for(float px=xStart; px<=xEnd+0.1f; px+=postSpacing){
        float py=fenceBase(px), top=py+postHeight;
        drawLine(px-postHW, py,  px-postHW, top,  1.2f);  // left edge
        drawLine(px-postHW, top, px+postHW, top,  1.2f);  // top cap
    }
}

// ============================================================================
// CAT  (day only) – translated along fence baseline
//
//  Filled shapes   → fillCircle / GL_TRIANGLES / GL_QUADS
//  Tail outline    → drawLine (GL_LINES, thick)
//  Whiskers        → drawLine (GL_LINES, thin)
//  Mouth           → drawLine (GL_LINES)
//  Paw toe marks   → drawLine (GL_LINES)
// ============================================================================
void drawCat(){
    if(timeState!=DAY)return;

    float baseY=fenceBase(catX);
    glPushMatrix();
    glTranslatef(catX, baseY, 0.0f);  // TRANSFORM: Translation

    const float s=0.90f;

    // Tail
    glColor3f(0.82f,0.40f,0.06f);
    drawLine(-1.3f*s,0.5f*s,  -2.1f*s,1.2f*s,  3.2f);
    drawLine(-2.1f*s,1.2f*s,  -1.8f*s,2.0f*s,  3.2f);
    drawLine(-1.8f*s,2.0f*s,  -1.1f*s,2.4f*s,  3.2f);
    // tail highlight
    glColor3f(0.97f,0.65f,0.22f);
    drawLine(-1.3f*s,0.5f*s,  -2.1f*s,1.2f*s,  1.0f);
    drawLine(-2.1f*s,1.2f*s,  -1.8f*s,2.0f*s,  1.0f);
    drawLine(-1.8f*s,2.0f*s,  -1.1f*s,2.4f*s,  1.0f);

    // Body
    glColor3f(0.93f,0.52f,0.12f);
    glPushMatrix();
    glScalef(1.28f*s,1.0f*s,1.0f);
    fillCircle(0.0f,0.95f,1.02f);
    glPopMatrix();
    // belly shading
    glColor3f(0.85f,0.44f,0.08f);
    glPushMatrix();
    glScalef(0.68f*s,0.52f*s,1.0f);
    fillCircle(0.0f,1.6f,0.85f);
    glPopMatrix();

    // Head
    glColor3f(0.93f,0.52f,0.12f);
    fillCircle(0.38f*s,2.38f*s,0.74f*s);

    // Ears
    glColor3f(0.93f,0.52f,0.12f);
    glBegin(GL_TRIANGLES);
        glVertex2f(-0.08f*s,2.88f*s); glVertex2f(0.18f*s,3.48f*s); glVertex2f(0.50f*s,2.92f*s);
        glVertex2f( 0.56f*s,2.92f*s); glVertex2f(0.82f*s,3.48f*s); glVertex2f(1.08f*s,2.90f*s);
    glEnd();
    glColor3f(0.95f,0.70f,0.70f);
    glBegin(GL_TRIANGLES);
        glVertex2f(0.02f*s,2.92f*s); glVertex2f(0.18f*s,3.28f*s); glVertex2f(0.42f*s,2.96f*s);
        glVertex2f(0.62f*s,2.96f*s); glVertex2f(0.82f*s,3.28f*s); glVertex2f(1.00f*s,2.94f*s);
    glEnd();

    // Eyes
    glColor3f(0.28f,0.68f,0.18f);
    fillCircle(0.16f*s,2.44f*s,0.15f*s);
    fillCircle(0.60f*s,2.44f*s,0.15f*s);
    glColor3f(0.05f,0.05f,0.05f);
    fillCircle(0.16f*s,2.44f*s,0.08f*s);
    fillCircle(0.60f*s,2.44f*s,0.08f*s);
    glColor3f(1.0f,1.0f,1.0f);
    fillCircle(0.19f*s,2.47f*s,0.03f*s);
    fillCircle(0.63f*s,2.47f*s,0.03f*s);

    // Nose
    glColor3f(0.88f,0.40f,0.40f);
    glBegin(GL_TRIANGLES);
        glVertex2f(0.32f*s,2.22f*s);
        glVertex2f(0.46f*s,2.22f*s);
        glVertex2f(0.39f*s,2.14f*s);
    glEnd();

    // Mouth – GL_LINES
    glColor3f(0.25f,0.08f,0.03f);
    drawLine(0.39f*s,2.14f*s,  0.26f*s,2.06f*s,  1.5f);
    drawLine(0.39f*s,2.14f*s,  0.52f*s,2.06f*s,  1.5f);

    // Whiskers – GL_LINES (thin)
    glColor3f(0.95f,0.93f,0.82f);
    drawLine( 0.36f*s,2.22f*s,  -0.52f*s,2.33f*s,  1.2f);
    drawLine( 0.36f*s,2.18f*s,  -0.52f*s,2.15f*s,  1.2f);
    drawLine( 0.36f*s,2.14f*s,  -0.52f*s,1.98f*s,  1.2f);
    drawLine( 0.42f*s,2.22f*s,   1.28f*s,2.33f*s,  1.2f);
    drawLine( 0.42f*s,2.18f*s,   1.28f*s,2.15f*s,  1.2f);
    drawLine( 0.42f*s,2.14f*s,   1.28f*s,1.98f*s,  1.2f);

    // Front paws
    glColor3f(0.88f,0.48f,0.10f);
    glPushMatrix();
    glScalef(1.0f,0.42f,1.0f);
    fillCircle(-0.28f*s,0.0f,0.30f*s);
    fillCircle( 0.48f*s,0.0f,0.30f*s);
    glPopMatrix();
    glColor3f(0.70f,0.32f,0.04f);
    drawLine(-0.40f*s,0.14f*s, -0.16f*s,0.14f*s, 1.0f);
    drawLine( 0.34f*s,0.14f*s,  0.60f*s,0.14f*s, 1.0f);

    glPopMatrix();
}

// ============================================================================
// Timer
// ============================================================================
void timer(int){
    if(timeState==DAY){
        catX+=catDir*CAT_SPEED;
        if(catX>CAT_X_MAX){catX=CAT_X_MAX;catDir=-1.0f;}
        if(catX<CAT_X_MIN){catX=CAT_X_MIN;catDir= 1.0f;}
    }
    glutPostRedisplay();
    glutTimerFunc(16,timer,0);
}

// ============================================================================
// Display
// ============================================================================
void display(){
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
void keyboard(unsigned char key,int,int){
    switch(key){
        case ' ': timeState=(timeState==DAY)?NIGHT:DAY; glutPostRedisplay(); break;
        case 27: case 'q': case 'Q': exit(0);
    }
}

// ============================================================================
// Init & Main
// ============================================================================
void init(){
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluOrtho2D(WORLD_LEFT,WORLD_RIGHT,WORLD_BOTTOM,WORLD_TOP);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
}

int main(int argc,char** argv){
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);
    glutInitWindowSize(800,600);
    glutInitWindowPosition(100,100);
    glutCreateWindow("Zero Point - Forest Meets Village");
    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(16,timer,0);
    glutMainLoop();
    return 0;
}