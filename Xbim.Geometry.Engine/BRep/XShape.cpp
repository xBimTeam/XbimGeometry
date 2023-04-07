#include "XShape.h"
#include "XCompound.h"
#include "XSolid.h"
#include "XShell.h"
#include "XFace.h"
#include "XWire.h"
#include "XEdge.h"
#include "XVertex.h"
#include <TopExp.hxx>
#include <TopoDS.hxx>
#include <BRepLib.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <TopExp_Explorer.hxx>
#include "../Factories/Unmanaged/NShapeFactory.h"

namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			bool XShape::IsValidShape()
			{
				//				bool ok = false;
				//				try
				//				{
				//					
				//#if _DEBUG
				//					/*BRepCheck_Analyzer analyser(OccHandle(), true);
				//					ok = analyser.IsValid();*/
				//					//code below is useful to see errors
				//					//Handle(BRepCheck_Result) res = analyser.Result(OccHandle());
				//					//const BRepCheck_ListOfStatus& stati = res->Status();
				//					//BRepCheck_Status first = stati.First();
				//#else
				//					ok = true; //only do this check in debug mode, when we have a problem houston, its quite expensive
				//#endif
				//					
				//					
				//				}
				//				catch (Standard_Failure e)
				//				{
				//				}
				//				return ok;
				return !IsInvalid && !OccHandle().IsNull();
			}
			bool XShape::IsEqual(IXShape^ iXShape)
			{
				XShape^ xShape = dynamic_cast<XShape^>(iXShape);
				if (xShape == __nullptr) return false;
				bool isEqual = xShape->Ptr()->IsSame(*Ptr()); //we ignore orientation
				return isEqual;
			}


			IXShape^ XShape::GetXbimShape(const TopoDS_Shape& shape)
			{
				if (shape.IsNull())
					return nullptr;
				TopAbs_ShapeEnum shapeType = shape.ShapeType();
				switch (shapeType)
				{
				case TopAbs_VERTEX:
					return gcnew XVertex(TopoDS::Vertex(shape));
				case TopAbs_EDGE:
					return gcnew XEdge(TopoDS::Edge(shape));
				case TopAbs_WIRE:
					return gcnew XWire(TopoDS::Wire(shape));
				case TopAbs_FACE:
					return gcnew XFace(TopoDS::Face(shape));
				case TopAbs_SHELL:
					return gcnew XShell(TopoDS::Shell(shape));
				case TopAbs_SOLID:
					return gcnew XSolid(TopoDS::Solid(shape));
				case TopAbs_COMPOUND:
					return gcnew XCompound(TopoDS::Compound(shape));
				default:
					break;
				}
				throw gcnew XbimGeometryServiceException("Build not supported for shape type");

			}

			bool XShape::Triangulate(IXMeshFactors^ meshFactors)
			{
				IMeshTools_Parameters meshParams;
				meshParams.Deflection = meshFactors->LinearDefection;
				meshParams.Angle = meshFactors->AngularDeflection;
				meshParams.Relative = meshFactors->Relative;
				meshParams.InParallel = false;
				/*meshParams.InternalVerticesMode = meshFactors->InternalVerticesMode;
				meshParams.ControlSurfaceDeflection = meshFactors->ControlSurfaceDeflection;	*/
				return NShapeFactory::Triangulate(Ref(), meshParams);
			}

			IEnumerable<IXFace^>^ XShape::AllFaces()
			{
				TopTools_IndexedMapOfShape map;
				TopExp::MapShapes(Ref(), TopAbs_FACE, map);
				List<IXFace^>^ faces = gcnew  List<IXFace^>(map.Extent());
				for (int i = 1; i <= map.Extent(); i++)
					faces->Add(gcnew XFace(TopoDS::Face(map(i))));
				return faces;
			}


		}
	}
}
