#include "EdgeFactory.h"
#include "CurveFactory.h"

#include "../BRep/XEdge.h"
#include "../BRep/XCurve.h"
#include "../BRep/XCurve2d.h"
using namespace Xbim::Geometry::BRep;
namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			IXEdge^ EdgeFactory::BuildEdge(IXPoint^ start, IXPoint^ end)
			{
				TopoDS_Edge edge = EXEC_NATIVE->BuildEdge(gp_Pnt(start->X, start->Y, start->Z), gp_Pnt(end->X, end->Y, end->Z));
				if (edge.IsNull())
					RaiseGeometryFactoryException("Failed to build edge, invalid points, see logs");
				return gcnew XEdge(edge);
			}

			IXEdge^ EdgeFactory::Build(IIfcCurve^ curve)
			{

				return gcnew XEdge(BuildEdge(curve));
			}

			IXEdge^ EdgeFactory::BuildEdge(IXCurve^ curve)
			{
				if (curve->Is3d)
				{
					Handle(Geom_Curve) hCurve = XCurve::GeomCurve(curve);
					TopoDS_Edge edge = EXEC_NATIVE->BuildEdge(hCurve);
					if (edge.IsNull())
						RaiseGeometryFactoryException("Failed to build edge, invalid curve 3D");
					return gcnew XEdge(edge);
				}
				else
				{
					const Handle(Geom2d_Curve) hCurve = XCurve2d::GeomCurve2d(curve);
					TopoDS_Edge edge = EXEC_NATIVE->BuildEdge(hCurve);
					if (edge.IsNull())
						RaiseGeometryFactoryException("Failed to build edge, invalid curve 2D");
					return gcnew XEdge(edge);
				}
			}
#pragma region  Methods returning Opencascade native types, internal use only

			TopoDS_Edge EdgeFactory::BuildEdge(IIfcCurve^ curve)
			{
				XCurveType curveType;
				int dim = (int)curve->Dim;
				if (dim == 2)
				{
					Handle(Geom2d_Curve) hCurve2d = CURVE_FACTORY->BuildCurve2d(curve, curveType); //throws an exception if failure				
					return BuildEdge(hCurve2d);
				}
				else
				{
					Handle(Geom_Curve) hCurve3d = CURVE_FACTORY->BuildCurve3d(curve, curveType); //throws an exception if failure					
					return BuildEdge(hCurve3d);
				}
			}

			TopoDS_Edge EdgeFactory::BuildEdge(Handle(Geom2d_Curve) hCurve2d)
			{
				TopoDS_Edge edge = EXEC_NATIVE->BuildEdge(hCurve2d);
				if (edge.IsNull())
					RaiseGeometryFactoryException("Failed to build edge, invalid curve 2D");
				return edge;
			}
			TopoDS_Edge EdgeFactory::BuildEdge(Handle(Geom_Curve) hCurve3d)
			{
				TopoDS_Edge edge = EXEC_NATIVE->BuildEdge(hCurve3d);
				if (edge.IsNull())
					RaiseGeometryFactoryException("Failed to build edge, invalid curve 3D");
				return edge;
			}

#pragma endregion
		}
	}
}
