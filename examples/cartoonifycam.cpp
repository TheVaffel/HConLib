#define WINVAL_IMPLEMENTATION
#include <Winval/Winval.h>
#include <cartoonify.h>

#include <webcam/webcam.h>

int main(){
	const int w = 1280, h = 720;
	webcam_init(w, h, 1, WEBCAM_MODE_BAYER);

	Winval win(w, h);

	unsigned char* buffer = new unsigned char[w*h*4];
	unsigned char* cartoon = new unsigned char[w*h*4];

	while(true){
		webcam_capture_image(buffer);
		cartoonify(buffer, cartoon, w, h);
		win.drawBuffer(cartoon, w, h);
		win.flushEvents();
		if(win.isKeyPressed(WK_ESC)){
			break;
		}
	}
	webcam_close();
}

