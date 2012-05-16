/*
Allocore Example: FBO

Description:
This demonstrates how to use an FBO

Author:
Graham Wakefield, 2012
*/

#include "allocore/al_Allocore.hpp"

using namespace al;

Graphics gl;
Texture fbotex(512, 512);
RBO rbo;
FBO fbo;

struct MyWindow : Window{

	bool onCreate(){
		rbo.resize(512, 512);
		fbotex.validate();
		
		fbo.attachRBO(rbo, FBO::DEPTH_ATTACHMENT);
		fbo.attachTexture2D(fbotex.id(), FBO::COLOR_ATTACHMENT0);

		return true;
	}

	bool onFrame(){
		
		fbo.begin();
		
			gl.viewport(0, 0, width(), height());
			gl.clearColor(1, 1, 0, 0);
			gl.clear(Graphics::COLOR_BUFFER_BIT | Graphics::DEPTH_BUFFER_BIT);
		
		fbo.end();
		
		gl.viewport(0, 0, width(), height());
		gl.clearColor(0, 1, 1, 0);
		gl.clear(Graphics::COLOR_BUFFER_BIT | Graphics::DEPTH_BUFFER_BIT);
		
		gl.projection(Matrix4d::ortho(0, 1, 1, 0, -1, 1));
		gl.modelView(Matrix4d::identity());
		fbotex.quad(gl);

		return true;
	}
};

MyWindow win;

int main(){

	win.append(*new StandardWindowKeyControls);
	win.create(Window::Dim(800, 600));

	MainLoop::start();
	return 0;
}
