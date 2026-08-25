#include<iostream>
#include<graphics.h>
#include<cmath>
#include<stack>
#include<array>

using namespace std;

void DDAline(int x1, int y1, int x2, int y2)
{
    int dx = x2 - x1;
    int dy = y2 - y1;

    int length = max(abs(dx), abs(dy));

    if (length == 0)
    {
        putpixel(x1, y1, CYAN);
        return;
    }

    float x_inc = (float)dx / length;
    float y_inc = (float)dy / length;

    float x = x1;
    float y = y1;

    for (int i = 0; i <= length; i++)
    {
        putpixel(x, y, CYAN);

        x += x_inc;
        y += y_inc;
    }
}

void drawPolygon(int n, int points[][2])
{
	for(int i = 0; i < n; i++)
	{
		int next = (i+1)%n;
		DDAline(points[i][0], points[i][1], points[next][0], points[next][1]);
	}
	return;
}

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

        // Check if horizontal ray crosses this edge
        if((y1 > y) != (y2 > y))
        {
            double intersectionX =
                x1 + (double)(y - y1) * (x2 - x1) / (y2 - y1);

            if(intersectionX > x)
            {
                count++;
            }
        }
    }

    // Odd = inside, Even = outside
    return (count % 2 == 1);
}

void seedFill(int seedX, int seedY, int color, int boundaryColor)
{		
		// Getting maximum dimensions of output window
		int maxX = getmaxx();
    		int maxY = getmaxy();
		
		// Check whether seed is inside screen
		if(seedX < 0 || seedX > maxX || seedY < 0 || seedY > maxY)
		    {
			cout << "Error: Seed point is outside the screen!\n";
			return;
		    }
		   
		// seed should not be on boundary 
		if(getpixel(seedX, seedY) == boundaryColor)
		    {
			cout << "Error: Seed point is on the boundary!\n";
			return;
		    }
		    
		
		stack<array<int, 2>> s;
		
		s.push({seedX,seedY});
		
		while(!s.empty())
		{
			auto p = s.top();
			s.pop();
			
			int x = p[0];
			int y = p[1];
			
			// preventing access outside the screen
			if(x < 0 || x > maxX || y < 0 || y > maxY)
            			continue;
            			
			int currentColor = getpixel(x,y);
			
			if(currentColor == boundaryColor || currentColor == color)
				continue;
				
			
			putpixel(x,y,color);
			
			// x
			s.push({x+1, y});
			s.push({x-1, y});
			
			// y
			s.push({x, y+1});
			s.push({x, y-1});
		}
}


int main()
{
	int gd = DETECT, gm;
	
	initgraph(&gd, &gm, (char*)"");
	
	// Taking no. of points as input
	int n;
	cout << "Enter No. of points: ";
	cin >> n;
	
	if(n < 3)
	{
		cout << "\nInvalid : Enter Atleast 3 points to make a polygon!\n";
		return 0;		
	}
	
	// Taking coordinates in 2D-Array (x & y coordinate)
	int points[n][2];
	for(int i = 0; i < n; i++)
	{
		int x,y;
		cout << "\n----------- Enter Coordinates in Cyclic Order -----------\n";
		cout << "Enter Coordinate of Point " << i+1 << ": ";
		cin >> x >> y;
		
		int X = getmaxx();
		int Y = getmaxy();
		
		if((x < 0 || x > X) || (y < 0 || y > Y))
		{
			cout << "\nError : Valid Range of x = [0, " << X << "]" << " and y = [0, " << Y << "]\n";
			return 0;			
		}
		
		points[i][0] = x;
		points[i][1] = y;
	}
	
	int seedX, seedY;
	bool validSeed;

	int maxX = getmaxx();
	int maxY = getmaxy();

	do
	{
	    cout << "\nEnter Seed Coordinates (x y): ";
	    cin >> seedX >> seedY;

	    validSeed = true;

	    if(seedX < 0 || seedX > maxX ||
	       seedY < 0 || seedY > maxY)
	    {
		cout << "Error: Seed point is outside the screen!\n";
		cout << "Valid X: 0 to " << maxX << endl;
		cout << "Valid Y: 0 to " << maxY << endl;

		validSeed = false;
	    }

	    else if(!isInsidePolygon(seedX, seedY, n, points))
	    {
		cout << "Error: Seed point is outside the polygon!\n";
		validSeed = false;
	    }

} while(!validSeed);
	
	
	
	// drawing skeleton of polygon
	drawPolygon(n, points);
	
	// Color Filling using Seed fill Algorithm
	seedFill(seedX, seedY, YELLOW, CYAN);
	
	getch();
	closegraph();
	
	return 0;
}

/*
	Test Coordinate: 
	n = 3
	Point 1: 200 150
	Point 2: 500 150
	Point 3: 350 400
	Seed Point: 350 250
*/
