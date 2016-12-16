#include <iostream>
#include "Intility/Intility.h"

using namespace std;

int main(){
  char c[8];
  ((long long*)c)[0] = 254 + (255 << 16);
  for(int i = 0; i < 64; i++){
    cout<<(((((long long*)c)[0])&(1LL<<(63 -i))) !=0);
  }
  cout<<endl;

  c[0] = 1;
  for(int i = 0; i < 64; i++){
    cout<<(((((long long*)c)[0])&(1LL<<(63 -i))) !=0);
  }
  cout<<endl;
  
  
  return 0;
}
