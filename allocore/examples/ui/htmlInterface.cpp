

#include "allocore/ui/al_HtmlInterfaceServer.hpp"
#include "allocore/ui/al_Parameter.hpp"
#include "allocore/io/al_App.hpp"

using namespace al;


// The parameters
Parameter X("X", "Pos", 0, "", -2, 2);
Parameter Y("Y", "Pos", 0, "", -2, 2);
Parameter Brightness("Brightness", "Pos", 0.5, "", 0, 1);

HtmlInterfaceServer htmlServer;
ParameterServer parameterServer;
PresetHandler presetHandler;
// If we pass the parameter server to the preset server constructor,
// the preset server will reuse the OSC server instead of creating a new one.
// This means they are both exposed on the same OSC port.

PresetServer presetServer(parameterServer);


class MyApp : public App
{

public:
	MyApp()
	{
		addDodecahedron(graphics().mesh(), 0.3);
		graphics().mesh().generateNormals();
		nav().pos() = Vec3d(0, 0, 6);

		// Connect parameters and presets to servers
		parameterServer << X << Y << Brightness;
		presetServer << presetHandler;

		// Then expose the servers in the html server
		htmlServer << parameterServer;
		htmlServer << presetServer;

		// Display value of X on stdout
		X.registerChangeCallback(
		            [](float value, void *sender, void *userData, void *blockThis) {std::cout << "X = " << value << std::endl; });
		// Display the preset change processed
		presetHandler.registerPresetCallback(
		            [](int index, void *sender, void *userData) {
		                std::cout << "Preset change received: " << index << std::endl;
		            });

		initWindow();
	}

	virtual void onDraw(Graphics &g) override
	{
		g.depthTesting(true);
		light();
		g.pushMatrix();
		g.color(Brightness.get());
		g.translate(X.get(),Y.get(), 0);
		g.draw(g.mesh());
		g.popMatrix();
	}

	virtual void onExit() {
		presetServer.stopServer();
		parameterServer.stopServer();
	}

	Light light;
};

int main(int argc, char *argv[])
{
	MyApp().start();
	return 0;
}
