class Solution {
  public:
    int maxSubarraySum(vector<int> &arr) {

    int res = arr[0];
    int maxEnding = arr[0];
    
    for(int i = 1; i < arr.size(); i++)
    {
        maxEnding = max(arr[i], maxEnding + arr[i]);
        
        res = max(maxEnding, res);
    }
    
        return res;
    }
};
