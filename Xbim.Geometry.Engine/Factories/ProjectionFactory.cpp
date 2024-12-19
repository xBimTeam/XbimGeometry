#include "ProjectionFactory.h"
#include <TopoDS_Shape.hxx>
#include <Poly_Polygon2D.hxx>

#include "../BRep/XShape.h"
#include "../BRep/XCompound.h"
#include "../BRep/XEdge.h"
#include "../BRep/XSolid.h"
#include "../BRep/XFace.h"
#include "../BRep/XShell.h"
#include "../BRep/XVertex.h"
#include "../BRep/XWire.h"
#include "../BRep/XFootprint.h"
#include "../BRep/XPlane.h"
#include <TopoDS.hxx>
using namespace System;
using namespace System::Linq;
using namespace Xbim::Geometry::BRep;

namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			IXFootprint^ ProjectionFactory::CreateFootprint(IXShape^ shape, bool createExactFootprint)
			{
				if (!shape->IsValidShape())
					throw RaiseGeometryFactoryException("Footprinting error: invalid shape");

				TopoDS_Shape topoShape = TOPO_SHAPE(shape);
				XFootprint^ footprint = gcnew XFootprint();
				bool isMetric = false;
				double oneMillimeter = _modelService->OneMillimeter;
				if(oneMillimeter==1 || oneMillimeter == 0.001 || oneMillimeter == 0.000001 || oneMillimeter == 0.01 || oneMillimeter == 0.1 || oneMillimeter == 10 || oneMillimeter == 100 || oneMillimeter == 0.00001 || oneMillimeter == 0.0001)
				{
					isMetric = true;
				};
				double deflection = isMetric ? oneMillimeter * 25 : oneMillimeter * 25.4;	
				double angularDeflection = M_PI / 6;
				EXEC_NATIVE->CreateFootPrint(topoShape, deflection, angularDeflection, _modelService->Precision, footprint->Ref(), !createExactFootprint);
				return footprint;
			}
			 
			IXFootprint^ ProjectionFactory::CreateFootprint(IXShape^ shape, double linearDeflection, double angularDeflection, bool createExactFootprint)
			{

				if (!shape->IsValidShape())
					throw RaiseGeometryFactoryException("Footprinting error: invalid shape");

				TopoDS_Shape topoShape = TOPO_SHAPE(shape);
				XFootprint^ footprint = gcnew XFootprint();
				EXEC_NATIVE->CreateFootPrint(topoShape, linearDeflection, angularDeflection, _modelService->Precision, footprint->Ref(), !createExactFootprint);
				return footprint;
			}

			IXCompound^ ProjectionFactory::GetOutline(IXShape^ shape)
			{
				if (!shape->IsValidShape())
					throw RaiseGeometryFactoryException("Get outline error: invalid shape");

				TopoDS_Shape topoShape = TOPO_SHAPE(shape);
				TopoDS_Compound outline = EXEC_NATIVE->GetOutline(topoShape);
				return  (IXCompound^)XShape::GetXbimShape(outline);
			}

			IEnumerable<IXFace^>^ ProjectionFactory::CreateSection(IXShape^ shape, IXPlane^ cutPlane)
			{

				if (!shape->IsValidShape())
					throw RaiseGeometryFactoryException("Create section error: invalid shape");

				TopoDS_Shape topoShape = TOPO_SHAPE(shape);
				XPlane^ plane = dynamic_cast<XPlane^>(cutPlane);
				if (plane == nullptr)
					throw gcnew InvalidCastException("Unsupported native plane object");
				TopTools_ListOfShape faces;
				
				bool success = EXEC_NATIVE->CreateSection(topoShape, plane->Ref(), _modelService->Precision, faces);
				if (!success) 
					throw RaiseGeometryFactoryException("Failed to create section");
				List<IXFace^>^ faceList = gcnew List<IXFace^>(faces.Size());
				for (auto& faceIt = faces.cbegin(); faceIt != faces.cend(); faceIt++)
					faceList->Add(gcnew XFace(TopoDS::Face(*faceIt)));
				return  faceList;
			}
		}
	}
}