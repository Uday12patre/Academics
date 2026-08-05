#include <graphics.h>
#include <iostream>
#include <cmath>

using namespace std;

// -------------------------------------------------------------
// 1. Digital Differential Analyzer (DDA) Line Algorithm
// Used for static elements: Road Borders and Lane Markings
// -------------------------------------------------------------
void drawLineDDA(int x1, int y1, int x2, int y2, int color) {
    int dx = x2 - x1;
    int dy = y2 - y1;

    int steps = max(abs(dx), abs(dy));

    float xInc = (float)dx / steps;
    float yInc = (float)dy / steps;

    float x = x1;
    float y = y1;

    for (int i = 0; i <= steps; i++) {
        putpixel(round(x), round(y), color);
        x += xInc;
        y += yInc;
    }
}

// -------------------------------------------------------------
// 2. Bresenham's Line Algorithm
// Used for precise rendering: Car Body Outlines
// -------------------------------------------------------------
void drawLineBresenham(int x1, int y1, int x2, int y2, int color) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    
    int err = dx - dy;

    while (true) {
        putpixel(x1, y1, color);

        if (x1 == x2 && y1 == y2) break;

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

// -------------------------------------------------------------
// Dr. Driving Scene Assembly
// -------------------------------------------------------------
void renderDrDrivingScene() {
    // A. Road Outer Boundaries (DDA)
    drawLineDDA(150, 50, 150, 450, WHITE); // Left road edge
    drawLineDDA(450, 50, 450, 450, WHITE); // Right road edge

    // B. Road Center Divider Line (DDA)
    drawLineDDA(300, 50, 300, 120, YELLOW);
    drawLineDDA(300, 160, 300, 230, YELLOW);
    drawLineDDA(300, 270, 300, 340, YELLOW);
    drawLineDDA(300, 380, 300, 450, YELLOW);

    // C. Parking Bay Lines on Left Lane (DDA)
    drawLineDDA(150, 100, 230, 100, WHITE);
    drawLineDDA(150, 180, 230, 180, WHITE);

    // D. Car 1 Outline - Player's Car (Bresenham)
    // Drawn in the right lane (red color)
    drawLineBresenham(350, 300, 400, 300, RED); // Front bumper
    drawLineBresenham(400, 300, 400, 380, RED); // Right side
    drawLineBresenham(400, 380, 350, 380, RED); // Rear bumper
    drawLineBresenham(350, 380, 350, 300, RED); // Left side

    // E. Car 2 Outline - Traffic / Obstacle Car (Bresenham)
    // Drawn ahead in left lane (cyan color)
    drawLineBresenham(200, 120, 240, 120, CYAN); // Front bumper
    drawLineBresenham(240, 120, 240, 170, CYAN); // Right side
    drawLineBresenham(240, 170, 200, 170, CYAN); // Rear bumper
    drawLineBresenham(200, 170, 200, 120, CYAN); // Left side
}

int main() {
    // Initialize Graphics Window (640x480 resolution)
    int gd = DETECT, gm;
    initgraph(&gd, &gm, (char*)"");

    // Title text
    outtextxy(200, 20, (char*)"Dr. Driving - Line Rendering Scene");

    // Call game renderer
    renderDrDrivingScene();

    // Hold screen until key press
    getch();
    closegraph();

    return 0;
}
