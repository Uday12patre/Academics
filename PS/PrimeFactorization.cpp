#include<iostream>
#include<cmath>
#include<vector>

using namespace std;

class prime
{
	public:
	// Function to check if a number is prime
	bool isPrime(int n) 
	{
	    // 1 or less are not prime numbers
	    if (n <= 1) return false;
	    
	    // 2 and 3 are prime numbers
	    if (n <= 3) return true;
	    
	    // Eliminate multiples of 2 and 3
	    if (n % 2 == 0 || n % 3 == 0) return false;
	    
	    // Check factors from 5 up to sqrt(n)
	    // All primes greater than 3 are of the form 6k ± 1
	    for (int i = 5; i * i <= n; i += 6) {
		if (n % i == 0 || n % (i + 2) == 0) {
		    return false;
		}
	    }
	    
    		return true;
	}	
	    
	void PrimeFactors(int n)
	{
		vector<int> b;
		int original_n = n;
		
		if(isPrime(n))
		{
			cout << n << " ";
			return;
		}
		
		for(int i = 2; i < sqrt(n); i++)
		{
			if(isPrime(i))
			{
				while(n%i == 0)
				{
					b.push_back(i);
					n = n / i;
				}
			}
		}
		
		if(n > 1)
		{
			b.push_back(n);
		}
		
		
		for(int factor : b)
			cout << factor << " ";
		cout << endl;
		return;
	}   
};

int main()
{
	prime p;
	
	srand(time(nullptr));
	int n = rand()%100 + 1;
	cout << "Factors of " << n << " : ";
	p.PrimeFactors(n);
	
	return 0;
}
