#include<iostream>
#include<graphics.h>
#include<cmath>
#include<stack>
#include<array>

using namespace std;


// --------------------------------------------------
// DDA LINE
// --------------------------------------------------
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


// --------------------------------------------------
// DRAW POLYGON
// --------------------------------------------------
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


// --------------------------------------------------
// CHECK POINT INSIDE POLYGON
// --------------------------------------------------
bool isInsidePolygon(int x, int y, int n, int points[][2])
{
    int count = 0;

    for(int i = 0; i < n; i++)
    {
        int j = (i + 1) % n;

        int x1 = points[i][0];
        int y1 = points[i][1];

        int x2 = points[j][0];
        int y2 = points[j][1];

        if((y1 > y) != (y2 > y))
        {
            double intersectionX =
                x1 +
                (double)(y - y1) *
                (x2 - x1) /
                (y2 - y1);

            if(intersectionX > x)
            {
                count++;
            }
        }
    }

    return (count % 2 == 1);
}


// --------------------------------------------------
// SEED FILL
// --------------------------------------------------
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

        s.push({x + 1, y});
        s.push({x - 1, y});
        s.push({x, y + 1});
        s.push({x, y - 1});
    }
}


// --------------------------------------------------
// AUTOMATICALLY FILL REMAINING REGIONS
// --------------------------------------------------
void fillRemainingRegions(
    int n,
    int points[][2],
    int color,
    int boundaryColor
)
{
    int maxX = getmaxx();
    int maxY = getmaxy();

    for(int y = 0; y <= maxY; y++)
    {
        for(int x = 0; x <= maxX; x++)
        {
            int currentColor = getpixel(x, y);

            // Ignore boundary and already-filled pixels
            if(currentColor == boundaryColor ||
               currentColor == color)
            {
                continue;
            }

            // Check whether this pixel belongs
            // to another enclosed region
            if(isInsidePolygon(x, y, n, points))
            {
                cout << "Automatically filling another region at: "
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
}


// --------------------------------------------------
// MAIN
// --------------------------------------------------
int main()
{
    int gd = DETECT, gm;

    initgraph(&gd, &gm, (char*)"");


    // ----------------------------------------------
    // NUMBER OF POINTS
    // ----------------------------------------------

    int n;

    cout << "Enter No. of points: ";
    cin >> n;

    if(n < 3)
    {
        cout << "\nInvalid: Enter at least 3 points!\n";
        closegraph();
        return 0;
    }


    // ----------------------------------------------
    // INPUT POINTS
    // ----------------------------------------------

    int points[n][2];

    for(int i = 0; i < n; i++)
    {
        int x, y;

        cout << "\n----------- Enter Coordinates in Cyclic Order -----------\n";

        cout << "Enter Coordinate of Point "
             << i + 1 << ": ";

        cin >> x >> y;

        int X = getmaxx();
        int Y = getmaxy();

        if(x < 0 || x > X ||
           y < 0 || y > Y)
        {
            cout << "\nError: Invalid coordinates!\n";

            cout << "Valid X = 0 to " << X << endl;
            cout << "Valid Y = 0 to " << Y << endl;

            closegraph();
            return 0;
        }

        points[i][0] = x;
        points[i][1] = y;
    }


    // ----------------------------------------------
    // ONE SEED INPUT ONLY
    // ----------------------------------------------

    int seedX, seedY;

    int maxX = getmaxx();
    int maxY = getmaxy();

    while(true)
    {
        cout << "\nEnter Seed Coordinates (x y): ";
        cin >> seedX >> seedY;

        if(seedX < 0 || seedX > maxX ||
           seedY < 0 || seedY > maxY)
        {
            cout << "Error: Seed is outside screen!\n";
            continue;
        }

        if(!isInsidePolygon(
                seedX,
                seedY,
                n,
                points))
        {
            cout << "Error: Seed is outside polygon!\n";
            continue;
        }

        break;
    }


    // ----------------------------------------------
    // DRAW FIGURE
    // ----------------------------------------------

    drawPolygon(n, points);


    // ----------------------------------------------
    // FILL FIRST REGION
    // ----------------------------------------------

    seedFill(
        seedX,
        seedY,
        YELLOW,
        CYAN
    );


    // ----------------------------------------------
    // AUTOMATICALLY FILL OTHER REGION(S)
    // ----------------------------------------------

    fillRemainingRegions(
        n,
        points,
        YELLOW,
        CYAN
    );


    getch();

    closegraph();

    return 0;
}

/*
no. of points = 7

points[7][2]
120 98
264 106
272 235
420 80
402 245
272 235
120 345

{seedx, seedy}
180 190

*/ 
