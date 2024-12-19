#include "XPolyLoop2d.h"
#include "XWire.h"
#include "XShape.h"
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			IXWire^ XPolyLoop2d::BuildWire(double zDim)
			{		
				if (Ref().IsNull() || Ref()->Nodes().Size()==0) return nullptr;
				auto& polyIt = Ref()->Nodes().cbegin();
				if (polyIt == Ref()->Nodes().cend()) return nullptr;
				gp_Pnt startPoint = gp_Pnt(polyIt->X(), polyIt->Y(), zDim);
				BRepBuilderAPI_MakeWire wireMaker;
				for (polyIt++; polyIt!= Ref()->Nodes().cend(); polyIt++)
				{
					gp_Pnt endPoint = gp_Pnt(polyIt->X(), polyIt->Y(), zDim);
					TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(startPoint, endPoint);					
					wireMaker.Add(edge);
					startPoint = endPoint;
				}
				return (IXWire^) XShape::GetXbimShape(wireMaker.Wire());
			}
		}
	}
}