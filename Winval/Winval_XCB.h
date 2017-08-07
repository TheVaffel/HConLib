#ifndef INCLUDED_WINVAL
#define INCLUDED_WINVAL

#include <xcb/xcb.h>
#include <xcb/xcb_image.h>
#include <xkbcommon/xkbcommon-x11.h>

#include <string> //std::string
#include <string.h> //strlen

#ifdef WINVAL_VULKAN
class Winval;
#include "Wingine.h"
#endif //WINVAL_VULKAN

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

    
#ifdef WINVAL_VULKAN
  Wingine wingine;
#endif
  
 public:
  Winval(int w, int h, char** pointer);
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
	const char* getTitle();
  void setTitle(const char* window_name);
  int getWidth();
  int getHeight();
  xcb_window_t getWindow();
  xcb_connection_t* getConnection();
};

#endif // INCLUDED_WINVAL

#ifdef WINVAL_IMPLEMENTATION

#ifdef WINVAL_VULKAN

#define WINVAL_VULKAN_IMPLEMENTATION
#include "Wingine.h"

#endif //WINVAL_VULKAN


#define WINVAL_KEYMAP_OFFSET 8
Winval::Winval(){

}

Winval::Winval(int width, int height, char** p = 0){
  int screens;
  xcb_screen_iterator_t iter;
  connection = xcb_connect(NULL, &screens);
  const xcb_setup_t *setup = xcb_get_setup(connection);
  iter = xcb_setup_roots_iterator(setup);
  while(screens-- > 0) xcb_screen_next(&iter);
  
  screen = iter.data;

  window = xcb_generate_id(connection);

  uint32_t mask, values[2];
  mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
  values[0] = screen->black_pixel;
  values[1] = XCB_EVENT_MASK_EXPOSURE       | XCB_EVENT_MASK_BUTTON_PRESS   |
              XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_POINTER_MOTION |
              XCB_EVENT_MASK_KEY_PRESS      | XCB_EVENT_MASK_KEY_RELEASE;

  xcb_create_window(connection,
		    XCB_COPY_FROM_PARENT,
		    window,
		    screen->root,
		    50, 50,
		    width, height,
		    1,
		    XCB_WINDOW_CLASS_INPUT_OUTPUT,
		    screen->root_visual,
		    mask, values);

  w = width;
  h = height;
  
  /* Magic code that will send notification when window is destroyed */
  xcb_intern_atom_cookie_t cookie = xcb_intern_atom(connection, 1, 12, "WM_PROTOCOLS");
  xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(connection, cookie, 0);

  xcb_intern_atom_cookie_t cookie2 = xcb_intern_atom(connection, 0, 16, "WM_DELETE_WINDOW");
  xcb_intern_atom_reply_t* atom_wm_delete_window = xcb_intern_atom_reply(connection, cookie2, 0);

  xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window, (*reply).atom, 4, 32, 1,
		      &(*atom_wm_delete_window).atom);
  free(reply);
  
  xcb_map_window(connection, window);

  
#ifndef WINVAL_VULKAN
  pmap = xcb_generate_id(connection);
  xcb_create_pixmap(connection, 24, pmap, window, w, h);
  
  mask = XCB_GC_FOREGROUND | XCB_GC_BACKGROUND;
  values[0] = screen->black_pixel;
  values[1] = 0xFFFFFFFF;
  graphics_context = xcb_generate_id(connection);
  xcb_create_gc(connection, graphics_context, pmap, XCB_GC_FOREGROUND | XCB_GC_BACKGROUND,
		values);
  
  xcb_copy_area(connection, pmap, window, graphics_context,
		0, 0, 0, 0,
		w, h);
  xcb_flush(connection);

  
  fmt = xcb_setup_pixmap_formats(setup);
  xcb_format_t *fmtend = fmt + xcb_setup_pixmap_formats_length(setup);
  for(; fmt != fmtend; ++fmt)
    if((fmt->depth == 24) && (fmt->bits_per_pixel == 32)) {
      //printf("fmt %p has pad %d depth %d, bpp %d\n",
      //fmt,fmt->scanline_pad, depth,bpp);
      break;
    }
#endif

  xkb_context * xbcontext;
  xbcontext = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
  if(!xbcontext) printf("Could not create xkb_context\n");
  xkb_x11_setup_xkb_extension(connection,
			      XKB_X11_MIN_MAJOR_XKB_VERSION,
			      XKB_X11_MIN_MINOR_XKB_VERSION,
			      XKB_X11_SETUP_XKB_EXTENSION_NO_FLAGS,
			      NULL,
			      NULL,
			      NULL,
			      NULL);

  xkb_keymap* keymap;

  int keyboard_id = xkb_x11_get_core_keyboard_device_id(connection);

  if(keyboard_id == -1) printf("Could not get xkb_keyboard id\n");

  keymap = xkb_x11_keymap_new_from_device(xbcontext, connection, keyboard_id,
					  XKB_KEYMAP_COMPILE_NO_FLAGS);
  if(!keymap) printf("Could not get xkb_keyboard map\n");

  kstate = xkb_x11_state_new_from_device(keymap, connection, keyboard_id);

  if(!kstate) printf("Could not initialize xkb_state\n");

  

#ifdef WINVAL_VULKAN
  wingine.init_vulkan(this);
#endif
}

Winval::~Winval(){
  xcb_free_pixmap(connection, pmap);
  xcb_disconnect(connection);

}

int Winval::waitForKey(){
  xcb_generic_event_t* event;
  do{
    event = xcb_wait_for_event(connection);
    handleEventProperly(event);
   }while(event->response_type != XCB_KEY_PRESS);
  
  return xkb_state_key_get_one_sym(kstate, ((xcb_key_press_event_t*)event)->detail);
}

void Winval::waitForButtonPress(int* x, int* y){
  xcb_generic_event_t* event;
  do{
    event = xcb_wait_for_event(connection);
    handleEventProperly(event);
  }while(event->response_type != XCB_BUTTON_PRESS);

  *x = ((xcb_button_press_event_t*)event)->event_x;
  *y = ((xcb_button_press_event_t*)event)->event_y;
}

void Winval::handleEventProperly(xcb_generic_event_t* event){
  xcb_expose_event_t* ee;
  switch(event->response_type){
  case XCB_EXPOSE:
    ee = (xcb_expose_event_t*)event;
    xcb_copy_area(connection, pmap, window,  graphics_context,
		  ee->x, ee->y, ee->x, ee->y, ee->width, ee->height);
    xcb_flush(connection);
    break;
  case XCB_KEY_PRESS:

    xcb_keysym_t keysym;
    xcb_keycode_t keycode;
    keycode = ((xcb_key_press_event_t*)event)->detail;
    keysym = xkb_state_key_get_one_sym(kstate, keycode);
    //printf("Pressed key %x!\n", keysym);
    isDown[keysym] = true;
    break;

  case XCB_KEY_RELEASE:
    keycode = ((xcb_key_press_event_t*)event)->detail;
    keysym = xkb_state_key_get_one_sym(kstate, keycode);
    //printf("Pressed key %x!\n", keysym);
    isDown[keysym] = false;
    break;
  case XCB_BUTTON_PRESS:
    //printf("Pressed some button!\n");
    mouseButtonPressed = true;
    break;

  case XCB_BUTTON_RELEASE:
    mouseButtonPressed = false;
    break;

  case XCB_MOTION_NOTIFY:
    pointerX = ((xcb_motion_notify_event_t*)event)->event_x;
    pointerY = ((xcb_motion_notify_event_t*)event)->event_y;
    break;
  }
}

void Winval::getPointerPosition(int* x, int* y){
  *x = pointerX; *y = pointerY;
}

bool Winval::isMouseButtonPressed(){
  return mouseButtonPressed;
}

bool Winval::isKeyPressed(int i){
  return isDown[i];
}

void Winval::flushEvents(){
  xcb_generic_event_t* event;
  while((event = xcb_poll_for_event(connection))){
    handleEventProperly(event);
  }
}


void Winval::drawBuffer(char* buffer, int w, int h){
  xcb_image_t * image = xcb_image_create(w, h,
					 XCB_IMAGE_FORMAT_Z_PIXMAP,
					 fmt->scanline_pad,
					 fmt->depth,
					 fmt->bits_per_pixel,
					 0,
					 (xcb_image_order_t)xcb_get_setup(connection)->image_byte_order,
					 XCB_IMAGE_ORDER_LSB_FIRST,
					 (unsigned char*)buffer,
					 w*h*4,
					 (unsigned char*)buffer);


  
  if(image == NULL){
    printf("Cannot create image\n");
  }
  xcb_image_put(connection, pmap, graphics_context, image, 0, 0, 0);
  xcb_copy_area(connection, pmap, window, graphics_context, 0, 0, 0, 0, w, h);
  
  xcb_flush(connection);
  
  image->base = 0; //Ensure buffer is not deleted
  xcb_image_destroy(image);

}
void Winval::drawBuffer(unsigned char* buffer, int w, int h){
  drawBuffer((char*)buffer, w, h);
}


void Winval::setTitle(const char* window_name){
  xcb_change_property(connection, 
		      XCB_PROP_MODE_REPLACE, 
		      window, 
		      XCB_ATOM_WM_NAME, 
		      XCB_ATOM_STRING, 
		      8, 
		      strlen(window_name), 
		      window_name); 
  xcb_flush(connection); 
  window_title = window_name;
}

void Winval::enableAutoRepeat(bool enable){
  autoRepeat = enable;
}

const char* Winval::getTitle(){
  return window_title.c_str();
}

int Winval::getWidth(){
  return w;
}

int Winval::getHeight(){
  return h;
}

xcb_window_t Winval::getWindow(){
  return window;
}

xcb_connection_t* Winval::getConnection(){
  return connection;
}

#endif // WINVAL_IMPLEMENTATION


