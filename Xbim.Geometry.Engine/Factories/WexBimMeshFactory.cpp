#include "WexBimMeshFactory.h" 
#include "../BRep/XShape.h"

using namespace Xbim::Geometry::BRep;

 
array<System::Byte>^ Xbim::Geometry::Factories::WexBimMeshFactory::CreateWexBimMesh(IXShape^ shape, IXAxisAlignedBoundingBox^% bounds)
{
	bool hasCurves;
	return CreateWexBimMesh(shape, _modelService->MeshFactors, 1 / _modelService->Model->ModelFactors->OneMeter, bounds, hasCurves);
}

array<System::Byte>^ Xbim::Geometry::Factories::WexBimMeshFactory::CreateWexBimMesh(IXShape^ shape, IXAxisAlignedBoundingBox^% bounds, bool% hasCurves)
{
	return createWexBimMesh(shape, _modelService->MeshFactors, 1 / _modelService->Model->ModelFactors->OneMeter, bounds, hasCurves, false, true, true);
}


array<System::Byte>^ Xbim::Geometry::Factories::WexBimMeshFactory::CreateWexBimMesh(IXShape^ shape, IXMeshFactors^ meshFactors, double scale, IXAxisAlignedBoundingBox^% bounds)
{
	bool hasCurves;
	return CreateWexBimMesh(shape, meshFactors, scale, bounds, hasCurves);
}

array<System::Byte>^ Xbim::Geometry::Factories::WexBimMeshFactory::CreateWexBimMesh(IXShape^ shape, IXMeshFactors^ meshFactors, double scale, IXAxisAlignedBoundingBox^% bounds, bool% hasCurves)
{
	return createWexBimMesh(shape, meshFactors, scale, bounds, hasCurves, false, true, true);
}


array<System::Byte>^ Xbim::Geometry::Factories::WexBimMeshFactory::CreateWexBimMesh(IXShape^ shape, double tolerance, double linearDeflection, double angularDeflection, double scale, IXAxisAlignedBoundingBox^% bounds)
{
	bool hasCurves;
	return CreateWexBimMesh(shape, tolerance, linearDeflection, angularDeflection, scale, bounds, hasCurves);
}

array<System::Byte>^ Xbim::Geometry::Factories::WexBimMeshFactory::CreateWexBimMesh(IXShape^ shape, double tolerance, double linearDeflection, double angularDeflection, double scale, IXAxisAlignedBoundingBox^% bounds, bool% hasCurves)
{
	return createWexBimMesh(shape, tolerance, linearDeflection, angularDeflection, scale, bounds, hasCurves, false, true, true);
}


array<System::Byte>^ Xbim::Geometry::Factories::WexBimMeshFactory::createWexBimMesh
	(IXShape^ shape, IXMeshFactors^ meshFactors, double scale, IXAxisAlignedBoundingBox^% bounds, bool% hasCurves, bool checkEdges, bool cleanBefore, bool cleanAfter)
{  
	return createWexBimMesh(shape, meshFactors->Tolerance, meshFactors->LinearDefection, meshFactors->AngularDeflection, scale, bounds, hasCurves, checkEdges, cleanBefore, cleanAfter);
}
			 
array<System::Byte>^ Xbim::Geometry::Factories::WexBimMeshFactory::createWexBimMesh
	(IXShape^ shape, double tolerance, double linearDeflection, double angularDeflection, double scale, IXAxisAlignedBoundingBox^% bounds, bool% hasCurves, bool checkEdges, bool cleanBefore, bool cleanAfter)
{ 
	if (!shape->IsValidShape())
		throw RaiseGeometryFactoryException("WexBim meshing error: invalid shape");

	TopoDS_Shape topoShape = static_cast<XShape^>(shape)->GetTopoShape();

	auto mesh = EXEC_NATIVE->CreateMesh(topoShape, tolerance, linearDeflection, angularDeflection, scale);

	auto bMin = mesh.BndBox.CornerMin();
	auto bMax = mesh.BndBox.CornerMax();

	bounds = gcnew XAxisAlignedBox(Bnd_Box(gp_Pnt(bMin[0], bMin[1], bMin[2]), gp_Pnt(bMax[0], bMax[1], bMax[2])));

	std::stringstream output;
	mesh.WriteToStream(output);
	int size = (int)output.str().length();
	auto buffer = std::make_unique<char[]>(size);
	output.seekg(0);
	output.read(buffer.get(), size);
	cli::array<System::Byte>^ byteArray = gcnew cli::array<System::Byte>(size);
	System::Runtime::InteropServices::Marshal::Copy((System::IntPtr)buffer.get(), byteArray, 0, size);
	return byteArray;
 
}

