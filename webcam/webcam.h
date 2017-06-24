
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <getopt.h>             /* getopt_long() */

#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <cmath>
#include <algorithm> //std::min/max

//Define this if you want super-ultra-fast decompression of jpeg
#ifdef USE_TURBOJPEG
#include <turbojpeg.h>

tjhandle _webcam_decompressor;
#else
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#include <linux/videodev2.h>

#define WEBCAM_MODE_JPEG 0
#define WEBCAM_MODE_BAYER 1
#define WEBCAM_MODE_YUYV 2

#define CLEAR(x) memset(&(x), 0, sizeof(x))

bool _webcam_camActive = false;
bool _webcam_quit = false;
bool _webcam_upside_down = false;

int _webcam_input_mode;

int _webcam_fd;

struct _webcam_buffer {
        void   *start;
        size_t  length;
};

int _webcam_width, _webcam_height;
struct _webcam_buffer          *_webcam_buffers;
static unsigned int     _webcam_n_buffers;

static void _webcam_errno_exit(const char* s, int u)
{
  fprintf(stderr, "%s error %d, %s at line %d\n", s, errno, strerror(errno), u);
  exit(EXIT_FAILURE);
}
  
char _webcam_name[100];

static int _webcam_xioctl(int fh, int request, void *arg)
{
         int r;

        do {
                r = ioctl(fh, request, arg);
        } while (-1 == r && EINTR == errno);

        return r;
}

void _webcam_start_capture(){
  unsigned int i;
  v4l2_buf_type type;

  for (i = 0; i < _webcam_n_buffers; ++i) {
    struct v4l2_buffer buf;

    CLEAR(buf);
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = i;

    if (-1 == _webcam_xioctl(_webcam_fd, VIDIOC_QBUF, &buf))
      _webcam_errno_exit("VIDIOC_QBUF", __LINE__);
  }
  
  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (-1 == _webcam_xioctl(_webcam_fd, VIDIOC_STREAMON, &type))
    _webcam_errno_exit("VIDIOC_STREAMON", __LINE__);
}
 
int _webcam_init_mmap()
{
    struct v4l2_requestbuffers req;

    CLEAR(req);

    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (-1 == _webcam_xioctl(_webcam_fd, VIDIOC_REQBUFS, &req)) {
      if (EINVAL == errno) {
	fprintf(stderr, "%s does not support "
		"memory mapping\n", _webcam_name);
	exit(EXIT_FAILURE);
      } else {
	_webcam_errno_exit("VIDIOC_REQBUFS", __LINE__);
      }
    }

    if (req.count < 2) {
      fprintf(stderr, "Insufficient buffer memory on %s\n",
	      _webcam_name);
      exit(EXIT_FAILURE);
    }

    _webcam_buffers = (_webcam_buffer*)calloc(req.count, sizeof(*_webcam_buffers));

    if (!_webcam_buffers) {
      fprintf(stderr, "Out of memory\n");
      exit(EXIT_FAILURE);
    }

    for (_webcam_n_buffers = 0; _webcam_n_buffers < req.count; ++_webcam_n_buffers) {
      struct v4l2_buffer buf;

      CLEAR(buf);

      buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf.memory      = V4L2_MEMORY_MMAP;
      buf.index       = _webcam_n_buffers;

      if (-1 == _webcam_xioctl(_webcam_fd, VIDIOC_QUERYBUF, &buf))
	_webcam_errno_exit("VIDIOC_QUERYBUF", __LINE__);

      _webcam_buffers[_webcam_n_buffers].length = buf.length;
      _webcam_buffers[_webcam_n_buffers].start =
	mmap(NULL /* start anywhere */,
	     buf.length,
	     PROT_READ | PROT_WRITE /* required */,
	     MAP_SHARED /* recommended */,
	     _webcam_fd, buf.m.offset);

      if (MAP_FAILED == _webcam_buffers[_webcam_n_buffers].start)
	_webcam_errno_exit("mmap", __LINE__);
    }

    //printf("Length: %d\nAddress: %p\n", buf.length, buffer);
    //printf("Image Length: %d\n", buf.bytesused);
 
    return 0;
}

static void YUVtoRGB(int width, int height, unsigned char *src, unsigned char *dst)
{
  int line, column;
  unsigned char *py, *pu, *pv;
  unsigned char *tmp = dst;

#define CLIP(x) ( (x)>=0xFF ? 0xFF : ( (x) <= 0x00 ? 0x00 : (x) ) )

  int increment;

  int pyinc = 2, puinc = 4, pvinc = 4;
  if(_webcam_upside_down){
    py = src + (width*height-1)*2;
    pu = py - 1;
    pv = py + 1;
    increment = -1;
  }else{
    py = src;
    pu = py + 1;
    pv = py + 3;
    increment = 1;
  }

  pyinc *= increment;
  puinc *= increment;
  pvinc *= increment;
  

  for (line = 0; line < height; line++){
    for (column = 0; column < width; column++) {
      *tmp++ = CLIP((double)*py + 1.772*((double)*pu-128.0));
      *tmp++ = CLIP((double)*py - 0.344*((double)*pu-128.0) - 0.714*((double)*pv-128.0));
      *tmp++ = CLIP((double)*py + 1.402*((double)*pv-128.0));

      *tmp++;
      // increase py every time
      py += pyinc;
      // increase pu,pv every second time
      if ((column & 1)==1) {
	pu += puinc;
	pv += pvinc;
      }
    }
  }
  
}

static void BayerToRGB(int width, int height, unsigned char *src, unsigned char *dst)
{ // NB: Ignores border pixels
  int w = width;
  int h = height;

  unsigned short *col = (unsigned short*)src;
  unsigned short mx = 0;
  //int tr = 0, tg = 0, tb = 0, tg2 = 0;
    
  for(int i = 1; i < h-1; i++){
    int write_row = _webcam_upside_down?h -i - 1: i;
    for(int j = 1; j < w-1; j++){
      int r, g, b;
      if(i&1){
	if(j&1){
	  g = col[i*w + j];
	  b = (col[(i - 1)*w + j] + col[(i + 1)*w + j]) >> 1;
	  r = (col[i*w + j - 1] + col[i*w + j + 1]) >> 1;
	}else{
	  g = (col[(i - 1)*w + j] + col[(i + 1)*w + j] + col[i*w + j - 1] + col[i*w + j + 1]) >> 2;
	  b = (col[(i - 1)*w + j - 1] + col[(i - 1)*w + j + 1] +
	       col[(i + 1)*w + j - 1] + col[(i + 1)*w + j + 1]) >> 2;
	  r = col[i*w + j];
	}
      }else{
	if(j&1){
	  g = (col[(i - 1)*w + j] + col[(i + 1)*w + j] + col[i*w + j - 1] + col[i*w + j + 1]) >> 2;
	  b = col[i*w + j];
	  r = (col[(i - 1)*w + j - 1] + col[(i - 1)*w + j + 1] +
	       col[(i + 1)*w + j - 1] + col[(i + 1)*w + j + 1]) >> 2;
	}else{
	  g = col[i*w + j];
	  b = (col[i*w + j - 1] + col[i*w + j + 1]) >> 1;
	  r = (col[(i - 1)*w + j] + col[(i + 1)*w + j]) >> 1;
	}
      }

      int write_column = _webcam_upside_down? w-j-1 : j;
      
      //Since we get 12 bits for every channel
      dst[4*(write_row*w + write_column) + 2] = b>>4, 255;
      dst[4*(write_row*w + write_column) + 1] = g>>4;
      dst[4*(write_row*w + write_column) ] = r>>4;
    }
  }
}

 
int webcam_capture_image(unsigned char* rgb_buffer)
{
  
  fd_set fds;
  struct timeval tv;
  int r = 1;


  struct v4l2_buffer buf;
  unsigned int i;

  

  CLEAR(buf);

  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;
  

    {
      FD_ZERO(&fds);
      FD_SET(_webcam_fd, &fds);
      r = 0;
      
      tv.tv_sec = 2;
      tv.tv_usec = 0;
      
      _webcam_xioctl(_webcam_fd, VIDIOC_DQBUF, &buf);
      tv.tv_sec = 0;
      r = select(_webcam_fd + 1, &fds, NULL, NULL, &tv);

      /*if (0 == r) {
	fprintf(stderr, "select timeout\n");
	exit(EXIT_FAILURE);
	}*/
      
      while(r == 1){
	_webcam_xioctl(_webcam_fd, VIDIOC_QBUF, &buf);

        if (-1 == _webcam_xioctl(_webcam_fd, VIDIOC_DQBUF, &buf)) {
	  switch (errno) {
	  case EAGAIN:
	    printf("Got EAGAIN error \n");
	    return 0;

	  case EIO:
	    /* Could ignore EIO, see spec. */

	    /* fall through */

	  default:
	    _webcam_errno_exit("VIDIOC_DQBUF", __LINE__);
	  }
	}
	r = select(_webcam_fd + 1, &fds, NULL, NULL, &tv);
      }
    }




  assert(buf.index < _webcam_n_buffers);

  //process_image(buffers[buf.index].start, buf.bytesused);

  //std::cout<<buf.bytesused<<std::endl;
  if(_webcam_input_mode == WEBCAM_MODE_YUYV) {
    
    YUVtoRGB(_webcam_width, _webcam_height, (unsigned char*)_webcam_buffers[buf.index].start, rgb_buffer);
    
  }else if(_webcam_input_mode == WEBCAM_MODE_BAYER){
    
    BayerToRGB(_webcam_width, _webcam_height, (unsigned char*)_webcam_buffers[buf.index].start, rgb_buffer);
    
  }else if(_webcam_input_mode == WEBCAM_MODE_JPEG){

    int w, h, c;
#ifdef USE_TURBOJPEG
    int jpg_subsamples;
      
    tjDecompressHeader2(_webcam_decompressor,
			(unsigned char*)_webcam_buffers[buf.index].start,
			buf.length,
			&w,
			&h,
			&jpg_subsamples);

    tjDecompress2(_webcam_decompressor,
		  (unsigned char*)_webcam_buffers[buf.index].start,
		  buf.length,
		  rgb_buffer,
		  w,
		  0,
		  h,
		  TJPF_BGRA,
		  TJFLAG_FASTDCT);
#else
    
    unsigned char* temp  = stbi_load_from_memory((unsigned char*)_webcam_buffers[buf.index].start, buf.length, &w, &h, &c, 4);
    if(temp){
      memcpy(rgb_buffer, temp, w*h*4); //Hopefully, the compiler optimizes this away
      stbi_image_free(temp);
    }
      
#endif
  }
  if(-1 == _webcam_xioctl(_webcam_fd, VIDIOC_QBUF, &buf))
    _webcam_errno_exit("VIDIOC_QBUF", __LINE__);
  
  return 0;
}

void webcam_init_full_name(int width, int height, const char* deviceName, int mode = WEBCAM_MODE_JPEG, bool upsideDown = false) {
  memcpy(_webcam_name, deviceName, strlen(deviceName));
  _webcam_input_mode = mode;
  _webcam_upside_down = upsideDown;
  
  struct stat st;

  _webcam_width = width;
  _webcam_height = height;

  if (-1 == stat(_webcam_name, &st)) {
    fprintf(stderr, "Cannot identify '%s': %d, %s\n",
	    _webcam_name, errno, strerror(errno));
    exit(EXIT_FAILURE);
  }

  if (!S_ISCHR(st.st_mode)) {
    fprintf(stderr, "%s is no device\n", _webcam_name);
    exit(EXIT_FAILURE);
  }

  _webcam_fd = open(_webcam_name, O_RDWR /* required */, 0);

  if (-1 == _webcam_fd) {
    fprintf(stderr, "Cannot open '%s': %d, %s\n",
	    _webcam_name, errno, strerror(errno));
    exit(EXIT_FAILURE);
  }

  struct v4l2_capability cap;
  struct v4l2_cropcap cropcap;
  struct v4l2_crop crop;
  struct v4l2_format fmt;
  unsigned int min;

  if (-1 == _webcam_xioctl(_webcam_fd, VIDIOC_QUERYCAP, &cap)) {
    if (EINVAL == errno) {
      fprintf(stderr, "%s is no V4L2 device\n",
	      _webcam_name);
      exit(EXIT_FAILURE);
    } else {
      _webcam_errno_exit("VIDIOC_QUERYCAP", __LINE__);
    }
  }

  if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
    fprintf(stderr, "%s is no video capture device\n",
	    _webcam_name);
    exit(EXIT_FAILURE);
  }

  if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
    fprintf(stderr, "%s does not support streaming i/o\n",
	    _webcam_name);
    exit(EXIT_FAILURE);
  }

  /* Select video input, video standard and tune here. */


  /*CLEAR(cropcap);

  cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  if (0 == _webcam_xioctl(_webcam_fd, VIDIOC_CROPCAP, &cropcap)) {
    crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    crop.c = cropcap.defrect; // reset to default 

    if (-1 == _webcam_xioctl(_webcam_fd, VIDIOC_S_CROP, &crop)) {
      switch (errno) {
      case EINVAL:
	// Cropping not supported. 
	break;
      default:
	// Errors ignored. 
	break;
      }
    }
  }*/

  const int formats[] = {V4L2_PIX_FMT_MJPEG, V4L2_PIX_FMT_YUYV, V4L2_PIX_FMT_YUYV};

  CLEAR(fmt);

  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  
  //fprintf(stderr, "Set YUYV\r\n");
  fmt.fmt.pix.width       = width;
  fmt.fmt.pix.height      = height; 
  fmt.fmt.pix.pixelformat = formats[mode];
  fmt.fmt.pix.field       = V4L2_FIELD_ANY;
    
  if (-1 == _webcam_xioctl(_webcam_fd, VIDIOC_S_FMT, &fmt))
    _webcam_errno_exit("VIDIOC_S_FMT", __LINE__);

  v4l2_streamparm parm = {};
  parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  parm.parm.output.timeperframe.numerator = 1;
  parm.parm.output.timeperframe.denominator = 30;

  assert(fmt.fmt.pix.pixelformat == formats[mode]);

  if(-1 == _webcam_xioctl(_webcam_fd, VIDIOC_S_PARM, &parm)){
    _webcam_errno_exit("VIDIOC_S_PARM", __LINE__);
  }

  /* Note VIDIOC_S_FMT may change width and height. */
  if (width != fmt.fmt.pix.width) {
    width = fmt.fmt.pix.width;
    fprintf(stderr,"Image width set to %i by device %s.\n",width, _webcam_name);
  }
  if (height != fmt.fmt.pix.height) {
    height = fmt.fmt.pix.height;
    fprintf(stderr,"Image height set to %i by device %s.\n",height,_webcam_name);
  }

  printf("Width is %d and height is %d\n", width, height);
  
  /* Buggy driver paranoia. */
  min = fmt.fmt.pix.width * 2;
  if (fmt.fmt.pix.bytesperline < min)
    fmt.fmt.pix.bytesperline = min;
  min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
  if (fmt.fmt.pix.sizeimage < min)
    fmt.fmt.pix.sizeimage = min;

  _webcam_init_mmap();
#ifdef USE_TURBOJPEG
  _webcam_decompressor = tjInitDecompress();
#endif
  _webcam_start_capture();
}

void webcam_init(int width, int height, int deviceNum, int mode = WEBCAM_MODE_JPEG, bool upsideDown = false){
  const char* pattern = "/dev/video%d";
  char temp_name[100];
  memset(temp_name, 0, 100);
  sprintf(temp_name, pattern, deviceNum);
  
  webcam_init_full_name(width, height, temp_name, mode, upsideDown);
}

void _webcam_stop_capture(){
  enum v4l2_buf_type type;

  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (-1 == _webcam_xioctl(_webcam_fd, VIDIOC_STREAMOFF, &type))
    _webcam_errno_exit("VIDIOC_STREAMOFF", __LINE__);
}

void webcam_close(){
  _webcam_stop_capture();

  unsigned int i;

  for (i = 0; i < _webcam_n_buffers; ++i)
    if (-1 == munmap(_webcam_buffers[i].start, _webcam_buffers[i].length))
      _webcam_errno_exit("munmap", __LINE__);
  free(_webcam_buffers);
  
  close(_webcam_fd);
}
