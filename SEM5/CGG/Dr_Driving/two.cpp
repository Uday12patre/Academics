#include <graphics.h>
#include <iostream>
#include <cmath>

using namespace std;

//====================================================
// DDA LINE ALGORITHM
//====================================================
void drawLineDDA(int x1, int y1, int x2, int y2, int color)
{
    int dx = x2 - x1;
    int dy = y2 - y1;

    int steps = max(abs(dx), abs(dy));

    float xInc = (float)dx / steps;
    float yInc = (float)dy / steps;

    float x = x1;
    float y = y1;

    for (int i = 0; i <= steps; i++)
    {
        putpixel(round(x), round(y), color);
        x += xInc;
        y += yInc;
    }
}

//====================================================
// BRESENHAM LINE ALGORITHM
//====================================================
void drawLineBresenham(int x1, int y1, int x2, int y2, int color)
{
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);

    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;

    int err = dx - dy;

    while (true)
    {
        putpixel(x1, y1, color);

        if (x1 == x2 && y1 == y2)
            break;

        int e2 = 2 * err;

        if (e2 > -dy)
        {
            err -= dy;
            x1 += sx;
        }

        if (e2 < dx)
        {
            err += dx;
            y1 += sy;
        }
    }
}

//====================================================
// BRESENHAM CIRCLE HELPER
//====================================================
void plot8Symmetry(int xc, int yc, int x, int y, int color)
{
    putpixel(xc + x, yc + y, color);
    putpixel(xc - x, yc + y, color);
    putpixel(xc + x, yc - y, color);
    putpixel(xc - x, yc - y, color);

    putpixel(xc + y, yc + x, color);
    putpixel(xc - y, yc + x, color);
    putpixel(xc + y, yc - x, color);
    putpixel(xc - y, yc - x, color);
}

//====================================================
// BRESENHAM CIRCLE ALGORITHM
//====================================================
void bresenhamCircleGraphics(int xc, int yc, int R, int color)
{
    int x = 0;
    int y = R;
    int delta = 2 * (1 - R);
    int limit = 0;

    while (true)
    {
        plot8Symmetry(xc, yc, x, y, color);

        if (y <= limit)
            break;

        if (delta < 0)
        {
            int d = 2 * delta + 2 * y - 1;

            if (d <= 0)
            {
                x = x + 1;
                delta = delta + 2 * x + 1;
            }
            else
            {
                x = x + 1;
                y = y - 1;
                delta = delta + 2 * x - 2 * y + 2;
            }
        }
        else if (delta > 0)
        {
            int d_prime = 2 * delta - 2 * x - 1;

            if (d_prime <= 0)
            {
                x = x + 1;
                y = y - 1;
                delta = delta + 2 * x - 2 * y + 2;
            }
            else
            {
                y = y - 1;
                delta = delta - 2 * y + 1;
            }
        }
        else
        {
            x = x + 1;
            y = y - 1;
            delta = delta + 2 * x - 2 * y + 2;
        }
    }
}

//====================================================
// DRAW PLAYER CAR
//====================================================
void drawPlayerCar(int x, int y)
{
    // Car Body
    drawLineBresenham(x, y, x + 50, y, RED);
    drawLineBresenham(x + 50, y, x + 50, y + 80, RED);
    drawLineBresenham(x + 50, y + 80, x, y + 80, RED);
    drawLineBresenham(x, y + 80, x, y, RED);

    // Windshield
    drawLineBresenham(x + 10, y + 20, x + 40, y + 20, WHITE);
    drawLineBresenham(x + 10, y + 60, x + 40, y + 60, WHITE);

    // Wheels (Bresenham Circle)
    bresenhamCircleGraphics(x, y + 10, 8, BLACK);
    bresenhamCircleGraphics(x + 50, y + 10, 8, BLACK);
    bresenhamCircleGraphics(x, y + 70, 8, BLACK);
    bresenhamCircleGraphics(x + 50, y + 70, 8, BLACK);
}

//====================================================
// DRAW OBSTACLE CAR
//====================================================
void drawObstacleCar(int x, int y)
{
    drawLineBresenham(x, y, x + 40, y, CYAN);
    drawLineBresenham(x + 40, y, x + 40, y + 50, CYAN);
    drawLineBresenham(x + 40, y + 50, x, y + 50, CYAN);
    drawLineBresenham(x, y + 50, x, y, CYAN);

    // Window
    drawLineBresenham(x + 8, y + 15, x + 32, y + 15, WHITE);

    // Wheels
    bresenhamCircleGraphics(x, y + 10, 6, BLACK);
    bresenhamCircleGraphics(x + 40, y + 10, 6, BLACK);
    bresenhamCircleGraphics(x, y + 40, 6, BLACK);
    bresenhamCircleGraphics(x + 40, y + 40, 6, BLACK);
}

//====================================================
// DRAW ROAD
//====================================================
void drawRoad()
{
    // Road edges
    drawLineDDA(150,50,150,450,WHITE);
    drawLineDDA(450,50,450,450,WHITE);

    // Left boundary
    drawLineDDA(140,50,140,450,LIGHTGRAY);

    // Right boundary
    drawLineDDA(460,50,460,450,LIGHTGRAY);

    // Center lane divider
    for(int y=50;y<450;y+=110)
    {
        drawLineDDA(300,y,300,y+60,YELLOW);
    }

    // Parking Bay 1
    drawLineDDA(150,100,230,100,WHITE);
    drawLineDDA(150,180,230,180,WHITE);

    // Parking Bay 2
    drawLineDDA(150,260,230,260,WHITE);
    drawLineDDA(150,340,230,340,WHITE);
}

//====================================================
// RENDER BASIC SCENE
//====================================================
void renderScene()
{
    drawRoad();

    // Player Car
    drawPlayerCar(350,300);

    // Obstacle Cars
    drawObstacleCar(200,120);
    drawObstacleCar(205,220);
}

//====================================================
// DRAW COINS
//====================================================
void drawCoins()
{
    bresenhamCircleGraphics(300,90,8,YELLOW);
    bresenhamCircleGraphics(300,140,8,YELLOW);
    bresenhamCircleGraphics(300,190,8,YELLOW);
    bresenhamCircleGraphics(300,240,8,YELLOW);
    bresenhamCircleGraphics(300,290,8,YELLOW);
    bresenhamCircleGraphics(300,340,8,YELLOW);
    bresenhamCircleGraphics(300,390,8,YELLOW);
}

//====================================================
// TRAFFIC SIGNAL
//====================================================
void drawTrafficSignal()
{
    // Pole
    drawLineDDA(500,70,500,200,WHITE);

    // Signal Box
    drawLineBresenham(480,70,520,70,WHITE);
    drawLineBresenham(520,70,520,140,WHITE);
    drawLineBresenham(520,140,480,140,WHITE);
    drawLineBresenham(480,140,480,70,WHITE);

    // Lights
    bresenhamCircleGraphics(500,85,6,RED);
    bresenhamCircleGraphics(500,105,6,YELLOW);
    bresenhamCircleGraphics(500,125,6,GREEN);
}

//====================================================
// SPEEDOMETER
//====================================================
void drawSpeedometer()
{
    bresenhamCircleGraphics(70,70,30,WHITE);

    drawLineBresenham(70,70,85,55,WHITE);

    outtextxy(55,65,(char*)"60");
}

//====================================================
// STEERING WHEEL
//====================================================
void drawSteeringWheel()
{
    bresenhamCircleGraphics(70,400,25,WHITE);

    drawLineBresenham(45,400,95,400,WHITE);
    drawLineBresenham(70,375,70,425,WHITE);
}

//====================================================
// TREES
//====================================================
void drawTrees()
{
    // Tree 1
    drawLineDDA(80,120,80,150,BROWN);
    bresenhamCircleGraphics(80,105,15,GREEN);
    bresenhamCircleGraphics(70,110,10,GREEN);
    bresenhamCircleGraphics(90,110,10,GREEN);

    // Tree 2
    drawLineDDA(80,250,80,280,BROWN);
    bresenhamCircleGraphics(80,235,15,GREEN);
    bresenhamCircleGraphics(70,240,10,GREEN);
    bresenhamCircleGraphics(90,240,10,GREEN);

    // Tree 3
    drawLineDDA(520,280,520,310,BROWN);
    bresenhamCircleGraphics(520,265,15,GREEN);
    bresenhamCircleGraphics(510,270,10,GREEN);
    bresenhamCircleGraphics(530,270,10,GREEN);
}

//====================================================
// MAIN
//====================================================
int main()
{
    int gd = DETECT, gm;
    initgraph(&gd,&gm,(char*)"");

    setbkcolor(DARKGRAY);
    cleardevice();

    outtextxy(180,20,(char*)"Dr. Driving 2D - Version 2");

    drawRoad();

    drawCoins();

    drawTrafficSignal();

    drawTrees();

    drawSpeedometer();

    drawSteeringWheel();

    drawPlayerCar(350,300);

    drawObstacleCar(200,120);

    drawObstacleCar(205,220);

    getch();

    closegraph();

    return 0;
}
