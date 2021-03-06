#include <HCam.hpp>
#include <Winval.hpp>

#include <algorithm>
#include <cmath>

using namespace std;

void toGray(uint8_t* src, uint8_t* dst, int w , int h){
  uint32_t* srci = (uint32_t*)src;
  uint8_t* dstt = dst;
  for(int i = 0; i < w * h; i++){
    *dstt = (((*srci>>16)&255)*2 + ((*srci>>8)&255)*3 + (*srci & 255))/6;
    dstt++; srci++;
  }
}

void toRGB(uint8_t* src, uint8_t* dst, int w, int h){
  uint32_t* dsti = (uint32_t*)dst;
  uint8_t* srcc = src;
  for(int i = 0; i < w * h; i++){
    *dsti = 0x10101 * (*srcc);
    dsti++; srcc++;
  }
}

void filterGray2D(uint8_t* src, uint8_t* dst, int w, int h, const float* kernel, int k, int offset, float coeff){
  for(int i = 1; i < h-1; i++){
    for(int j = 1; j < w-1; j++){
      float sum = 0;
      for(int ki = -k/2; ki <= k/2; ki++){
        for(int kj = -k/2; kj <= k/2; kj++){
          sum += src[(i + ki) * w + (j + kj)]*kernel[(ki + k/2) * k + (kj + k/2)];
        }
      }
      dst[i * w + j] = max(min((int)(sum*coeff) + offset, 255), 0);
    }
  }
}

int main(){
  const int w = 640, h = 480;

  const int kw = 3;
  
  /*const int nTimes = 1;
  const int offset = 0;
  const float coeff = 1.0f;
  const float kernel[kw * kw] = {0, 0, 0,
                                 0, 1, 0,
                                 0, 0, 0};
    */                             
  /*const int nTimes = 3;
  const int offset = 0;
  const float coeff = 1.0f/9.0f;
  const float kernel[kw * kw] = {1, 1, 1,
                                 1, 1, 1,
                                 1, 1, 1};
  */
  /*const int offset = 0;
  const int nTimes = 1;
  const float coeff = 1.0f/16.0f;
  const float kernel[kw * kw] = {1, 2, 1,
                                 2, 4, 2,
                                 1, 2, 1};
  */
  
  const int nTimes = 1;
  const int offset = 128;
  const float coeff = 1.0f;
  const float kernel[kw * kw] = {0, 0, 0,
                                 -1, 0, 1,
                                 0, 0, 0};
  
  
  HCam hcam(w, h, "/dev/video1");
  Winval win(w, h);

  uint8_t* rgb_buffer = new uint8_t[w * h * 4];

  uint8_t* gray_buffer = new uint8_t[w * h];
  uint8_t* gray_temp = new uint8_t[w * h];

  while(win.isOpen()){
    hcam.capture_image(rgb_buffer);
    toGray(rgb_buffer, gray_buffer, w, h);

    uint8_t* bufs[] = {gray_buffer, gray_temp};
    int i;
    for(i = 0; i < nTimes; i++){
      filterGray2D(bufs[i&1], bufs[(i&1)^1], w, h, kernel, kw, offset, coeff);
    }

    //filterGray2D(gray_buffer, gray_temp, w, h, kernel, kw, offset, coeff);

    toRGB(bufs[nTimes&1], rgb_buffer, w, h);
    win.drawBuffer(rgb_buffer, w, h);

    win.flushEvents();
    if(win.isKeyPressed(WK_ESC)){
      break;
    }
  }

  delete[] rgb_buffer;
  delete[] gray_buffer;

  return 0;

}
