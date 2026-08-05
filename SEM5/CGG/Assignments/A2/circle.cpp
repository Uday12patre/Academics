#include <stdio.h>
#include <graphics.h>
#include <iostream>
using namespace std;

 
void plot8Symmetry(int xc, int yc, int x, int y, int color) {

    // getmaxx() and getmaxy(): These are functions from <graphics.h>. They return the highest valid x and y pixel coordinates on the active screen
    int maxX = getmaxx();
    int maxY = getmaxy();

    // Helper to handle full boundary checks in one place
    auto safePutPixel = [&](int px, int py) {
        if (px >= 0 && px <= maxX && py >= 0 && py <= maxY) {
            putpixel(px, py, color);
        }
    };

    safePutPixel(xc + x, yc + y);
    safePutPixel(xc - x, yc + y);
    safePutPixel(xc + x, yc - y);
    safePutPixel(xc - x, yc - y);
    safePutPixel(xc + y, yc + x);
    safePutPixel(xc - y, yc + x);
    safePutPixel(xc + y, yc - x);
    safePutPixel(xc - y, yc - x);
}

void bresenhamCircleGraphics(int xc, int yc, int R, int color) {
    int x = 0;
    int y = R;
    if(R <= 0)
    	{
    		cout << "Error: Please enter valid radius.\n";
    		return;
	}
    int delta = 2 * (1 - R);
    int limit = 0;
	
	
    while (x <= y) {
    	
		    plot8Symmetry(xc, yc, x, y, color);
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

	// setviewport(MIN_X, MIN_Y, MAX_X, MAX_Y, 1);
    int xc, yc, R;
    cout << "Valid X is 0 to 639 and Y is 0 to 479)\n";
    cout << "Enter Circle Center (xc, yc): ";
    cin >> xc >> yc;
    cout << endl;
    cout << "Enter radius of Circle: ";
    cin >> R;

    putpixel(xc, yc, RED);
    int gd = DETECT, gm;
    initgraph(&gd, &gm, (char*)"");

    setbkcolor(WHITE);
    cleardevice();

    bresenhamCircleGraphics(xc, yc, R, BLACK);
	putpixel(xc,yc,RED);
    getch();
    closegraph();

    return 0;
}
