
#include "allocore/io/al_App.hpp"
#include "allocore/graphics/al_Shapes.hpp"
#include "alloGLV/al_ParameterGUI.hpp"
#include "allocore/ui/al_ParameterMIDI.hpp"
#include "allocore/ui/al_HtmlInterfaceServer.hpp"

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
		mParameterGUI << new glv::Label("Parameter GUI example");
		// Add parameters to GUI
		mParameterGUI << x << y << z ;
		nav() = Vec3d(0, 0, 2);
		// Add parameters to OSC server
		mServer << x << y << z;
		mServer.print();

		// Expose parameter server in html interface
		mInterfaceServer << mServer;

		addSphere(graphics().mesh(), 0.1);
		graphics().mesh().generateNormals();

		// Connect MIDI CC #1 on channel 1 to parameter x
		mParameterMIDI.connectControl(x, 1, 1);

		// Broadcast parameter changes to localhost por 9011
		mServer.addListener("127.0.0.1", 9011);

	}

	virtual void onDraw(Graphics &g) {
		light();
		g.pushMatrix();
		g.translate(x.get(),y.get(), z.get());
		g.draw(g.mesh());
		g.popMatrix();
	}

	virtual void onExit() override {
		mServer.stopServer(); // We need to manually stop the server to keep it from crashing.
	}

private:
	ParameterGUI mParameterGUI;
	ParameterServer mServer;
	ParameterMIDI mParameterMIDI;
	HtmlInterfaceServer mInterfaceServer;

	Light light;
};


int main(int argc, char *argv[])
{
	MyApp().start();
}

