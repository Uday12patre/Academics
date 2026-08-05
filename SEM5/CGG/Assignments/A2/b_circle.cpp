#include <graphics.h>
#include <iostream>

using namespace std;

void plot8Symmetry(int xc, int yc, int x, int y, int color) {
    putpixel(xc + x, yc + y, color);
    putpixel(xc - x, yc + y, color);
    putpixel(xc + x, yc - y, color);
    putpixel(xc - x, yc - y, color);
    putpixel(xc + y, yc + x, color);
    putpixel(xc - y, yc + x, color);
    putpixel(xc + y, yc - x, color);
    putpixel(xc - y, yc - x, color);
}

void bresenhamCircleGraphics(int xc, int yc, int R, int color) {
    int x = 0;
    int y = R;
    int delta = 2 * (1 - R);
    int limit = 0;

    while (true) {
        plot8Symmetry(xc, yc, x, y, color);

        if (y <= limit) {
            break;
        }

        if (delta < 0) {
            int d = 2 * delta + 2 * y - 1;
            if (d <= 0) {
                x = x + 1;
                delta = delta + 2 * x + 1;
            } else {
                x = x + 1;
                y = y - 1;
                delta = delta + 2 * x - 2 * y + 2;
            }
        } 
        else if (delta > 0) {
            int d_prime = 2 * delta - 2 * x - 1;
            if (d_prime <= 0) {
                x = x + 1;
                y = y - 1;
                delta = delta + 2 * x - 2 * y + 2;
            } else {
                y = y - 1;
                delta = delta - 2 * y + 1;
            }
        } 
        else {
            x = x + 1;
            y = y - 1;
            delta = delta + 2 * x - 2 * y + 2;
        }
    }
}

int main() {
    int xc, yc, R;
    cout << "Enter Circle Center (xc, yc): ";
    cin >> xc >> yc;
    cout << endl;
    cout << "Enter radius of Circle: ";
    cin >> R;

    int gd = DETECT, gm;
    initgraph(&gd, &gm, (char*)"");

    // Set background to white and clear screen to apply it
    setbkcolor(WHITE);
    cleardevice();

    // Draw circle with BLACK pixels (change to WHITE if specifically required)
    bresenhamCircleGraphics(xc, yc, R, BLACK);

    getch();
    closegraph();

    return 0;
}
