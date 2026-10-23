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
// DR. DRIVING 2D - VERSION 9 (PARKING MISSION REMOVED)
// REBUILT FOR SDL_bgi / Linux
// ============================================================

const double PI = 3.14159265358979323846;

// ------------------------------------------------------------
// GAME STATES & MODES (Parking removed)
// ------------------------------------------------------------
enum GameState { STATE_MENU, STATE_PLAYING, STATE_GAMEOVER };
enum GameMode { MODE_ENDLESS, MODE_COIN_TARGET, MODE_FUEL_RUN };

GameState gameState = STATE_MENU;
GameMode currentMode = MODE_ENDLESS;

// ------------------------------------------------------------
// COHEN-SUTHERLAND LINE CLIPPING CONSTANTS & FUNCTIONS
// ------------------------------------------------------------
const int REGION_INSIDE         = 0b0000;
const int REGION_LEFT           = 0b0001;
const int REGION_RIGHT          = 0b0010;
const int REGION_BOTTOM         = 0b0100;
const int REGION_BOTTOM_LEFT    = 0b0101;
const int REGION_BOTTOM_RIGHT   = 0b0110;
const int REGION_TOP            = 0b1000;
const int REGION_TOP_LEFT       = 0b1001;
const int REGION_TOP_RIGHT      = 0b1010;

const int CLIP_LEFT   = 0b0001;
const int CLIP_RIGHT  = 0b0010;
const int CLIP_BOTTOM = 0b0100;
const int CLIP_TOP    = 0b1000;

// ------------------------------------------------------------
// WORLD / GAME DATA
// ------------------------------------------------------------
struct Point { double x; double y; };
struct RoadObject { double x; double worldY; int type; };
struct Coin { double x; double worldY; bool active; };
struct FuelCan { double x; double worldY; bool active; };
struct TrafficCar { double x; double worldY; double speed; int color; };

vector<RoadObject> roadside;
vector<Coin> coins;
vector<FuelCan> fuels;
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
int coinsCollected;
int targetCoins = 20;
double fuelLevel;

bool crashed = false;
bool missionCompleted = false;

// Held keys
bool keyLeft = false;
bool keyRight = false;
bool keyUp = false;
bool keyDown = false;

// Forward Declarations
void initializeWorld();
void resetGame();

// ------------------------------------------------------------
// DDA LINE ALGORITHM
// ------------------------------------------------------------
void drawLineDDA(float x1, float y1, float x2, float y2, int color)
{
    float dx = x2 - x1;
    float dy = y2 - y1;
    int length = (abs(dx) >= abs(dy)) ? abs(dx) : abs(dy);
    if (length == 0) { putpixel((int)round(x1), (int)round(y1), color); return; }
    float x_inc = dx / length;
    float y_inc = dy / length;
    float x = x1, y = y1;
    for (int i = 0; i <= length; i++) {
        putpixel((int)round(x), (int)round(y), color);
        x += x_inc; y += y_inc;
    }
}

// ------------------------------------------------------------
// COHEN-SUTHERLAND IMPLEMENTATION
// ------------------------------------------------------------
int computeCode(double x, double y, double x_min, double y_min, double x_max, double y_max)
{
    int code = REGION_INSIDE;
    if (x < x_min) code |= CLIP_LEFT;
    else if (x > x_max) code |= CLIP_RIGHT;
    if (y < y_min) code |= CLIP_TOP;
    else if (y > y_max) code |= CLIP_BOTTOM;
    return code;
}

void drawClippedLineDDA(double x1, double y1, double x2, double y2, double x_min, double y_min, double x_max, double y_max, int color)
{
    int code1 = computeCode(x1, y1, x_min, y_min, x_max, y_max);
    int code2 = computeCode(x2, y2, x_min, y_min, x_max, y_max);
    bool accept = false;
    while (true) {
        if (!(code1 | code2)) { accept = true; break; }
        else if (code1 & code2) { break; }
        else {
            double x = 0, y = 0;
            int codeOut = code1 ? code1 : code2;
            if (codeOut & CLIP_TOP) {
                x = x1 + (x2 - x1) * (y_min - y1) / (y2 - y1); y = y_min;
            } else if (codeOut & CLIP_BOTTOM) {
                x = x1 + (x2 - x1) * (y_max - y1) / (y2 - y1); y = y_max;
            } else if (codeOut & CLIP_RIGHT) {
                y = y1 + (y2 - y1) * (x_max - x1) / (x2 - x1); x = x_max;
            } else if (codeOut & CLIP_LEFT) {
                y = y1 + (y2 - y1) * (x_min - x1) / (x2 - x1); x = x_min;
            }
            if (codeOut == code1) { x1 = x; y1 = y; code1 = computeCode(x1, y1, x_min, y_min, x_max, y_max); }
            else { x2 = x; y2 = y; code2 = computeCode(x2, y2, x_min, y_min, x_max, y_max); }
        }
    }
    if (accept) drawLineDDA(x1, y1, x2, y2, color);
}

// ------------------------------------------------------------
// MATRIX TRANSFORMATIONS
// ------------------------------------------------------------
void setIdentity(double M[3][3]) {
    for (int i = 0; i < 3; ++i) for (int j = 0; j < 3; ++j) M[i][j] = (i == j) ? 1.0 : 0.0;
}
void createTranslation(double M[3][3], double tx, double ty) { setIdentity(M); M[2][0] = tx; M[2][1] = ty; }
void createScaling(double M[3][3], double sx, double sy) { setIdentity(M); M[0][0] = sx; M[1][1] = sy; }
void createRotation(double M[3][3], double degrees) {
    double r = degrees * PI / 180.0; setIdentity(M);
    M[0][0] = cos(r); M[0][1] = sin(r); M[1][0] = -sin(r); M[1][1] = cos(r);
}
Point transformPoint(Point p, double M[3][3]) {
    Point out;
    out.x = p.x * M[0][0] + p.y * M[1][0] + M[2][0];
    out.y = p.x * M[0][1] + p.y * M[1][1] + M[2][1];
    return out;
}
vector<Point> transformPolygon(const vector<Point>& input, double sx, double sy, double angle, double tx, double ty) {
    double S[3][3], R[3][3], T[3][3], SR[3][3], M[3][3];
    createScaling(S, sx, sy); createRotation(R, angle); createTranslation(T, tx, ty);
    for (int i = 0; i < 3; ++i) for (int j = 0; j < 3; ++j) {
        SR[i][j] = 0; for (int k = 0; k < 3; ++k) SR[i][j] += S[i][k] * R[k][j];
    }
    for (int i = 0; i < 3; ++i) for (int j = 0; j < 3; ++j) {
        M[i][j] = 0; for (int k = 0; k < 3; ++k) M[i][j] += SR[i][k] * T[k][j];
    }
    vector<Point> result;
    for (const Point& p : input) result.push_back(transformPoint(p, M));
    return result;
}

// ------------------------------------------------------------
// FAST POLYGON FILLING
// ------------------------------------------------------------
void fillPolygonFast(const vector<Point>& poly, int fillColor) {
    if (poly.size() < 3) return;
    vector<int> pts;
    for (const Point& p : poly) { pts.push_back((int)round(p.x)); pts.push_back((int)round(p.y)); }
    setfillstyle(SOLID_FILL, fillColor);
    fillpoly((int)poly.size(), pts.data());
}
void drawPolygonOutlineFast(const vector<Point>& poly, int color) {
    setcolor(color);
    for (size_t i = 0; i < poly.size(); ++i) {
        const Point& a = poly[i]; const Point& b = poly[(i + 1) % poly.size()];
        line((int)round(a.x), (int)round(a.y), (int)round(b.x), (int)round(b.y));
    }
}
void fillAndOutline(const vector<Point>& poly, int fillColor, int outlineColor) {
    fillPolygonFast(poly, fillColor); drawPolygonOutlineFast(poly, outlineColor);
}

double screenWorldY(double worldY) {
    double y = worldY + scrollY;
    while (y > SCREEN_H + 100) y -= 1200.0;
    while (y < -100) y += 1200.0;
    return y;
}

// ------------------------------------------------------------
// DETAILED ROAD & ENVIRONMENT GRAPHICS
// ------------------------------------------------------------
void drawRoad() {
    setfillstyle(SOLID_FILL, GREEN);
    bar(0, 0, SCREEN_W - 1, SCREEN_H - 1);

    vector<Point> road = {{ROAD_LEFT, 0}, {ROAD_RIGHT, 0}, {ROAD_RIGHT, (double)SCREEN_H}, {ROAD_LEFT, (double)SCREEN_H}};
    fillAndOutline(road, DARKGRAY, WHITE);

    double clip_xmin = 0, clip_ymin = 0, clip_xmax = SCREEN_W - 1, clip_ymax = SCREEN_H - 1;

    drawClippedLineDDA(ROAD_LEFT, 0, ROAD_LEFT, SCREEN_H - 1, clip_xmin, clip_ymin, clip_xmax, clip_ymax, LIGHTGRAY);
    drawClippedLineDDA(ROAD_RIGHT, 0, ROAD_RIGHT, SCREEN_H - 1, clip_xmin, clip_ymin, clip_xmax, clip_ymax, LIGHTGRAY);

    for (double worldY = -200.0; worldY < 1200.0; worldY += 100.0) {
        double y1 = screenWorldY(worldY);
        double y2 = screenWorldY(worldY + 55.0);
        if (fabs(y2 - y1) <= 100)
            drawClippedLineDDA(ROAD_CENTER, y1, ROAD_CENTER, y2, clip_xmin, clip_ymin, clip_xmax, clip_ymax, YELLOW);
    }

    double lane1 = ROAD_LEFT + (ROAD_RIGHT - ROAD_LEFT) / 3.0;
    double lane2 = ROAD_LEFT + 2.0 * (ROAD_RIGHT - ROAD_LEFT) / 3.0;
    for (double worldY = -200.0; worldY < 1200.0; worldY += 85.0) {
        double y1 = screenWorldY(worldY);
        double y2 = screenWorldY(worldY + 38.0);
        if (fabs(y2 - y1) <= 100) {
            drawClippedLineDDA(lane1, y1, lane1, y2, clip_xmin, clip_ymin, clip_xmax, clip_ymax, WHITE);
            drawClippedLineDDA(lane2, y1, lane2, y2, clip_xmin, clip_ymin, clip_xmax, clip_ymax, WHITE);
        }
    }

    for (double worldY = -120.0; worldY < 1200.0; worldY += 70.0) {
        double y = screenWorldY(worldY);
        int y1 = (int)y, y2 = (int)(y + 35);
        if (y2 < 0 || y1 >= SCREEN_H) continue;
        int curbColor = (((int)floor(worldY / 70.0)) % 2 == 0) ? WHITE : LIGHTGRAY;
        setfillstyle(SOLID_FILL, curbColor);
        bar((int)ROAD_LEFT - 12, max(0, y1), (int)ROAD_LEFT - 2, min(SCREEN_H - 1, y2));
        bar((int)ROAD_RIGHT + 2, max(0, y1), (int)ROAD_RIGHT + 12, min(SCREEN_H - 1, y2));
    }
}

// ------------------------------------------------------------
// ROADSIDE OBJECTS
// ------------------------------------------------------------
void drawTree(double x, double y) {
    vector<Point> trunk = {{x - 5, y}, {x + 5, y}, {x + 5, y + 24}, {x - 5, y + 24}};
    fillAndOutline(trunk, BROWN, WHITE);
    setfillstyle(SOLID_FILL, GREEN);
    fillellipse((int)x, (int)y - 15, 15, 15);
    fillellipse((int)x - 12, (int)y - 8, 11, 11);
    fillellipse((int)x + 12, (int)y - 8, 11, 11);
}

void drawLamp(double x, double y) {
    drawClippedLineDDA(x, y, x, y + 48, 0, 0, SCREEN_W - 1, SCREEN_H - 1, WHITE);
    drawClippedLineDDA(x, y, x + ((x < ROAD_CENTER) ? 15 : -15), y, 0, 0, SCREEN_W - 1, SCREEN_H - 1, WHITE);
    setfillstyle(SOLID_FILL, YELLOW);
    fillellipse((int)x + ((x < ROAD_CENTER) ? 15 : -15), (int)y + 5, 5, 5);
}

void drawBuilding(double x, double y, int type) {
    int w = 48 + type * 8;
    int h = 58 + type * 12;
    vector<Point> building = {{x, y}, {x + w, y}, {x + w, y + h}, {x, y + h}};
    fillAndOutline(building, LIGHTGRAY, WHITE);
    vector<Point> roof = {{x - 4, y}, {x + w + 4, y}, {x + w / 2.0, y - 18}};
    fillAndOutline(roof, BROWN, WHITE);
    for (int row = 0; row < 2; ++row) {
        for (int col = 0; col < 2; ++col) {
            int wx = (int)x + 10 + col * (w - 24);
            int wy = (int)y + 15 + row * 20;
            setfillstyle(SOLID_FILL, CYAN);
            bar(wx, wy, wx + 8, wy + 10);
        }
    }
}

void drawRoadsideWorld() {
    for (const RoadObject& obj : roadside) {
        double y = screenWorldY(obj.worldY);
        if (y < -100 || y > SCREEN_H + 100) continue;
        if (obj.type == 0) drawTree(obj.x, y);
        else if (obj.type == 1) drawLamp(obj.x, y);
        else drawBuilding(obj.x, y, obj.type);
    }
}

// ------------------------------------------------------------
// COINS & FUEL CANISTERS
// ------------------------------------------------------------
void drawCoin(double x, double y) {
    setcolor(YELLOW);
    ellipse((int)x, (int)y, 0, 360, 11, 11);
    ellipse((int)x, (int)y, 0, 360, 7, 7);
    drawClippedLineDDA(x - 2, y - 5, x + 2, y - 5, 0, 0, SCREEN_W - 1, SCREEN_H - 1, WHITE);
}

void drawFuel(double x, double y) {
    setfillstyle(SOLID_FILL, BROWN);
    bar((int)x - 9, (int)y - 14, (int)x + 9, (int)y + 14);
    setcolor(WHITE);
    rectangle((int)x - 9, (int)y - 14, (int)x + 9, (int)y + 14);
    setfillstyle(SOLID_FILL, RED);
    bar((int)x - 5, (int)y - 6, (int)x + 5, (int)y + 6);
}

// ------------------------------------------------------------
// DETAILED CAR GRAPHICS
// ------------------------------------------------------------
void drawDetailedCar(double cx, double cy, double angle, int bodyColor) {
    vector<Point> body = {{-24, -42}, { 24, -42}, { 28, -25}, { 28, 30}, { 18, 43}, {-18, 43}, {-28, 30}, {-28, -25}};
    vector<Point> cabin = {{-16, -20}, { 16, -20}, { 13, 12}, {-13, 12}};
    vector<Point> windshield = {{-12, -17}, { 12, -17}, { 10, -2}, {-10, -2}};
    vector<Point> rearGlass = {{-10, 2}, { 10, 2}, { 12, 10}, {-12, 10}};
    vector<Point> bonnet = {{-17, -39}, { 17, -39}, { 20, -24}, {-20, -24}};

    fillAndOutline(transformPolygon(body, 1, 1, angle, cx, cy), bodyColor, WHITE);
    fillAndOutline(transformPolygon(cabin, 1, 1, angle, cx, cy), DARKGRAY, WHITE);
    fillAndOutline(transformPolygon(windshield, 1, 1, angle, cx, cy), CYAN, WHITE);
    fillAndOutline(transformPolygon(rearGlass, 1, 1, angle, cx, cy), BLUE, WHITE);
    fillAndOutline(transformPolygon(bonnet, 1, 1, angle, cx, cy), LIGHTRED, WHITE);

    vector<Point> wheelCenters = {{-27, -23}, { 27, -23}, {-27, 23}, { 27, 23}};
    double R[3][3];
    createRotation(R, angle);
    for (const Point& local : wheelCenters) {
        Point p = transformPoint(local, R);
        setfillstyle(SOLID_FILL, BLACK);
        fillellipse((int)round(cx + p.x), (int)round(cy + p.y), 6, 6);
    }
}

// ------------------------------------------------------------
// HUD & STYLIZED MENU
// ------------------------------------------------------------
void drawHUD() {
    setfillstyle(SOLID_FILL, BLACK);
    bar(0, 0, SCREEN_W - 1, 42);
    setcolor(WHITE);
    char text[120];
    sprintf(text, "SCORE: %d   COINS: %d/%d", score, coinsCollected, (currentMode == MODE_COIN_TARGET ? targetCoins : 999));
    outtextxy(12, 10, text);

    bar(SCREEN_W - 170, 10, SCREEN_W - 15, 30);
    setfillstyle(SOLID_FILL, fuelLevel > 25 ? GREEN : RED);
    bar(SCREEN_W - 168, 12, SCREEN_W - 168 + (int)(151 * (fuelLevel / 100.0)), 28);
    setcolor(WHITE);
    rectangle(SCREEN_W - 170, 10, SCREEN_W - 15, 30);

    bar(0, SCREEN_H - 28, SCREEN_W - 1, SCREEN_H - 1);
    outtextxy(10, SCREEN_H - 20, (char*)"W/S SPEED   A/D STEER   R RESTART   Q MENU");
}

void drawMenu() {
    cleardevice();
    
    // Background
    setfillstyle(SOLID_FILL, DARKGRAY);
    bar(0, 0, SCREEN_W - 1, SCREEN_H - 1);

    // Title banner panel
    setfillstyle(SOLID_FILL, BLUE);
    bar(SCREEN_W / 2 - 200, 45, SCREEN_W / 2 + 200, 105);
    setcolor(WHITE);
    rectangle(SCREEN_W / 2 - 200, 45, SCREEN_W / 2 + 200, 105);
    outtextxy(SCREEN_W / 2 - 75, 68, (char*)"DR. DRIVING 2D");

    // Menu options panel cards (3 modes now)
    int startY = 140;
    int boxWidth = 360;
    int boxHeight = 46;
    int leftX = SCREEN_W / 2 - boxWidth / 2;

    const char* options[3] = {
        "  1. Endless Highway Mode",
        "  2. Coin Target Mission",
        "  3. Fuel Run Challenge"
    };

    for (int i = 0; i < 3; ++i) {
        int y = startY + i * 65;
        setfillstyle(SOLID_FILL, BLACK);
        bar(leftX, y, leftX + boxWidth, y + boxHeight);
        setcolor(LIGHTCYAN);
        rectangle(leftX, y, leftX + boxWidth, y + boxHeight);
        
        setcolor(WHITE);
        outtextxy(leftX + 20, y + 16, (char*)options[i]);
    }

    // Footer instructions card
    setfillstyle(SOLID_FILL, DARKGRAY);
    bar(leftX, 360, leftX + boxWidth, 415);
    setcolor(YELLOW);
    rectangle(leftX, 360, leftX + boxWidth, 415);
    outtextxy(leftX + 25, 378, (char*)"Press [1-3] to Select, [Q] to Quit");
}

// ------------------------------------------------------------
// WORLD SETUP & GAME LOOP
// ------------------------------------------------------------
void initializeWorld() {
    roadside.clear(); coins.clear(); fuels.clear(); traffic.clear();

    for (int i = 0; i < 10; ++i) {
        double y = 50.0 + i * 120.0;
        roadside.push_back({ROAD_LEFT - 65.0, y, 0});
        roadside.push_back({ROAD_RIGHT + 30.0, y + 40.0, 0});
        roadside.push_back({ROAD_LEFT - 35.0, y + 20.0, 1});
        roadside.push_back({ROAD_RIGHT + 10.0, y + 60.0, 1});
        if (i % 2 == 0) {
            roadside.push_back({ROAD_LEFT - 125.0, y + 10.0, 2});
            roadside.push_back({ROAD_RIGHT + 70.0, y + 50.0, 2});
        }
    }

    for (int i = 0; i < 20; ++i) {
        int lane = i % 3;
        double laneX = (lane == 0) ? ROAD_LEFT + 40 : (lane == 1) ? ROAD_CENTER : ROAD_RIGHT - 40;
        coins.push_back({laneX, 60.0 + i * 75.0, true});
        if (i % 3 == 0) fuels.push_back({ROAD_LEFT + 25, 100.0 + i * 110.0, true});
    }

    traffic.push_back({ROAD_LEFT + 44, 150.0, 1.0, BLUE});
    traffic.push_back({ROAD_RIGHT - 44, 450.0, 0.9, MAGENTA});
    traffic.push_back({ROAD_CENTER, 850.0, 1.1, CYAN});
}

void resetGame() {
    playerX = ROAD_CENTER;
    playerY = SCREEN_H - 100;
    playerAngle = 0.0;
    scrollY = 0.0;
    playerSpeed = 0.0;
    targetSpeed = 5.0;
    steering = 0.0;
    score = 0;
    coinsCollected = 0;
    fuelLevel = 100.0;
    crashed = false;
    missionCompleted = false;
    initializeWorld();
}

void updateGame(double dt) {
    if (crashed || missionCompleted) return;

    if (keyUp) targetSpeed = 8.0;
    else if (keyDown) targetSpeed = 2.0;
    else targetSpeed = 5.0;

    if (playerSpeed < targetSpeed) playerSpeed += 8.0 * dt;
    else if (playerSpeed > targetSpeed) playerSpeed -= 8.0 * dt;

    steering = 0.0;
    if (keyLeft) steering -= 1.0;
    if (keyRight) steering += 1.0;

    if (steering != 0.0) {
        playerX += steering * 190.0 * dt;
        playerAngle = steering * 10.0;
    } else {
        playerAngle *= 0.82;
        if (fabs(playerAngle) < 0.2) playerAngle = 0.0;
    }

    fuelLevel -= dt * 2.2;
    if (fuelLevel <= 0) { crashed = true; }

    double margin = 34.0;
    if (playerX < ROAD_LEFT + margin) playerX = ROAD_LEFT + margin;
    if (playerX > ROAD_RIGHT - margin) playerX = ROAD_RIGHT - margin;

    scrollY += playerSpeed * 52.0 * dt;

    for (TrafficCar& car : traffic) {
        car.worldY += car.speed * 42.0 * dt;
        if (car.worldY > 1300.0) car.worldY -= 1400.0;

        double sy = screenWorldY(car.worldY);
        if (fabs(playerX - car.x) < 35 && fabs(playerY - sy) < 55) {
            crashed = true;
            playerSpeed = 0.0;
        }
    }

    for (Coin& c : coins) {
        if (!c.active) continue;
        double sy = screenWorldY(c.worldY);
        if (fabs(playerX - c.x) < 28 && fabs(playerY - sy) < 28) {
            c.active = false;
            coinsCollected++;
            score += 10;
            if (currentMode == MODE_COIN_TARGET && coinsCollected >= targetCoins) {
                missionCompleted = true;
            }
        }
    }

    for (FuelCan& f : fuels) {
        if (!f.active) continue;
        double sy = screenWorldY(f.worldY);
        if (fabs(playerX - f.x) < 28 && fabs(playerY - sy) < 28) {
            f.active = false;
            fuelLevel = min(100.0, fuelLevel + 35.0);
        }
    }
}

int main() {
    initwindow(800, 600);
    sdlbgifast();

    if (graphresult() != grOk) {
        cout << "Graphics initialization failed." << endl;
        return 1;
    }

    SCREEN_W = getmaxx() + 1;
    SCREEN_H = getmaxy() + 1;
    ROAD_LEFT = SCREEN_W * 0.28;
    ROAD_RIGHT = SCREEN_W * 0.72;
    ROAD_CENTER = (ROAD_LEFT + ROAD_RIGHT) / 2.0;

    bool running = true;
    Uint32 previous = SDL_GetTicks();

    while (running) {
        Uint32 now = SDL_GetTicks();
        double dt = (double)(now - previous) / 1000.0;
        previous = now;
        if (dt > 0.05) dt = 0.05;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            else if (event.type == SDL_KEYDOWN) {
                if (event.key.repeat) continue;
                SDL_Keycode key = event.key.keysym.sym;

                if (gameState == STATE_MENU) {
                    if (key == SDLK_1) { currentMode = MODE_ENDLESS; resetGame(); gameState = STATE_PLAYING; }
                    else if (key == SDLK_2) { currentMode = MODE_COIN_TARGET; targetCoins = 20; resetGame(); gameState = STATE_PLAYING; }
                    else if (key == SDLK_3) { currentMode = MODE_FUEL_RUN; resetGame(); gameState = STATE_PLAYING; }
                    else if (key == SDLK_q || key == SDLK_ESCAPE) running = false;
                } else {
                    if (key == SDLK_q || key == SDLK_ESCAPE) gameState = STATE_MENU;
                    else if (key == SDLK_a || key == SDLK_LEFT) keyLeft = true;
                    else if (key == SDLK_d || key == SDLK_RIGHT) keyRight = true;
                    else if (key == SDLK_w || key == SDLK_UP) keyUp = true;
                    else if (key == SDLK_s || key == SDLK_DOWN) keyDown = true;
                    else if (key == SDLK_r) resetGame();
                }
            } else if (event.type == SDL_KEYUP) {
                SDL_Keycode key = event.key.keysym.sym;
                if (key == SDLK_a || key == SDLK_LEFT) keyLeft = false;
                else if (key == SDLK_d || key == SDLK_RIGHT) keyRight = false;
                else if (key == SDLK_w || key == SDLK_UP) keyUp = false;
                else if (key == SDLK_s || key == SDLK_DOWN) keyDown = false;
            }
        }

        if (gameState == STATE_MENU) {
            drawMenu();
        } else {
            updateGame(dt);
            cleardevice();
            
            drawRoad();
            drawRoadsideWorld();

            for (const Coin& c : coins) {
                if (c.active) {
                    double y = screenWorldY(c.worldY);
                    if (y >= -30 && y <= SCREEN_H + 30) drawCoin(c.x, y);
                }
            }

            for (const FuelCan& f : fuels) {
                if (f.active) {
                    double y = screenWorldY(f.worldY);
                    if (y >= -30 && y <= SCREEN_H + 30) drawFuel(f.x, y);
                }
            }

            for (const TrafficCar& car : traffic) {
                double y = screenWorldY(car.worldY);
                if (y >= -100 && y <= SCREEN_H + 100)
                    drawDetailedCar(car.x, y, 0.0, car.color);
            }

            drawDetailedCar(playerX, playerY, playerAngle, RED);
            drawHUD();

            if (crashed) {
                setfillstyle(SOLID_FILL, BLACK);
                bar(SCREEN_W / 2 - 130, SCREEN_H / 2 - 40, SCREEN_W / 2 + 130, SCREEN_H / 2 + 40);
                setcolor(RED);
                outtextxy(SCREEN_W / 2 - 45, SCREEN_H / 2 - 12, (char*)"CRASHED!");
                setcolor(WHITE);
                outtextxy(SCREEN_W / 2 - 85, SCREEN_H / 2 + 10, (char*)"Press R to Restart");
            } else if (missionCompleted) {
                setfillstyle(SOLID_FILL, BLACK);
                bar(SCREEN_W / 2 - 140, SCREEN_H / 2 - 40, SCREEN_W / 2 + 140, SCREEN_H / 2 + 40);
                setcolor(YELLOW);
                outtextxy(SCREEN_W / 2 - 75, SCREEN_H / 2 - 12, (char*)"MISSION COMPLETED!");
                setcolor(WHITE);
                outtextxy(SCREEN_W / 2 - 85, SCREEN_H / 2 + 10, (char*)"Press R to Restart");
            }
        }

        refresh();
        SDL_Delay(8);
    }

    closegraph();
    return 0;
}
