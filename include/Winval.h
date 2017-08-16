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
  int width, height;
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
  Atom wm_delete_window;

  int getKeySym(int keycode);
 public:
  Winval(int w, int h);
  Winval();
  ~Winval();

  std::string window_title;

  void handleEventProperly(XEvent& e);

  void flushEvents();

  void getPointerPosition(int* x, int* y);
  bool isMouseButtonPressed();
  bool isKeyPressed(int i);
  int waitForKey(); 
  void getButtonStateAndMotion(bool& valid, int& x, int& y);
  void waitForButtonPress(int& x, int& y);
  void drawBuffer(char* p, int w, int h);
  void drawBuffer(unsigned char* p, int w, int h);
  void enableAutoRepeat(bool enable);
  const char* getTitle() const;
  void setTitle(const char* window_name);
  Window getWindow() const;
  Display* getDisplay() const;
  int getWidth() const;
  int getHeight() const;
  bool isOpen() const;
};

#endif // INCLUDED_WINVAL
