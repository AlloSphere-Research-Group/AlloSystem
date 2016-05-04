

#include "allocore/io/al_HtmlParameterServer.hpp"
#include "allocore/system/al_Parameter.hpp"
#include "allocore/io/al_App.hpp"

using namespace al;

Parameter X("X", "Pos", 0, "", -2, 2);
Parameter Y("Y", "Pos", 0, "", -2, 2);
Parameter Brightness("Brightness", "Pos", 0.5, "", 0, 1);

class MyApp : public App
{
public:
	MyApp() {
		addDodecahedron(graphics().mesh(), 0.3);
		graphics().mesh().generateNormals();
		nav().pos() = Vec3d(0, 0, 6);

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

	Light light;
};

int main(int argc, char *argv[])
{
	HtmlParameterServer server;

	server << X << Y << Brightness;

	// Display value of X on stdout
	X.registerChangeCallback(
	            [](float value, void *sender, void *userData) {std::cout << "X = " << value << std::endl; });

	MyApp().start();
	return 0;
}
