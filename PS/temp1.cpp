#include<iostream>
#include<cstdlib>
#include<ctime>

using namespace std;

void exchange(int *a, int *b) // swapping with temporary variable
{
	int temp = *a;
	*a = *b;
	*b = temp;
	return;
}

int main()
{
	srand(time(nullptr));
	
	int a = rand()%100 + 1;
	int b = rand()%100 + 1;
	
	cout << "Here, a=" << a << " & b=" << b;
	exchange(&a, &b);
	cout << ", After swapping a=" << a << " & b=" << b << endl;
	
	return 0;
}

