/*
	Test input for exCompiler
*/

#include "graphics/al_BufferObject.hpp"
#include "graphics/al_Common.hpp"
#include "graphics/al_Config.h"
#include "graphics/al_Context.hpp"
#include "graphics/al_Debug.hpp"
#include "graphics/al_GPUObject.hpp"
#include "graphics/al_Isosurface.hpp"
#include "graphics/al_Light.hpp"
#include "graphics/al_Shader.hpp"
#include "graphics/al_Texture.hpp"

#include "io/al_AudioIO.hpp"
#include "io/al_File.hpp"
#include "io/al_Socket.hpp"
#include "io/al_WindowGL.hpp"

#include "math/al_Complex.hpp"
#include "math/al_Constants.hpp"
#include "math/al_Frustum.hpp"
#include "math/al_Functions.hpp"
#include "math/al_Generators.hpp"
#include "math/al_Interpolation.hpp"
#include "math/al_Matrix4.hpp"
#include "math/al_Plane.hpp"
#include "math/al_Quat.hpp"
#include "math/al_Random.hpp"
#include "math/al_Vec.hpp"

#include "protocol/al_Graphics.hpp"
#include "protocol/al_GraphicsBackendOpenGL.hpp"
#include "protocol/al_OSC.hpp"
#include "protocol/al_OSCAPR.hpp"
#include "protocol/al_Serialize.hpp"

#include "sound/al_Ambisonics.hpp"
#include "sound/al_AudioScene.hpp"
#include "sound/al_Reverb.hpp"

#include "spatial/al_Camera.hpp"
#include "spatial/al_CoordinateFrame.hpp"

#include "system/al_Config.h"
#include "system/al_MainLoop.h"
#include "system/al_MainLoop.hpp"
#include "system/al_Printing.hpp"
#include "system/al_Thread.hpp"
#include "system/al_Time.h"
#include "system/al_Time.hpp"

#include "types/al_Buffer.hpp"
#include "types/al_Color.hpp"
#include "types/al_Conversion.hpp"
#include "types/al_MsgQueue.hpp"
#include "types/al_MsgTube.hpp"
#include "types/al_types.h"
#include "types/al_types.hpp"


using namespace al;

gfx::GraphicsBackendOpenGL backend;
gfx::Graphics gl(&backend);

const int NUM_VERTICES = 100;

struct MyWindow : WindowGL{

	void onCreate(){ 					printf("onCreate\n"); }
	void onDestroy(){					printf("onDestroy\n"); }
	void onResize(int w, int h){		printf("onResize     %d, %d\n", w, h); }
	void onVisibility(bool v){			printf("onVisibility %s\n", v?"true":"false"); }
	
	void onKeyDown(const Keyboard& k){	printf("onKeyDown    "); printKey(); 
		switch(k.key()){
			case Key::Escape: fullScreenToggle(); break;
			case 'q': if(k.ctrl()) WindowGL::stopLoop(); break;
			case 'w': if(k.ctrl()) destroy(); break;
			case 'h': if(k.ctrl()) hide(); break;
			case 'm': if(k.ctrl()) iconify(); break;
			case 'c': if(k.ctrl()) cursorHideToggle(); break;
		}
	}
	void onKeyUp(const Keyboard& k){	printf("onKeyUp      "); printKey(); }
	
	void onMouseDown(const Mouse& m){	printf("onMouseDown  "); printMouse(); }
	void onMouseUp(const Mouse& m){		printf("onMouseUp    "); printMouse(); }
	void onMouseDrag(const Mouse& m){	printf("onMouseDrag  "); printMouse(); }
	//void onMouseMove(const Mouse& m){	printf("onMouseMove  "); printMouse(); }
	
	void printMouse(){
		const Mouse& m = mouse();
		printf("x:%4d y:%4d b:%d,%d\n", m.x(), m.y(), m.button(), m.down());
	}

	void printKey(){
		const Keyboard& k = keyboard();
		printf("k:%3d, %d s:%d c:%d a:%d\n", k.key(), k.down(), k.shift(), k.ctrl(), k.alt());
	}

	void onFrame(){
		gl.begin(gfx::LINES);
			for (int i=0; i< NUM_VERTICES; i++) {
				vertex& v = vertices[i];
				gl.color(v.r, v.g, v.b, 0.5);
				gl.vertex(v.x, v.y, v.z);
			}
			
			gl.color(1, 0, 0.5, 1);
			gl.vertex(-1, 0, 0);
			
			gl.color(1, 0, 0.5, 1);
			gl.vertex(1, 0, 0);
		gl.end();

	}
	
	void freqs(float v1, float v2){ freq1=v1; freq2=v2; }
	float freq1, freq2;
};


int main(){

	MyWindow win;
	MyWindow win2;
	
//	gl.setBackend(GraphicsBackend::None);
	
//	printf("setBackendOpenGL %d\n", setBackendOpenGL(&gl));

//	struct Func:TimedFunction{
//		void onExecute(){ printf("hello\n"); }
//	};
//
//	Func tf;
//	tf(1000);

	win.create(WindowGL::Dim(200,200,100), "Window 1", 40);
	win2.create(WindowGL::Dim(200,200,300), "Window 2", 40);
//	win2.create(WindowGL::Dim(200,200,300), "Window 2", 40, SingleBuf);

	win.freqs(1,2);
	win2.freqs(3,4);

//win.cursorHide(true);

	WindowGL::startLoop();
	return 0;
}