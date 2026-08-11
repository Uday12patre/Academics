#include<iostream>
#include<cmath>
#include<graphics.h>

using namespace std;


double DegreeToRadian(double Deg)
{
	const double PI = 22.0/7.0;
	return Deg*(PI/180.0);
	
}


void init(int n)
{	
	// Pre-Multiplication i.e. P X R 
	float **P = new float*[n];
	int z;
	for(int j = 0; j < n; j++)
	{
		P[j] = new float[3];
		
		for(int k = 0; k < 3; k++)
		{
			cout << "Enter Points for ["<<j<<","<<k<<"]: ";
			cin >> P[j][k];
		}
		cout << endl;
	}
	
	// Display points in row vector form
	for(int j = 0; j < n; j++)
	{
		for(int k = 0; k < 3; k++)
		{
			cout << P[j][k] << " ";
		}
		cout << endl;
	}
	
	// R: Rotation Matrix
	// Counter Clockwise Rotation for Row Vector Form
	
	float theta;
	cout << "Enter theta in degrees: ";
	cin >> theta;
	
	double R[3][3];
	
	R[0][0] = cos(DegreeToRadian(theta));
	R[0][1] = sin(DegreeToRadian(theta));
	R[0][2] = 0;
	
	R[1][0] = -sin(DegreeToRadian(theta));
	R[1][1] = cos(DegreeToRadian(theta));
	R[1][2] = 0;
	
	R[2][0] = 0;
	R[2][1] = 0;
	R[2][2] = 1;
	
	// Display R
	cout << endl;
	for(int j = 0; j < 3; j++)
	{
		for(int k = 0; k < 3; k++)
		{
			cout << R[j][k] << " ";
		}
		cout << endl;
	}
	
	// R X T
	cout << "\n\nR X T: \n";
	
	double r[n][3], a;
	for(int i=0; i<n; i++)
    {
        for(int j=0; j<3; j++)
        {
            for(int k=0; k<3; k++)
            { 
		        r[i][j] = P[i][k] * R[k][j];
		        a = a + r[i][j];
            }
            
		    cout << a << " ";
		    a=0;
        }
        cout << endl;
    }
	return;
}

void translation(float &P, float &T)
{
	return;
}


int main()
{
	int n;
	cout << "Enter No. of Points: ";
	cin >> n;
	cout << "Enter Row Vector Points\n\n";
	init(n);
	
	return 0;
}
