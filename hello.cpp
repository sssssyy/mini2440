#include "hello.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>	
#include <sys/stat.h>

#include <linux/videodev2.h>

#include <qapplication.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qtooltip.h>
#include <qpainter.h>
#include <qimage.h>



#define WIDTH 160
#define HEIGHT 120 
#define FRAME_COUNT 1 //�建�����
#define FRAME_IDLE 60

typedef struct VideoBuffer 
{
  void   *start;
  size_t  length;
}VideoBuffer;

static int fd;
static VideoBuffer *buffers = NULL;
static unsigned int numBufs = 0;


// 鑾峰緱camara鎬ц兘
static int v4l2_get_capability(void)
{
  struct v4l2_capability cap;

  if (::ioctl (fd, VIDIOC_QUERYCAP, &cap) == -1) 
  {
    printf("Get camara capability is fail.\n");
    return -1;
  }
  
  printf("camara driver: %s\n", cap.driver);
  printf("camara card: %s\n", cap.card);
  printf("camara bus info: %s\n", cap.bus_info);
  printf("camara version: %d\n", cap.version);
  
  if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) 
  {
    printf("Camara does not support VIDEO CAPTURE.\n");
    return -1;
  }
  if (!(cap.capabilities & V4L2_CAP_STREAMING)) 
  {
    printf("Camara does not support STREAMING.\n");
    return -1;
  }
  return 0;
}

// 璁剧疆瑙嗛鎹曡幏鏍煎紡
static int v4l2_set_fmt(void)
{
  struct v4l2_format fmt;
  
  memset(&fmt, 0, sizeof(fmt));
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt.fmt.pix.width = WIDTH;
  fmt.fmt.pix.height = HEIGHT;
  // V4L2_PIX_FMT_MJPEG
  fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
  fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;
  if (::ioctl(fd, VIDIOC_S_FMT, &fmt) == -1)
  {
    printf("Camara set fmt is errror.\n");
    return -1;
  }
  return 0;
}

// 鍒嗛厤鏄犲皠鍐呭瓨
static int v4l2_set_memory(void)
{
  struct v4l2_requestbuffers req;
  enum v4l2_buf_type type;
  struct v4l2_buffer buf;
  
  // 鍒嗛厤camara鍐呭瓨
  memset(&req, 0, sizeof (req));
  req.count = FRAME_COUNT;
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;
  
  if (::ioctl(fd, VIDIOC_REQBUFS, &req) < 0)
  {
    printf("VIDIOC_REQBUFS failed\n");
    return -1;
  }
  
  // 灏哻amara鍐呭瓨鏄犲皠鍒扮敤鎴风┖闂�  
  buffers = (VideoBuffer *)calloc(req.count, sizeof(*buffers));
  
  for (numBufs = 0; numBufs < req.count; numBufs++) 
  {
    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = numBufs;
    // 璇诲彇缂撳瓨
    if (::ioctl(fd, VIDIOC_QUERYBUF, &buf) == -1)
      return -1;

    buffers[numBufs].length = buf.length;
    buffers[numBufs].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
    if (buffers[numBufs].start == MAP_FAILED)
      return -1;
    
    // 鏀惧叆缂撳瓨闃熷垪
    if (::ioctl(fd, VIDIOC_QBUF, &buf) == -1)
      return -1;
  }
  
  // 寮�濮嬭棰戞樉绀哄嚱鏁�  
  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (::ioctl (fd, VIDIOC_STREAMON, &type) < 0) 
  {
    printf("VIDIOC_STREAMON error\n");
    return -1;
  }
  
  return 0;
}

// 璇诲彇camara鏁版嵁
static int v4l2_read_pic(int index)
{
  struct v4l2_buffer buf;
  
  memset(&buf,0,sizeof(buf));
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;
  buf.index = index;
  //璇诲彇缂撳瓨
  if (::ioctl(fd, VIDIOC_DQBUF, &buf) == -1)
    return -1;
  
  //閲嶆柊鏀惧叆缂撳瓨闃熷垪
  if (::ioctl(fd, VIDIOC_QBUF, &buf) == -1) 
    return -1;
  
  return 0;
}

static int v4l2_init(const char *dev_name)
{
  // 鎵撳紑video device
  fd = ::open(dev_name, O_RDWR);
  if (fd == -1)
  {
    printf("open camera is fail.\n");
    return -1;
  }

  // 妫�鏌amara鎬ц兘
  if (v4l2_get_capability())
    return -1;

  // 璁剧疆瑙嗛鎹曡幏鏍煎紡
  if (v4l2_set_fmt())
    return -1;

  // 鍒嗛厤鏄犲皠鍐呭瓨
  if (v4l2_set_memory())
    return -1;

  return 0;
}



MyHelloForm::MyHelloForm( QWidget* parent, const char* name, WFlags fl)
	    :HelloBaseForm(parent, name, fl)
{

  if (v4l2_init("/dev/video0"))
  {
    printf("v4l2 init is error!\n");
    emit(close_signal());
  }
 
 
  connect (this, SIGNAL(close_signal()), this, SLOT(camara_quit()));  
  connect (this, SIGNAL(quit_signal()), qApp, SLOT(quit()));
  connect (CloseButton, SIGNAL(clicked()), this, SLOT(camara_quit()));
  connect (SnapButton, SIGNAL(clicked()), this, SLOT(camara_snap()));
    
  timer = new QTimer(this); // 瀹氫箟QT瀹氭椂鍣�	
  connect (timer, SIGNAL(timeout()), this, SLOT(showMe()));
  timer->start(FRAME_IDLE); // 寮�鍚畾鏃跺櫒10ms瀹氭椂)
	
}

MyHelloForm::~MyHelloForm()
{
}


void MyHelloForm::showMe()
{

  QPainter painter(this);
  QImage image;

  if (v4l2_read_pic(0))
    return;
  image.loadFromData((uchar *)buffers[0].start, buffers[0].length, "JPEG");
  
  painter.drawImage(40, 30, image, 0, 0, WIDTH, HEIGHT);

}

void MyHelloForm::camara_snap()
{
  int pic_fd, i;
  time_t t = time(NULL);
  char name[50], *pic_name, *dir_name = "/root/Documents/";
  size_t len, len1;
  
  timer->stop();

  pic_name = ctime(&t);
  len = strlen(pic_name);
  len1 = strlen(dir_name);
  dir_name = strcpy(name, dir_name);
  for (i=0; i<len-1; i++)
    dir_name[len1+i] = pic_name[i];
  dir_name[len1+len-1] = '\0';
  dir_name = strcat(dir_name, ".jpg");
  printf("dir_name: %s\n", dir_name);
  
  pic_fd = ::open(dir_name, O_RDWR | O_CREAT, S_IRWXU);
  if (pic_fd == -1)
  {
    printf("save jpg file fail\n");
    return;
  }
  ::write(pic_fd, buffers[0].start, buffers[0].length);
  ::close(pic_fd);
  
  sleep(1);
  timer->start(FRAME_IDLE);
}

void MyHelloForm::camara_quit()
{
  munmap(buffers[0].start, FRAME_COUNT*buffers[0].length);
  ::close(fd);
  timer->stop();
  emit(quit_signal());
}

