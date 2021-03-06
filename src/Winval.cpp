#include <Winval.hpp>

#include <iostream>

#ifdef WIN32
Winval::Winval(){}

Winval::Winval(int w, int h, bool fullscreen){
    hinstance = GetModuleHandle(0);

    wc = WNDCLASSEX{};

    LPCSTR AppTitle = "Winval";
    wc.cbSize = sizeof(wc);
    wc.style= CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc=WindowProc;
    wc.cbClsExtra=0;
    wc.cbWndExtra=0;
    wc.hInstance=hinstance;
    wc.hIcon=LoadIcon(NULL,IDI_WINLOGO);
    wc.hCursor=LoadCursor(NULL,IDC_ARROW);
    wc.hbrBackground=(HBRUSH)COLOR_WINDOWFRAME;
    wc.lpszMenuName=NULL;
    wc.lpszClassName=AppTitle;

    if (!RegisterClassEx(&wc)) {
        std::cout << "Could not register Winval class" << std::endl;
        exit(0);
    }
    hwnd = CreateWindow(AppTitle,AppTitle,
                        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                        CW_USEDEFAULT,CW_USEDEFAULT,w,h,
                        NULL,NULL,hinstance,NULL);
    width = w;
    height = h;

    windowOpen = true;

    if (!hwnd) {
        std::cout << "Could not create window" << std::endl;
        exit(0);
    }

    pixelData = new COLORREF[w*h];

    hdc = GetDC(hwnd);

    if (pixelData == 0) {
        std::cout << "Pixel data could not be allocated" << std::endl;
        exit(0);
    }

    if (fullscreen) {
        DEVMODE newSettings;
        EnumDisplaySettings(0, 0, &newSettings);
        long result = ChangeDisplaySettings(&newSettings, CDS_FULLSCREEN);
    }

    for (int i = 0; i < this->_numKeys; i++) {
        isDown[i] = false;
    }

    for (int i = 0; i < 6; i++) {
        this->mouseButtonPressed[i] = false;
    }

    lockedPointer = false;
    autoRepeat = false;

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
    if(lockedPointer){
        POINT p = {lockedPointerX, lockedPointerY};
        ClientToScreen(hwnd, &p);
        SetCursorPos(p.x, p.y);
    }

    while(PeekMessage(&msg, hwnd, 0, 0,PM_NOREMOVE)){
        GetMessage(&msg, hwnd, 0, 0);
        handleEventProperly(msg);
    }
}

void Winval::getPointerPosition(int* x, int* y) const {
    *x = pointerX; *y = pointerY;
}

bool Winval::isMouseButtonPressed() const {
    return mouseButtonPressed;
}

bool Winval::isKeyPressed(int i) const {
    return isDown[i];
}

void Winval::setTitle(const char* window_name) {
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

    DeleteObject(bitmap);
}

bool Winval::isOpen(){
    flushEvents();
    return windowOpen;
}

void Winval::enableAutoRepeat(bool en){
    autoRepeat = en;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam){
    return DefWindowProc(hwnd, msg, wparam, lparam);
}

HINSTANCE Winval::getInstance() const {
    return hinstance;
}

HWND Winval::getHWND() const {
    return hwnd;
}


winval_type_0 Winval::getWinProp0() const {
    return this->getInstance();
}
winval_type_1 Winval::getWinProp1() const {
    return this->getHWND();
}

const char* Winval::getTitle() const{
    return window_title.c_str();
}

int Winval::getWidth() const {
    return width;
}

int Winval::getHeight() const {
    return height;
}

void Winval::sleepMilliseconds(int u){
    Sleep(u);
}

void Winval::setPointerVisible(bool visible){
    ShowCursor(visible);
}

void Winval::lockPointer(bool lock, int x, int y){
    lockedPointer = lock;
    lockedPointerX = x;
    lockedPointerY = y;

    if(lockedPointer){
        POINT p = {x, y};
        ClientToScreen(hwnd, &p);
        SetCursorPos(p.x, p.y);
    }
}

#else //WIN32

#define WINVAL_KEYMAP_OFFSET 8
Winval::Winval(){

}

Winval::Winval(int w, int h, bool fullscreen){
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

    if(fullscreen){
        Atom atoms[2] = { XInternAtom(dsp, "_NET_WM_STATE_FULLSCREEN", False), None };
        XChangeProperty(
            dsp,
            win,
            XInternAtom(dsp, "_NET_WM_STATE", False),
            XA_ATOM, 32, PropModeReplace, (unsigned char*)atoms, 1
            );
    } else {
        // Ensure window becomes floating by default in tiling window managers
        Atom atoms[2] = { XInternAtom(dsp, "_NET_WM_WINDOW_TYPE_SPLASH", False), None };
        XChangeProperty(dsp,
                        win,
                        XInternAtom(dsp, "_NET_WM_WINDOW_TYPE", False),
                        XA_ATOM, 32, PropModeReplace, (unsigned char*) atoms, 1);
    }



    long eventMask = StructureNotifyMask | ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask;
    XSelectInput(dsp, win, eventMask);

    XMapWindow(dsp, win);

    XEvent e;

    ks = XGetKeyboardMapping(dsp, WINVAL_KEYMAP_OFFSET, 256 - WINVAL_KEYMAP_OFFSET, &keysyms);
    for(int i = 0; i < _numKeys; i++){
        isDown[i] = false;
    }

    for (int i = 0; i < 6; i++) {
        this->mouseButtonsPressed[i] = 0;
    }

    pointerX = pointerY = 0;

    do{
        XNextEvent(dsp, &e);
    }while(e.type != MapNotify);

    gc = XCreateGC(dsp, win,
                   0,
                   NULL );
    image = 0;

    autoRepeat = false;
    lockedPointer = false;

    width = w;
    height = h;

    wm_delete_window = XInternAtom(dsp, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(dsp, win, &wm_delete_window, 1);
}

Winval::~Winval(){
// flushEvents();
    if(!dsp)
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
            /*if(!autoRepeat){
              printf("Not autorepeating\n");
              if(XEventsQueued(dsp, QueuedAfterFlush)){
              //if(XQLength(dsp)){
              printf("More events were queued\n");
              XEvent nextEvent;
              XPeekEvent(dsp, &nextEvent);
              if(nextEvent.type == KeyPress && nextEvent.xkey.keycode == e.xkey.keycode &&
              e.xkey.time == nextEvent.xkey.time){
              printf("Next event was corresponding press\n");
              XNextEvent(dsp, &nextEvent);
              pressDown = true;
              }
              }
              }*/
            isDown[key] = pressDown;
        }
        break;
    case MotionNotify:
        pointerX = e.xmotion.x, pointerY = e.xmotion.y;
        break;
    case ButtonRelease:
        if (e.xbutton.button >= 4) {
            break; // Don't release scrolls
        }
        mouseButtonsPressed[e.xbutton.button] = 0;
        break;
    case ButtonPress:
        mouseButtonsPressed[e.xbutton.button]++;
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

void Winval::getPointerPosition(int* x, int*y) const {
    *x = pointerX; *y = pointerY;
}

bool Winval::isMouseButtonPressed() const {
    return mouseButtonsPressed[1];
}

int Winval::getScroll() const {
    return - mouseButtonsPressed[4] + mouseButtonsPressed[5];
}

bool Winval::isKeyPressed(int i) const {
    return isDown[i];
}

void Winval::resetBeforeFlush() {

    this->mouseButtonsPressed[4] = 0;
    this->mouseButtonsPressed[5] = 0;

    // Safe?
    if (lockedPointer) {
        XWarpPointer(dsp, None, win, 0, 0, 0, 0, lockedPointerX, lockedPointerY);
    }
}

void Winval::flushEvents(){

    resetBeforeFlush();

    int num = XEventsQueued(dsp, QueuedAfterFlush);
    XEvent e2;
    while(num--){
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

void Winval::drawBuffer(unsigned char* buffer, int w, int h){
    if(!isOpen())
        return;
    XImage* im = XCreateImage(dsp,XDefaultVisual(dsp, screenNum), 24, ZPixmap, 0, (char*)buffer, w, h, 32, 0);
    XPutImage(dsp, win, gc, im, 0, 0, 0, 0, w, h);
    XFlush(dsp);
    //XDestroyImage(im);
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

winval_type_0 Winval::getWinProp0() const {
    return getWindow();
}

winval_type_1 Winval::getWinProp1() const {
    return getDisplay();
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

void Winval::setPointerVisible(bool visible){
    if(visible){
        XUndefineCursor(dsp, win);
    }else{
        const char invisibleData[] = {0, 0, 0, 0, 0, 0, 0, 0};
        XColor col;

        Pixmap bitmap = XCreateBitmapFromData(dsp, win, invisibleData, 8, 8);

        Cursor c = XCreatePixmapCursor(dsp,bitmap, bitmap,&col,&col, 0, 0);
        XDefineCursor(dsp, win, c);

        XFreeCursor(dsp, c);
        XFreePixmap(dsp,bitmap);
    }
}

void Winval::lockPointer(bool lock, int x, int y){
    lockedPointer = lock;
    lockedPointerX = x;
    lockedPointerY = y;

    if(lockedPointer){
        XWarpPointer(dsp, None, win, 0, 0, 0, 0, lockedPointerX, lockedPointerY);
    }
}

#include <unistd.h>
void Winval::sleepMilliseconds(int u){
    usleep(u * 1000);
}

#endif //WIN32

// Cross-platform code

bool Winval::isPointerLocked(){
    return lockedPointer;
}

void Winval::getPointerLockPosition(int* x, int* y){
    *x = lockedPointerX;
    *y = lockedPointerY;
}
