

#include "allocore/io/al_App.hpp"
#include "allocore/system/al_Parameter.hpp"

using namespace al;

Parameter X("X", "Position", 0.0, "", -1.0, 1.0);
Parameter Y("Y", "Position", 0.0, "", -1.0, 1.0);

PresetHandler presets;

class MyApp : public App
{
public:
	MyApp() {
		initWindow(Window::Dim(800,600), "Presets", 15);
		nav().pos(Vec3d(0,0,8));
	}

	virtual void onDraw(Graphics &g) override
	{
		g.lighting(true);
		light();
		addCone(g.mesh());
		g.pushMatrix();
		float x = X.get() + rng.uniform(-0.1, 0.1);
		float y = Y.get() + rng.uniform(-0.1, 0.1);
		X = x; // Parameters can be assigned values directly
		Y = y;
		g.translate(x, y, 0);
		g.draw(g.mesh());
		g.popMatrix();
	}

	virtual void onKeyDown(const Keyboard &k) override
	{
		switch(k.key()) {
		case '1':
			presets.storePreset("pos");
			std::cout << "Preset stored." << std::endl;
			break;
		case '2':
			presets.recallPreset("pos");
			std::cout << "Preset loaded." << std::endl;
			break;
		}
	}
private:
	rnd::Random<> rng; // Random number generator
	Light light;
};


int main(int argc, char *argv[])
{
	std::cout << "Press 1 to store location, 2 to recall." << std::endl;
	presets << X << Y; // Add parameters to preset handling
	MyApp().start();
	return 0;
}
