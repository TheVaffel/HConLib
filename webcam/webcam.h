
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

#include <linux/videodev2.h>
#define STBI_ONLY_JPEG
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define CLEAR(x) memset(&(x), 0, sizeof(x))

bool webcam_camActive = false;
bool webcam_quit = false;

int webcam_fd;

struct webcam_buffer {
        void   *start;
        size_t  length;
};

int webcam_width, webcam_height;
struct webcam_buffer          *webcam_buffers;
static unsigned int     webcam_n_buffers;

static void webcam_errno_exit(const char* s)
{
        fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
        exit(EXIT_FAILURE);
}
  
char webcam_name[100];

static int webcam_xioctl(int fh, int request, void *arg)
{
         int r;

        do {
                r = ioctl(fh, request, arg);
        } while (-1 == r && EINTR == errno);

        return r;
}

void webcam_start_capture(){
  unsigned int i;
  v4l2_buf_type type;

  for (i = 0; i < webcam_n_buffers; ++i) {
    struct v4l2_buffer buf;

    CLEAR(buf);
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = i;

    if (-1 == webcam_xioctl(webcam_fd, VIDIOC_QBUF, &buf))
      webcam_errno_exit("VIDIOC_QBUF");
  }
  
  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (-1 == webcam_xioctl(webcam_fd, VIDIOC_STREAMON, &type))
    webcam_errno_exit("VIDIOC_STREAMON");
}
 
int webcam_init_mmap()
{
    struct v4l2_requestbuffers req;

    CLEAR(req);

    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (-1 == webcam_xioctl(webcam_fd, VIDIOC_REQBUFS, &req)) {
      if (EINVAL == errno) {
	fprintf(stderr, "%s does not support "
		"memory mapping\n", webcam_name);
	exit(EXIT_FAILURE);
      } else {
	webcam_errno_exit("VIDIOC_REQBUFS");
      }
    }

    if (req.count < 2) {
      fprintf(stderr, "Insufficient buffer memory on %s\n",
	      webcam_name);
      exit(EXIT_FAILURE);
    }

    webcam_buffers = (webcam_buffer*)calloc(req.count, sizeof(*webcam_buffers));

    if (!webcam_buffers) {
      fprintf(stderr, "Out of memory\n");
      exit(EXIT_FAILURE);
    }

    for (webcam_n_buffers = 0; webcam_n_buffers < req.count; ++webcam_n_buffers) {
      struct v4l2_buffer buf;

      CLEAR(buf);

      buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf.memory      = V4L2_MEMORY_MMAP;
      buf.index       = webcam_n_buffers;

      if (-1 == webcam_xioctl(webcam_fd, VIDIOC_QUERYBUF, &buf))
	webcam_errno_exit("VIDIOC_QUERYBUF");

      webcam_buffers[webcam_n_buffers].length = buf.length;
      webcam_buffers[webcam_n_buffers].start =
	mmap(NULL /* start anywhere */,
	     buf.length,
	     PROT_READ | PROT_WRITE /* required */,
	     MAP_SHARED /* recommended */,
	     webcam_fd, buf.m.offset);

      if (MAP_FAILED == webcam_buffers[webcam_n_buffers].start)
	webcam_errno_exit("mmap");
    }

    //printf("Length: %d\nAddress: %p\n", buf.length, buffer);
    //printf("Image Length: %d\n", buf.bytesused);
 
    return 0;
}

static void YUV422toRGB888(int width, int height, unsigned char *src, unsigned char *dst)
{
  int line, column;
  unsigned char *py, *pu, *pv;
  unsigned char *tmp = dst;

  /* In this format each four bytes is two pixels. Each four bytes is two Y's, a Cb and a Cr. 
     Each Y goes to one of the pixels, and the Cb and Cr belong to both pixels. */
  py = src;
  pu = src + 1;
  pv = src + 3;

  #define CLIP(x) ( (x)>=0xFF ? 0xFF : ( (x) <= 0x00 ? 0x00 : (x) ) )

  for (line = 0; line < height; ++line) {
    for (column = 0; column < width; ++column) {
      *tmp++ = CLIP((double)*py + 1.772*((double)*pu-128.0));
      *tmp++ = CLIP((double)*py - 0.344*((double)*pu-128.0) - 0.714*((double)*pv-128.0));
      *tmp++ = CLIP((double)*py + 1.402*((double)*pv-128.0));
      *tmp++;
      // increase py every time
      py += 2;
      // increase pu,pv every second time
      if ((column & 1)==1) {
        pu += 4;
        pv += 4;
      }
    }
  }
}

 
int webcam_capture_image(unsigned char** rgb_buffer)
{
  fd_set fds;
  struct timeval tv;
  int r;

  do
    {
      FD_ZERO(&fds);
      FD_SET(webcam_fd, &fds);
      
      tv.tv_sec = 2;
      tv.tv_usec = 0;
      r = select(webcam_fd + 1, &fds, NULL, NULL, &tv);
    } while ((r == -1 && (errno = EINTR)));

  if (-1 == r) {
    if (EINTR == errno){}else{
      webcam_errno_exit("select");
    }
  }

  if (0 == r) {
    fprintf(stderr, "select timeout\n");
    exit(EXIT_FAILURE);
  }

  struct v4l2_buffer buf;
  unsigned int i;

  CLEAR(buf);

  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;

  if (-1 == webcam_xioctl(webcam_fd, VIDIOC_DQBUF, &buf)) {
    switch (errno) {
    case EAGAIN:
      return 0;

    case EIO:
      /* Could ignore EIO, see spec. */

      /* fall through */

    default:
      webcam_errno_exit("VIDIOC_DQBUF");
    }
  }

  assert(buf.index < webcam_n_buffers);

  //process_image(buffers[buf.index].start, buf.bytesused);

  
  YUV422toRGB888(webcam_width, webcam_height, (unsigned char*)webcam_buffers[buf.index].start, *rgb_buffer);

  if (-1 == webcam_xioctl(webcam_fd, VIDIOC_QBUF, &buf))
    webcam_errno_exit("VIDIOC_QBUF");
    
  /*int w, h, c;
    if(rgb_buffer && *rgb_buffer){
    stbi_image_free(*rgb_buffer);
    }
    
    *rgb_buffer = stbi_load_from_memory((unsigned char*)webcam_buffer, buf.length, &w, &h, &c, 4);*/
  
  return 0;
}

void webcam_init(int width, int height, int deviceNum){

  
  const char* pattern = "/dev/video%d";
  sprintf(webcam_name, pattern, deviceNum);

  struct stat st;

  webcam_width = width;
  webcam_height = height;

  if (-1 == stat(webcam_name, &st)) {
    fprintf(stderr, "Cannot identify '%s': %d, %s\n",
	    webcam_name, errno, strerror(errno));
    exit(EXIT_FAILURE);
  }

  if (!S_ISCHR(st.st_mode)) {
    fprintf(stderr, "%s is no device\n", webcam_name);
    exit(EXIT_FAILURE);
  }

  webcam_fd = open(webcam_name, O_RDWR /* required */ | O_NONBLOCK, 0);

  if (-1 == webcam_fd) {
    fprintf(stderr, "Cannot open '%s': %d, %s\n",
	    webcam_name, errno, strerror(errno));
    exit(EXIT_FAILURE);
  }

  // * * * * *

  struct v4l2_capability cap;
  struct v4l2_cropcap cropcap;
  struct v4l2_crop crop;
  struct v4l2_format fmt;
  unsigned int min;

  if (-1 == webcam_xioctl(webcam_fd, VIDIOC_QUERYCAP, &cap)) {
    if (EINVAL == errno) {
      fprintf(stderr, "%s is no V4L2 device\n",
	      webcam_name);
      exit(EXIT_FAILURE);
    } else {
      webcam_errno_exit("VIDIOC_QUERYCAP");
    }
  }

  if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
    fprintf(stderr, "%s is no video capture device\n",
	    webcam_name);
    exit(EXIT_FAILURE);
  }

  if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
    fprintf(stderr, "%s does not support streaming i/o\n",
	    webcam_name);
    exit(EXIT_FAILURE);
  }

  /* Select video input, video standard and tune here. */


  /*CLEAR(cropcap);

  cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  if (0 == webcam_xioctl(webcam_fd, VIDIOC_CROPCAP, &cropcap)) {
    crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    crop.c = cropcap.defrect; // reset to default 

    if (-1 == webcam_xioctl(webcam_fd, VIDIOC_S_CROP, &crop)) {
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


  CLEAR(fmt);

  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  
  //fprintf(stderr, "Set YUYV\r\n");
  fmt.fmt.pix.width       = width;
  fmt.fmt.pix.height      = height; 
  fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
  fmt.fmt.pix.field       = V4L2_FIELD_ANY;
    
  if (-1 == webcam_xioctl(webcam_fd, VIDIOC_S_FMT, &fmt))
    webcam_errno_exit("VIDIOC_S_FMT");

  v4l2_streamparm parm = {};
  parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  parm.parm.output.timeperframe.numerator = 1;
  parm.parm.output.timeperframe.denominator = 30;

  assert(fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_YUYV);

  if(-1 == webcam_xioctl(webcam_fd, VIDIOC_S_PARM, &parm)){
    webcam_errno_exit("VIDIOC_S_PARM");
  }

  /* Note VIDIOC_S_FMT may change width and height. */
  if (width != fmt.fmt.pix.width) {
    width = fmt.fmt.pix.width;
    fprintf(stderr,"Image width set to %i by device %s.\n",width, webcam_name);
  }
  if (height != fmt.fmt.pix.height) {
    height = fmt.fmt.pix.height;
    fprintf(stderr,"Image height set to %i by device %s.\n",height,webcam_name);
  }

  printf("Width is %d and height is %d\n", width, height);
  
  /* Buggy driver paranoia. */
  min = fmt.fmt.pix.width * 2;
  if (fmt.fmt.pix.bytesperline < min)
    fmt.fmt.pix.bytesperline = min;
  min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
  if (fmt.fmt.pix.sizeimage < min)
    fmt.fmt.pix.sizeimage = min;

  webcam_width = width;
  webcam_height = height;
  webcam_init_mmap();

  webcam_start_capture();
}

void webcam_stop_capture(){
  enum v4l2_buf_type type;

  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (-1 == webcam_xioctl(webcam_fd, VIDIOC_STREAMOFF, &type))
    webcam_errno_exit("VIDIOC_STREAMOFF");
}

void webcam_close(){
  webcam_stop_capture();

  unsigned int i;

  for (i = 0; i < webcam_n_buffers; ++i)
    if (-1 == munmap(webcam_buffers[i].start, webcam_buffers[i].length))
      webcam_errno_exit("munmap");
  free(webcam_buffers);
  
  close(webcam_fd);
}
