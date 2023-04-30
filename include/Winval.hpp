#ifndef INCLUDED_WINVAL
#define INCLUDED_WINVAL

#include <string> //std::string

#include "./WinvalKeys.h"

#ifdef WIN32
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

typedef HINSTANCE winval_type_0;
typedef HWND winval_type_1;
#else //WIN32

#include "X11/Xlib.h"
#include "X11/Xutil.h"
#include "X11/Xatom.h"

typedef Window winval_type_0;
typedef Display* winval_type_1;
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

    static const int _numKeys = 1 << 16;
    bool isDown[_numKeys];
    int mouseButtonsPressed[6];

    bool autoRepeat;

    int pointerX, pointerY;

    bool lockedPointer;
    int lockedPointerX, lockedPointerY;

    void resetBeforeFlush();

public:
    Winval(int w, int h, bool fullscreen = false);
    Winval();
    ~Winval();

    std::string window_title;

    void flushEvents();

    void getPointerPosition(int* x, int* y) const;
    bool isMouseButtonPressed() const;
    int getScroll() const;
    bool isKeyPressed(int i) const;
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
    typedef HINSTANCE winval_type_0;
    typedef HWND winval_type_1;

    HINSTANCE getInstance() const;
    HWND getHWND() const;

    winval_type_0 getWinProp0() const;
    winval_type_1 getWinProp1() const;
#else //WIN32
    typedef Window winval_type_0;
    typedef Display* winval_type_1;

    Window getWindow() const;
    Display* getDisplay() const;

    winval_type_0 getWinProp0() const;
    winval_type_1 getWinProp1() const;
#endif //WIN32

};


#endif // INCLUDED_WINVAL
