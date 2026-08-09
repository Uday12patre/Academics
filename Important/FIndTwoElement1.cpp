// Approch: Using Frequency Array

#include <iostream>
#include <vector>
using namespace std;

vector<int> findTwoElement(vector<int> &arr)
{

    int n = arr.size();
    // frequency array to count occurrences
    vector<int> freq(n + 1, 0);
    int repeating = -1, missing = -1;

    // count frequency of each element
    for (int i = 0; i < n; i++)
    {
        freq[arr[i]]++;
    }

    // identify repeating and missing elements
    for (int i = 1; i <= n; i++)
    {
        if (freq[i] == 0)
            missing = i;
        else if (freq[i] == 2)
            repeating = i;
    }

    return {repeating, missing};
}

int main()
{
    vector<int> arr = {3, 1, 3};
    vector<int> ans = findTwoElement(arr);
    cout << ans[0] << " " << ans[1] << endl;
    return 0;
}