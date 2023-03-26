#include "FaceFactory.h"

#include "SurfaceFactory.h"
#include "EdgeFactory.h"
#include "WireFactory.h"
#include <BRepBuilderAPI_MakeFace.hxx>
#include <Geom_Plane.hxx>
#include <TopLoc_Datum3D.hxx>
#include <gp_Pln.hxx>
#include <ShapeAnalysis.hxx>


#include "../BRep/XSurface.h"
#include "../BRep/XPlane.h"
#include "../BRep/XFace.h"
#include "../BRep/XWire.h"


using namespace Xbim::Geometry::BRep;

using namespace Xbim::Geometry::Abstractions;
using namespace System::Linq;
namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{

			gp_Vec Xbim::Geometry::Factories::FaceFactory::Normal(const TopoDS_Face& face)
			{
				return OccHandle().Normal(face);
			}

			TopoDS_Face Xbim::Geometry::Factories::FaceFactory::BuildPlanarFace(IXPlane^ planeDef)
			{
				gp_Dir xDir(planeDef->RefDirection->X, planeDef->RefDirection->Y, planeDef->RefDirection->Is3d ? planeDef->RefDirection->Z : 0);
				gp_Dir zDir(planeDef->Axis->X, planeDef->Axis->Y, planeDef->Axis->Is3d ? planeDef->Axis->Z : 0);
				gp_Pnt loc(planeDef->Location->X, planeDef->Location->Y, planeDef->Location->Is3d ? planeDef->Location->Z : 0);
				gp_Ax3 ax3(loc, zDir, xDir);
				gp_Pln pln(ax3);
				BRepBuilderAPI_MakeFace  builder(pln);
				return builder.Face();
			}

			IXFace^ Xbim::Geometry::Factories::FaceFactory::BuildFace(IXSurface^ surface, array<IXWire^>^ wires)
			{
				XSurface^ xbimSurface = dynamic_cast<XSurface^>(surface);
				if (wires->Length < 1)
					throw gcnew System::ArgumentOutOfRangeException("Wires array must contain at least one wire");
				if (xbimSurface == nullptr)
					throw gcnew System::InvalidCastException("Invalid surface object not created by Xbim");
				//find the largest wire, this will definitely be the outer bound
				double maxArea = 0;
				IXWire^ outerWire;
				for each (IXWire ^ wire in wires)
				{
					double absArea = std::abs(wire->ContourArea);
					if (absArea > maxArea)
					{
						maxArea = absArea;
						outerWire = wire;
					}
				}

				const TopoDS_Wire& topoWire = TopoDS::Wire(static_cast<XWire^>(outerWire)->GetTopoShape());
				BRepBuilderAPI_MakeFace  builder(xbimSurface->Ref(), topoWire);

				for each (IXWire ^ wire in wires)
				{
					if (wire != outerWire)
					{
						const TopoDS_Wire& topoInnerWire = TopoDS::Wire(static_cast<XWire^>(wire)->GetTopoShape());
						builder.Add(topoInnerWire);
					}
				}
				TopoDS_Face result = builder.Face();
				result.Closed(outerWire->IsClosed);

				return gcnew XFace(result);
			}

			

			TopoDS_Face FaceFactory::BuildAdvancedFace(IIfcAdvancedFace^ advancedFace, TopTools_DataMapOfIntegerShape& edgeCurves, TopTools_DataMapOfIntegerShape& vertices)
			{
				int numberOfBounds = advancedFace->Bounds->Count;
				//workaround for badly defined linear extrusions in old Revit files
				IIfcSurfaceOfLinearExtrusion^ solExtrusion = dynamic_cast<IIfcSurfaceOfLinearExtrusion^>(advancedFace->FaceSurface);
				bool buildRuledSurface = (solExtrusion != nullptr);
				bool isBspline = dynamic_cast<IIfcBSplineSurface^>(advancedFace->FaceSurface)!=nullptr;
				TopoDS_Wire outerLoop;
				TopTools_SequenceOfShape  innerLoops;
				XSurfaceType surfaceType;
				Handle(Geom_Surface) surface = SURFACE_FACTORY->BuildSurface(advancedFace->FaceSurface, surfaceType); //throws exception
				
				if (!advancedFace->SameSense && !isBspline)
				{
					surface->UReverse();					
				}
				for each (IIfcFaceBound ^ ifcBound in advancedFace->Bounds) //build all the loops
				{
					TopTools_SequenceOfShape loopEdges;
					bool isOuter = (numberOfBounds == 1) || (dynamic_cast<IIfcFaceOuterBound^>(ifcBound) != nullptr);
					IIfcEdgeLoop^ edgeLoop = dynamic_cast<IIfcEdgeLoop^>(ifcBound->Bound);

					if (edgeLoop == nullptr) throw RaiseGeometryFactoryException("Advanced face bounds should be IfcEdgeLoop", ifcBound->Bound);//they always should be
					for each (IIfcOrientedEdge ^ orientedEdge in edgeLoop->EdgeList)
					{
						IIfcEdgeCurve^ edgeCurve = dynamic_cast<IIfcEdgeCurve^>(orientedEdge->EdgeElement);
						TopoDS_Edge edge;
						if (!edgeCurves.IsBound(orientedEdge->EdgeElement->EntityLabel)) //need to create the raw edge curve
						{

							if (edgeCurve == nullptr) 
								throw RaiseGeometryFactoryException("Advanced face oriented edge elements should be IfcEdgeCurve");//they always should be
							edge = EDGE_FACTORY->BuildEdgeCurve(edgeCurve, vertices); //throws exception
							if(edge.IsNull())
								throw RaiseGeometryFactoryException("Advanced face oriented edge curve should be built", orientedEdge);
							edgeCurves.Bind(edgeCurve->EntityLabel, edge);
							//take an  copy that is ready for parameterising to this surface		
							if (!orientedEdge->Orientation)
								edge = TopoDS::Edge(edge.Reversed());
							else edge = edge; //take a copy
						}
						else
						{
							//find the raw edge and take an  copy so we can reverse and set surface params
							if (!orientedEdge->Orientation)
								edge = TopoDS::Edge(edgeCurves.Find(edgeCurve->EntityLabel).Reversed());
							else
								edge = TopoDS::Edge(edgeCurves.Find(edgeCurve->EntityLabel));
						}
						//System::String^ edgeBrep = _modelService->GetBrep(edge);
						loopEdges.Append(edge);
					}
					TopoDS_Wire wire = WIRE_FACTORY->BuildWire(loopEdges);
					if (!ifcBound->Orientation)
						wire.Reverse();
					if (isOuter) // this may not be known but if it is a single bound or the bound has been correctly typed it simplifies things later
						outerLoop = wire;
					else
						innerLoops.Append(wire);
					
				}

				//no outer loop defined, find the biggest
				if (outerLoop.IsNull())
				{
					double area = 0;
					int foundIndex = -1;
					int idx = 0;
					for (auto&& innerLoop : innerLoops)
					{
						idx++;
						double loopArea = ShapeAnalysis::ContourArea(TopoDS::Wire(innerLoop));
						if (loopArea > area)
						{
							outerLoop = TopoDS::Wire(innerLoop);
							area = loopArea;
							foundIndex = idx;
						}
					}
					if (foundIndex > 0)
						innerLoops.Remove(foundIndex); //remove outer loop from inner loops
				}
				if (outerLoop.IsNull())
					throw RaiseGeometryFactoryException("Face has no outer bound", advancedFace);//no bounded face
				TopoDS_Face face = BuildFace(surface, outerLoop, innerLoops); //throws exception
				//System::String^ brep = _modelService->GetBrep(face);
				if (!advancedFace->SameSense && isBspline)
					face.Reverse();
				//brep = _modelService->GetBrep(face);
		
 				

				return face;

			}
		
			TopoDS_Face FaceFactory::BuildFace(const Handle(Geom_Surface)& surface)
			{

				TopoDS_Face face = EXEC_NATIVE->BuildFace(surface, _modelService->Precision);
				if (face.IsNull())
					throw RaiseGeometryFactoryException("Could not apply bounds to face");

				return face;
			}

			TopoDS_Face FaceFactory::BuildFace(const Handle(Geom_Surface)& surface, const TopoDS_Wire& outerLoop, const TopTools_SequenceOfShape& innerLoops)
			{

				TopoDS_Face face = EXEC_NATIVE->BuildFace(surface, outerLoop, innerLoops, _modelService->Precision);
				if (face.IsNull())
					throw RaiseGeometryFactoryException("Could not apply bounds to face");

				return face;
			}
			
		}
	}
}