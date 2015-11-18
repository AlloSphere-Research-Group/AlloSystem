

#include "allocore/io/al_Window.hpp"
#include "allovtk/al_VTKRenderer.hpp"

#include "vtkCylinderSource.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkProperty.h"
#include "vtkCamera.h"

using namespace al;


class MyWin : public VTKRenderer
{
public:
  vtkCylinderSource *cylinder;
  vtkPolyDataMapper *cylinderMapper;
  vtkActor *cylinderActor;
  
  MyWin() {
    
  }
  
  virtual void onCreate()
  {
    cylinder = vtkCylinderSource::New();
    cylinder->SetResolution(8);
    
    // The mapper is responsible for pushing the geometry into the graphics
    // library. It may also do color mapping, if scalars or other attributes
    // are defined.
    //
    cylinderMapper = vtkPolyDataMapper::New();
    cylinderMapper->SetInputConnection(cylinder->GetOutputPort());
    
    // The actor is a grouping mechanism: besides the geometry (mapper), it
    // also has a property, transformation matrix, and/or texture map.
    // Here we set its color and rotate it -22.5 degrees.
    cylinderActor = vtkActor::New();
    cylinderActor->SetMapper(cylinderMapper);
    cylinderActor->GetProperty()->SetColor(1.0000, 0.3882, 0.2784);
    cylinderActor->RotateX(30.0);
    cylinderActor->RotateY(-45.0);
    
    // Create the graphics structure. The renderer renders into the
      // render window. The render window interactor captures mouse events
      // and will perform appropriate camera or actor manipulation
      // depending on the nature of the events.
      //
//      vtkRenderer *ren1 = vtkRenderer::New();
//      vtkRenderWindow *renWin = vtkRenderWindow::New();
//      renWin->AddRenderer(ren1);
//      vtkRenderWindowInteractor *iren = vtkRenderWindowInteractor::New();
//      iren->SetRenderWindow(renWin);
     
      // Add the actors to the renderer, set the background and size
      //
      AddActor(cylinderActor);
//      this->SetBackground(0.1, 0.2, 0.4);
//      this->SetSize(200, 200);
     
//      // We'll zoom in a little by accessing the camera and invoking a "Zoom"
//      // method on it.
//      this->ResetCamera();
//      this->GetActiveCamera()->Zoom(1.5);
//      renWin->Render();
     
     
      // Exiting from here, we have to delete all the instances that
      // have been created.
      
  }
  
  virtual void onDestroy() {
    cylinder->Delete();
    cylinderMapper->Delete();
    cylinderActor->Delete();
  }
};


int main()
{
  MyWin w;
//  w.start();
}
