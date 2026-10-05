#include <iostream>
#include <iomanip>
#include <cmath>
#include <graphics.h>

using namespace std;

#define MAX_POINTS 50
const double PI = 3.14159265358979323846;

// ============================================================
// 1. MATRIX UTILITY FUNCTIONS (First-Principles)
// ============================================================

// Initialize a 3x3 identity matrix
void setIdentity(double M[3][3])
{
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            M[i][j] = (i == j) ? 1.0 : 0.0;
        }
    }
}

// Copy source matrix to destination matrix
void copyMatrix(double source[3][3], double destination[3][3])
{
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            destination[i][j] = source[i][j];
        }
    }
}

// Matrix Multiplication for 3x3 matrices: Result = A x B
void multiplyMatrix3x3(double A[3][3], double B[3][3], double result[3][3])
{
    double temp[3][3] = {0};
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            temp[i][j] = 0.0;
            for (int k = 0; k < 3; k++)
            {
                temp[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    copyMatrix(temp, result);
}

// Transform row points: P' = P x M
// P has dimensions [n x 3], M has dimensions [3 x 3]
void transformPoints(double P[][3], double result[][3], int n, double M[3][3])
{
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            result[i][j] = 0.0;
            for (int k = 0; k < 3; k++)
            {
                result[i][j] += P[i][k] * M[k][j];
            }
        }
    }
}

// Display 3x3 transformation matrix
void displayMatrix(double M[3][3], const char title[])
{
    cout << "\n--------------------------------------------\n";
    cout << title << "\n";
    cout << "--------------------------------------------\n";
    for (int i = 0; i < 3; i++)
    {
        cout << "| ";
        for (int j = 0; j < 3; j++)
        {
            cout << setw(8) << fixed << setprecision(2) << M[i][j] << " ";
        }
        cout << "|\n";
    }
}

// Display polygon vertices in homogeneous matrix form [x, y, 1]
void displayPoints(double P[][3], int n, const char title[])
{
    cout << "\n============================================\n";
    cout << title << " (Matrix Form: [x, y, 1])\n";
    cout << "============================================\n";
    for (int i = 0; i < n; i++)
    {
        cout << "Point " << setw(2) << i + 1 << ": [ ";
        for (int j = 0; j < 3; j++)
        {
            cout << setw(8) << fixed << setprecision(2) << P[i][j] << " ";
        }
        cout << "]\n";
    }
}

// ============================================================
// 2. ELEMENTARY 2D TRANSFORMATION MATRICES (Row-Vector Convention)
// ============================================================

// Translation Matrix:
// [ 1   0   0 ]
// [ 0   1   0 ]
// [ tx  ty  1 ]
void getTranslationMatrix(double tx, double ty, double T[3][3])
{
    setIdentity(T);
    T[2][0] = tx;
    T[2][1] = ty;
}

// Scaling Matrix:
// [ sx  0   0 ]
// [ 0   sy  0 ]
// [ 0   0   1 ]
void getScalingMatrix(double sx, double sy, double S[3][3])
{
    setIdentity(S);
    S[0][0] = sx;
    S[1][1] = sy;
}

// Counter-Clockwise Rotation about Origin:
// [  cos(θ)  sin(θ)  0 ]
// [ -sin(θ)  cos(θ)  0 ]
// [    0       0     1 ]
void getRotationMatrix(double angleDeg, double R[3][3])
{
    double rad = angleDeg * (PI / 180.0);
    setIdentity(R);
    R[0][0] = cos(rad);
    R[0][1] = sin(rad);
    R[1][0] = -sin(rad);
    R[1][1] = cos(rad);
}

// Rotation about an Arbitrary Point (xr, yr):
// M = T(-xr, -yr) * R(θ) * T(xr, yr)
void getArbitraryPointRotationMatrix(double xr, double yr, double angleDeg, double M[3][3])
{
    double T_neg[3][3], R[3][3], T_pos[3][3], temp[3][3];

    getTranslationMatrix(-xr, -yr, T_neg);
    getRotationMatrix(angleDeg, R);
    getTranslationMatrix(xr, yr, T_pos);

    // Composite: (T_neg * R) * T_pos
    multiplyMatrix3x3(T_neg, R, temp);
    multiplyMatrix3x3(temp, T_pos, M);
}

// Reflection Matrices
void getReflectionMatrix(int type, double Ref[3][3])
{
    setIdentity(Ref);
    switch (type)
    {
        case 1: // Reflection about X-axis (y -> -y)
            Ref[1][1] = -1.0;
            break;
        case 2: // Reflection about Y-axis (x -> -x)
            Ref[0][0] = -1.0;
            break;
        case 3: // Reflection about Origin (x -> -x, y -> -y)
            Ref[0][0] = -1.0;
            Ref[1][1] = -1.0;
            break;
    }
}

// Shearing Matrices
void getShearMatrix(int type, double sh, double Sh[3][3])
{
    setIdentity(Sh);
    if (type == 1) // Shear along X-axis
    {
        Sh[1][0] = sh;
    }
    else if (type == 2) // Shear along Y-axis
    {
        Sh[0][1] = sh;
    }
}

// ============================================================
// 3. GRAPHICS & BRESENHAM LINE DRAWING (From First Principles)
// ============================================================

// Bresenham's integer line algorithm (All Octants)
void drawLineBresenham(int x1, int y1, int x2, int y2, int color)
{
    int x = x1;
    int y = y1;
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;

    putpixel(x, y, color);

    if (dx >= dy)
    {
        int p = 2 * dy - dx;
        for (int i = 0; i < dx; i++)
        {
            x += sx;
            if (p >= 0)
            {
                y += sy;
                p += 2 * (dy - dx);
            }
            else
            {
                p += 2 * dy;
            }
            putpixel(x, y, color);
        }
    }
    else
    {
        int p = 2 * dx - dy;
        for (int i = 0; i < dy; i++)
        {
            y += sy;
            if (p >= 0)
            {
                x += sx;
                p += 2 * (dx - dy);
            }
            else
            {
                p += 2 * dx;
            }
            putpixel(x, y, color);
        }
    }
}

// Draw centered Cartesian coordinate axes
void drawAxes(int cx, int cy)
{
    drawLineBresenham(0, cy, getmaxx(), cy, DARKGRAY);
    drawLineBresenham(cx, 0, cx, getmaxy(), DARKGRAY);
}

// Render polygon using Bresenham lines
// Scales Cartesian coords and converts to screen coordinates (inverting Y)
void drawPolygonObject(double P[][3], int n, int cx, int cy, int scale, int color)
{
    for (int i = 0; i < n; i++)
    {
        int next = (i + 1) % n;

        int x1 = cx + static_cast<int>(P[i][0] * scale);
        int y1 = cy - static_cast<int>(P[i][1] * scale);
        int x2 = cx + static_cast<int>(P[next][0] * scale);
        int y2 = cy - static_cast<int>(P[next][1] * scale);

        drawLineBresenham(x1, y1, x2, y2, color);
    }
}

// Render both Original (White) and Transformed (Yellow) polygons
void renderGraphics(double original[][3], double transformed[][3], int n)
{
    // Clear previous drawing without destroying the SDL window
    setbkcolor(BLACK);
    cleardevice();

    int cx = getmaxx() / 2;
    int cy = getmaxy() / 2;
    int scale = 25; // Scale factor

    drawAxes(cx, cy);

    // Draw Original (White) and Transformed (Yellow)
    drawPolygonObject(original, n, cx, cy, scale, WHITE);
    drawPolygonObject(transformed, n, cx, cy, scale, YELLOW);

    outtextxy(20, 20, (char*)"WHITE  : Original Object");
    outtextxy(20, 40, (char*)"YELLOW : Transformed Object");
    outtextxy(20, 60, (char*)"Check terminal for menu options.");
}

// ============================================================
// 4. TRANSFORMATION SELECTION HANDLERS
// ============================================================

void getSingleTransformation(double M[3][3])
{
    setIdentity(M);
    cout << "\n--- Select Transformation ---";
    cout << "\n1. Translation";
    cout << "\n2. Scaling";
    cout << "\n3. Rotation about Origin";
    cout << "\n4. Rotation about Arbitrary Point";
    cout << "\n5. Reflection about X-axis";
    cout << "\n6. Reflection about Y-axis";
    cout << "\n7. Reflection about Origin";
    cout << "\n8. Shear in X-direction";
    cout << "\n9. Shear in Y-direction";
    cout << "\nEnter choice: ";

    int choice;
    cin >> choice;

    switch (choice)
    {
        case 1: {
            double tx, ty;
            cout << "Enter tx and ty: ";
            cin >> tx >> ty;
            getTranslationMatrix(tx, ty, M);
            break;
        }
        case 2: {
            double sx, sy;
            cout << "Enter sx and sy: ";
            cin >> sx >> sy;
            getScalingMatrix(sx, sy, M);
            break;
        }
        case 3: {
            double angle;
            cout << "Enter angle in degrees (CCW): ";
            cin >> angle;
            getRotationMatrix(angle, M);
            break;
        }
        case 4: {
            double xr, yr, angle;
            cout << "Enter arbitrary point coordinates (xr, yr): ";
            cin >> xr >> yr;
            cout << "Enter angle in degrees (CCW): ";
            cin >> angle;
            getArbitraryPointRotationMatrix(xr, yr, angle, M);
            break;
        }
        case 5:
            getReflectionMatrix(1, M);
            break;
        case 6:
            getReflectionMatrix(2, M);
            break;
        case 7:
            getReflectionMatrix(3, M);
            break;
        case 8: {
            double shx;
            cout << "Enter shx: ";
            cin >> shx;
            getShearMatrix(1, shx, M);
            break;
        }
        case 9: {
            double shy;
            cout << "Enter shy: ";
            cin >> shy;
            getShearMatrix(2, shy, M);
            break;
        }
        default:
            cout << "Invalid choice! Using Identity.\n";
            setIdentity(M);
            break;
    }
}

// Sequential/Composite pipeline: composite = M1 * M2 * M3 ...
void executeSequentialPipeline(double original[][3], double transformed[][3], int n)
{
    int k;
    cout << "\nEnter number of sequential transformations: ";
    cin >> k;

    if (k <= 0) return;

    double composite[3][3];
    setIdentity(composite);

    double currentStepMatrix[3][3];

    for (int step = 1; step <= k; step++)
    {
        cout << "\n>>> Config Transformation Step " << step << " of " << k << " <<<";
        getSingleTransformation(currentStepMatrix);

        // Accumulate: Composite = Composite * CurrentStep
        multiplyMatrix3x3(composite, currentStepMatrix, composite);

        // Update transformed points after current step
        transformPoints(original, transformed, n, composite);

        cout << "\n--- Status after Step " << step << " ---";
        displayMatrix(currentStepMatrix, "STEP MATRIX");
        displayMatrix(composite, "CUMULATIVE COMPOSITE MATRIX");
        displayPoints(transformed, n, "POINTS AFTER STEP");
    }

    renderGraphics(original, transformed, n);
}

// ============================================================
// 5. MAIN MENU DRIVEN SYSTEM
// ============================================================

int main()
{
    int n;
    double original[MAX_POINTS][3];
    double transformed[MAX_POINTS][3];

    cout << "====================================================\n";
    cout << "          2D GEOMETRIC TRANSFORMATIONS              \n";
    cout << "====================================================\n";

    cout << "\nEnter number of polygon vertices: ";
    cin >> n;

    if (n <= 0 || n > MAX_POINTS)
    {
        cout << "Invalid point count. Exiting.\n";
        return 1;
    }

    cout << "\nEnter homogeneous coordinates for each vertex (x y 1):\n";
    for (int i = 0; i < n; i++)
    {
        cout << "Vertex " << i + 1 << " (x y 1): ";
        cin >> original[i][0] >> original[i][1] >> original[i][2];
        original[i][2] = 1.0;
    }

    // --- INITIALIZE GRAPHICS ONCE HERE ---
    int gd = DETECT, gm;
    initgraph(&gd, &gm, (char*)"");

    // Show initial object
    renderGraphics(original, original, n);

    displayPoints(original, n, "ORIGINAL POLYGON VERTICES");

    int choice;
    do
    {
        cout << "\n============================================\n";
        cout << "                 MAIN MENU                  \n";
        cout << "============================================\n";
        cout << "1. Apply Individual Transformation\n";
        cout << "2. Apply Sequential / Composite Transformations\n";
        cout << "3. Reset/Display Original Points\n";
        cout << "0. Exit\n";
        cout << "Enter your choice: ";
        cin >> choice;

        if (choice == 1)
        {
            double M[3][3];
            getSingleTransformation(M);
            displayMatrix(M, "APPLIED TRANSFORMATION MATRIX");

            transformPoints(original, transformed, n, M);
            displayPoints(transformed, n, "TRANSFORMED VERTICES");

            renderGraphics(original, transformed, n);
        }
        else if (choice == 2)
        {
            executeSequentialPipeline(original, transformed, n);
        }
        else if (choice == 3)
        {
            renderGraphics(original, original, n);
            displayPoints(original, n, "ORIGINAL POLYGON VERTICES");
        }
        else if (choice == 0)
        {
            cout << "\nExiting Assignment 3 Program.\n";
        }
        else
        {
            cout << "\nInvalid choice, please re-enter.\n";
        }

    } while (choice != 0);

    // --- CLOSE GRAPHICS ONCE UPON EXIT ---
    closegraph();

    return 0;
}
