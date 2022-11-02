#include "EdgeFactory.h"
#include <Geom_Curve.hxx>
#include <Geom2d_Curve.hxx>
#define CURVEFACTORY() dynamic_cast<CurveFactory^>(_curveFactory)
namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			IXEdge^ EdgeFactory::BuildEdge(IXPoint^ start, IXPoint^ end)
			{
				TopoDS_Edge edge =  Ptr()->BuildEdge(gp_Pnt(start->X, start->Y, start->Z), gp_Pnt(end->X, end->Y, end->Z));
				if (edge.IsNull()) 
					throw gcnew XbimGeometryFactoryException("Failed to build edge, invalid points");
				else
					return gcnew XEdge(edge);
			}

			IXEdge^ EdgeFactory::Build(IIfcCurve^ curve)
			{
				return gcnew XEdge(BuildCurve(curve));
			}

			IXEdge^ EdgeFactory::BuildEdge(IXCurve^ curve)
			{
				if (curve->Is3d)
				{
					 Handle(Geom_Curve) hCurve = XCurve::GeomCurve(curve);
					TopoDS_Edge edge = Ptr()->BuildEdge(hCurve);
					if (edge.IsNull())
						throw gcnew XbimGeometryFactoryException("Failed to build edge, invalid curve 2D");
					else
						return gcnew XEdge(edge);
				}
				else
				{
					const Handle(Geom2d_Curve) hCurve = XCurve2d::GeomCurve2d(curve);
					TopoDS_Edge edge = Ptr()->BuildEdge(hCurve);
					if (edge.IsNull())
						throw gcnew XbimGeometryFactoryException("Failed to build edge, invalid curve 2D");
					else
						return gcnew XEdge(edge);

				}

			}

			


			TopoDS_Edge EdgeFactory::BuildCurve(IIfcCurve^ curve)
			{
				XCurveType curveType;
				int dim = (int)curve->Dim;
				if (dim == 2)
				{
					Handle(Geom2d_Curve) hCurve2d = CURVEFACTORY()->BuildGeom2d(curve, curveType);
					if (hCurve2d.IsNull()) throw gcnew XbimGeometryFactoryException("Failed to build curve 2D");
					TopoDS_Edge edge = Ptr()->BuildEdge(hCurve2d);
					if (edge.IsNull())
						throw gcnew XbimGeometryFactoryException("Failed to build edge, invalid curve 2D");
					else
						return edge;
				}
				else
				{
					Handle(Geom_Curve) hCurve = CURVEFACTORY()->BuildGeom3d(curve, curveType);
					if (hCurve.IsNull()) 
						throw gcnew XbimGeometryFactoryException("Failed to build curve 3D");
					TopoDS_Edge edge = Ptr()->BuildEdge(hCurve);
					if (edge.IsNull())
						throw gcnew XbimGeometryFactoryException("Failed to build edge, invalid curve 3D");
					else
						return edge;
				}
			}
		}
	}
}
