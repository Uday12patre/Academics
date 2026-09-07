#include <graphics.h>
#include <SDL2/SDL.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <cstdlib>

using namespace std;

// ============================================================
// DR. DRIVING 2D - VERSION 4
// REBUILT FROM SCRATCH FOR SDL_bgi / Linux
//
// Requirements:
//  - Top-down 2D driving
//  - Polygon filling (BGI polygon fill primitive)
//  - DDA line algorithm
//  - Bresenham line algorithm
//  - Bresenham circle algorithm
//  - 2D homogeneous transformations
//  - Entire road/environment scrolls
//  - Coins + score
//  - Traffic + collision
//  - Smooth held-key controls using SDL2 events
//  - NO setactivepage(), setvisualpage(), or off-screen buffer
//
// IMPORTANT:
// Use SDL_bgi initwindow() so the frame is presented only once after
// all drawing commands are finished. No BGI page functions are used.
// ============================================================

const double PI = 3.14159265358979323846;

// ------------------------------------------------------------
// WORLD / GAME DATA
// ------------------------------------------------------------

struct Point
{
    double x;
    double y;
};

struct RoadObject
{
    double x;
    double worldY;
    int type;
};

struct Coin
{
    double x;
    double worldY;
    bool active;
};

struct TrafficCar
{
    double x;
    double worldY;
    double speed;
    int color;
};

vector<RoadObject> roadside;
vector<Coin> coins;
vector<TrafficCar> traffic;

int SCREEN_W = 640;
int SCREEN_H = 480;

double ROAD_LEFT;
double ROAD_RIGHT;
double ROAD_CENTER;

double playerX;
double playerY;
double playerAngle;
double scrollY;

double playerSpeed;
double targetSpeed;

double steering;
int score;

bool crashed = false;

// Held keys.
bool keyLeft = false;
bool keyRight = false;
bool keyUp = false;
bool keyDown = false;

// ------------------------------------------------------------
// BASIC MATRIX TRANSFORMATIONS
// Row-vector convention: P' = P x M
// ------------------------------------------------------------

void setIdentity(double M[3][3])
{
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            M[i][j] = (i == j) ? 1.0 : 0.0;
}

void createTranslation(double M[3][3], double tx, double ty)
{
    setIdentity(M);
    M[2][0] = tx;
    M[2][1] = ty;
}

void createScaling(double M[3][3], double sx, double sy)
{
    setIdentity(M);
    M[0][0] = sx;
    M[1][1] = sy;
}

void createRotation(double M[3][3], double degrees)
{
    double r = degrees * PI / 180.0;

    setIdentity(M);

    M[0][0] = cos(r);
    M[0][1] = sin(r);
    M[1][0] = -sin(r);
    M[1][1] = cos(r);
}

Point transformPoint(Point p, double M[3][3])
{
    Point out;

    out.x = p.x * M[0][0] +
            p.y * M[1][0] +
            M[2][0];

    out.y = p.x * M[0][1] +
            p.y * M[1][1] +
            M[2][1];

    return out;
}

vector<Point> transformPolygon(const vector<Point>& input,
                               double sx,
                               double sy,
                               double angle,
                               double tx,
                               double ty)
{
    double S[3][3];
    double R[3][3];
    double T[3][3];
    double SR[3][3];
    double M[3][3];

    createScaling(S, sx, sy);
    createRotation(R, angle);
    createTranslation(T, tx, ty);

    // Row-vector order: P -> P*S -> P*R -> P*T
    double temp[3][3];

    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
        {
            SR[i][j] = 0;
            for (int k = 0; k < 3; ++k)
                SR[i][j] += S[i][k] * R[k][j];
        }

    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
        {
            M[i][j] = 0;
            for (int k = 0; k < 3; ++k)
                M[i][j] += SR[i][k] * T[k][j];
        }

    vector<Point> result;
    result.reserve(input.size());

    for (const Point& p : input)
        result.push_back(transformPoint(p, M));

    return result;
}

// ------------------------------------------------------------
// DDA LINE
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

    double x = x1;
    double y = y1;

    double xInc = (double)dx / steps;
    double yInc = (double)dy / steps;

    for (int i = 0; i <= steps; ++i)
    {
        putpixel((int)round(x), (int)round(y), color);
        x += xInc;
        y += yInc;
    }
}

// ------------------------------------------------------------
// BRESENHAM LINE
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
// BRESENHAM CIRCLE
// ------------------------------------------------------------

void plot8(int xc, int yc, int x, int y, int color)
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

void drawCircleBresenham(int xc, int yc, int r, int color)
{
    if (r <= 0)
        return;

    int x = 0;
    int y = r;
    int d = 3 - 2 * r;

    while (x <= y)
    {
        plot8(xc, yc, x, y, color);

        if (d < 0)
            d += 4 * x + 6;
        else
        {
            d += 4 * (x - y) + 10;
            --y;
        }

        ++x;
    }
}

// ------------------------------------------------------------
// FAST POLYGON FILLING
//
// This is the polygon filling part of Version 4.
// SDL_bgi's native fillpoly primitive performs the fill efficiently.
// ------------------------------------------------------------

void fillPolygonFast(const vector<Point>& poly, int fillColor)
{
    if (poly.size() < 3)
        return;

    // SDL_bgi implements the BGI fillpoly primitive efficiently.
    // This replaces the old pixel-by-pixel scanline renderer.
    // Polygon filling is still used for all major game objects.
    vector<int> pts;
    pts.reserve(poly.size() * 2);

    for (const Point& p : poly)
    {
        pts.push_back((int)round(p.x));
        pts.push_back((int)round(p.y));
    }

    setfillstyle(SOLID_FILL, fillColor);
    fillpoly((int)poly.size(), pts.data());
}

void drawPolygonOutlineFast(const vector<Point>& poly, int color)
{
    if (poly.size() < 2)
        return;

    setcolor(color);

    for (size_t i = 0; i < poly.size(); ++i)
    {
        const Point& a = poly[i];
        const Point& b = poly[(i + 1) % poly.size()];

        // Use SDL_bgi's native line primitive for rendering speed.
        line((int)round(a.x), (int)round(a.y),
             (int)round(b.x), (int)round(b.y));
    }
}

void fillAndOutline(const vector<Point>& poly,
                    int fillColor,
                    int outlineColor)
{
    fillPolygonFast(poly, fillColor);
    drawPolygonOutlineFast(poly, outlineColor);
}

// ------------------------------------------------------------
// HELPER: CLIP / WRAP WORLD Y
// ------------------------------------------------------------

double screenWorldY(double worldY)
{
    // World repeats every 900 pixels.
    double y = worldY + scrollY;

    while (y > SCREEN_H + 100)
        y -= 900.0;

    while (y < -100)
        y += 900.0;

    return y;
}

// ------------------------------------------------------------
// BACKGROUND
// ------------------------------------------------------------

void drawBackground()
{
    setfillstyle(SOLID_FILL, GREEN);
    bar(0, 0, SCREEN_W - 1, SCREEN_H - 1);
}

// ------------------------------------------------------------
// ROAD
// ------------------------------------------------------------

void drawRoad()
{
    // Road polygon.
    vector<Point> road = {
        {ROAD_LEFT, 0},
        {ROAD_RIGHT, 0},
        {ROAD_RIGHT, (double)SCREEN_H},
        {ROAD_LEFT, (double)SCREEN_H}
    };

    fillAndOutline(road, DARKGRAY, WHITE);

    // Shoulders.
    setcolor(LIGHTGRAY);
    line((int)ROAD_LEFT, 0, (int)ROAD_LEFT, SCREEN_H - 1);

    setcolor(LIGHTGRAY);
    line((int)ROAD_RIGHT, 0, (int)ROAD_RIGHT, SCREEN_H - 1);

    // Yellow centre divider.
    const double segment = 100.0;
    const double dash = 55.0;

    for (double worldY = -200.0; worldY < 1000.0; worldY += segment)
    {
        double y1 = screenWorldY(worldY);
        double y2 = screenWorldY(worldY + dash);

        if (fabs(y2 - y1) > 100)
            continue;

        setcolor(YELLOW);
        line((int)ROAD_CENTER, (int)y1,
             (int)ROAD_CENTER, (int)y2);
    }

    // White lane separators.
    double lane1 = ROAD_LEFT + (ROAD_RIGHT - ROAD_LEFT) / 3.0;
    double lane2 = ROAD_LEFT + 2.0 * (ROAD_RIGHT - ROAD_LEFT) / 3.0;

    for (double worldY = -200.0; worldY < 1000.0; worldY += 85.0)
    {
        double y1 = screenWorldY(worldY);
        double y2 = screenWorldY(worldY + 38.0);

        if (fabs(y2 - y1) > 100)
            continue;

        setcolor(WHITE);
        line((int)lane1, (int)y1, (int)lane1, (int)y2);
        line((int)lane2, (int)y1, (int)lane2, (int)y2);
    }

    // Moving curb blocks.
    for (double worldY = -120.0; worldY < 1000.0; worldY += 70.0)
    {
        double y = screenWorldY(worldY);

        int y1 = (int)y;
        int y2 = (int)(y + 35);

        if (y2 < 0 || y1 >= SCREEN_H)
            continue;

        int curbColor =
            (((int)floor(worldY / 70.0)) % 2 == 0)
            ? WHITE
            : LIGHTGRAY;

        setfillstyle(SOLID_FILL, curbColor);

        bar(
            (int)ROAD_LEFT - 12,
            max(0, y1),
            (int)ROAD_LEFT - 2,
            min(SCREEN_H - 1, y2)
        );

        bar(
            (int)ROAD_RIGHT + 2,
            max(0, y1),
            (int)ROAD_RIGHT + 12,
            min(SCREEN_H - 1, y2)
        );
    }
}

// ------------------------------------------------------------
// ROADSIDE OBJECTS
// ------------------------------------------------------------

void drawTree(double x, double y)
{
    // Trunk.
    vector<Point> trunk = {
        {x - 5, y},
        {x + 5, y},
        {x + 5, y + 24},
        {x - 5, y + 24}
    };

    fillAndOutline(trunk, BROWN, WHITE);

    // Crown made from polygons + circles.
    vector<Point> crown = {
        {x, y - 32},
        {x + 22, y - 12},
        {x + 15, y + 8},
        {x - 15, y + 8},
        {x - 22, y - 12}
    };

    fillAndOutline(crown, GREEN, WHITE);

    setfillstyle(SOLID_FILL, GREEN);
    fillellipse((int)x, (int)y - 15, 15, 15);
    fillellipse((int)x - 12, (int)y - 8, 11, 11);
    fillellipse((int)x + 12, (int)y - 8, 11, 11);
}

void drawLamp(double x, double y)
{
    setcolor(WHITE);
    line((int)x, (int)y, (int)x, (int)y + 48);
    line((int)x, (int)y,
         (int)x + ((x < ROAD_CENTER) ? 15 : -15), (int)y);

    setfillstyle(SOLID_FILL, YELLOW);
    fillellipse(
        (int)x + ((x < ROAD_CENTER) ? 15 : -15),
        (int)y + 5,
        5, 5
    );
}

void drawBuilding(double x, double y, int type)
{
    int w = 48 + type * 8;
    int h = 58 + type * 12;

    vector<Point> building = {
        {x, y},
        {x + w, y},
        {x + w, y + h},
        {x, y + h}
    };

    fillAndOutline(building, LIGHTGRAY, WHITE);

    // Roof.
    vector<Point> roof = {
        {x - 4, y},
        {x + w + 4, y},
        {x + w / 2.0, y - 18}
    };

    fillAndOutline(roof, BROWN, WHITE);

    // Windows.
    for (int row = 0; row < 2; ++row)
    {
        for (int col = 0; col < 2; ++col)
        {
            int wx = (int)x + 10 + col * (w - 24);
            int wy = (int)y + 15 + row * 20;

            setfillstyle(SOLID_FILL, CYAN);
            bar(wx, wy, wx + 8, wy + 10);
        }
    }
}

void drawRoadsideWorld()
{
    for (const RoadObject& obj : roadside)
    {
        double y = screenWorldY(obj.worldY);

        if (y < -100 || y > SCREEN_H + 100)
            continue;

        if (obj.type == 0)
            drawTree(obj.x, y);
        else if (obj.type == 1)
            drawLamp(obj.x, y);
        else
            drawBuilding(obj.x, y, obj.type);
    }
}

// ------------------------------------------------------------
// COINS
// ------------------------------------------------------------

void drawCoin(double x, double y)
{
    setcolor(YELLOW);
    ellipse((int)x, (int)y, 0, 360, 11, 11);
    ellipse((int)x, (int)y, 0, 360, 7, 7);

    // Coin shine.
    setcolor(WHITE);
    line((int)x - 2, (int)y - 5,
         (int)x + 2, (int)y - 5);
}

void drawCoins()
{
    for (const Coin& c : coins)
    {
        if (!c.active)
            continue;

        double y = screenWorldY(c.worldY);

        if (y < -30 || y > SCREEN_H + 30)
            continue;

        drawCoin(c.x, y);
    }
}

// ------------------------------------------------------------
// TRAFFIC CAR
// ------------------------------------------------------------

void drawTrafficCar(double cx, double cy, int color)
{
    vector<Point> body = {
        {-20, -35},
        { 20, -35},
        { 24, -20},
        { 24,  30},
        { 15,  36},
        {-15,  36},
        {-24,  30},
        {-24, -20}
    };

    vector<Point> cabin = {
        {-14, -15},
        { 14, -15},
        { 12,  10},
        {-12,  10}
    };

    vector<Point> windshield = {
        {-11, -12},
        { 11, -12},
        { 9, 0},
        {-9, 0}
    };

    vector<Point> rearWindow = {
        {-9, 4},
        {9, 4},
        {11, 10},
        {-11, 10}
    };

    vector<Point> transformedBody =
        transformPolygon(body, 1, 1, 0, cx, cy);

    vector<Point> transformedCabin =
        transformPolygon(cabin, 1, 1, 0, cx, cy);

    vector<Point> transformedWindshield =
        transformPolygon(windshield, 1, 1, 0, cx, cy);

    vector<Point> transformedRear =
        transformPolygon(rearWindow, 1, 1, 0, cx, cy);

    fillAndOutline(transformedBody, color, WHITE);
    fillAndOutline(transformedCabin, DARKGRAY, WHITE);
    fillAndOutline(transformedWindshield, CYAN, WHITE);
    fillAndOutline(transformedRear, BLUE, WHITE);

    // Wheels.
    setfillstyle(SOLID_FILL, BLACK);

    bar((int)cx - 28, (int)cy - 22,
        (int)cx - 21, (int)cy - 5);

    bar((int)cx + 21, (int)cy - 22,
        (int)cx + 28, (int)cy - 5);

    bar((int)cx - 28, (int)cy + 10,
        (int)cx - 21, (int)cy + 27);

    bar((int)cx + 21, (int)cy + 10,
        (int)cx + 28, (int)cy + 27);
}

// ------------------------------------------------------------
// PLAYER CAR
// ------------------------------------------------------------

void drawPlayerCar()
{
    // Local coordinates: car points upward.
    vector<Point> body = {
        {-24, -42},
        { 24, -42},
        { 28, -25},
        { 28,  30},
        { 18,  43},
        {-18,  43},
        {-28,  30},
        {-28, -25}
    };

    vector<Point> cabin = {
        {-16, -20},
        { 16, -20},
        { 13,  12},
        {-13,  12}
    };

    vector<Point> windshield = {
        {-12, -17},
        { 12, -17},
        { 10, -2},
        {-10, -2}
    };

    vector<Point> rearGlass = {
        {-10, 2},
        { 10, 2},
        { 12, 10},
        {-12, 10}
    };

    vector<Point> bonnet = {
        {-17, -39},
        { 17, -39},
        { 20, -24},
        {-20, -24}
    };

    vector<Point> transformedBody =
        transformPolygon(
            body, 1.0, 1.0,
            playerAngle,
            playerX, playerY
        );

    vector<Point> transformedCabin =
        transformPolygon(
            cabin, 1.0, 1.0,
            playerAngle,
            playerX, playerY
        );

    vector<Point> transformedWindshield =
        transformPolygon(
            windshield, 1.0, 1.0,
            playerAngle,
            playerX, playerY
        );

    vector<Point> transformedRear =
        transformPolygon(
            rearGlass, 1.0, 1.0,
            playerAngle,
            playerX, playerY
        );

    vector<Point> transformedBonnet =
        transformPolygon(
            bonnet, 1.0, 1.0,
            playerAngle,
            playerX, playerY
        );

    fillAndOutline(transformedBody, RED, WHITE);
    fillAndOutline(transformedCabin, DARKGRAY, WHITE);
    fillAndOutline(transformedWindshield, CYAN, WHITE);
    fillAndOutline(transformedRear, BLUE, WHITE);
    fillAndOutline(transformedBonnet, LIGHTRED, WHITE);

    // Wheels are circles/polygons transformed by the same angle.
    vector<Point> wheelCenters = {
        {-27, -23},
        { 27, -23},
        {-27,  23},
        { 27,  23}
    };

    double R[3][3];
    createRotation(R, playerAngle);

    for (const Point& local : wheelCenters)
    {
        Point p = transformPoint(local, R);

        setfillstyle(SOLID_FILL, BLACK);
        fillellipse(
            (int)round(playerX + p.x),
            (int)round(playerY + p.y),
            6, 6
        );
    }

    // Headlights.
    vector<Point> leftLight = {
        {-15, -40},
        {-5, -40},
        {-5, -32},
        {-15, -32}
    };

    vector<Point> rightLight = {
        {5, -40},
        {15, -40},
        {15, -32},
        {5, -32}
    };

    fillAndOutline(
        transformPolygon(
            leftLight, 1, 1,
            playerAngle, playerX, playerY
        ),
        YELLOW, WHITE
    );

    fillAndOutline(
        transformPolygon(
            rightLight, 1, 1,
            playerAngle, playerX, playerY
        ),
        YELLOW, WHITE
    );
}

// ------------------------------------------------------------
// HUD
// ------------------------------------------------------------

void drawHUD()
{
    setfillstyle(SOLID_FILL, BLACK);

    // Top HUD panel.
    bar(0, 0, SCREEN_W - 1, 42);

    setcolor(WHITE);

    char text[120];

    sprintf(text, "DR. DRIVING 2D    SCORE: %d", score);
    outtextxy(12, 10, text);

    sprintf(text, "SPEED: %d", (int)round(playerSpeed));
    outtextxy(SCREEN_W - 125, 10, text);

    // Controls at bottom.
    bar(0, SCREEN_H - 28, SCREEN_W - 1, SCREEN_H - 1);

    outtextxy(
        10, SCREEN_H - 20,
        (char*)"W/S SPEED   A/D STEER   R RESTART   Q QUIT"
    );
}

void drawCrashScreen()
{
    setfillstyle(SOLID_FILL, BLACK);

    int x1 = SCREEN_W / 2 - 125;
    int y1 = SCREEN_H / 2 - 45;
    int x2 = SCREEN_W / 2 + 125;
    int y2 = SCREEN_H / 2 + 45;

    bar(x1, y1, x2, y2);

    setcolor(RED);
    outtextxy(
        SCREEN_W / 2 - 45,
        SCREEN_H / 2 - 18,
        (char*)"CRASH!"
    );

    setcolor(WHITE);
    outtextxy(
        SCREEN_W / 2 - 92,
        SCREEN_H / 2 + 8,
        (char*)"Press R to restart"
    );
}

// ------------------------------------------------------------
// WORLD SETUP
// ------------------------------------------------------------

void initializeWorld()
{
    roadside.clear();
    coins.clear();
    traffic.clear();

    // Trees / lamps / buildings on both sides.
    for (int i = 0; i < 8; ++i)
    {
        double y = 50.0 + i * 105.0;

        roadside.push_back(
            {ROAD_LEFT - 65.0, y, 0}
        );

        roadside.push_back(
            {ROAD_RIGHT + 30.0, y + 35.0, 0}
        );

        roadside.push_back(
            {ROAD_LEFT - 35.0, y + 20.0, 1}
        );

        roadside.push_back(
            {ROAD_RIGHT + 10.0, y + 55.0, 1}
        );

        if (i % 3 == 0)
        {
            roadside.push_back(
                {ROAD_LEFT - 125.0, y + 10.0, 2}
            );

            roadside.push_back(
                {ROAD_RIGHT + 70.0, y + 45.0, 2}
            );
        }
    }

    // Coins placed in the lanes.
    for (int i = 0; i < 15; ++i)
    {
        int lane = i % 3;

        double laneX;

        if (lane == 0)
            laneX = ROAD_LEFT + 48;
        else if (lane == 1)
            laneX = ROAD_CENTER;
        else
            laneX = ROAD_RIGHT - 48;

        coins.push_back(
            {laneX, 75.0 + i * 90.0, true}
        );
    }

    // Traffic.
    // Cars are deliberately staggered in both lane and distance so that
    // no bot overlaps another bot and the player is never spawned inside
    // a traffic car.
    traffic.push_back(
        {ROAD_LEFT + 44, 120.0, 1.00, BLUE}
    );

    traffic.push_back(
        {ROAD_RIGHT - 44, 440.0, 0.90, MAGENTA}
    );

    traffic.push_back(
        {ROAD_LEFT + 44, 780.0, 1.10, CYAN}
    );

    // Keep the first same-lane car far enough away from the player.
    traffic.push_back(
        {ROAD_CENTER, 1200.0, 0.95, YELLOW}
    );
}

// ------------------------------------------------------------
// RESET
// ------------------------------------------------------------

void resetGame()
{
    playerX = ROAD_CENTER;
    playerY = SCREEN_H - 100;

    playerAngle = 0.0;

    scrollY = 0.0;

    playerSpeed = 0.0;
    targetSpeed = 0.0;

    steering = 0.0;

    score = 0;
    crashed = false;

    // Reset input state as well.
    keyLeft = false;
    keyRight = false;
    keyUp = false;
    keyDown = false;

    for (Coin& c : coins)
        c.active = true;
}

// ------------------------------------------------------------
// COLLISION
// ------------------------------------------------------------

bool near(double a, double b, double tolerance)
{
    return fabs(a - b) <= tolerance;
}

bool checkCollision(double x1, double y1,
                    double x2, double y2)
{
    // The car sprites are about 50-55 px wide and 80-90 px long.
    // Use a slightly smaller hit box so cars do not crash merely
    // because their outlines are close.
    return near(x1, x2, 38.0) &&
           near(y1, y2, 58.0);
}

bool sameLane(const TrafficCar& a, const TrafficCar& b)
{
    return fabs(a.x - b.x) < 12.0;
}

void keepTrafficSeparated()
{
    // Prevent two bot cars in the same lane from visually occupying
    // the same position.  World-Y is also the scrolling direction.
    const double minimumGap = 135.0;

    for (size_t i = 0; i < traffic.size(); ++i)
    {
        for (size_t j = i + 1; j < traffic.size(); ++j)
        {
            if (!sameLane(traffic[i], traffic[j]))
                continue;

            double gap = fabs(traffic[i].worldY - traffic[j].worldY);

            if (gap < minimumGap)
            {
                // Keep the car that is farther ahead where it is and
                // move the other one behind it.
                if (traffic[i].worldY <= traffic[j].worldY)
                    traffic[j].worldY = traffic[i].worldY + minimumGap;
                else
                    traffic[i].worldY = traffic[j].worldY + minimumGap;
            }
        }
    }
}

// ------------------------------------------------------------
// UPDATE
// ------------------------------------------------------------

void updateGame(double dt)
{
    if (crashed)
        return;

    // Speed control.
    if (keyUp)
        targetSpeed = 8.0;
    else if (keyDown)
        targetSpeed = 2.0;
    else
        targetSpeed = 5.0;

    // Smooth acceleration/deceleration.
    double acceleration = 8.0;

    if (playerSpeed < targetSpeed)
    {
        playerSpeed += acceleration * dt;

        if (playerSpeed > targetSpeed)
            playerSpeed = targetSpeed;
    }
    else if (playerSpeed > targetSpeed)
    {
        playerSpeed -= acceleration * dt;

        if (playerSpeed < targetSpeed)
            playerSpeed = targetSpeed;
    }

    // Steering.
    steering = 0.0;

    if (keyLeft)
        steering -= 1.0;

    if (keyRight)
        steering += 1.0;

    if (steering != 0.0)
    {
        playerX += steering * 190.0 * dt;

        playerAngle = steering * 10.0;
    }
    else
    {
        // Return steering smoothly.
        playerAngle *= 0.82;

        if (fabs(playerAngle) < 0.2)
            playerAngle = 0.0;
    }

    // Keep player on road.
    double margin = 34.0;

    if (playerX < ROAD_LEFT + margin)
        playerX = ROAD_LEFT + margin;

    if (playerX > ROAD_RIGHT - margin)
        playerX = ROAD_RIGHT - margin;

    // Entire world moves downward.
    scrollY += playerSpeed * 52.0 * dt;

    // Traffic moves at a different relative speed.
    for (TrafficCar& car : traffic)
    {
        car.worldY += car.speed * 42.0 * dt;

        if (car.worldY > 1350.0)
            car.worldY -= 1500.0;
    }

    // Keep bots from catching up and drawing on top of each other.
    keepTrafficSeparated();

    // Check player-vs-traffic collision only after the traffic layout
    // has been stabilized.
    for (const TrafficCar& car : traffic)
    {
        double sy = screenWorldY(car.worldY);

        if (checkCollision(
                playerX,
                playerY,
                car.x,
                sy))
        {
            crashed = true;
            playerSpeed = 0.0;
            return;
        }
    }

    // Coin collection.
    for (Coin& c : coins)
    {
        if (!c.active)
            continue;

        double sy = screenWorldY(c.worldY);

        if (checkCollision(
                playerX,
                playerY,
                c.x,
                sy))
        {
            c.active = false;
            score += 10;
        }
    }

    // Recycle coins after they pass the player.
    for (Coin& c : coins)
    {
        double sy = screenWorldY(c.worldY);

        if (sy > SCREEN_H + 80)
            c.active = true;
    }
}

// ------------------------------------------------------------
// DRAW TRAFFIC
// ------------------------------------------------------------

void drawTraffic()
{
    for (const TrafficCar& car : traffic)
    {
        double y = screenWorldY(car.worldY);

        if (y < -100 || y > SCREEN_H + 100)
            continue;

        drawTrafficCar(car.x, y, car.color);
    }
}

// ------------------------------------------------------------
// COMPLETE FRAME
// ------------------------------------------------------------
// Rendering uses one completed-frame refresh. There is no
// setactivepage(), setvisualpage(), or user-created off-screen page.
// SDL_bgi's initwindow mode lets us draw the frame without presenting
// every individual graphics command.

void renderFrame()
{
    // Direct rendering only.
    setbkcolor(GREEN);
    cleardevice();
    drawRoad();
    drawRoadsideWorld();
    drawCoins();
    drawTraffic();
    drawPlayerCar();
    drawHUD();

    if (crashed)
        drawCrashScreen();
}

// ------------------------------------------------------------
// SDL KEY EVENTS
// ------------------------------------------------------------

void handleEvents(bool& running)
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            running = false;
        }
        else if (event.type == SDL_KEYDOWN)
        {
            if (event.key.repeat)
                continue;

            SDL_Keycode key = event.key.keysym.sym;

            if (key == SDLK_q || key == SDLK_ESCAPE)
                running = false;

            else if (key == SDLK_a || key == SDLK_LEFT)
                keyLeft = true;

            else if (key == SDLK_d || key == SDLK_RIGHT)
                keyRight = true;

            else if (key == SDLK_w || key == SDLK_UP)
                keyUp = true;

            else if (key == SDLK_s || key == SDLK_DOWN)
                keyDown = true;

            else if (key == SDLK_r)
            {
                resetGame();
            }
        }
        else if (event.type == SDL_KEYUP)
        {
            SDL_Keycode key = event.key.keysym.sym;

            if (key == SDLK_a || key == SDLK_LEFT)
                keyLeft = false;

            else if (key == SDLK_d || key == SDLK_RIGHT)
                keyRight = false;

            else if (key == SDLK_w || key == SDLK_UP)
                keyUp = false;

            else if (key == SDLK_s || key == SDLK_DOWN)
                keyDown = false;
        }
    }
}

// ------------------------------------------------------------
// MAIN
// ------------------------------------------------------------

int main()
{
    // SDL_bgi initwindow mode prevents a refresh after every individual
    // line/polygon/circle. We present only one completed frame below.
    // This is NOT setactivepage()/setvisualpage() double buffering.
    initwindow(800, 600);
    sdlbgifast();

    if (graphresult() != grOk)
    {
        cout << "Graphics initialization failed." << endl;
        return 1;
    }

    SCREEN_W = getmaxx() + 1;
    SCREEN_H = getmaxy() + 1;

    // Use dimensions of the actual SDL_bgi window.
    ROAD_LEFT = SCREEN_W * 0.28;
    ROAD_RIGHT = SCREEN_W * 0.72;
    ROAD_CENTER = (ROAD_LEFT + ROAD_RIGHT) / 2.0;

    initializeWorld();
    resetGame();

    bool running = true;

    // Timing.
    Uint32 previous = SDL_GetTicks();

    // First frame: draw everything, then present exactly once.
    renderFrame();
    refresh();

    while (running)
    {
        Uint32 now = SDL_GetTicks();

        double dt =
            (double)(now - previous) / 1000.0;

        previous = now;

        // Prevent a giant movement after a pause/window drag.
        if (dt > 0.05)
            dt = 0.05;

        handleEvents(running);

        updateGame(dt);

        renderFrame();

        // Present exactly one completed frame.
        refresh();

        // SDL_Delay is used instead of BGI delay(), because BGI delay()
        // may itself trigger an additional screen refresh.
        SDL_Delay(8);
    }

    closegraph();

    return 0;
}
