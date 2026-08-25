#include <iostream>
#include <iomanip>
#include <cmath>
#include <graphics.h>

using namespace std;

#define MAX 50

// ------------------------------------------------------------
// Display points in matrix form
// ------------------------------------------------------------
void displayPoints(double P[][3], int n, const char title[])
{
    cout << "\n============================================\n";
    cout << title << "\n";
    cout << "============================================\n";

    for (int i = 0; i < n; i++)
    {
        cout << "[ ";
        for (int j = 0; j < 3; j++)
            cout << fixed << setprecision(2) << P[i][j] << " ";
        cout << "]\n";
    }
}

// ------------------------------------------------------------
// Display a 3 x 3 transformation matrix
// ------------------------------------------------------------
void displayMatrix(double M[3][3], const char title[])
{
    cout << "\n" << title << "\n";
    cout << "-------------------------\n";

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
            cout << fixed << setprecision(2) << M[i][j] << "\t";
        cout << endl;
    }
}

// ------------------------------------------------------------
// Copy one 3 x 3 matrix into another
// ------------------------------------------------------------
void copyMatrix(double source[3][3], double destination[3][3])
{
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            destination[i][j] = source[i][j];
}

// ------------------------------------------------------------
// Matrix multiplication: A x B
// ------------------------------------------------------------
void multiplyMatrix(double A[3][3], double B[3][3], double result[3][3])
{
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            result[i][j] = 0;
            for (int k = 0; k < 3; k++)
                result[i][j] += A[i][k] * B[k][j];
        }
    }
}

// ------------------------------------------------------------
// Point transformation for row vectors:
// P' = P x M
// ------------------------------------------------------------
void transformPoints(
    double P[][3],
    double result[][3],
    int n,
    double M[3][3])
{
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            result[i][j] = 0;
            for (int k = 0; k < 3; k++)
                result[i][j] += P[i][k] * M[k][j];
        }
    }
}

// ------------------------------------------------------------
// Translation matrix - row-vector form
// [x y 1] [1  0  0]
//          [0  1  0]
//          [tx ty 1]
// ------------------------------------------------------------
void createTranslation(double T[3][3])
{
    double tx, ty;

    cout << "\nEnter tx: ";
    cin >> tx;
    cout << "Enter ty: ";
    cin >> ty;

    T[0][0] = 1;  T[0][1] = 0;  T[0][2] = 0;
    T[1][0] = 0;  T[1][1] = 1;  T[1][2] = 0;
    T[2][0] = tx; T[2][1] = ty; T[2][2] = 1;
}

// ------------------------------------------------------------
// Scaling matrix
// ------------------------------------------------------------
void createScaling(double S[3][3])
{
    double sx, sy;

    cout << "\nEnter sx: ";
    cin >> sx;
    cout << "Enter sy: ";
    cin >> sy;

    S[0][0] = sx; S[0][1] = 0;  S[0][2] = 0;
    S[1][0] = 0;  S[1][1] = sy; S[1][2] = 0;
    S[2][0] = 0;  S[2][1] = 0;  S[2][2] = 1;
}

// ------------------------------------------------------------
// Rotation matrix - counter-clockwise, row-vector form
// ------------------------------------------------------------
void createRotation(double R[3][3])
{
    double theta;

    cout << "\nEnter angle in degrees: ";
    cin >> theta;

    const double PI = acos(-1.0);
    double rad = theta * PI / 180.0;

    R[0][0] = cos(rad);  R[0][1] = sin(rad);  R[0][2] = 0;
    R[1][0] = -sin(rad); R[1][1] = cos(rad); R[1][2] = 0;
    R[2][0] = 0;         R[2][1] = 0;         R[2][2] = 1;
}

// ------------------------------------------------------------
// Reflection about X-axis
// ------------------------------------------------------------
void createReflectionX(double R[3][3])
{
    R[0][0] = 1; R[0][1] = 0;  R[0][2] = 0;
    R[1][0] = 0; R[1][1] = -1; R[1][2] = 0;
    R[2][0] = 0; R[2][1] = 0;  R[2][2] = 1;
}

// ------------------------------------------------------------
// Reflection about Y-axis
// ------------------------------------------------------------
void createReflectionY(double R[3][3])
{
    R[0][0] = -1; R[0][1] = 0; R[0][2] = 0;
    R[1][0] = 0;  R[1][1] = 1; R[1][2] = 0;
    R[2][0] = 0;  R[2][1] = 0; R[2][2] = 1;
}

// ------------------------------------------------------------
// Shearing in X direction
// ------------------------------------------------------------
void createShearX(double S[3][3])
{
    double shx;

    cout << "\nEnter shear factor shx: ";
    cin >> shx;

    S[0][0] = 1;   S[0][1] = 0; S[0][2] = 0;
    S[1][0] = shx; S[1][1] = 1; S[1][2] = 0;
    S[2][0] = 0;   S[2][1] = 0; S[2][2] = 1;
}

// ------------------------------------------------------------
// Shearing in Y direction
// ------------------------------------------------------------
void createShearY(double S[3][3])
{
    double shy;

    cout << "\nEnter shear factor shy: ";
    cin >> shy;

    S[0][0] = 1; S[0][1] = shy; S[0][2] = 0;
    S[1][0] = 0; S[1][1] = 1;   S[1][2] = 0;
    S[2][0] = 0; S[2][1] = 0;   S[2][2] = 1;
}

// ------------------------------------------------------------
// Create transformation from menu choice
// ------------------------------------------------------------
void createTransformation(int choice, double M[3][3])
{
    switch (choice)
    {
        case 1: createTranslation(M);  break;
        case 2: createScaling(M);      break;
        case 3: createRotation(M);     break;
        case 4: createReflectionX(M);  break;
        case 5: createReflectionY(M);  break;
        case 6: createShearX(M);       break;
        case 7: createShearY(M);       break;
    }
}

// ------------------------------------------------------------
// Bresenham line drawing algorithm
// Used to draw the original and transformed objects.
// ------------------------------------------------------------
void drawLineBresenham(int x1, int y1, int x2, int y2, int color)
{
    int x = x1;
    int y = y1;

    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);

    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;

    int p;

    putpixel(x, y, color);

    if (dx >= dy)
    {
        p = 2 * dy - dx;

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
            delay(2);
        }
    }
    else
    {
        p = 2 * dx - dy;

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
            delay(2);
        }
    }
}

// ------------------------------------------------------------
// Draw coordinate axes
// ------------------------------------------------------------
void drawAxes(int cx, int cy)
{
    drawLineBresenham(0, cy, getmaxx(), cy, LIGHTGRAY);
    drawLineBresenham(cx, 0, cx, getmaxy(), LIGHTGRAY);
}

// ------------------------------------------------------------
// Draw a closed polygon/object
// ------------------------------------------------------------
void drawObject(
    double P[][3],
    int n,
    int cx,
    int cy,
    int scale,
    int color)
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

// ------------------------------------------------------------
// Display original and transformed objects graphically
// ------------------------------------------------------------
void displayObjects(
    double original[][3],
    double transformed[][3],
    int n)
{
    int gd = DETECT;
    int gm;

    initgraph(&gd, &gm, (char*)"");

    setbkcolor(BLACK);
    cleardevice();

    int cx = getmaxx() / 2;
    int cy = getmaxy() / 2;

    drawAxes(cx, cy);

    // Original = WHITE
    drawObject(original, n, cx, cy, 30, WHITE);

    // Transformed = YELLOW
    drawObject(transformed, n, cx, cy, 30, YELLOW);

    outtextxy(20, 20, (char*)"WHITE = ORIGINAL");
    outtextxy(20, 40, (char*)"YELLOW = TRANSFORMED");
    outtextxy(20, 60, (char*)"Press any key to close");

    getch();
    closegraph();
}

// ------------------------------------------------------------
// Individual transformation
// ------------------------------------------------------------
void individualTransformation(
    double original[][3],
    double transformed[][3],
    int n,
    int choice)
{
    double M[3][3];

    createTransformation(choice, M);

    displayMatrix(M, "Transformation Matrix");

    transformPoints(original, transformed, n, M);

    displayPoints(transformed, n, "TRANSFORMED POINTS");

    displayObjects(original, transformed, n);
}

// ------------------------------------------------------------
// Composite / sequential transformation
// P' = P x M1 x M2 x M3 ...
// ------------------------------------------------------------
void compositeTransformation(
    double original[][3],
    double transformed[][3],
    int n)
{
    int count;

    cout << "\nEnter number of transformations: ";
    cin >> count;

    if (count <= 0)
    {
        cout << "\nInvalid number of transformations.\n";
        return;
    }

    double composite[3][3] =
    {
        {1, 0, 0},
        {0, 1, 0},
        {0, 0, 1}
    };

    double M[3][3];
    double temp[3][3];

    for (int i = 0; i < count; i++)
    {
        cout << "\n=====================================\n";
        cout << "Transformation " << i + 1 << "\n";
        cout << "=====================================\n";

        cout << "\n1. Translation";
        cout << "\n2. Scaling";
        cout << "\n3. Rotation";
        cout << "\n4. Reflection about X-axis";
        cout << "\n5. Reflection about Y-axis";
        cout << "\n6. Shearing in X-direction";
        cout << "\n7. Shearing in Y-direction";

        int choice;
        cout << "\n\nEnter choice: ";
        cin >> choice;

        if (choice < 1 || choice > 7)
        {
            cout << "\nInvalid choice. Try again.\n";
            i--;
            continue;
        }

        createTransformation(choice, M);

        displayMatrix(M, "CURRENT TRANSFORMATION MATRIX");

        // Row-vector convention:
        // P' = P x M1 x M2 x M3 ...
        multiplyMatrix(composite, M, temp);
        copyMatrix(temp, composite);

        transformPoints(original, transformed, n, composite);

        displayPoints(
            transformed,
            n,
            "POINTS AFTER THIS TRANSFORMATION"
        );
    }

    displayMatrix(composite, "FINAL COMPOSITE MATRIX");

    transformPoints(original, transformed, n, composite);

    displayPoints(
        transformed,
        n,
        "FINAL TRANSFORMED POINTS"
    );

    displayObjects(original, transformed, n);
}

// ------------------------------------------------------------
// Main program
// ------------------------------------------------------------
int main()
{
    int n;
    int choice;

    double original[MAX][3];
    double transformed[MAX][3];

    cout << "============================================\n";
    cout << "          2D TRANSFORMATION PROGRAM\n";
    cout << "============================================\n";

    cout << "\nEnter number of points (1-" << MAX << "): ";
    cin >> n;

    if (n <= 0 || n > MAX)
    {
        cout << "\nInvalid number of points.\n";
        return 1;
    }

    cout << "\nEnter points in matrix form.\n";
    cout << "Use homogeneous coordinates: x y 1\n\n";

    for (int i = 0; i < n; i++)
    {
        cout << "Point " << i + 1 << ": ";
        cin >> original[i][0]
            >> original[i][1]
            >> original[i][2];
    }

    displayPoints(original, n, "ORIGINAL POINTS");

    do
    {
        cout << "\n\n============================================\n";
        cout << "                 MAIN MENU\n";
        cout << "============================================\n";
        cout << "\n1. Translation";
        cout << "\n2. Scaling";
        cout << "\n3. Rotation";
        cout << "\n4. Reflection about X-axis";
        cout << "\n5. Reflection about Y-axis";
        cout << "\n6. Shearing in X-direction";
        cout << "\n7. Shearing in Y-direction";
        cout << "\n8. Composite / Sequential Transformation";
        cout << "\n9. Display Original Points";
        cout << "\n0. Exit";

        cout << "\n\nEnter your choice: ";
        cin >> choice;

        switch (choice)
        {
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
                individualTransformation(
                    original,
                    transformed,
                    n,
                    choice
                );
                break;

            case 8:
                compositeTransformation(
                    original,
                    transformed,
                    n
                );
                break;

            case 9:
                displayPoints(
                    original,
                    n,
                    "ORIGINAL POINTS"
                );
                break;

            case 0:
                cout << "\nExiting program...\n";
                break;

            default:
                cout << "\nInvalid choice!\n";
        }

    } while (choice != 0);

    return 0;
}
