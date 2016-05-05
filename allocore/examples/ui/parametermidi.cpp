
#include "allocore/io/al_App.hpp"
#include "allocore/ui/al_ParameterMIDI.hpp"
#include <cmath>

using namespace al;

Parameter Size("Size", "", 1.0, "", 0, 1.0);
Parameter Speed("Speed", "", 0.05, "", 0.01, 0.3);

ParameterMIDI parameterMIDI;


class MyApp : public App {
public:
	MyApp () {
		x = 0;
		addSphere(graphics().mesh());

		initWindow();
		nav().pos(0, 0, 4);
	}

	virtual void onDraw(Graphics &g) override {
		g.pushMatrix();
		g.translate(std::sin(x), 0, 0);
		g.scale(Size.get());
		g.draw(g.mesh());
		g.popMatrix();
		x += Speed.get();
		if (x >= M_2PI) {
			x -= M_2PI;
		}
	}

private:
	float x;
};

int main(int argc, char *argv[])
{
	parameterMIDI.connectControl(Size, 1, 1);
	parameterMIDI.connectControl(Speed, 10, 1);

	MyApp().start();

	return 0;
}

