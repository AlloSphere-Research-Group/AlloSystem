/*
Allocore Example: Mouse To World Space

Description:
Draws an object, in world space, at the mouse cursor position.

Author:
Lance Putnam, June 2022
*/
#include "allocore/io/al_App.hpp"
using namespace al;

class MyApp : public App{
public:
	Mesh shape;
	Vec3f pos;

	MyApp(){
		addSphere(shape);
		shape.scale(0.2);
		nav().pullBack(4);
		initWindow();
	}

	void onAnimate(double dt) override {
		// Convert pixel coordinate of mouse to world space coordinate
		const auto& mouse = window().mouse();
		pos = stereo().pixelToWorld(mouse.x(), mouse.y());

		/* If you want to learn the math, here it is:
		auto mat = stereo().modelViewProjection();
		Vec3f ndc; // normalized device coordinate
		ndc.x = mouseX1() * 2. - 1.;
		ndc.y = mouseY1() *-2. + 1.;
		ndc.z = mat.at<2,3>() / mat.at<3,3>();
		invert(mat); // MVP^-1 (NDC to world)
		auto pos4 = mat * Vec4f(ndc,1);
		pos = pos4.xyz() / pos4.w;
		//*/
	}

	void onDraw(Graphics& g) override {
		g.wireframe(true);
		g.matrixScope([&](){
			g.translate(pos);
			g.draw(shape);
		});
	}
};

int main(){
	MyApp().start();
}
