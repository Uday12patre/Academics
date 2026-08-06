#include<iostream>
using namespace std;

void factorial(long long int n)
{
	if(!(n > 0))
	{
		cout << "Invalid Integer Input.\n";
		return;
	}
	
	if(n == 0)
	{
		cout << "!" << n << "=1\n";
	}
	
	
	long long int s = 1;
	for(int i = 1; i <=n; i++)
	{
		s = s*i;		
	}
	
	cout << " & !" << n << "=" << s << endl;
	return;
}

int main()
{
	srand(time(nullptr));
	
	int n = rand()%15 + 1;
	
	cout << "Here, n=" << n;
	
	factorial(n);
	return 0;
}
