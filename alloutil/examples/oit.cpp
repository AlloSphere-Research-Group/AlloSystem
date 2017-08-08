#include "alloutil/al_OITFbo.hpp"
#include "allocore/io/al_App.hpp"

using namespace al;
using namespace std;

const unsigned int width = 800;
const unsigned int height = 600;

struct MyApp : public App {
    OITFbo oit;
    Mesh shape;

    void onCreate(const ViewpointWindow& w){
        oit.init(width, height);

        addSphere(shape);
        shape.generateNormals();
        nav().pullBack(10);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    }

    void draw_opaque(Graphics& g) {
        g.pushMatrix();
        glColor4f(0.6, 0.4, 0.3, 1.0);
        g.draw(shape);
        g.popMatrix();
    }

    void draw_transparent(Graphics& g) {
        g.pushMatrix();
        g.translate(1, -0.5, -1);
        glColor4f(0.3, 0.7, 0.9, 0.6);
        g.draw(shape);
        g.popMatrix();

        g.pushMatrix();
        g.translate(-1.5, 1, 1.5);
        glColor4f(0.1, 0.7, 0.2, 0.6);
        g.draw(shape);
        g.popMatrix();

        g.pushMatrix();
        g.translate(-1, 0.5, 1);
        glColor4f(0.7, 0.1, 0.4, 0.6);
        g.draw(shape);
        g.popMatrix();
    }

    void onDraw(Graphics& g){
        oit.beginOpaque();
        glClearColor(0.9, 0.9, 0.8, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        draw_opaque(g);
        oit.endOpaque();

        oit.beginAccum();
        draw_transparent(g);
        oit.endAccum();

        oit.beginReveal();
        draw_transparent(g);
        oit.endReveal();

        oit.composite();

        oit.result().quadViewport(g);
    }

};

int main() {
    MyApp app;
    app.initWindow(Window::Dim(width, height));
    app.start();
}

