#ifndef AL_VTKRENDERER_H
#define AL_VTKRENDERER_H

#include <vector>

#include "vtkProp.h"

namespace  al {
class VTKRenderer
{
public:
  VTKRenderer();
  
  void AddActor(vtkProp *p);
  
private:
  std::vector<vtkProp *> mProps;
  
//  alvtkViewport * mViewport;
};
}



#endif // AL_VTKRENDERER_H
