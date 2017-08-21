#include <Winval.h>

#ifdef WIN32
Winval::Winval(int w, int h){
  const char* AppTitle = "Winval";
  wc.style=CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc=WindowProc;
  wc.cbClsExtra=0;
  wc.cbWndExtra=0;
  wc.hInstance=GetModuleHandle(0);
  wc.hIcon=LoadIcon(NULL,IDI_WINLOGO);
  wc.hCursor=LoadCursor(NULL,IDC_ARROW);
  wc.hbrBackground=(HBRUSH)COLOR_WINDOWFRAME;
  wc.lpszMenuName=NULL;
  wc.lpszClassName=AppTitle;

  if (!RegisterClass(&wc))
    exit(0);
  hwnd = CreateWindow(AppTitle,AppTitle,
    WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT,CW_USEDEFAULT,w,h,
    NULL,NULL,0,NULL);
    width = w;
    height = h;

  windowOpen = true;
  if (!hwnd)
    exit(0);
  /*std::cout<<"Starting to fill out BMI"<<std::endl;
  memset(&bmi, 0, sizeof(bmi));
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth = w;
  bmi.bmiHeader.biHeight = h;
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 32;
  bmi.bmiHeader.biCompression = BI_RGB;
  HDC hDesktopDC = GetDC(GetDesktopWindow());
  HBITMAP hDib = CreateDIBSection(hDesktopDC, &bmi, DIB_RGB_COLORS, (void**)&pixelData, 0,0);*/
  pixelData = new COLORREF[w*h];
  hdc = GetDC(hwnd);
  if(pixelData == 0)
    exit(0);

  /*hDibDC = CreateCompatibleDC(hDesktopDC);
  HGDIOBJ hOldObj = SelectObject(hDibDC, hDib);
  if(pointer != 0)
    memcpy(((void*)pixelData), *pointer, w*h*4);
  ReleaseDC(GetDesktopWindow(), hDesktopDC);*/

  ShowWindow(hwnd,10);
  UpdateWindow(hwnd);
}

Winval::~Winval(){
  //I don't know the Windows API that well, but...
}

void Winval::handleEventProperly(MSG& msg){
  TranslateMessage(&msg);
  DispatchMessage(&msg);
  switch(msg.message){
    case WM_PAINT: {
      HBITMAP bitmap = CreateBitmap(width, height, 1, 8*4, (void*)pixelData);
      HDC src = CreateCompatibleDC(hdc);
      SelectObject(src, bitmap);

      BitBlt(hdc, 0, 0, width, height, src, 0, 0, SRCCOPY);
      DeleteDC(src);
      /*PAINTSTRUCT paint;
      HDC hWndDC = BeginPaint(hwnd, &paint);
      BitBlt(hWndDC, 0, 0, w, h, hDibDC, 0, 0, SRCCOPY);
      EndPaint(hwnd, &paint);*/
      break;
    }
    case WM_LBUTTONDOWN: {
      mouseButtonPressed = true;
      break;
    }
    case WM_LBUTTONUP: {
      mouseButtonPressed = false;
      break;
    }
    case WM_KEYDOWN: {
      isDown[msg.wParam] = true;
      break;
    }
    case WM_KEYUP: {
      isDown[msg.wParam] = false;
      break;
    }
    case WM_MOUSEMOVE: {
      pointerX = LOWORD(msg.lParam), pointerY = HIWORD(msg.lParam);
      break;
    }
    case WM_DESTROY: {
      PostQuitMessage(0);
      windowOpen = false;
      break;
    }
  }
}

void Winval::flushEvents(){
  while(PeekMessage(&msg, hwnd, 0, 0,PM_NOREMOVE)){
    GetMessage(&msg, hwnd, 0, 0);
    handleEventProperly(msg);
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

void Winval::setTitle(const char* window_name){
  SetWindowText(hwnd, window_name);
  window_title = window_name;
}

int Winval::waitForKey(){
  while(GetMessage(&msg, hwnd,0, 0)){
    handleEventProperly(msg);
    if(msg.message == WM_KEYDOWN)
      break;
  }

  return msg.wParam;
}

void Winval::getButtonStateAndMotion(bool& valid, int& x, int& y){
  flushEvents();
  valid = mouseButtonPressed;
  x = pointerX;
  y = pointerY;

  return;
}
void Winval::waitForButtonPress(int& x, int& y){
  while(GetMessage(&msg, hwnd,0, 0)){
    handleEventProperly(msg);
    if(msg.message == WM_LBUTTONDOWN){
      break;
    }
  }
}

void Winval::drawBuffer(unsigned char* p, int w, int h){
  memcpy((void*)pixelData, (void*)p, w*h*4);

  HBITMAP bitmap = CreateBitmap(w, h, 1, 8*4, (void*)pixelData);
  HDC src = CreateCompatibleDC(hdc);
  SelectObject(src, bitmap);

  BitBlt(hdc, 0, 0, w, h, src, 0, 0, SRCCOPY);
  DeleteDC(src);

  /*PAINTSTRUCT paint;
  HDC hWndDC = BeginPaint(hwnd, &paint);
  BitBlt(hWndDC, 0, 0, w, h, hDibDC, 0, 0, SRCCOPY);
  EndPaint(hwnd, &paint);*/
}

bool Winval::isOpen(){
  flushEvents();
  return windowOpen;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam){
  return DefWindowProc(hwnd, msg, wparam, lparam);
}

#else //WIN32

#define WINVAL_KEYMAP_OFFSET 8
Winval::Winval(){

}

Winval::Winval(int w, int h){
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

  autoRepeat = false;

  width = w;
  height = h;

  wm_delete_window = XInternAtom(dsp, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(dsp, win, &wm_delete_window, 1);
}

Winval::~Winval(){
  flushEvents();
  
  if(!isOpen())
    return;
  
  if(image)
    XDestroyImage(image);
  if(win)
    XDestroyWindow(dsp, win);
}

int Winval::getKeySym(int keyCode){
  int index = keyCode - WINVAL_KEYMAP_OFFSET;
  int key = ks[index*keysyms];
  return key < 1<<16 ? key : 0;
}

int Winval::waitForKey(){
  if(!isOpen())
    return 0;
  
  XEvent e;
  do{
    XNextEvent(dsp, &e);
    handleEventProperly(e);
  }while(e.type != KeyPress && isOpen());
  
  return getKeySym(e.xkey.keycode);
}

void Winval::waitForButtonPress(int& x, int& y){
  if(!isOpen())
    return;
  
  XEvent e;
  do {
    XNextEvent(dsp, &e);
    handleEventProperly(e);
  }while(e.type != ButtonPress && isOpen());

  x = e.xbutton.x, y = e.xbutton.y;
}

void Winval::handleEventProperly(XEvent& e){
  int index;
  int key;
  switch(e.type){
  case Expose:
    if(!image)
      break;
    XPutImage(dsp, win, gc, image, 0, 0, 0, 0, width, height);
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

  case MappingNotify:
    ks = XGetKeyboardMapping(dsp, WINVAL_KEYMAP_OFFSET, 256 - WINVAL_KEYMAP_OFFSET, &keysyms);
    break;

  case ClientMessage:
    if((Atom)e.xclient.data.l[0] == wm_delete_window){
      XCloseDisplay(dsp);
      dsp = 0;
    }
    break;
  }
}

void Winval::getPointerPosition(int* x, int*y){
  *x = pointerX; *y = pointerY;
}

bool Winval::isMouseButtonPressed(){
  return mouseButtonPressed;
}

bool Winval::isKeyPressed(int i){
  return isDown[i];
}

void Winval::flushEvents(){
  if(!isOpen())
    return;
  
  int num = XEventsQueued(dsp, QueuedAfterFlush);
  XEvent e2;
  while(num-- && isOpen()){
    XNextEvent(dsp, &e2);

    handleEventProperly(e2);
  }
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
  }while(e.type != MotionNotify && isOpen());

  x = e.xmotion.x, y = e.xmotion.y;
  valid = true;
  return;
}

void Winval::drawBuffer(char* buffer, int w, int h){
  if(!isOpen())
    return;
  XImage* im = XCreateImage(dsp,XDefaultVisual(dsp, screenNum), 24, ZPixmap, 0, buffer, w, h, 32, 0);
  XPutImage(dsp, win, gc, im, 0, 0, 0, 0, w, h);
  XFlush(dsp);
  //XDestroyImage(im);
}

void Winval::drawBuffer(unsigned char* buffer, int w, int h){
  drawBuffer((char*)buffer, w, h);
}

void Winval::setTitle(const char* window_name){
  if(!isOpen())
    return;
  XStoreName(dsp, win, window_name);
  window_title = window_name;
}

void Winval::enableAutoRepeat(bool enable){
  autoRepeat = enable;
}

const char* Winval::getTitle() const{
  return window_title.c_str();
}

Window Winval::getWindow() const{
  return win;
}

Display* Winval::getDisplay() const{
  return dsp;
}

int Winval::getWidth() const {
  return width;
}

int Winval::getHeight() const {
  return height;
}

bool Winval::isOpen() {
  flushEvents();
  return dsp;
}

#endif //WIN32
