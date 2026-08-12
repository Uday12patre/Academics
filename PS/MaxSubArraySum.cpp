#include<bits/stdc++.h>

using namespace std;

int MaxSubArraySum(int A[], int n, int k)
{
	// edge case

	if(k > n || k < 0)
		return - 1;
		
	// Computing sum of First kth Block
	int window_sum = 0;
	for(int i = 0; i < k; i++)
	{
		window_sum += A[i];
	}
	
	int max_sum = window_sum;
	
	// Sliding Window
	for(int i = k; i < n; i++)
	{
		max_sum += A[i] - A[i - k];
		max_sum = max(max_sum, window_sum);
	}
	
	return max_sum;
}

int main()
{
	int A[] = {10, 50, 56, 65, 76, 98, 100};
	int size = sizeof(A)/sizeof(A[0]);
	cout << "Max SubArray Sum: " << MaxSubArraySum(A,size,100) << endl;
	return 0;
}
