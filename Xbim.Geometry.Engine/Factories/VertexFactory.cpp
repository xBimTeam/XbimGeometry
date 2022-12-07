#include "VertexFactory.h"
#include "../BRep/XVertex.h"
using namespace Xbim::Geometry::BRep;

TopoDS_Vertex Xbim::Geometry::Factories::VertexFactory::Build(IIfcCartesianPoint^ ifcCartesianPoint)
{
	if (3 == (int)ifcCartesianPoint->Dim)
		return EXEC_NATIVE->BuildVertex(ifcCartesianPoint->X, ifcCartesianPoint->Y, ifcCartesianPoint->Z, ModelGeometryService->Precision);
	else
		return EXEC_NATIVE->BuildVertex(ifcCartesianPoint->X, ifcCartesianPoint->Y, 0., ModelGeometryService->Precision);
}

bool Xbim::Geometry::Factories::VertexFactory::IsGeometricallySame(const TopoDS_Vertex& vertex1, const TopoDS_Vertex& vertex2)
{
	
	return BRep_Tool::Pnt(vertex1).IsEqual(BRep_Tool::Pnt(vertex2), ModelGeometryService->Precision);
}

IXVertex^ Xbim::Geometry::Factories::VertexFactory::Build(double x, double y, double z)
{
	return gcnew XVertex(EXEC_NATIVE->BuildVertex(x, y, z));
}


