#include <cmath>
#include <algorithm>

void cartoonify(unsigned char* buffer, unsigned char* result, int w, int h){
	for(int i = 0; i < h; i++){
		for(int j = 0; j < w; j++){
			((int*)buffer)[i*w + j] = (buffer[4*(i*w + j)] + 3*buffer[4*(i*w+j)] + 2*buffer[4*(i*w + j)])/6;
		}
	}
	for(int i = 0; i < h; i++){
		for(int j = 0; j < w; j++){
			int sumx = ((int*)buffer)[(i -1)*w + (j-1)] + 2*((int*)buffer)[(i)*w + (j-1)] + ((int*)buffer)[(i +1)*w + (j-1)]
				- ((int*)buffer)[(i -1)*w + (j+1)] - 2*((int*)buffer)[(i)*w + (j+1)] - ((int*)buffer)[(i + 1)*w + (j+1)]; 

			int sumy = ((int*)buffer)[(i - 1)*w + (j-1)] + 2* ((int*)buffer)[(i - 1)*w + (j)] +  ((int*)buffer)[(i - 1)*w + (j+1)]
 			- ((int*)buffer)[(i - 1)*w + (j-1)] - 2* ((int*)buffer)[(i)*w + (j)] -  ((int*)buffer)[(i + 1)*w + (j+1)];
			
			((int*)result)[i*w + j] = 0x10101*(255 - std::min(255, 16*(std::abs(sumx) + std::abs(sumy))/2));
		}
	}
}
