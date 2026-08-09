#include <iostream>
#include <vector>
using namespace std;

// Function to find next gap
int nextGap(int gap)
{
    if (gap <= 1)
        return 0;
    return (gap / 2) + (gap % 2);
}

void merge(vector<int> &arr1, vector<int> &arr2, int n,
           int m)
{
    int gap = n + m;
    gap = nextGap(gap);
    while (gap > 0)
    {
        int i = 0;
        // comparing elements in the first array
        while (i + gap < n)
        {
            if (arr1[i] > arr1[i + gap])
            {
                swap(arr1[i], arr1[i + gap]);
            }
            i++;
        }

        // comparing elements in both arrays
        int j = gap > n ? gap - n : 0;
        while (i < n && j < m)
        {
            if (arr1[i] > arr2[j])
            {
                swap(arr1[i], arr2[j]);
            }
            i++;
            j++;
        }

        if (j < m)
        {
            // comparing elements in the second array
            j = 0;
            while (j + gap < m)
            {
                if (arr2[j] > arr2[j + gap])
                {
                    swap(arr2[j], arr2[j + gap]);
                }
                j++;
            }
        }

        gap = nextGap(gap);
    }
}

// Driver code
int main()
{
    vector<int> a1 = {10, 27, 38, 43, 82};
    vector<int> a2 = {3, 9};
    int n = a1.size();
    int m = a2.size();

    // Function Call
    merge(a1, a2, n, m);

    cout << "First Array: ";
    for (int i = 0; i < n; i++)
        cout << a1[i] << " ";
    cout << endl;

    cout << "Second Array: ";
    for (int i = 0; i < m; i++)
        cout << a2[i] << " ";
    cout << endl;

    return 0;
}