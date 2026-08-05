#include<iostream>
#include<graphics.h>
#include<cmath>

using namespace std;

int main()
{
	float x1 = 46, y1 = 216;
	float x2 = 180, y2 = 40;
	
	float dx = x2 - x1;
	float dy = y2 - y1;
	
	int length = (abs(dx) >= abs(dy)) ? abs(dx) : abs(dy);
	
	float x_inc = dx/length;
	float y_inc = dy/length;
	
	int gd = DETECT, gm;
	initgraph(&gd,&gm, (char*)"");
	
	int i = 0;
	while(i <= length)
	{
		putpixel(x1, y1, CYAN);
		x1 = x1 + x_inc;
		y1 = y1 + y_inc;
		i++;
	}
	
	getch();
	closegraph();
	return 0;
}
