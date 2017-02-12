#include <iostream>
#include <vector>
using namespace std;

int removeDuplicates(vector<int>& nums) {
int i=1,j=0;
if (nums.size()==0) return 0;
while(i<nums.size())
 {
    if(nums[j]!=nums[i])
     {
         nums[++j]=nums[i];
     }
     i++;
 }
return j+1;
}

int main ()
{
  vector<int> v;
  v.push_back(1);
  v.push_back(1);
  cout<<removeDuplicates(v);
  for(int i=0;i<v.size();i++)
  	cout<<v[i];
  return 0;
}
