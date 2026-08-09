// Approach : Mark Visited Elements By negating the value at that index

#include <iostream>
#include <vector>
using namespace std;

vector<int> findTwoElement(vector<int> &arr)
{
    int n = arr.size();
    int repeating = -1;

    // mark visited indices by negating the value at
    // that index
    for (int i = 0; i < n; i++)
    {
        int val = abs(arr[i]);

        if (arr[val - 1] > 0)
        {
            arr[val - 1] = -arr[val - 1];
        }
        else
        {
            // found the repeating element
            repeating = val;
        }
    }

    int missing = -1;

    // the index with a positive value is the
    // missing number
    for (int i = 0; i < n; i++)
    {
        if (arr[i] > 0)
        {
            missing = i + 1;
            break;
        }
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