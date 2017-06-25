#define WINVAL_IMPLEMENTATION
#define WINVAL_VULKAN
#include <Winval/Winval.h>
#include <iostream>

using namespace std;


int main(){
	Winval win(400, 400);
  
	win.waitForKey();
	return 0;
}
