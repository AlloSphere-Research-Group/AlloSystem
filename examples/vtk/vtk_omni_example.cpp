
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

    //
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
    //
	// virtual void onDraw(Graphics& g) override {
    //
    //     // Draw VTK stuff
    //     externalVTKWidget->GetRenderWindow()->Render();
    //
    //     // Draw Allosystem stuff
    //     shader().uniform("lighting", 1.0);
    //     light();

	// }

    MyApp() {
      mesh.primitive(Graphics::TRIANGLES);
      addSphere(mesh);
      nav().pos(0,0,4);
    //   for (int i = 0; i < mesh.vertices().size(); ++i) {
    //     float f = (float)i / mesh.vertices().size();
    //     mesh.color(Color(HSV(f, 1 - f, 1), 1));
    //   }
    //   mesh.generateNormals();
    //   light.ambient(Color(0.4, 0.4, 0.4, 1.0));
    //   light.pos(5, 5, 5);
    //   initAudio();
    }

    virtual ~MyApp() {}

    virtual void onDraw(Graphics& g) {
    // Draw VTK stuff
    glPushAttrib(GL_VIEWPORT_BIT);
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

    virtual void onAnimate(al_sec dt) {
      // light.pos(nav().pos());

      actor->RotateX(2);
      pose = nav();
      // std::cout << dt << std::endl;
    }
};


int main(){
	MyApp().start();
}
