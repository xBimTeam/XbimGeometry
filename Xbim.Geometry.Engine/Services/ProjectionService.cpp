#include "ProjectionService.h"
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
using namespace Xbim::Geometry::BRep;

namespace Xbim
{
	namespace Geometry
	{
		namespace Services
		{
			IXFootprint^ ProjectionService::CreateFootprint(IXShape^ shape)
			{
				const TopoDS_Shape& topoShape = TOPO_SHAPE(shape);
				XFootprint^ footprint = gcnew XFootprint();
				bool isMetric = false;
				double oneMillimeter = _modelService->OneMillimeter;
				if(oneMillimeter==1 || oneMillimeter == 0.001 || oneMillimeter == 0.000001 || oneMillimeter == 0.01 || oneMillimeter == 0.1 || oneMillimeter == 10 || oneMillimeter == 100 || oneMillimeter == 0.00001 || oneMillimeter == 0.0001)
				{
					isMetric = true;
				};
				double deflection = isMetric ? oneMillimeter * 25 : oneMillimeter * 25.4;	
				double angularDeflection = M_PI / 6;
				OccHandle().CreateFootPrint(topoShape, deflection, angularDeflection, _modelService->Precision, footprint->Ref());
				return footprint;
			}

			IXFootprint^ ProjectionService::CreateFootprint(IXShape^ shape, double linearDeflection, double angularDeflection)
			{
				const TopoDS_Shape& topoShape = TOPO_SHAPE(shape);
				XFootprint^ footprint = gcnew XFootprint();
				OccHandle().CreateFootPrint(topoShape, linearDeflection, angularDeflection, _modelService->Precision, footprint->Ref());
				return footprint;
			}
			IXCompound^ ProjectionService::GetOutline(IXShape^ shape)
			{
				const TopoDS_Shape& topoShape = TOPO_SHAPE(shape);
				TopoDS_Compound outline = OccHandle().GetOutline(topoShape);
				return  (IXCompound^)XShape::GetXbimShape(outline);
			}

			IEnumerable<IXFace^>^ ProjectionService::CreateSection(IXShape^ shape, IXPlane^ cutPlane)
			{
				const TopoDS_Shape& topoShape = TOPO_SHAPE(shape);
				XPlane^ plane = dynamic_cast<XPlane^>(cutPlane);
				if (plane == nullptr)
					throw gcnew InvalidCastException("Unsupported native plane object");
				TopTools_ListOfShape faces;
				
				bool success = OccHandle().CreateSection(topoShape, plane->Ref(), _modelService->Precision, faces);
				if (!success) 
					throw gcnew XbimGeometryServiceException("Failed to create section");
				List<IXFace^>^ faceList = gcnew List<IXFace^>(faces.Size());
				for (auto& faceIt = faces.cbegin(); faceIt != faces.cend(); faceIt++)
					faceList->Add(gcnew XFace(TopoDS::Face(*faceIt)));
				return  faceList;
			}
		}
	}
}