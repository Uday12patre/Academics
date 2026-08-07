#include<iostream>
#include<cmath>
#include<ctime>
#include<cstdlib>

using namespace std;

void powerComputation(long long int a,long long int n)
{
	long long int s = 1;
	if(a == 0)
	{
		if(n < 0) cout << a << "^" << n << " = Undefined\n";
		else if(n == 0) cout << a << "^" << n << " = 1\n";
		else cout << a << "^" << n << " = 0\n";
		return;
	}
	
	if(n == 0)
	{
		cout << a << "^" << n << " = 1\n";
		return; 
	}
	
	if(n == 1)
	{
		cout << a << "^" << n << " = " << a << endl; 
		return;
	}
	
	for(int i = 0; i < abs(n); i++)
	{
		s = s*a;
	}
	
	if(n > 0)
	{
		cout << a << "^" << n << " = " << s << endl;		
	}
	
	else
	{
	    // If 's' is negative, move the negative sign to the front/numerator
	    if (s < 0) 
	    {
		cout << a << "^" << n << " = -1/" << abs(s) << endl;
	    } 
	    else 
	    {
		cout << a << "^" << n << " = 1/" << s << endl;
	    }
	}
	return;
}

int main()
{
	srand(time(nullptr));
	
	long long int a = rand()%50 + 1;
	long long int n = rand()%10 + 1;
	
	powerComputation(a,n);
	return 0;
}













