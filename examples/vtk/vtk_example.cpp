
#include "allocore/io/al_App.hpp"

// VTK includes
#include <ExternalVTKWidget.h>
#include <vtkActor.h>
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkCubeSource.h>
#include <vtkExternalOpenGLRenderWindow.h>
#include <vtkLight.h>
#include <vtkNew.h>
#include <vtkPolyDataMapper.h>


using namespace al;

class MyApp : public App{
public:

	Mesh mesh;
    Light light;
	double phase;

    // An ExternalVTKWidget and vtkExternalOpenGLRenderWindow are needed
    vtkNew<ExternalVTKWidget> externalVTKWidget;
    vtkNew<vtkExternalOpenGLRenderWindow> renWin;
    // Then actors, mappers and sources can be declared here
	vtkNew<vtkActor> actor;
	vtkNew<vtkPolyDataMapper> mapper;

	MyApp(): phase(0){

		addSphere(mesh);
		nav().pos(0,0,4);
		initWindow(Window::Dim(0,0, 600,400), "VTK example", 40);
		background(HSV(0.5, 1, 0.5));
	}

	virtual void onAnimate(double dt) override {
		double period = 10;
		phase += dt / period;
		if(phase >= 1.) phase -= 1.;
	}

    virtual void onCreate(const ViewpointWindow& win) override {
        // configure the external VTK widget
        externalVTKWidget->SetRenderWindow(renWin.GetPointer());

        // Connect the mapper to the actor
        actor->SetMapper(mapper.GetPointer());

        // Add actor to renderer
        vtkRenderer* ren = externalVTKWidget->AddRenderer();
        ren->AddActor(actor.GetPointer());

        // Create a source and connect it to the mapper
        vtkNew<vtkCubeSource> cs;
        mapper->SetInputConnection(cs->GetOutputPort());

        actor->RotateX(45.0);
        actor->RotateY(45.0);

    }

	virtual void onDraw(Graphics& g) override {

        // Draw VTK stuff
        actor->RotateX(2);
        externalVTKWidget->GetRenderWindow()->Render();

        // Draw Allosystem stuff
        light();
        g.polygonMode(Graphics::LINE); // wireframe mode
		g.pushMatrix();
		g.rotate(phase*360, 0,1,0);
		g.draw(mesh);
		g.popMatrix();
	}
};


int main(){
	MyApp().start();
}
