#include <Winval.h>
#include "cartoonify.h"

#include <HCam.h>

int main(){
	const int w = 640, h = 480;
	HCam cam(w, h, "/dev/video1");

	Winval win(w, h);

	unsigned char* buffer = new unsigned char[w*h*4];
	unsigned char* cartoon = new unsigned char[w*h*4];

	while(win.isOpen()){
		cam.capture_image(buffer);
		cartoonify(buffer, cartoon, w, h);
		win.drawBuffer(cartoon, w, h);
		win.flushEvents();
		if(win.isKeyPressed(WK_ESC)){
			break;
		}
	}
}
