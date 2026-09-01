#include <iostream>
#include <graphics.h>
#include <cmath>
#include <stack>
#include <array>
#include <vector>

using namespace std;


// ==================================================
// DDA LINE ALGORITHM
// ==================================================

void DDAline(int x1, int y1, int x2, int y2)
{
    int dx = x2 - x1;
    int dy = y2 - y1;

    int length = max(abs(dx), abs(dy));

    if(length == 0)
    {
        putpixel(x1, y1, CYAN);
        return;
    }

    float x_inc = (float)dx / length;
    float y_inc = (float)dy / length;

    float x = x1;
    float y = y1;

    for(int i = 0; i <= length; i++)
    {
        putpixel(round(x), round(y), CYAN);

        x += x_inc;
        y += y_inc;
    }
}


// ==================================================
// DRAW POLYGON
// ==================================================

void drawPolygon(int n, int points[][2])
{
    for(int i = 0; i < n; i++)
    {
        int next = (i + 1) % n;

        DDAline(
            points[i][0],
            points[i][1],
            points[next][0],
            points[next][1]
        );
    }
}


// ==================================================
// BASIC SEED FILL
// ==================================================

void seedFill(
    int seedX,
    int seedY,
    int color,
    int boundaryColor
)
{
    int maxX = getmaxx();
    int maxY = getmaxy();

    if(seedX < 0 || seedX > maxX ||
       seedY < 0 || seedY > maxY)
    {
        return;
    }

    stack<array<int, 2>> s;

    s.push({seedX, seedY});

    while(!s.empty())
    {
        auto p = s.top();
        s.pop();

        int x = p[0];
        int y = p[1];

        if(x < 0 || x > maxX ||
           y < 0 || y > maxY)
        {
            continue;
        }

        int currentColor = getpixel(x, y);

        if(currentColor == boundaryColor ||
           currentColor == color)
        {
            continue;
        }

        putpixel(x, y, color);

        // 4-connected neighbours
        s.push({x + 1, y});
        s.push({x - 1, y});
        s.push({x, y + 1});
        s.push({x, y - 1});
    }
}


// ==================================================
// MARK OUTSIDE REGION
//
// This is the important part.
//
// Instead of asking:
// "Is this point inside the polygon?"
//
// We ask:
// "Can this pixel reach the screen boundary
//  without crossing the polygon boundary?"
//
// If YES  -> outside
// If NO   -> enclosed region
// ==================================================

void markOutsideRegions(
    vector<vector<bool>>& outside,
    int boundaryColor
)
{
    int maxX = getmaxx();
    int maxY = getmaxy();

    stack<array<int, 2>> s;


    // ----------------------------------------------
    // Put all screen-edge pixels into the stack
    // ----------------------------------------------

    for(int x = 0; x <= maxX; x++)
    {
        // Top
        if(getpixel(x, 0) != boundaryColor &&
           !outside[x][0])
        {
            outside[x][0] = true;
            s.push({x, 0});
        }

        // Bottom
        if(getpixel(x, maxY) != boundaryColor &&
           !outside[x][maxY])
        {
            outside[x][maxY] = true;
            s.push({x, maxY});
        }
    }


    for(int y = 0; y <= maxY; y++)
    {
        // Left
        if(getpixel(0, y) != boundaryColor &&
           !outside[0][y])
        {
            outside[0][y] = true;
            s.push({0, y});
        }

        // Right
        if(getpixel(maxX, y) != boundaryColor &&
           !outside[maxX][y])
        {
            outside[maxX][y] = true;
            s.push({maxX, y});
        }
    }


    // ----------------------------------------------
    // Flood fill the complete outside area
    // ----------------------------------------------

    while(!s.empty())
    {
        auto p = s.top();
        s.pop();

        int x = p[0];
        int y = p[1];


        // Right
        if(x + 1 <= maxX &&
           !outside[x + 1][y] &&
           getpixel(x + 1, y) != boundaryColor)
        {
            outside[x + 1][y] = true;
            s.push({x + 1, y});
        }


        // Left
        if(x - 1 >= 0 &&
           !outside[x - 1][y] &&
           getpixel(x - 1, y) != boundaryColor)
        {
            outside[x - 1][y] = true;
            s.push({x - 1, y});
        }


        // Down
        if(y + 1 <= maxY &&
           !outside[x][y + 1] &&
           getpixel(x, y + 1) != boundaryColor)
        {
            outside[x][y + 1] = true;
            s.push({x, y + 1});
        }


        // Up
        if(y - 1 >= 0 &&
           !outside[x][y - 1] &&
           getpixel(x, y - 1) != boundaryColor)
        {
            outside[x][y - 1] = true;
            s.push({x, y - 1});
        }
    }
}


// ==================================================
// FILL ALL ENCLOSED REGIONS
// ==================================================

void fillAllEnclosedRegions(
    int color,
    int boundaryColor
)
{
    int maxX = getmaxx();
    int maxY = getmaxy();


    // ----------------------------------------------
    // outside[x][y] = true means pixel is connected
    // to the screen boundary.
    // ----------------------------------------------

    vector<vector<bool>> outside(
        maxX + 1,
        vector<bool>(maxY + 1, false)
    );


    // Find everything connected to outside
    markOutsideRegions(
        outside,
        boundaryColor
    );


    // ----------------------------------------------
    // Every non-boundary pixel that is NOT outside
    // belongs to an enclosed region.
    // ----------------------------------------------

    for(int y = 0; y <= maxY; y++)
    {
        for(int x = 0; x <= maxX; x++)
        {
            int currentColor = getpixel(x, y);


            // Ignore boundary
            if(currentColor == boundaryColor)
            {
                continue;
            }


            // Ignore already filled pixels
            if(currentColor == color)
            {
                continue;
            }


            // Ignore outside area
            if(outside[x][y])
            {
                continue;
            }


            // --------------------------------------
            // This pixel is inside an enclosed region
            // --------------------------------------

            cout << "Automatically filling region at: "
                 << x << " " << y << endl;


            seedFill(
                x,
                y,
                color,
                boundaryColor
            );
        }
    }
}


// ==================================================
// MAIN
// ==================================================

int main()
{
    int gd = DETECT, gm;

    initgraph(&gd, &gm, (char*)"");


    // ==============================================
    // NUMBER OF POINTS
    // ==============================================

    int n;

    cout << "Enter No. of points: ";
    cin >> n;


    if(n < 3)
    {
        cout << "\nInvalid: Enter at least 3 points!\n";

        closegraph();

        return 0;
    }


    // ==============================================
    // INPUT POINTS
    // ==============================================

    // Using vector instead of variable-length array
    // for standard C++ compatibility.

    vector<array<int, 2>> tempPoints(n);


    for(int i = 0; i < n; i++)
    {
        int x, y;

        cout << "\n----------- Enter Coordinates -----------\n";

        cout << "Enter Coordinate of Point "
             << i + 1 << ": ";

        cin >> x >> y;


        int X = getmaxx();
        int Y = getmaxy();


        if(x < 0 || x > X ||
           y < 0 || y > Y)
        {
            cout << "\nError: Invalid coordinates!\n";

            cout << "Valid X = 0 to "
                 << X << endl;

            cout << "Valid Y = 0 to "
                 << Y << endl;


            closegraph();

            return 0;
        }


        tempPoints[i][0] = x;
        tempPoints[i][1] = y;
    }


    // Convert vector to normal 2D array
    // because drawPolygon uses points[][2]

    int points[n][2];


    for(int i = 0; i < n; i++)
    {
        points[i][0] = tempPoints[i][0];
        points[i][1] = tempPoints[i][1];
    }


    // ==============================================
    // SEED INPUT
    // ==============================================

    int seedX, seedY;

    int maxX = getmaxx();
    int maxY = getmaxy();


    cout << "\nEnter Seed Coordinates (x y): ";
    cin >> seedX >> seedY;


    if(seedX < 0 || seedX > maxX ||
       seedY < 0 || seedY > maxY)
    {
        cout << "Error: Seed is outside screen!\n";

        closegraph();

        return 0;
    }


    // ==============================================
    // DRAW POLYGON
    // ==============================================

    drawPolygon(
        n,
        points
    );


    // ==============================================
    // CHECK WHETHER SEED IS ON BOUNDARY
    // ==============================================

    if(getpixel(seedX, seedY) == CYAN)
    {
        cout << "Error: Seed cannot be on boundary!\n";

        getch();

        closegraph();

        return 0;
    }


    // ==============================================
    // FIND OUTSIDE REGION
    // ==============================================

    vector<vector<bool>> outside(
        maxX + 1,
        vector<bool>(maxY + 1, false)
    );


    markOutsideRegions(
        outside,
        CYAN
    );


    // ==============================================
    // CHECK WHETHER SEED IS INSIDE AN ENCLOSED
    // REGION
    // ==============================================

    if(outside[seedX][seedY])
    {
        cout << "Error: Seed is outside the polygon!\n";

        getch();

        closegraph();

        return 0;
    }


    // ==============================================
    // FILL REGION CONTAINING SEED
    // ==============================================

    cout << "\nFilling seed region...\n";

    seedFill(
        seedX,
        seedY,
        YELLOW,
        CYAN
    );


    // ==============================================
    // AUTOMATICALLY FILL ALL OTHER ENCLOSED REGIONS
    // ==============================================

    cout << "\nSearching for other enclosed regions...\n";

    fillAllEnclosedRegions(
        YELLOW,
        CYAN
    );


    cout << "\nAll enclosed regions filled successfully!\n";


    // ==============================================
    // WAIT
    // ==============================================

    getch();

    closegraph();

    return 0;
}
