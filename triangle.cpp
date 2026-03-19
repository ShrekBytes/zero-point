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
