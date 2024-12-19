#pragma once  
#include "NFactoryBase.h"
#include <TopoDS_Shape.hxx> 
#include "../../Services/Unmanaged/NWexBimMesh.h"

  
class NWexBimMeshFactory : public NFactoryBase
{  

public:
	NWexBimMesh CreateMesh(const TopoDS_Shape& shape, double tolerance, double linearDeflection, double angularDeflection, double scale, bool checkEdges, bool cleanBefore, bool cleanAfter);
	NWexBimMesh CreateMesh(const TopoDS_Shape& shape, double tolerance, double linearDeflection, double angularDeflection, double scale);  

};
