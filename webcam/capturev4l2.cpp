#include <errno.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#define STBI_ONLY_JPEG
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

uint8_t *buffer;
unsigned char* rgb_buffer;
bool camActive = false;
bool quit = false;
int x,y,n;

struct byte3{
  char a;
  char b;
  char c;
  byte3 operator=(int rhs){
    a = rhs&0xff;
    b = (rhs>>8)&0xff;
    c = (rhs>>16)&0xff;
    return *this;
  }
};
 
static int xioctl(int fd, int request, void *arg)
{
        int r;
 
        do r = ioctl (fd, request, arg);
        while (-1 == r && EINTR == errno);
 
        return r;
}
 
int init_mmap(int fd)
{
    struct v4l2_format format;
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    format.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    format.fmt.pix.width = 640;
    format.fmt.pix.height = 480;

    if(ioctl(fd, VIDIOC_S_FMT, &format) < 0){
        perror("VIDIOC_S_FMT");
        exit(1);
    }

    struct v4l2_requestbuffers req = {0};
    req.count = 1;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
 
    if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req))
    {
        perror("Requesting Buffer");
        return 1;
    }
 
    struct v4l2_buffer buf = {0};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = 0;
    if(-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf))
    {
        perror("Querying Buffer");
        return 1;
    }
 
    buffer = static_cast<uint8_t*>(mmap (NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset));
    //printf("Length: %d\nAddress: %p\n", buf.length, buffer);
    //printf("Image Length: %d\n", buf.bytesused);
 
    return 0;
}
 
int capture_image(int fd)
{
    struct v4l2_buffer buf = {0};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = 0;
    if(-1 == xioctl(fd, VIDIOC_QBUF, &buf))
    {
        perror("Query Buffer");
        return 1;
    }
 
    fd_set fds;
    if(!camActive){
      if(-1 == xioctl(fd, VIDIOC_STREAMON, &buf.type))
      {
          perror("Start Capture");
          return 1;
      }
 
      FD_ZERO(&fds);
      FD_SET(fd, &fds);
      camActive = true;
    }
    struct timeval tv = {0};
    tv.tv_sec = 2;
    int r = select(fd+1, &fds, NULL, NULL, &tv);
    if(-1 == r)
    {
        perror("Waiting for Frame");
        return 1;
    }
 
    if(-1 == xioctl(fd, VIDIOC_DQBUF, &buf))
    {
        perror("Retrieving Frame");
        return 1;
    }

    rgb_buffer = stbi_load_from_memory((const unsigned char*)buffer, buf.length, &x, &y, &n, 4);

    return 0;
}
 
int main()
{
        int fd;
 
        fd = open("/dev/video1", O_RDWR);
        if (fd == -1)
        {
                perror("Opening video device");
                return 1;
        }
        
        if(init_mmap(fd))
            return 1;
        while(!quit)
        {
            if(capture_image(fd))
                return 1;
        }
        close(fd);
        return 0;
}
