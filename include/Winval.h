#ifndef INCLUDED_WINVAL
#define INCLUDED_WINVAL

#include <string> //std::string

#ifdef WIN32

#include <windows.h>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

#define WK_SPACE VK_SPACE
#define WK_ESC VK_ESCAPE

#define WK_0 0x30
#define WK_1 0x31
#define WK_2 0x32
#define WK_3 0x33
#define WK_4 0x34
#define WK_5 0x35
#define WK_6 0x36
#define WK_7 0x37
#define WK_8 0x38
#define WK_9 0x39

#define WK_A 0x41
#define WK_B 0x42
#define WK_C 0x43
#define WK_D 0x44
#define WK_E 0x45
#define WK_F 0x46
#define WK_G 0x47
#define WK_H 0x48
#define WK_I 0x49
#define WK_J 0x4A
#define WK_K 0x4B
#define WK_L 0x4C
#define WK_M 0x4D
#define WK_N 0x4E
#define WK_O 0x4F
#define WK_P 0x50
#define WK_Q 0x51
#define WK_R 0x52
#define WK_S 0x53
#define WK_T 0x54
#define WK_U 0x55
#define WK_V 0x56
#define WK_W 0x57
#define WK_X 0x58
#define WK_Y 0x59
#define WK_Z 0x5A

#define WK_LEFT VK_LEFT
#define WK_RIGHT VK_RIGHT
#define WK_DOWN VK_DOWN
#define WK_UP VK_UP

#else //WIN32

#include "X11/Xlib.h"
#include "X11/Xutil.h"
#include "X11/Xatom.h"
#include "X11/keysym.h"

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

#endif //WIN32

class Winval{
  int width, height;

#ifdef WIN32
  COLORREF* pixelData;

  WNDCLASSEX wc;
  HWND hwnd;
  HINSTANCE hinstance;
  MSG msg;
  HDC hdc;

  bool windowOpen;

  void handleEventProperly(MSG& msg);
#else //WIN32
  Display *dsp;
  int screenNum;
  GC gc;
  Window win;
  Window focusWindow;
  char* pixelData;
  XImage *image;
  KeySym *ks;
  Atom wm_delete_window;
  int keysyms;

  void handleEventProperly(XEvent& e);
  int getKeySym(int keycode);
#endif //WIN32

  bool isDown[65536];

  bool autoRepeat;

  int pointerX, pointerY;
  bool mouseButtonPressed;

  bool lockedPointer;
  int lockedPointerX, lockedPointerY;

 public:
  Winval(int w, int h, bool fullscreen = false);
  Winval();
  ~Winval();

  std::string window_title;


  void flushEvents();

  void getPointerPosition(int* x, int* y);
  bool isMouseButtonPressed();
  bool isKeyPressed(int i);
  int waitForKey();
  void getButtonStateAndMotion(bool& valid, int& x, int& y);
  void waitForButtonPress(int& x, int& y);
  void drawBuffer(unsigned char* p, int w, int h);
  void enableAutoRepeat(bool enable);
  const char* getTitle() const;
  void setTitle(const char* window_name);
  int getWidth() const;
  int getHeight() const;
  bool isOpen();
  void setPointerVisible(bool visible);
  void lockPointer(bool lock, int x = 300, int y = 300);
  bool isPointerLocked();
  void getPointerLockPosition(int* x, int* y);
  void sleepMilliseconds(int u);

#ifdef WIN32
  HINSTANCE getInstance() const;
  HWND getHWND() const;
#else //WIN32
  Window getWindow() const;
  Display* getDisplay() const;
#endif //WIN32
};

#endif // INCLUDED_WINVAL
