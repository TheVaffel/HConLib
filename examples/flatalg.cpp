#define FLATALG_IMPLEMENTATION
#include <FlatAlg.h>

#include <iostream>

using namespace std;

int main(){
	Vector3 v(100, 4, 10);


	cout<<v.normalized().str()<<endl;
	return 0;
	
}
