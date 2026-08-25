#include <graphics.h>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <cstdio>
#include <SDL2/SDL.h>

using namespace std;

// ============================================================
// DR. DRIVING 2D - VERSION 3
// Version 2 game + 2D homogeneous transformations
//
// Transformation convention:
//     P' = P x M
//
// No screen buffer / no double buffering is used.
// The scene is redrawn directly on the graphics screen.
// ============================================================

// ------------------------------------------------------------
// 3x3 MATRIX UTILITIES
// ------------------------------------------------------------
void setIdentity(double M[3][3])
{
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            M[i][j] = (i == j) ? 1.0 : 0.0;
}

void multiplyMatrix(double A[3][3], double B[3][3], double result[3][3])
{
    double temp[3][3] = {};

    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            for (int k = 0; k < 3; k++)
                temp[i][j] += A[i][k] * B[k][j];

    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            result[i][j] = temp[i][j];
}

void createTranslation(double T[3][3], double tx, double ty)
{
    setIdentity(T);
    T[2][0] = tx;
    T[2][1] = ty;
}

void createScaling(double S[3][3], double sx, double sy)
{
    setIdentity(S);
    S[0][0] = sx;
    S[1][1] = sy;
}

// Same row-vector rotation convention as the supplied transformation file.
void createRotation(double R[3][3], double theta)
{
    const double PI = acos(-1.0);
    double rad = theta * PI / 180.0;

    R[0][0] = cos(rad);
    R[0][1] = sin(rad);
    R[0][2] = 0;

    R[1][0] = -sin(rad);
    R[1][1] = cos(rad);
    R[1][2] = 0;

    R[2][0] = 0;
    R[2][1] = 0;
    R[2][2] = 1;
}

void transformPoint(double x, double y, double M[3][3],
                    int &outX, int &outY)
{
    double px = x * M[0][0] + y * M[1][0] + M[2][0];
    double py = x * M[0][1] + y * M[1][1] + M[2][1];

    outX = (int)round(px);
    outY = (int)round(py);
}

// ------------------------------------------------------------
// DDA LINE ALGORITHM
// ------------------------------------------------------------
void drawLineDDA(int x1, int y1, int x2, int y2, int color)
{
    int dx = x2 - x1;
    int dy = y2 - y1;
    int steps = max(abs(dx), abs(dy));

    if (steps == 0)
    {
        putpixel(x1, y1, color);
        return;
    }

    double xInc = (double)dx / steps;
    double yInc = (double)dy / steps;

    double x = x1;
    double y = y1;

    for (int i = 0; i <= steps; i++)
    {
        putpixel((int)round(x), (int)round(y), color);
        x += xInc;
        y += yInc;
    }
}

// ------------------------------------------------------------
// BRESENHAM LINE ALGORITHM
// ------------------------------------------------------------
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

// ------------------------------------------------------------
// BRESENHAM CIRCLE ALGORITHM
// ------------------------------------------------------------
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

void bresenhamCircleGraphics(int xc, int yc, int R, int color)
{
    int x = 0;
    int y = R;
    int d = 3 - 2 * R;

    while (x <= y)
    {
        plot8Symmetry(xc, yc, x, y, color);

        if (d < 0)
            d = d + 4 * x + 6;
        else
        {
            d = d + 4 * (x - y) + 10;
            y--;
        }
        x++;
    }
}

// ------------------------------------------------------------
// DRAW TRANSFORMED POLYGON
// ------------------------------------------------------------
void drawTransformedPolygon(const double local[][3], int n,
                            double M[3][3], int color)
{
    for (int i = 0; i < n; i++)
    {
        int j = (i + 1) % n;

        int x1, y1, x2, y2;

        transformPoint(local[i][0], local[i][1], M, x1, y1);
        transformPoint(local[j][0], local[j][1], M, x2, y2);

        drawLineBresenham(x1, y1, x2, y2, color);
    }
}

// ------------------------------------------------------------
// PLAYER CAR
//
// Local coordinates are centred around (0,0).
// Translation + rotation are applied to the complete car.
// Scaling is also part of the transformation pipeline.
// ------------------------------------------------------------
const double PLAYER_BODY[4][3] =
{
    {-25, -40, 1},
    { 25, -40, 1},
    { 25,  40, 1},
    {-25,  40, 1}
};

const double PLAYER_WINDOW1[4][3] =
{
    {-16, -25, 1},
    { 16, -25, 1},
    { 16,  -5, 1},
    {-16,  -5, 1}
};

const double PLAYER_WINDOW2[4][3] =
{
    {-16,  5, 1},
    { 16,  5, 1},
    { 16, 25, 1},
    {-16, 25, 1}
};

void makeCarMatrix(double M[3][3], double x, double y,
                   double angle, double scale)
{
    double S[3][3], R[3][3], T[3][3];
    double SR[3][3];

    // Composite transformation:
    // P' = P x S x R x T
    createScaling(S, scale, scale);
    createRotation(R, angle);
    createTranslation(T, x, y);

    multiplyMatrix(S, R, SR);
    multiplyMatrix(SR, T, M);
}

void drawWheelTransformed(double localX, double localY,
                          double M[3][3], int radius)
{
    int x, y;
    transformPoint(localX, localY, M, x, y);
    bresenhamCircleGraphics(x, y, radius, BLACK);
}

void drawPlayerCar(double x, double y, double angle, double scale)
{
    double M[3][3];

    makeCarMatrix(M, x, y, angle, scale);

    // Body
    drawTransformedPolygon(PLAYER_BODY, 4, M, RED);

    // Windows
    drawTransformedPolygon(PLAYER_WINDOW1, 4, M, WHITE);
    drawTransformedPolygon(PLAYER_WINDOW2, 4, M, WHITE);

    // Wheels
    drawWheelTransformed(-25, -28, M, (int)(8 * scale));
    drawWheelTransformed( 25, -28, M, (int)(8 * scale));
    drawWheelTransformed(-25,  28, M, (int)(8 * scale));
    drawWheelTransformed( 25,  28, M, (int)(8 * scale));

    // Front indicator line
    int x1, y1, x2, y2;
    transformPoint(-12, -40, M, x1, y1);
    transformPoint( 12, -40, M, x2, y2);
    drawLineBresenham(x1, y1, x2, y2, YELLOW);
}

// ------------------------------------------------------------
// OBSTACLE CAR
// ------------------------------------------------------------
void drawObstacleCar(double x, double y, double angle = 0)
{
    const double body[4][3] =
    {
        {-20, -25, 1},
        { 20, -25, 1},
        { 20,  25, 1},
        {-20,  25, 1}
    };

    const double window[4][3] =
    {
        {-12, -15, 1},
        { 12, -15, 1},
        { 12,   0, 1},
        {-12,   0, 1}
    };

    double M[3][3];
    makeCarMatrix(M, x, y, angle, 1.0);

    drawTransformedPolygon(body, 4, M, CYAN);
    drawTransformedPolygon(window, 4, M, WHITE);

    drawWheelTransformed(-20, -17, M, 6);
    drawWheelTransformed( 20, -17, M, 6);
    drawWheelTransformed(-20,  17, M, 6);
    drawWheelTransformed( 20,  17, M, 6);
}

// ------------------------------------------------------------
// ROAD / ENVIRONMENT
// ------------------------------------------------------------
void drawRoad()
{
    // Main road
    drawLineDDA(150, 50, 150, 450, WHITE);
    drawLineDDA(450, 50, 450, 450, WHITE);

    // Outer boundaries
    drawLineDDA(140, 50, 140, 450, LIGHTGRAY);
    drawLineDDA(460, 50, 460, 450, LIGHTGRAY);

    // Centre divider
    for (int y = 50; y < 450; y += 110)
        drawLineDDA(300, y, 300, y + 60, YELLOW);

    // Parking bays
    drawLineDDA(150, 100, 230, 100, WHITE);
    drawLineDDA(150, 180, 230, 180, WHITE);

    drawLineDDA(150, 260, 230, 260, WHITE);
    drawLineDDA(150, 340, 230, 340, WHITE);

    // Right-side shoulder markings
    drawLineDDA(370, 100, 450, 100, LIGHTGRAY);
    drawLineDDA(370, 180, 450, 180, LIGHTGRAY);
    drawLineDDA(370, 260, 450, 260, LIGHTGRAY);
    drawLineDDA(370, 340, 450, 340, LIGHTGRAY);
}

void drawCoins()
{
    // Coins are fixed on the centre track.
    bresenhamCircleGraphics(300, 90, 8, YELLOW);
    bresenhamCircleGraphics(300, 140, 8, YELLOW);
    bresenhamCircleGraphics(300, 190, 8, YELLOW);
    bresenhamCircleGraphics(300, 240, 8, YELLOW);
    bresenhamCircleGraphics(300, 290, 8, YELLOW);
    bresenhamCircleGraphics(300, 340, 8, YELLOW);
    bresenhamCircleGraphics(300, 390, 8, YELLOW);
}

void drawTrafficSignal()
{
    drawLineDDA(500, 70, 500, 200, WHITE);

    drawLineBresenham(480, 70, 520, 70, WHITE);
    drawLineBresenham(520, 70, 520, 140, WHITE);
    drawLineBresenham(520, 140, 480, 140, WHITE);
    drawLineBresenham(480, 140, 480, 70, WHITE);

    bresenhamCircleGraphics(500, 85, 6, RED);
    bresenhamCircleGraphics(500, 105, 6, YELLOW);
    bresenhamCircleGraphics(500, 125, 6, GREEN);
}

void drawSpeedometer(double speed)
{
    bresenhamCircleGraphics(70, 70, 30, WHITE);

    // Needle angle is calculated from speed.
    double angle = -135.0 + (speed / 100.0) * 270.0;
    const double PI = acos(-1.0);
    double rad = angle * PI / 180.0;

    int x2 = 70 + (int)(22 * cos(rad));
    int y2 = 70 + (int)(22 * sin(rad));

    drawLineBresenham(70, 70, x2, y2, WHITE);

    outtextxy(50, 108, (char*)"SPEED");
}

void drawSteeringWheel(double steering)
{
    bresenhamCircleGraphics(70, 400, 25, WHITE);

    drawLineBresenham(45, 400, 95, 400, WHITE);
    drawLineBresenham(70, 375, 70, 425, WHITE);

    // Steering indicator
    int cx = 70 + (int)(15 * steering);
    drawLineBresenham(70, 400, cx, 385, YELLOW);
}

void drawTrees()
{
    drawLineDDA(80, 120, 80, 150, BROWN);
    bresenhamCircleGraphics(80, 105, 15, GREEN);
    bresenhamCircleGraphics(70, 110, 10, GREEN);
    bresenhamCircleGraphics(90, 110, 10, GREEN);

    drawLineDDA(80, 250, 80, 280, BROWN);
    bresenhamCircleGraphics(80, 235, 15, GREEN);
    bresenhamCircleGraphics(70, 240, 10, GREEN);
    bresenhamCircleGraphics(90, 240, 10, GREEN);

    drawLineDDA(520, 280, 520, 310, BROWN);
    bresenhamCircleGraphics(520, 265, 15, GREEN);
    bresenhamCircleGraphics(510, 270, 10, GREEN);
    bresenhamCircleGraphics(530, 270, 10, GREEN);
}

// ------------------------------------------------------------
// TRACK / COLLISION HELPERS
// ------------------------------------------------------------
bool insideRoad(double x)
{
    return x >= 170 && x <= 430;
}

bool collision(double x1, double y1, double x2, double y2)
{
    return (fabs(x1 - x2) < 48 && fabs(y1 - y2) < 60);
}

void clampPlayer(double &x, double &y)
{
    // Keep the car on the road.
    if (x < 180) x = 180;
    if (x > 420) x = 420;

    if (y < 85)  y = 85;
    if (y > 415) y = 415;
}

// ------------------------------------------------------------
// HUD
// ------------------------------------------------------------
void drawHUD(double speed, double angle, int score)
{
    outtextxy(175, 465, (char*)"ARROWS / WASD = DRIVE");
    outtextxy(175, 480, (char*)"SPACE = BRAKE   R = RESET   Q = QUIT");

    char info[100];

    sprintf(info, "Speed: %d", (int)speed);
    outtextxy(20, 155, info);

    sprintf(info, "Angle: %d", (int)angle);
    outtextxy(20, 175, info);

    sprintf(info, "Score: %d", score);
    outtextxy(20, 195, info);
}

// ------------------------------------------------------------
// COMPLETE SCENE
//
// IMPORTANT:
// There is deliberately NO setactivepage(),
// setvisualpage(), page flipping, or off-screen buffer.
// Everything is drawn directly to the current screen.
// ------------------------------------------------------------
void renderScene(double playerX, double playerY,
                 double playerAngle, double playerScale,
                 double speed, double steering,
                 int score,
                 double obstacle1Y,
                 double obstacle2Y)
{
    setbkcolor(DARKGRAY);
    cleardevice();

    outtextxy(175, 20, (char*)"DR. DRIVING 2D - TRANSFORMATION EDITION");

    drawRoad();
    drawCoins();
    drawTrafficSignal();
    drawTrees();

    drawSpeedometer(speed);
    drawSteeringWheel(steering);

    // Moving obstacle cars
    drawObstacleCar(210, obstacle1Y, 0);
    drawObstacleCar(390, obstacle2Y, 0);

    // Player car transformed using:
    // Scaling -> Rotation -> Translation
    drawPlayerCar(playerX, playerY, playerAngle, playerScale);

    drawHUD(speed, playerAngle, score);
}

// ------------------------------------------------------------
// RESET GAME
// ------------------------------------------------------------
void resetGame(double &x, double &y, double &angle,
               double &speed, double &steering,
               double &ob1, double &ob2, int &score)
{
    x = 350;
    y = 350;
    angle = 0;
    speed = 0;
    steering = 0;

    ob1 = 130;
    ob2 = 250;

    score = 0;
}

// ------------------------------------------------------------
// MAIN GAME
// ------------------------------------------------------------
int main()
{
    int gd = DETECT, gm;
    initgraph(&gd, &gm, (char*)"");

    if (graphresult() != grOk)
    {
        cout << "Graphics initialization failed.\n";
        return 1;
    }

    double playerX, playerY;
    double playerAngle;
    double playerScale;

    double speed;
    double steering;

    double obstacle1Y;
    double obstacle2Y;

    int score;

    resetGame(playerX, playerY, playerAngle,
              speed, steering,
              obstacle1Y, obstacle2Y, score);

    playerScale = 1.0;

    bool running = true;
    bool crashed = false;

    // Initial frame
    renderScene(playerX, playerY,
                playerAngle, playerScale,
                speed, steering, score,
                obstacle1Y, obstacle2Y);

    while (running)
    {
        // ----------------------------------------------------
        // KEYBOARD CONTROL - SDL2 (WSL / Linux compatible)
        // ----------------------------------------------------
        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = false;
            }

            if (event.type == SDL_KEYDOWN)
            {
                SDL_Keycode key = event.key.keysym.sym;

                // QUIT
                if (key == SDLK_q || key == SDLK_ESCAPE)
                {
                    running = false;
                }

                // UP / W - accelerate
                else if (key == SDLK_UP || key == SDLK_w)
                {
                    speed += 5;
                }

                // DOWN / S - brake
                else if (key == SDLK_DOWN || key == SDLK_s)
                {
                    speed -= 5;
                }

                // LEFT / A - steer left
                else if (key == SDLK_LEFT || key == SDLK_a)
                {
                    playerX -= 8;
                    playerAngle -= 5;
                    steering = -1;
                }

                // RIGHT / D - steer right
                else if (key == SDLK_RIGHT || key == SDLK_d)
                {
                    playerX += 8;
                    playerAngle += 5;
                    steering = 1;
                }

                // SPACE - hard brake
                else if (key == SDLK_SPACE)
                {
                    speed -= 15;
                }

                // R - restart
                else if (key == SDLK_r)
                {
                    resetGame(playerX, playerY, playerAngle,
                               speed, steering,
                               obstacle1Y, obstacle2Y, score);

                    crashed = false;
                }

                // + / = - scaling up
                else if (key == SDLK_PLUS || key == SDLK_EQUALS)
                {
                    playerScale += 0.05;

                    if (playerScale > 1.25)
                        playerScale = 1.25;
                }

                // - - scaling down
                else if (key == SDLK_MINUS)
                {
                    playerScale -= 0.05;

                    if (playerScale < 0.80)
                        playerScale = 0.80;
                }

                // C - straighten car
                else if (key == SDLK_c)
                {
                    playerAngle = 0;
                    steering = 0;
                }
            }
        }

        // ----------------------------------------------------
        // AUTOMATIC FORWARD MOVEMENT
        // ----------------------------------------------------
        if (speed > 0)
        {
            // Car moves upward along the track.
            playerY -= speed * 0.045;

            // Small steering influence.
            if (playerAngle > 0)
                playerX += 0.8;
            else if (playerAngle < 0)
                playerX -= 0.8;
        }

        // Wrap the player to the bottom after reaching the top.
        if (playerY < 80)
        {
            playerY = 420;
            score += 10;
        }

        // ----------------------------------------------------
        // MOVING OBSTACLES
        // ----------------------------------------------------
        obstacle1Y += 1.2;
        obstacle2Y += 0.8;

        if (obstacle1Y > 440)
            obstacle1Y = 80;

        if (obstacle2Y > 440)
            obstacle2Y = 80;

        // ----------------------------------------------------
        // COLLISION
        // ----------------------------------------------------
        if (collision(playerX, playerY, 210, obstacle1Y) ||
            collision(playerX, playerY, 390, obstacle2Y))
        {
            crashed = true;
            speed = 0;
        }

        // ----------------------------------------------------
        // DRAW DIRECTLY TO SCREEN
        // ----------------------------------------------------
        renderScene(playerX, playerY,
                    playerAngle, playerScale,
                    speed, steering, score,
                    obstacle1Y, obstacle2Y);

        if (crashed)
        {
            outtextxy(205, 215, (char*)"!!! CRASH !!!");
            outtextxy(170, 235, (char*)"Press R to restart");
        }

        delay(25);
    }

    closegraph();
    return 0;
}
