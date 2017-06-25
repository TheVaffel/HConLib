#ifndef INCLUDED_WINVAL
#define INCLUDED_WINVAL

#include "X11/Xlib.h"
#include "X11/Xutil.h"
#include "X11/keysym.h"

#include <string> //std::string

#define WK_SPACE XK_space
#define WK_ESC XK_Escape

#define WK_0 XK_0                            
#define WK_1 XK_1                            
#define WK_2 XK_2                            
#define WK_3 XK_3 
#define WK_4 XK_4 
#define WK_5 XK_5 
#define WK_6 XK_6 
#define WK_7 XK_7 
#define WK_8 XK_8 
#define WK_9 XK_9

#define WK_A XK_a 
#define WK_B XK_b 
#define WK_C XK_c 
#define WK_D XK_d 
#define WK_E XK_e 
#define WK_F XK_f 
#define WK_G XK_g 
#define WK_H XK_h 
#define WK_I XK_i 
#define WK_J XK_j 
#define WK_K XK_k 
#define WK_L XK_l 
#define WK_M XK_m 
#define WK_N XK_n 
#define WK_O XK_o 
#define WK_P XK_p 
#define WK_Q XK_q 
#define WK_R XK_r 
#define WK_S XK_s 
#define WK_T XK_t 
#define WK_U XK_u 
#define WK_V XK_v 
#define WK_W XK_w 
#define WK_X XK_x 
#define WK_Y XK_y 
#define WK_Z XK_z

#define WK_LEFT XK_Left
#define WK_RIGHT XK_Right
#define WK_DOWN XK_Down
#define WK_UP XK_Up

class Winval{
  int w, h;
  Display *dsp;
  int screenNum;
  GC gc;
  Window win;
  Window focusWindow;
  char* pixelData;
  XImage *image;
  
  bool isDown[65536];
  int keysyms;

  bool autoRepeat;
  
  int pointerX, pointerY;
  bool mouseButtonPressed;

  KeySym *ks;
 public:
  Winval(int w, int h, char** pointer);
  Winval();
  ~Winval();

  std::string window_title;

  void handleEventProperly(XEvent& e);

  void flushEvents();

  void getPointerPosition(int& x, int& y);
  bool isMouseButtonPressed();
  bool isKeyPressed(int i);
  int waitForKey();  
  XEvent getNextEvent();
  void getButtonStateAndMotion(bool& valid, int& x, int& y);
  void waitForButtonPress(int& x, int& y);
  void drawBuffer(char* p, int w, int h);
  void drawBuffer(unsigned char* p, int w, int h);
	void enableAutoRepeat(bool enable);
	const char* getTitle();
  void setTitle(const char* window_name);
};

#endif // INCLUDED_WINVAL

#ifdef WINVAL_IMPLEMENTATION

#define WINVAL_KEYMAP_OFFSET 8
Winval::Winval(){

}

Winval::Winval(int w, int h, char** p = 0){
  dsp = XOpenDisplay(NULL);
  if(!dsp) return;

  
  screenNum = DefaultScreen(dsp);

  int Black = BlackPixel(dsp, screenNum);
  
  win = XCreateSimpleWindow(dsp,
			    DefaultRootWindow(dsp),
			    50, 50,
			    w, h,
			    0, Black,
			    Black);

  long eventMask = StructureNotifyMask | ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask;
  XSelectInput(dsp, win, eventMask);

  XMapWindow(dsp, win);

  XEvent e;

  ks = XGetKeyboardMapping(dsp, WINVAL_KEYMAP_OFFSET, 256 - WINVAL_KEYMAP_OFFSET, &keysyms);
  for(int i = 0; i < 256; i++){
    isDown[i] = false;
  }
  
  pointerX = pointerY = 0;
  mouseButtonPressed = false;

  do{
    XNextEvent(dsp, &e);
  }while(e.type != MapNotify);

  gc = XCreateGC(dsp, win,
		    0,
		    NULL );
  image = 0;
  if(p){
    *p = new char[4*w*h];
    pixelData = *p;
    Visual* visual = XDefaultVisual(dsp, screenNum);

    image = XCreateImage(dsp, visual, 24, ZPixmap,
		       0, *p, w, h, 8, 0);

    for(int i = 0; i < 4*w*h; i++){
      (*p)[i] = 0;
    }
    XPutImage(dsp, win, gc, image, 0, 0, 0, 0, w, h);
  }

  autoRepeat = false;
}

Winval::~Winval(){
  if(image)
    XDestroyImage(image);
  if(win)
    XDestroyWindow(dsp, win);
  XCloseDisplay(dsp);
}

int Winval::waitForKey(){
  XEvent e;
  do{
    XNextEvent(dsp, &e);
    handleEventProperly(e);
   }while(e.type != KeyPress);
  return e.xkey.keycode;
}

void Winval::waitForButtonPress(int& x, int& y){
  XEvent e;
  do {
    XNextEvent(dsp, &e);
    handleEventProperly(e);
  }while(e.type != ButtonPress);

  x = e.xbutton.x, y = e.xbutton.y;
  return;
  
}

void Winval::handleEventProperly(XEvent& e){
  int index;
  int key;
  switch(e.type){
  case Expose:
    if(!image)
      break;
    XPutImage(dsp, win, gc, image, 0, 0, 0, 0, w, h);
    XFlush(dsp);
    break;
  case KeyPress:
    index = e.xkey.keycode - WINVAL_KEYMAP_OFFSET;
    key = ks[index*keysyms];
    if(key < 1<<16)
      isDown[key] = true;
    break;
  case KeyRelease:
    index = e.xkey.keycode - WINVAL_KEYMAP_OFFSET;
    key = ks[index*keysyms];
    if(key < 1<<16){
      bool pressDown = false;
      if(!autoRepeat){
	if(XEventsQueued(dsp, QueuedAlready)){
	  
	  XEvent nextEvent;
	  XPeekEvent(dsp, &nextEvent);
	  if(nextEvent.type == KeyPress && nextEvent.xkey.keycode == e.xkey.keycode &&
	     e.xkey.time == nextEvent.xkey.time){
	    XNextEvent(dsp, &nextEvent);
	    pressDown = true;
	  }
	}
      }
      isDown[key] = pressDown;
    }
    break;
  case MotionNotify:
    pointerX = e.xmotion.x, pointerY = e.xmotion.y;
    break;
  case ButtonRelease:
    mouseButtonPressed = false;
    break;
  case ButtonPress:
    mouseButtonPressed = true;
    break;
  }
}

void Winval::getPointerPosition(int& x, int& y){
  x = pointerX; y = pointerY;
}

bool Winval::isMouseButtonPressed(){
  return mouseButtonPressed;
}

bool Winval::isKeyPressed(int i){
  return isDown[i];
}

void Winval::flushEvents(){
  int num = XEventsQueued(dsp, QueuedAfterFlush);
  XEvent e2;
  while(num--){
    XNextEvent(dsp, &e2);

    handleEventProperly(e2);
  }
}

XEvent Winval::getNextEvent(){
  XEvent e;
  XNextEvent(dsp, &e);
  return e;
}

void Winval::getButtonStateAndMotion(bool& valid, int& x, int& y){
  XEvent e;
  do {
    XNextEvent(dsp, &e);
    handleEventProperly(e);
    if(e.type == ButtonRelease){
      valid = false;
      return;
    }
  }while(e.type != MotionNotify);

  x = e.xmotion.x, y = e.xmotion.y;
  valid = true;
  return;
}

void Winval::drawBuffer(char* buffer, int w, int h){
  XImage* im = XCreateImage(dsp,XDefaultVisual(dsp, screenNum), 24, ZPixmap, 0, buffer, w, h, 32, 0);
  XPutImage(dsp, win, gc, im, 0, 0, 0, 0, w, h);
  XFlush(dsp);
  //XDestroyImage(im);
}

void Winval::drawBuffer(unsigned char* buffer, int w, int h){
  drawBuffer((char*)buffer, w, h);
}

void Winval::setTitle(const char* window_name){
  XStoreName(dsp, win, window_name);
  window_title = window_name;
}

void Winval::enableAutoRepeat(bool enable){
  autoRepeat = enable;
}

const char* Winval::getTitle(){
  return window_title.c_str();
}

#endif // WINVAL_IMPLEMENTATION


