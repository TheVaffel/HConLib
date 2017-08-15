#include <HCam.h>

HCam::HCam(int w, int h, const char* name, int mode, bool upsideDown){
  init_full_name(w, h, name, mode, upsideDown);
}

HCam::~HCam(){
  close();
}

void HCam::errno_exit(const char* s, int u)
{
  fprintf(stderr, "%s error %d, %s at line %d\n", s, errno, strerror(errno), u);
  exit(EXIT_FAILURE);
}

int HCam::xioctl(int fh, int request, void *arg)
{
  int r;

  do {
    r = ioctl(fh, request, arg);
  } while (-1 == r && EINTR == errno);

  return r;
}

void HCam::start_capture(){
  unsigned int i;
  v4l2_buf_type type;

  for (i = 0; i < numBuffers; ++i) {
    struct v4l2_buffer buf;

    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = i;

    if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
      errno_exit("VIDIOC_QBUF", __LINE__);
  }
  
  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
    errno_exit("VIDIOC_STREAMON", __LINE__);
}
 
int HCam::init_mmap()
{
  struct v4l2_requestbuffers req;

  memset(&req, 0, sizeof(req));

  req.count = 4;
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;

  if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
    if (EINVAL == errno) {
      fprintf(stderr, "%s does not support "
	      "memory mapping\n", webcamName);
      exit(EXIT_FAILURE);
    } else {
      errno_exit("VIDIOC_REQBUFS", __LINE__);
    }
  }

  if (req.count < 2) {
    fprintf(stderr, "Insufficient buffer memory on %s\n",
	    webcamName);
    exit(EXIT_FAILURE);
  }

  webcam_buffers = (webcam_buffer*)calloc(req.count, sizeof(*webcam_buffers));

  if (!webcam_buffers) {
    fprintf(stderr, "Out of memory\n");
    exit(EXIT_FAILURE);
  }

  for (numBuffers = 0; numBuffers < req.count; ++numBuffers) {
    struct v4l2_buffer buf;

    memset(&buf, 0, sizeof(buf));
    buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory      = V4L2_MEMORY_MMAP;
    buf.index       = numBuffers;

    if (-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf))
      errno_exit("VIDIOC_QUERYBUF", __LINE__);

    webcam_buffers[numBuffers].length = buf.length;
    webcam_buffers[numBuffers].start =
      mmap(NULL /* start anywhere */,
	   buf.length,
	   PROT_READ | PROT_WRITE /* required */,
	   MAP_SHARED /* recommended */,
	   fd, buf.m.offset);

    if (MAP_FAILED == webcam_buffers[numBuffers].start)
      errno_exit("mmap", __LINE__);
  }
 
  return 0;
}

void HCam::YUVtoRGB(int width, int height, unsigned char *src, unsigned char *dst)
{
  int line, column;
  unsigned char *py, *pu, *pv;
  unsigned char *tmp = dst;

  int increment;

  int pyinc = 2, puinc = 4, pvinc = 4;
  if(upsideDown){
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

void HCam::BayerToRGB(int width, int height, unsigned char *src, unsigned char *dst) // Assumes 12-bit pixels
{ // NB: Ignores border pixels
  int w = width;
  int h = height;

  unsigned short *col = (unsigned short*)src;
  unsigned short mx = 0;
  //int tr = 0, tg = 0, tb = 0, tg2 = 0;
    
  for(int i = 1; i < h-1; i++){
    int write_row = upsideDown?h -i - 1: i;
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

      int write_column = upsideDown? w-j-1 : j;
      
      //Since we get 12 bits for every channel
      dst[4*(write_row*w + write_column) + 2] = b>>4, 255;
      dst[4*(write_row*w + write_column) + 1] = g>>4;
      dst[4*(write_row*w + write_column) ] = r>>4;
    }
  }
}

 
int HCam::capture_image(unsigned char* rgb_buffer)
{
  
  fd_set fds;
  struct timeval tv;
  int r = 1;


  struct v4l2_buffer buf;
  unsigned int i;

  

  memset(&buf, 0, sizeof(buf));
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;
  

  {
    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    r = 0;
      
    tv.tv_sec = 2;
    tv.tv_usec = 0;
      
    xioctl(fd, VIDIOC_DQBUF, &buf);
    tv.tv_sec = 0;
    r = select(fd + 1, &fds, NULL, NULL, &tv);
      
    while(r == 1){
      xioctl(fd, VIDIOC_QBUF, &buf);

      if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf)) {
	switch (errno) {
	case EAGAIN:
	  printf("Got EAGAIN error \n");
	  return 0;

	case EIO:
	  /* Could ignore EIO, see spec. */

	  /* fall through */

	default:
	  errno_exit("VIDIOC_DQBUF", __LINE__);
	}
      }
      r = select(fd + 1, &fds, NULL, NULL, &tv);
    }
  }




  assert(buf.index < numBuffers);

  if(inputMode == HCAM_MODE_YUYV) {
    
    YUVtoRGB(width, height, (unsigned char*)webcam_buffers[buf.index].start, rgb_buffer);
    
  }else if(inputMode == HCAM_MODE_BAYER){
    
    BayerToRGB(width, height, (unsigned char*)webcam_buffers[buf.index].start, rgb_buffer);
    
  }else if(inputMode == HCAM_MODE_JPEG){

    int w, h, c;
#ifdef USE_TURBOJPEG
    int jpg_subsamples;
      
    tjDecompressHeader2(decompressor,
			(unsigned char*)webcam_buffers[buf.index].start,
			buf.length,
			&w,
			&h,
			&jpg_subsamples);

    tjDecompress2(decompressor,
		  (unsigned char*)webcam_buffers[buf.index].start,
		  buf.length,
		  rgb_buffer,
		  w,
		  0,
		  h,
		  TJPF_BGRA,
		  TJFLAG_FASTDCT);
#else
    
    unsigned char* temp  = stbi_load_from_memory((unsigned char*)webcam_buffers[buf.index].start, buf.length, &w, &h, &c, 4);
    if(temp){
      memcpy(rgb_buffer, temp, w*h*4); //Hopefully, the compiler optimizes this away
      stbi_image_free(temp);
    }
      
#endif
  }
  if(-1 == xioctl(fd, VIDIOC_QBUF, &buf))
    errno_exit("VIDIOC_QBUF", __LINE__);
  
  return 0;
}

void HCam::init_full_name(int w, int h, const char* deviceName, int mode, bool upDown) {
  memcpy(webcamName, deviceName, strlen(deviceName));
  inputMode = mode;
  upsideDown = upDown;
  
  struct stat st;

  width = w;
  height = h;

  if (-1 == stat(webcamName, &st)) {
    fprintf(stderr, "Cannot identify '%s': %d, %s\n",
	    webcamName, errno, strerror(errno));
    exit(EXIT_FAILURE);
  }

  if (!S_ISCHR(st.st_mode)) {
    fprintf(stderr, "%s is no device\n", webcamName);
    exit(EXIT_FAILURE);
  }

  fd = open(webcamName, O_RDWR /* required */, 0);

  if (-1 == fd) {
    fprintf(stderr, "Cannot open '%s': %d, %s\n",
	    webcamName, errno, strerror(errno));
    exit(EXIT_FAILURE);
  }

  struct v4l2_capability cap;
  struct v4l2_cropcap cropcap;
  struct v4l2_crop crop;
  struct v4l2_format fmt;
  unsigned int min;

  if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap)) {
    if (EINVAL == errno) {
      fprintf(stderr, "%s is no V4L2 device\n",
	      webcamName);
      exit(EXIT_FAILURE);
    } else {
      errno_exit("VIDIOC_QUERYCAP", __LINE__);
    }
  }

  if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
    fprintf(stderr, "%s is no video capture device\n",
	    webcamName);
    exit(EXIT_FAILURE);
  }

  if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
    fprintf(stderr, "%s does not support streaming i/o\n",
	    webcamName);
    exit(EXIT_FAILURE);
  }

  const int formats[] = {V4L2_PIX_FMT_MJPEG, V4L2_PIX_FMT_YUYV, V4L2_PIX_FMT_YUYV};

  memset(&fmt, 0, sizeof(fmt));
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  
  fmt.fmt.pix.width       = width;
  fmt.fmt.pix.height      = height; 
  fmt.fmt.pix.pixelformat = formats[mode];
  fmt.fmt.pix.field       = V4L2_FIELD_ANY;
    
  if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt))
    errno_exit("VIDIOC_S_FMT", __LINE__);

  v4l2_streamparm parm = {};
  parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  parm.parm.output.timeperframe.numerator = 1;
  parm.parm.output.timeperframe.denominator = 30;

  assert(fmt.fmt.pix.pixelformat == formats[mode]);

  if(-1 == xioctl(fd, VIDIOC_S_PARM, &parm)){
    errno_exit("VIDIOC_S_PARM", __LINE__);
  }

  /* Note VIDIOC_S_FMT may change width and height. */
  if (width != fmt.fmt.pix.width) {
    width = fmt.fmt.pix.width;
    fprintf(stderr,"Image width set to %i by device %s.\n",width, webcamName);
  }
  if (height != fmt.fmt.pix.height) {
    height = fmt.fmt.pix.height;
    fprintf(stderr,"Image height set to %i by device %s.\n",height,webcamName);
  }

  //printf("Width is %d and height is %d\n", width, height);
  
  /* Buggy driver paranoia. */
  min = fmt.fmt.pix.width * 2;
  if (fmt.fmt.pix.bytesperline < min)
    fmt.fmt.pix.bytesperline = min;
  min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
  if (fmt.fmt.pix.sizeimage < min)
    fmt.fmt.pix.sizeimage = min;

  init_mmap();
#ifdef USE_TURBOJPEG
  decompressor = tjInitDecompress();
#endif
  start_capture();
}

void HCam::stop_capture(){
  enum v4l2_buf_type type;

  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (-1 == xioctl(fd, VIDIOC_STREAMOFF, &type))
    errno_exit("VIDIOC_STREAMOFF", __LINE__);
}

void HCam::close(){
  stop_capture();

  unsigned int i;

  for (i = 0; i < numBuffers; ++i)
    if (-1 == munmap(webcam_buffers[i].start, webcam_buffers[i].length))
      errno_exit("munmap", __LINE__);
  free(webcam_buffers);
  
  ::close(fd);
}
