#include<iostream>
#include<graphics.h>
#include<cmath>

using namespace std;

int main()
{
	float x1 = 50, x2 = 250;
	float y1 = 60, y2 = 260;
	
	float dx = x2 - x1;
	float dy = y2 - y1;
	
	int length = (abs(dx) >= abs(dy)) ? abs(dx) : abs(dy);
	
	float x_inc = dx/length;
	float y_inc = dy/length;
	
	int gd = DETECT, gm;
	initgraph(&gd,&gm, (char*)"");
	
	int i = 0;
	while(i < length)
	{
		putpixel(x1, y1, CYAN);
		x1 = x1 + x_inc;
		y2 = y2 + y_inc;
		i++;
	}
	
	getch();
	closegraph();
	return 0;
}
