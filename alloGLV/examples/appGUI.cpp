/*
AlloGLV Example: App GUI

Description:
This demonstrates how to add a GUI to an al::App to control some properties of
a sphere.

Author:
Lance Putnam, Nov. 2013, putnam.lance@gmail.com
*/

#include "allocore/io/al_App.hpp"
#include "alloGLV/al_ControlGLV.hpp"
#include "GLV/glv.h"
using namespace al;

class MyApp : public App{
public:
	GLVBinding gui;
	glv::Slider slider;
	glv::ColorPicker colorPicker;
	glv::Slider2D slider2d;

	glv::Table layout;

	Mesh sphere;
	float scaling;
	Vec2f position;

	MyApp(){
	
		sphere.primitive(Graphics::LINE_STRIP);
		addSphere(sphere);
		sphere.color(Color(1));
		scaling = 1;
	
		nav().pos().set(0,0,4);
		initWindow();

		// Connect GUI to window
		gui.bindTo(window());

		// Configure GUI
		gui.style().color.set(glv::Color(0.7), 0.5);

		layout.arrangement(">p");

		slider.setValue(0.5);	
		layout << slider;
		layout << new glv::Label("scale");

		colorPicker.setValue(glv::HSV(0.1));
		layout << colorPicker;
		layout << new glv::Label("color");

		slider2d.interval(-1,1);
		layout << slider2d;
		layout << new glv::Label("position");

		layout.arrange();

		gui << layout;
	}

	void onAnimate(double dt){
		scaling = slider.getValue();

		sphere.colors()[0] = HSV(colorPicker.getValue().components);
		
		position.set(slider2d.getValue(0), slider2d.getValue(1));
	}

	virtual void onDraw(Graphics& g, const Viewpoint& v){
		g.lineWidth(1);
		g.pushMatrix(Graphics::MODELVIEW);
			g.translate(position.x, position.y);
			g.scale(scaling);
			g.draw(sphere);
		g.popMatrix();
	}

};

int main(){
	MyApp().start();
}
