#include<bits/stdc++.h>

using namespace std;

// Two Sum Solution for Sorted Array by Two-Pointer Technique (Directional State)
bool hasPairWithSum(int A[], int n, int target)
{
	int left = 0, right = n - 1;
	while(left < right)
	{
		int current_sum = A[left] + A[right];
		if(current_sum < target) left++;
		else if(current_sum > target) right--;
		else return true;
	}
	return false;
}

int main()
{
	// Sorted Array
	int A[] = {10, 50, 56, 65, 76, 98, 100};
	int size = sizeof(A)/sizeof(A[0]);
	cout << boolalpha <<hasPairWithSum(A, size, 110) << endl;
	return 0; 
}
