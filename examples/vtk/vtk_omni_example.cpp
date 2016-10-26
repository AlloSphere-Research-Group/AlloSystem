
#include "alloutil/al_OmniApp.hpp"

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

class MyApp : public OmniApp {
public:

	Mesh mesh;
    Light light;

    // An ExternalVTKWidget and vtkExternalOpenGLRenderWindow are needed
    vtkNew<ExternalVTKWidget> externalVTKWidget;
    vtkNew<vtkExternalOpenGLRenderWindow> renWin;
    // Then actors, mappers and sources can be declared here
	vtkNew<vtkActor> actor;
	vtkNew<vtkPolyDataMapper> mapper;

    MyApp() {
      mesh.primitive(Graphics::TRIANGLES);
      addSphere(mesh);
      nav().pos(0,0,2);
    }

    virtual ~MyApp() {}

    virtual void onDraw(Graphics& g) {
    // Draw VTK stuff
      externalVTKWidget->GetRenderWindow()->Render();
      glPopAttrib();
      light();
      // say how much lighting you want
      shader().uniform("lighting", 1.0);
      g.polygonMode(Graphics::LINE); // wireframe mode
      g.pushMatrix();
    //   g.rotate(phase*360, 0,1,0);
      g.draw(mesh);
      g.popMatrix();
    }

    virtual bool onCreate() override {
        OmniApp::onCreate();

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

        return true;
    }

    virtual void onAnimate(al_sec dt) {
      actor->RotateX(2);
      pose = nav();
    }
};


int main(){
	MyApp().start();
}
