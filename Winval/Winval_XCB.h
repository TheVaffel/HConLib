#ifndef INCLUDED_WINVAL
#define INCLUDED_WINVAL

#include <xcb/xcb.h>
#include <xcb/xcb_image.h>
#include <xkbcommon/xkbcommon-x11.h>

#include <string> //std::string
#include <string.h> //strlen

#define WK_SPACE XKB_KEY_space
#define WK_ESC XKB_KEY_Escape

#define WK_0 XKB_KEY_0                            
#define WK_1 XKB_KEY_1                            
#define WK_2 XKB_KEY_2                            
#define WK_3 XKB_KEY_3 
#define WK_4 XKB_KEY_4 
#define WK_5 XKB_KEY_5 
#define WK_6 XKB_KEY_6 
#define WK_7 XKB_KEY_7 
#define WK_8 XKB_KEY_8 
#define WK_9 XKB_KEY_9

#define WK_A XKB_KEY_a 
#define WK_B XKB_KEY_b 
#define WK_C XKB_KEY_c 
#define WK_D XKB_KEY_d 
#define WK_E XKB_KEY_e 
#define WK_F XKB_KEY_f 
#define WK_G XKB_KEY_g 
#define WK_H XKB_KEY_h 
#define WK_I XKB_KEY_i 
#define WK_J XKB_KEY_j 
#define WK_K XKB_KEY_k 
#define WK_L XKB_KEY_l 
#define WK_M XKB_KEY_m 
#define WK_N XKB_KEY_n 
#define WK_O XKB_KEY_o 
#define WK_P XKB_KEY_p 
#define WK_Q XKB_KEY_q 
#define WK_R XKB_KEY_r 
#define WK_S XKB_KEY_s 
#define WK_T XKB_KEY_t 
#define WK_U XKB_KEY_u 
#define WK_V XKB_KEY_v 
#define WK_W XKB_KEY_w 
#define WK_X XKB_KEY_x 
#define WK_Y XKB_KEY_y 
#define WK_Z XKB_KEY_z

#define WK_LEFT XKB_KEY_Left
#define WK_RIGHT XKB_KEY_Right
#define WK_DOWN XKB_KEY_Down
#define WK_UP XKB_KEY_Up

class Winval{
  int w, h;
  
  bool isDown[65536];
  int keysyms;

  bool autoRepeat;
  
  int pointerX, pointerY;
  bool mouseButtonPressed;

  xcb_connection_t* connection;
  xcb_screen_t* screen;
  xcb_window_t window;
  xcb_pixmap_t pmap;
  xcb_gcontext_t graphics_context;
  xcb_format_t* fmt;
  
  xkb_state * kstate;

 public:
  
  Winval(int w, int h);
  Winval();
  ~Winval();

  std::string window_title;

  void handleEventProperly(xcb_generic_event_t *);
  //void handleEventProperly(XEvent& e);

  void flushEvents();

  void getPointerPosition(int* x, int* y);
  bool isMouseButtonPressed();
  bool isKeyPressed(int i);
  int waitForKey();  
  //XEvent getNextEvent();
  void getButtonStateAndMotion(bool& valid, int& x, int& y);
  void waitForButtonPress(int* x, int* y);
  void drawBuffer(char* p, int w, int h);
  void drawBuffer(unsigned char* p, int w, int h);
  void enableAutoRepeat(bool enable);
  const char* getTitle() const;
  void setTitle(const char* window_name);
  int getWidth() const;
  int getHeight() const;
  xcb_window_t getWindow() const;
  xcb_connection_t* getConnection() const;
};

#endif // INCLUDED_WINVAL
