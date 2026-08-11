#include<iostream>
#include<graphics.h>

using namespace std;

void init(int n)
{	
	// Pre-Multiplication i.e. P X S 
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
	
	// S: Scaling Matrix
	float sx,sy;
	cout << "Enter sx and sy: ";
	cin >> sx >> sy;
	
	float S[3][3];
	
	S[0][0] = sx;
	S[0][1] = 0;
	S[0][2] = 0;
	
	S[1][0] = 0;
	S[1][1] = sy;
	S[1][2] = 0;
	
	S[2][0] = 0;
	S[2][1] = 0;
	S[2][2] = 1;
	
	// Display S
	cout << endl;
	for(int j = 0; j < 3; j++)
	{
		for(int k = 0; k < 3; k++)
		{
			cout << S[j][k] << " ";
		}
		cout << endl;
	}
	
	// S X T
	cout << "\n\nS X T: \n";
	
	float s[n][3], a;
	for(int i=0; i<n; i++)
    {
        for(int j=0; j<3; j++)
        {
            for(int k=0; k<3; k++)
            { 
		        s[i][j] = P[i][k] * S[k][j];
		        a = a + s[i][j];
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
