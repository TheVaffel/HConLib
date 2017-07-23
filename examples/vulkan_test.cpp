#define WINVAL_IMPLEMENTATION
#define WINVAL_VULKAN
#include <Winval/Winval_XCB.h>
#include <iostream>

using namespace std;


int main(){
	Winval win(1280, 720);
	win.waitForKey();
	return 0;
}
