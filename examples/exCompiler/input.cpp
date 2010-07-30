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

const int N = 32;
float volData[N*N*N];
Isosurface<float> iso;
double phase=0;

struct MyWindow : WindowGL{
	
	void onKeyDown(const Keyboard& k){
		switch(k.key()){
			case Key::Escape: fullScreenToggle(); break;
			case 'q': if(k.ctrl()) WindowGL::stopLoop(); break;
		}
	}

	void onFrame(){
		gl.clear(gfx::AttributeBit::ColorBuffer | gfx::AttributeBit::DepthBuffer);
		gl.loadIdentity();
		gl.viewport(0,0, dimensions().w, dimensions().h);

		if((phase += 0.001) > 2*M_PI) phase -= 2*M_PI;

		for(int k=0; k<N; ++k){
		for(int j=0; j<N; ++j){
		for(int i=0; i<N; ++i){
			double x = double(i)/N * 4*M_PI;
			double y = double(j)/N * 4*M_PI;
			double z = double(k)/N * 4*M_PI;
			volData[k*N*N + j*N + i] 
				= cos(x * cos(phase*7)) 
				+ cos(y * cos(phase*8)) 
				+ cos(z * cos(phase*9));
		}
		}
		}

		iso.primitive(gfx::TRIANGLES);
		iso.level(0);
		iso.resetBuffers();
		iso.generate(volData, N, 1./N);
		gl.draw(iso);
	}
};


int main (int argc, char * argv[]){

	MyWindow win;
	win.create(WindowGL::Dim(800,600), "Isosurface Example", 40);

	WindowGL::startLoop();
	return 0;
}