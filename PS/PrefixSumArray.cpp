#include<bits/stdc++.h>

using namespace std;

vector<int> PrefixSumArray(vector<int> A)
{
	vector<int> PrefixSum(A.size(), 0);
	
	PrefixSum[0] = A[0];
	for(int i = 1; i < A.size(); i++)
	{
		PrefixSum[i] = PrefixSum[i-1] + A[i];
	}
	
	return PrefixSum;
}

int main()
{
	srand(time(nullptr));
	
	int n;
	cout << "Enter size of Array: ";
	cin >> n;
	
	vector<int> A(n);
	for(int i = 0; i < n; i++)
	{
		A[i] = rand()%100 + 1;
	}
	
	cout << "Original Array: ";
	for(int i : A)
		cout << i << " ";
		
	vector<int> answer;
	
	// PrefixSumArray
	answer = PrefixSumArray(A);
	
	cout << "\nPrefixSumArray: ";
	for(int i : answer)
		cout << i << " ";
	cout << endl; 
	return 0;
}
