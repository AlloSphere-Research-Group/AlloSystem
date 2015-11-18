#include "allovtk/al_VTKRenderer.hpp"

using namespace al;

VTKRenderer::VTKRenderer()
{
	
}

void VTKRenderer::AddActor(vtkProp *p)
{
	//FIXME check if already there
	mProps.push_back(p);
}

