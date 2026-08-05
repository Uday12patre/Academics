#include <graphics.h>
#include <iostream>
#include <cmath>

using namespace std;

// Function to draw line using Bresenham's Line Algorithm
void drawLineBresenham(int x1, int y1, int x2, int y2) {
    int x = x1;
    int y = y1;

    // Calculate changes in coordinates
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);

    // Determine direction of step for x and y
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;

    // Plot initial point
    putpixel(x, y, WHITE);

    // Case 1: Slope |m| <= 1 (Gentle slope, driving axis is X)
    if (dx > dy) {
        int p = 2 * dy - dx; // Initial decision parameter

        for (int i = 0; i < dx; i++) {
            x += sx;
            if (p >= 0) {
                y += sy;
                p += 2 * (dy - dx);
            } else {
                p += 2 * dy;
            }
            putpixel(x, y, WHITE);
            delay(10); // Optional: delay to see the line drawing effect
        }
    } 
    // Case 2: Slope |m| > 1 (Steep slope, driving axis is Y)
    else {
        int p = 2 * dx - dy; // Initial decision parameter

        for (int i = 0; i < dy; i++) {
            y += sy;
            if (p >= 0) {
                x += sx;
                p += 2 * (dx - dy);
            } else {
                p += 2 * dx;
            }
            putpixel(x, y, WHITE);
            delay(10); // Optional: delay to see the line drawing effect
        }
    }
}

int main() {
    // Initialize graphics window (640x480 pixels)
    int gd = DETECT, gm;
    initgraph(&gd, &gm, (char*)"");

    int x1 = 100, y1 = 100;
    int x2 = 400, y2 = 300;

    // Draw the line
    drawLineBresenham(x1, y1, x2, y2);

    // Wait for key press before closing
    getch();
    closegraph();

    return 0;
}
