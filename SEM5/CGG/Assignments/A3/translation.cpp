#include<iostream>
#include<graphics.h>

using namespace std;

void init(int n)
{	
	// Pre-Multiplication i.e. P X T 
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
	
	// Display
	for(int j = 0; j < n; j++)
	{
		for(int k = 0; k < 3; k++)
		{
			cout << P[j][k] << " ";
		}
		cout << endl;
	}
	
	// T: Translation Matrix
	float tx,ty;
	cout << "Enter tx and ty: ";
	cin >> tx >> ty;
	
	float T[3][3];
	
	T[0][0] = 1;
	T[0][1] = 0;
	T[0][2] = 0;
	
	T[1][0] = 0;
	T[1][1] = 1;
	T[1][2] = 0;
	
	T[2][0] = tx;
	T[2][1] = ty;
	T[2][2] = 1;
	
	// Display T
	cout << endl;
	for(int j = 0; j < 3; j++)
	{
		for(int k = 0; k < 3; k++)
		{
			cout << T[j][k] << " ";
		}
		cout << endl;
	}
	
	// P X T
	cout << "\n\nP X T: \n";
	
	float t[n][3], a;
	for(int i=0; i<n; i++)
    {
        for(int j=0; j<3; j++)
        {
            for(int k=0; k<3; k++)
            { 
		        t[i][j] = P[i][k] * T[k][j];
		        a = a + t[i][j];
            }
            
		    cout << a << " ";
		    a=0;
        }
        cout << endl;
    }
	return;
}

/*void display(float &P)
{
	for(int j = 0; j < n; j++)
	{
		for(int k = 0; k < 3; k++)
		{
			cout << P[j][k] << " ";
		}
		cout << endl;
	}
	return;
}*/

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
