//Webcamera utility

#ifndef INCLUDED_HCAM
#define INCLUDED_HCAM

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef WIN32
#include <dshow.h>

#import "qedit.dll" raw_interfaces_only named_guids

#else //WIN32
#include <getopt.h>             /* getopt_long() */

#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <linux/videodev2.h>
#endif //WIN32

#include <cmath>
#undef min
#undef max
#include <algorithm> //std::min/max

//Define this if you want super-ultra-fast decompression of jpeg
#ifdef USE_TURBOJPEG
#include <turbojpeg.h>

tjhandle decompressor;
#else
#include "external/stb_image.h"
#endif


#define HCAM_MODE_JPEG 0
#define HCAM_MODE_BAYER 1
#define HCAM_MODE_YUYV 2

class HCam{

  bool camActive = false;
  bool quit = false;
  bool upsideDown = false;

  int inputMode;

  int fd;

  struct webcam_buffer {
    void   *start;
    size_t  length;
  };

  int width, height;

  char webcamName[100];
  struct webcam_buffer    *webcam_buffers;
  unsigned int     numBuffers;

  int CLIP(int x){ return ::std::min(255, ::std::max(0, x));}

  #ifdef WIN32

  DexterLib::ISampleGrabber *sampleGrabber = NULL;
  long int bufferSize = 0;
  IGraphBuilder *graph;
  ICaptureGraphBuilder2 *builder;
  ICreateDevEnum *deviceEnum;
  IEnumMoniker *enumMonik;
  IMoniker *moniker;
  IPropertyBag *propBag;
  IBaseFilter *capFilter = NULL;
  IBaseFilter *sampleGrabberFilter = NULL;
  IBaseFilter *nullRenderer = NULL;
  IMediaControl *mediaControl = NULL;
  AM_MEDIA_TYPE mt;

  #else //WIN32
	void errno_exit(const char* s, int u);

	int xioctl(int fh, int request, void *arg);

	int init_mmap();

	void stop_capture();

	void start_capture();

	void close();
  #endif //WIN32

	void init_full_name(int w, int h, const char* deviceName, int mode, bool upsideDown);

	void YUVtoRGB(int width, int height, unsigned char *src, unsigned char *dst);

	void BayerToRGB(int width, int height, unsigned char *src, unsigned char *dst);

 public:

  HCam(int width, int height, const char* deviceName = "/dev/video0", int mode = HCAM_MODE_JPEG, bool upsideDown = false);
  ~HCam();

  int capture_image(unsigned char* rgb_buffer);
};
#endif //INCLUDED_HCAM
