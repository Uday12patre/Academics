#include<graphics.h>
#include<iostream>
#include<stdio.h>

using namespace std;
	
void initRegionCode(int* RegionCode	,int x_max, int y_max, int xl, int yl, int xr, int yr)
{
	
	for(int y = 0; y < y_max; y++)
	{
		for(int x = 0; x < x_max; x++)
		{
			int code = 0;		

			if(x < xl && y < yr)
				code = 1001;
			else if((x >= xl && x <= xr) && y < yr)
				code = 1000;
			else if(x > xr && y < yr)
				code = 1010;
			else if(x 	< xl && (y >= yr && y <= yl))
				code = 0001;
			else if((x >= xl && x <= xr) && (y >= yr && y <= yl))
				code = 0000;
			else if(x > xr && (y >= yr && y <= yl))
				code = 0010;
			else if(x < xl && y > yl)
				code = 0101;
			else if((x >= xl && x <= xr) && y > yl)
				code = 0100;
			else if(x > xr && y > yl)
				code = 	0110;
			
			RegionCode[(x * y_max) + y] = code;
		}
	}
		
	return;
}

int main()
{
		
	int gd = DETECT, gm;
	initgraph(&gd, &gm, (char*)"");
	
	int x_max = getmaxx() + 1;
	int y_max = getmaxy() + 1;
	
	
	cout << "x_max = " << x_max << endl;
	cout << "y_max = " << y_max << endl;
	
	int *RegionCode = new int[x_max * y_max]; 
	
	int xl = 150, yl = 400, xr = 450, yr = 150;
	
	initRegionCode(RegionCode, x_max, y_max, xl, yl, xr, yr);
	
	cout << "Region code at (10, 10): " << RegionCode[(10 * y_max) + 10] << endl;
	
	delete[] RegionCode;
	getch();
	closegraph();
	return 0;	
}
