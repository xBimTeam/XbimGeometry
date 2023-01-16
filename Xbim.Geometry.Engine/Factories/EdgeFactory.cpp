#include "EdgeFactory.h"
#include "CurveFactory.h"
#include "GeometryFactory.h"
#include "VertexFactory.h"
#include "../BRep/XEdge.h"
#include "../BRep/XCurve.h"
#include "../BRep/XCurve2d.h"
#include <GCE2d_MakeCircle.hxx>
using namespace Xbim::Geometry::BRep;
namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			IXEdge^ EdgeFactory::Build(IXPoint^ start, IXPoint^ end)
			{
				TopoDS_Edge edge = OccHandle().BuildEdge(gp_Pnt(start->X, start->Y, start->Z), gp_Pnt(end->X, end->Y, end->Z));
				if (edge.IsNull())
					RaiseGeometryFactoryException("Failed to build edge, invalid points, see logs");
				return gcnew XEdge(edge);
			}

			IXEdge^ EdgeFactory::Build(IIfcCurve^ curve)
			{

				return gcnew XEdge(BuildEdge(curve));
			}

			IXEdge^ EdgeFactory::Build(IXCurve^ curve)
			{
				if (curve->Is3d)
				{
					Handle(Geom_Curve) hCurve = XCurve::GeomCurve(curve);
					TopoDS_Edge edge = OccHandle().BuildEdge(hCurve);
					if (edge.IsNull())
						RaiseGeometryFactoryException("Failed to build edge, invalid curve 3D");
					return gcnew XEdge(edge);
				}
				else
				{
					const Handle(Geom2d_Curve) hCurve = XCurve2d::GeomCurve2d(curve);
					TopoDS_Edge edge = OccHandle().BuildEdge(hCurve);
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
				TopoDS_Edge edge = OccHandle().BuildEdge(hCurve2d);
				if (edge.IsNull())
					RaiseGeometryFactoryException("Failed to build edge, invalid curve 2D");
				return edge;
			}
			TopoDS_Edge EdgeFactory::BuildEdge(Handle(Geom_Curve) hCurve3d)
			{
				TopoDS_Edge edge = OccHandle().BuildEdge(hCurve3d);
				if (edge.IsNull())
					RaiseGeometryFactoryException("Failed to build edge, invalid curve 3D");
				return edge;
			}

			/// <summary>
			/// Pass an empty vertices collection if not using in the context of building an advanced brep
			/// </summary>
			/// <param name="ifcEdgeCurve"></param>
			/// <param name="vertices"></param>
			/// <returns></returns>
			TopoDS_Edge EdgeFactory::BuildEdgeCurve(IIfcEdgeCurve^ ifcEdgeCurve, TopTools_DataMapOfIntegerShape& verticesContext)
			{
				TopoDS_Vertex startVertex;
				TopoDS_Vertex endVertex;
				TopoDS_Edge edge;
				if (!verticesContext.IsBound(ifcEdgeCurve->EdgeStart->EntityLabel))
				{
					IIfcCartesianPoint^ edgeStart = ((IIfcCartesianPoint^)((IIfcVertexPoint^)ifcEdgeCurve->EdgeStart)->VertexGeometry);
					startVertex = VERTEX_FACTORY->Build(edgeStart);
					verticesContext.Bind(ifcEdgeCurve->EdgeStart->EntityLabel, startVertex);
				}
				else
					startVertex = TopoDS::Vertex(verticesContext.Find(ifcEdgeCurve->EdgeStart->EntityLabel));

				if (!verticesContext.IsBound(ifcEdgeCurve->EdgeEnd->EntityLabel))
				{
					IIfcCartesianPoint^ edgeEnd = ((IIfcCartesianPoint^)((IIfcVertexPoint^)ifcEdgeCurve->EdgeEnd)->VertexGeometry);
					endVertex = VERTEX_FACTORY->Build(edgeEnd);
					//if start and end are geometrically within tolerance use the start
					if(VERTEX_FACTORY->IsGeometricallySame(startVertex, endVertex))
						endVertex = startVertex;
					else
						verticesContext.Bind(ifcEdgeCurve->EdgeEnd->EntityLabel, endVertex);
				}
				else
					endVertex = TopoDS::Vertex(verticesContext.Find(ifcEdgeCurve->EdgeEnd->EntityLabel));


				Handle(Geom_Curve) edgeCurve = CURVE_FACTORY->BuildCurve3d(ifcEdgeCurve->EdgeGeometry);


				if (!ifcEdgeCurve->SameSense)
					edgeCurve->Reverse(); //reverse the geometry if the parameterisation is in a different direction to the edge start and end vertices

				if (edgeCurve->IsClosed() && startVertex.IsSame(endVertex))// we have a closed shape and we want the whole loop
				{				
					edge = OccHandle().BuildEdge(edgeCurve);
				}
				else
				{
					edge = OccHandle().BuildEdge(edgeCurve,startVertex, endVertex, ModelGeometryService->MinimumGap);
				}

				return edge;

			}

			TopoDS_Edge EdgeFactory::BuildCircle(double radius, gp_Ax22d axis)
			{
				gp_Circ2d outer(axis, radius);
				Handle(Geom2d_Circle) hOuter = GCE2d_MakeCircle(outer);
				TopoDS_Edge  edge = EXEC_NATIVE->BuildEdge(hOuter);
				if (edge.IsNull())
					throw RaiseGeometryFactoryException("Failed to create circle edge");
				else
					return edge;
			}

#pragma endregion
		}
	}
}
