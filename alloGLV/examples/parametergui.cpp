
#include "allocore/io/al_App.hpp"
#include "allocore/graphics/al_Shapes.hpp"
#include "alloGLV/al_ParameterGUI.hpp"

using namespace al;


Parameter x("X", "",0, "", -2.0,2.0);
Parameter y("Y", "",0, "", -2.0,2.0);
Parameter z("Z", "",0, "", -2.0,2.0);

class MyApp : public App
{
public:
	MyApp()
	{
		initWindow();
		mParameterGUI.setParentApp(this);
		mParameterGUI << x << y << z ;
		nav() = Vec3d(0, 0, 2);
		mServer << x << y << z;
		mServer.print();

		addSphere(graphics().mesh(), 0.1);
		graphics().mesh().generateNormals();
	}

	virtual void onDraw(Graphics &g) {
		light();
		g.pushMatrix();
		g.translate(x.get(),y.get(), z.get());
		g.draw(g.mesh());
		g.popMatrix();
	}


private:
	ParameterGUI mParameterGUI;
	ParameterServer mServer;
	Light light;
};


int main(int argc, char *argv[])
{
	MyApp().start();
}

