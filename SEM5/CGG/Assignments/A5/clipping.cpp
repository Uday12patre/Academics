#include <iostream>
#include <graphics.h>
#include <cmath>

using namespace std;

// -------------------------------------------------------------
// The 9 valid 4-bit region codes in the Cohen-Sutherland scheme
// Bit Format: [Top (Bit 3), Bottom (Bit 2), Right (Bit 1), Left (Bit 0)]
// -------------------------------------------------------------
const int REGION_INSIDE       = 0b0000; // Inside window
const int REGION_LEFT         = 0b0001; // Left
const int REGION_RIGHT        = 0b0010; // Right
const int REGION_BOTTOM       = 0b0100; // Bottom
const int REGION_BOTTOM_LEFT  = 0b0101; // Bottom-Left
const int REGION_BOTTOM_RIGHT = 0b0110; // Bottom-Right
const int REGION_TOP          = 0b1000; // Top
const int REGION_TOP_LEFT     = 0b1001; // Top-Left
const int REGION_TOP_RIGHT    = 0b1010; // Top-Right

// Bit masks for boundary testing
const int LEFT   = 0b0001;
const int RIGHT  = 0b0010;
const int BOTTOM = 0b0100;
const int TOP    = 0b1000;

// DDA Line Drawing Algorithm
void drawLineDDA(float x1, float y1, float x2, float y2, int color)
{
    float dx = x2 - x1;
    float dy = y2 - y1;

    int length = (abs(dx) >= abs(dy)) ? abs(dx) : abs(dy);

    if (length == 0)
    {
        putpixel(round(x1), round(y1), color);
        return;
    }

    float x_inc = dx / length;
    float y_inc = dy / length;

    float x = x1;
    float y = y1;

    for (int i = 0; i <= length; i++)
    {
        putpixel(round(x), round(y), color);
        x += x_inc;
        y += y_inc;
    }
}

// Draw rectangle boundaries using DDA
void drawWindowDDA(float x_min, float y_min, float x_max, float y_max, int color)
{
    drawLineDDA(x_min, y_min, x_max, y_min, color); // Top border
    drawLineDDA(x_max, y_min, x_max, y_max, color); // Right border
    drawLineDDA(x_max, y_max, x_min, y_max, color); // Bottom border
    drawLineDDA(x_min, y_max, x_min, y_min, color); // Left border
}

// Assigns one of the 9 specific 4-bit region codes to a point (x, y)
int computeCode(double x, double y, double x_min, double y_min, double x_max, double y_max)
{
    if (x < x_min && y < y_min)
        return REGION_TOP_LEFT;     // 1001
    else if (x > x_max && y < y_min)
        return REGION_TOP_RIGHT;    // 1010
    else if (y < y_min)
        return REGION_TOP;          // 1000
    else if (x < x_min && y > y_max)
        return REGION_BOTTOM_LEFT;  // 0101
    else if (x > x_max && y > y_max)
        return REGION_BOTTOM_RIGHT; // 0110
    else if (y > y_max)
        return REGION_BOTTOM;       // 0100
    else if (x < x_min)
        return REGION_LEFT;         // 0001
    else if (x > x_max)
        return REGION_RIGHT;        // 0010
    else
        return REGION_INSIDE;       // 0000
}

// Cohen-Sutherland Clipping using bitwise logic and iterative intersection
void cohenSutherlandClip(double x1, double y1, double x2, double y2,
                         double x_min, double y_min, double x_max, double y_max)
{
    int code1 = computeCode(x1, y1, x_min, y_min, x_max, y_max);
    int code2 = computeCode(x2, y2, x_min, y_min, x_max, y_max);
    bool accept = false;

    while (true)
    {
        if ((code1 == REGION_INSIDE) && (code2 == REGION_INSIDE))
        {
            // Both endpoints inside: Trivial Accept
            accept = true;
            break;
        }
        else if (code1 & code2)
        {
            // Both share an external region: Trivial Reject
            break;
        }
        else
        {
            // At least one point is outside; pick it for intersection clipping
            double x = 0, y = 0;
            int codeOut = (code1 != REGION_INSIDE) ? code1 : code2;

            // Calculate intersection using line equation
            if (codeOut & TOP)
            {
                x = x1 + (x2 - x1) * (y_min - y1) / (y2 - y1);
                y = y_min;
            }
            else if (codeOut & BOTTOM)
            {
                x = x1 + (x2 - x1) * (y_max - y1) / (y2 - y1);
                y = y_max;
            }
            else if (codeOut & RIGHT)
            {
                y = y1 + (y2 - y1) * (x_max - x1) / (x2 - x1);
                x = x_max;
            }
            else if (codeOut & LEFT)
            {
                y = y1 + (y2 - y1) * (x_min - x1) / (x2 - x1);
                x = x_min;
            }

            // Replace outside point with intersection point and re-classify
            if (codeOut == code1)
            {
                x1 = x;
                y1 = y;
                code1 = computeCode(x1, y1, x_min, y_min, x_max, y_max);
            }
            else
            {
                x2 = x;
                y2 = y;
                code2 = computeCode(x2, y2, x_min, y_min, x_max, y_max);
            }
        }
    }

    if (accept)
    {
        // Draw the clipped visible segment in GREEN using DDA
        drawLineDDA(x1, y1, x2, y2, GREEN);
    }
}

int main()
{
    double x_min, y_min, x_max, y_max;
    double x1, y1, x2, y2;

    // 1. User Input for window boundaries
    cout << "Enter clipping window coordinates (x_min y_min x_max y_max):\n";
    cout << "Example: 150 150 450 400\n> ";
    cin >> x_min >> y_min >> x_max >> y_max;

    // 2. User Input for line endpoints
    cout << "Enter line starting point (x1 y1):\n> ";
    cin >> x1 >> y1;
    cout << "Enter line ending point (x2 y2):\n> ";
    cin >> x2 >> y2;

    // 3. Initialize graphics
    int gd = DETECT, gm;
    initgraph(&gd, &gm, (char*)"");

    // Display original boundary and unclipped line (RED)
    drawWindowDDA(x_min, y_min, x_max, y_max, WHITE);
    drawLineDDA(x1, y1, x2, y2, RED);

    outtextxy(10, 10, (char*)"Original line in RED. Press any key to clip...");
    getch();

    // Clear and redraw clipped line (GREEN)
    cleardevice();
    drawWindowDDA(x_min, y_min, x_max, y_max, WHITE);
    cohenSutherlandClip(x1, y1, x2, y2, x_min, y_min, x_max, y_max);

    outtextxy(10, 10, (char*)"Clipped line in GREEN. Press any key to exit.");
    getch();

    closegraph();
    return 0;
}
