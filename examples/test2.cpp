#include <iostream>
#include <unistd.h>
#include <ctime>

using namespace std;

int main(){

  clock_t start = clock();
  
  int count = 0;
  cout<<"Clocks per sec: "<<CLOCKS_PER_SEC<<endl;
  while(1){
    usleep(100);
    clock_t end = clock();
    unsigned long long diff = end - start;
    //cout<<"Diff was "<<diff<<endl;
    count++;
    if(diff > CLOCKS_PER_SEC){
      cout<<"Counted to "<<count<<endl;
      count = 0;
      start = end;
    }
  }

  return 0;
}
