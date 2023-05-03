

#include <TopTools_IndexedMapOfShape.hxx>
#include <TopExp.hxx>
#include <BRep_Builder.hxx>
#include "./Services/ModelGeometryService.h"
#include <BRepBuilderAPI_Sewing.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Section.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepTools.hxx>
#include <ShapeFix_ShapeTolerance.hxx>
#include <Message_ProgressIndicator.hxx>
#include <ShapeFix_Shape.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <ShapeUpgrade_UnifySameDomain.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include "XbimNativeApi.h"
#include "XbimGeometryObjectSet.h"
#include "XbimSolidSet.h"
#include "XbimShellSet.h"
#include "XbimFaceSet.h"
#include "XbimEdgeSet.h"
#include "XbimVertexSet.h"

#include "./BRep/XCompound.h"
#include "./Factories/BooleanFactory.h"
using namespace System::ComponentModel;
namespace Xbim
{
	namespace Geometry
	{

		XbimGeometryObjectSet::XbimGeometryObjectSet(System::Collections::Generic::IEnumerable<IXbimGeometryObject^>^ objects, ModelGeometryService^ modelService) : XbimSetObject(modelService)
		{
			geometryObjects = gcnew List<IXbimGeometryObject^>(objects);
		}

		XbimGeometryObjectSet::XbimGeometryObjectSet(const TopoDS_Shape& shape, ModelGeometryService^ modelService) : XbimSetObject(modelService)
		{
			geometryObjects = gcnew List<IXbimGeometryObject^>(1);
			geometryObjects->Add(gcnew XbimCompound(shape, modelService));
		}

		XbimGeometryObjectSet::XbimGeometryObjectSet(ModelGeometryService^ modelService) : XbimSetObject(modelService)
		{
			geometryObjects = gcnew List<IXbimGeometryObject^>();
		}

		IXbimGeometryObject^ XbimGeometryObjectSet::First::get()
		{
			if (geometryObjects->Count == 0) return nullptr;
			return geometryObjects[0];
		}

		int XbimGeometryObjectSet::Count::get()
		{
			return geometryObjects->Count;
		}

		System::Collections::Generic::IEnumerator<IXbimGeometryObject^>^ XbimGeometryObjectSet::GetEnumerator()
		{
			return geometryObjects->GetEnumerator();
		}

		bool XbimGeometryObjectSet::Sew()
		{
			bool sewn = false;
			for each (IXbimGeometryObject ^ geom in geometryObjects)
			{
				XbimCompound^ comp = dynamic_cast<XbimCompound^>(geom);
				if (comp != nullptr)
					if (comp->Sew()) sewn = true;
			}
			return sewn;
		}

		IXbimGeometryObject^ XbimGeometryObjectSet::Transformed(IIfcCartesianTransformationOperator^ transformation)
		{
			if (!IsValid) return this;
			XbimGeometryObjectSet^ result = gcnew XbimGeometryObjectSet(_modelServices);
			result->Tag = Tag;
			for each (IXbimGeometryObject ^ geometryObject in geometryObjects)
			{
				XbimOccShape^ occShape = dynamic_cast<XbimOccShape^>(geometryObject);
				XbimSetObject^ occSet = dynamic_cast<XbimSetObject^>(geometryObject);
				if (occShape != nullptr)
					result->Add(occShape->Transformed(transformation));
				else if (occSet != nullptr)
					result->Add(occSet->Transformed(transformation));
			}
			return result;
		}

		IXbimGeometryObject^ XbimGeometryObjectSet::Moved(IIfcPlacement^ placement)
		{
			if (!IsValid) return this;
			XbimGeometryObjectSet^ result = gcnew XbimGeometryObjectSet(_modelServices);
			result->Tag = Tag;
			for each (IXbimGeometryObject ^ geometryObject in geometryObjects)
			{
				XbimOccShape^ occShape = dynamic_cast<XbimOccShape^>(geometryObject);
				XbimSetObject^ occSet = dynamic_cast<XbimSetObject^>(geometryObject);
				if (occShape != nullptr)
					result->Add(occShape->Moved(placement));
				else if (occSet != nullptr)
					result->Add(occSet->Moved(placement));
			}
			return result;
		}

		IXbimGeometryObject^ XbimGeometryObjectSet::Moved(IIfcObjectPlacement^ objectPlacement, ILogger^ logger)
		{
			if (!IsValid) return this;
			XbimGeometryObjectSet^ result = gcnew XbimGeometryObjectSet(_modelServices);
			result->Tag = Tag;
			for each (IXbimGeometryObject ^ geometryObject in geometryObjects)
			{
				XbimOccShape^ occShape = dynamic_cast<XbimOccShape^>(geometryObject);
				XbimSetObject^ occSet = dynamic_cast<XbimSetObject^>(geometryObject);
				if (occShape != nullptr)
					result->Add(occShape->Moved(objectPlacement, logger));
				else if (occSet != nullptr)
					result->Add(occSet->Moved(objectPlacement, logger));
			}
			return result;
		}

		XbimGeometryObjectSet::operator TopoDS_Shape ()
		{
			return CreateCompound(geometryObjects);
		}

		IXCompound^ XbimGeometryObjectSet::ToXCompound()
		{
			return gcnew Xbim::Geometry::BRep::XCompound(XbimGeometryObjectSet::CreateCompound(geometryObjects));
		}

		void XbimGeometryObjectSet::Mesh(IXbimMeshReceiver^ mesh, double precision, double deflection, double angle)
		{
			for each (IXbimGeometryObject ^ geometryObject  in geometryObjects)
			{
				XbimSetObject^ objSet = dynamic_cast<XbimSetObject^>(geometryObject);
				XbimOccShape^ occObject = dynamic_cast<XbimOccShape^>(geometryObject);
				if (objSet != nullptr)
					objSet->Mesh(mesh, precision, deflection, angle);
				else if (occObject != nullptr)
					occObject->Mesh(mesh, precision, deflection, angle);
				else
					throw gcnew System::Exception("Unsupported geometry type cannot be meshed");
			}
		}

		IXbimGeometryObject^ XbimGeometryObjectSet::Transform(XbimMatrix3D matrix3D)
		{
			List<IXbimGeometryObject^>^ result = gcnew List<IXbimGeometryObject^>(geometryObjects->Count);
			for each (IXbimGeometryObject ^ geomObj in geometryObjects)
			{
				result->Add(geomObj->Transform(matrix3D));
			}
			return gcnew XbimGeometryObjectSet(result, _modelServices);
		}

		IXbimGeometryObject^ XbimGeometryObjectSet::TransformShallow(XbimMatrix3D matrix3D)
		{
			List<IXbimGeometryObject^>^ result = gcnew List<IXbimGeometryObject^>(geometryObjects->Count);
			for each (IXbimGeometryObject ^ geomObj in geometryObjects)
			{
				result->Add(((XbimGeometryObject^)geomObj)->TransformShallow(matrix3D));
			}
			return gcnew XbimGeometryObjectSet(result, _modelServices);
		}

		XbimRect3D XbimGeometryObjectSet::BoundingBox::get()
		{
			XbimRect3D result = XbimRect3D::Empty;
			for each (IXbimGeometryObject ^ geomObj in geometryObjects)
			{
				XbimRect3D bbox = geomObj->BoundingBox;
				if (result.IsEmpty) result = bbox;
				else
					result.Union(bbox);
			}
			return result;
		}

		IXbimSolidSet^ XbimGeometryObjectSet::Solids::get()
		{
			XbimSolidSet^ solids = gcnew XbimSolidSet(_modelServices); //we need to avoid this by passing the model service through
			for each (IXbimGeometryObject ^ geomObj in geometryObjects)
			{
				XbimOccShape^ occ = dynamic_cast<XbimOccShape^>(geomObj);
				XbimSolidSet^ ss = dynamic_cast<XbimSolidSet^>(geomObj);
				if (occ != nullptr)
				{
					TopTools_IndexedMapOfShape map;
					TopExp::MapShapes(occ, TopAbs_SOLID, map);
					for (int i = 1; i <= map.Extent(); i++)
						solids->Add(gcnew XbimSolid(TopoDS::Solid(map(i)), _modelServices));
				}
				else if (ss != nullptr)
				{
					for each (XbimSolid ^ solid in ss)
					{
						solids->Add(solid);
					}
				}

			}
			return solids;
		}

		IXbimShellSet^ XbimGeometryObjectSet::Shells::get()
		{
			List<IXbimShell^>^ shells = gcnew List<IXbimShell^>();
			for each (IXbimGeometryObject ^ geomObj in geometryObjects)
			{
				XbimOccShape^ occ = dynamic_cast<XbimOccShape^>(geomObj);
				XbimShellSet^ ss = dynamic_cast<XbimShellSet^>(geomObj);
				if (occ != nullptr)
				{
					TopTools_IndexedMapOfShape map;
					TopExp::MapShapes(occ, TopAbs_SHELL, map);
					for (int i = 1; i <= map.Extent(); i++)
						shells->Add(gcnew XbimShell(TopoDS::Shell(map(i)), _modelServices));
				}
				else if (ss != nullptr)
				{
					for each (XbimShell ^ shell in ss)
					{
						shells->Add(shell);
					}
				}
			}
			return gcnew XbimShellSet(shells, _modelServices);

		}

		IXbimFaceSet^ XbimGeometryObjectSet::Faces::get()
		{
			List<IXbimFace^>^ faces = gcnew List<IXbimFace^>();
			for each (IXbimGeometryObject ^ geomObj in geometryObjects)
			{
				XbimOccShape^ occ = dynamic_cast<XbimOccShape^>(geomObj);
				XbimFaceSet^ fs = dynamic_cast<XbimFaceSet^>(geomObj);
				if (occ != nullptr)
				{
					TopTools_IndexedMapOfShape map;
					TopExp::MapShapes(occ, TopAbs_FACE, map);
					for (int i = 1; i <= map.Extent(); i++)
						faces->Add(gcnew XbimFace(TopoDS::Face(map(i)), _modelServices));
				}
				else if (fs != nullptr)
				{
					for each (XbimFace ^ face in fs)
					{
						faces->Add(face);
					}
				}
			}
			return gcnew XbimFaceSet(faces, _modelServices);
		}

		IXbimEdgeSet^ XbimGeometryObjectSet::Edges::get()
		{
			List<IXbimEdge^>^ edges = gcnew List<IXbimEdge^>();
			for each (IXbimGeometryObject ^ geomObj in geometryObjects)
			{
				XbimOccShape^ occ = dynamic_cast<XbimOccShape^>(geomObj);
				XbimEdgeSet^ es = dynamic_cast<XbimEdgeSet^>(geomObj);
				if (occ != nullptr)
				{
					TopTools_IndexedMapOfShape map;
					TopExp::MapShapes(occ, TopAbs_EDGE, map);
					for (int i = 1; i <= map.Extent(); i++)
						edges->Add(gcnew XbimEdge(TopoDS::Edge(map(i)), _modelServices));
				}
				else if (es != nullptr)
				{
					for each (XbimEdge ^ edge in es)
					{
						edges->Add(edge);
					}
				}
			}
			return gcnew XbimEdgeSet(edges, _modelServices);
		}

		IXbimVertexSet^ XbimGeometryObjectSet::Vertices::get()
		{
			List<IXbimVertex^>^ vertices = gcnew List<IXbimVertex^>();
			for each (IXbimGeometryObject ^ geomObj in geometryObjects)
			{
				XbimOccShape^ occ = dynamic_cast<XbimOccShape^>(geomObj);
				XbimVertexSet^ vs = dynamic_cast<XbimVertexSet^>(geomObj);
				if (occ != nullptr)
				{
					TopTools_IndexedMapOfShape map;
					TopExp::MapShapes(occ, TopAbs_VERTEX, map);
					for (int i = 1; i <= map.Extent(); i++)
						vertices->Add(gcnew XbimVertex(TopoDS::Vertex(map(i))));
				}
				else if (vs != nullptr)
				{
					for each (XbimVertex ^ vertex in vs)
					{
						vertices->Add(vertex);
					}
				}
			}
			return gcnew XbimVertexSet(vertices, _modelServices);
		}
		void XbimGeometryObjectSet::GetShapeList(TopTools_ListOfShape& shapes)
		{
			for each (IXbimGeometryObject ^ geom in geometryObjects)
			{
				XbimCompound^ compound = dynamic_cast<XbimCompound^>(geom);
				XbimSolid^ solid = dynamic_cast<XbimSolid^>(geom);
				XbimShell^ shell = dynamic_cast<XbimShell^>(geom);
				XbimGeometryObjectSet^ geomSet = dynamic_cast<XbimGeometryObjectSet^>(geom);
				XbimFace^ face = dynamic_cast<XbimFace^>(geom);
				XbimWire^ wire = dynamic_cast<XbimWire^>(geom);
				XbimEdge^ edge = dynamic_cast<XbimEdge^>(geom);
				if (compound != nullptr) shapes.Append(compound);
				else if (solid != nullptr) shapes.Append(solid);
				else if (shell != nullptr) shapes.Append(shell);
				else if (geomSet != nullptr) geomSet->GetShapeList(shapes);
				else if (face != nullptr) shapes.Append(face); 
				else if (wire != nullptr) shapes.Append(wire);
				else if (edge != nullptr) shapes.Append(edge);
				else
					_modelServices->LogInformation("Unknown shape type in XbimGeometryObjectSet");
			}
		}

		bool XbimGeometryObjectSet::ParseGeometry(System::Collections::Generic::IEnumerable<IXbimGeometryObject^>^ geomObjects, TopTools_ListOfShape& toBeProcessed, Bnd_Array1OfBox& aBoxes,
			TopoDS_Shell& passThrough, double tolerance)
		{
			ShapeFix_ShapeTolerance FTol;
			BRep_Builder builder;
			TopoDS_Shell shellBeingBuilt;
			builder.MakeShell(shellBeingBuilt);
			bool hasFacesToProcess = false;
			bool hasContent = false;
			for each (IXbimGeometryObject ^ iGeom in geomObjects)
			{
				// four types of geometry are found in the geomObjects
				XbimShell^ shell = dynamic_cast<XbimShell^>(iGeom);
				XbimSolid^ solid = dynamic_cast<XbimSolid^>(iGeom);
				System::Collections::Generic::IEnumerable<IXbimGeometryObject^>^ geomSet = dynamic_cast<System::Collections::Generic::IEnumerable<IXbimGeometryObject^>^>(iGeom);
				XbimFace^ face = dynamic_cast<XbimFace^>(iGeom);

				// type 1
				if (solid != nullptr)
				{
					FTol.LimitTolerance(solid, tolerance);
					toBeProcessed.Append(solid);
					hasContent = true;
				}
				// type 2
				else if (shell != nullptr)
				{
					for (TopExp_Explorer expl(shell, TopAbs_FACE); expl.More(); expl.Next())
					{
						Bnd_Box bbFace;
						BRepBndLib::Add(expl.Current(), bbFace);
						for (int i = 1; i <= aBoxes.Length(); i++)
						{
							if (!bbFace.IsOut(aBoxes(i)))
							{
								//check if the face is not sitting on the cut
								builder.Add(shellBeingBuilt, expl.Current());
								hasFacesToProcess = true;
							}
							else
								builder.Add(passThrough, expl.Current());
						}
					}
				}
				// type 3
				else if (geomSet != nullptr)
				{
					// iteratively trying again
					if (ParseGeometry(geomSet, toBeProcessed, aBoxes, passThrough, tolerance))
					{
						hasContent = true;
					}
				}
				// type 4
				else if (face != nullptr)
				{
					Bnd_Box bbFace;
					BRepBndLib::Add(face, bbFace);
					for (int i = 1; i <= aBoxes.Length(); i++)
					{
						if (!bbFace.IsOut(aBoxes(i)))
						{
							//check if the face is not sitting on the cut
							builder.Add(shellBeingBuilt, face);
							hasFacesToProcess = true;
						}
						else
							builder.Add(passThrough, face);
					}
				}
			}
			if (hasFacesToProcess)
			{
				hasContent = true;
				//sew the bits we are going to cut

				TopoDS_Shape shape = shellBeingBuilt;
				std::string errMsg;
				if (!XbimNativeApi::SewShape(shape, tolerance, XbimGeometryCreator::BooleanTimeOut, errMsg))
				{
					/*String^ err = gcnew String(errMsg.c_str());
					XbimGeometryCreator::LogWarning(logger, nullptr, "Failed to sew shape: " + err);*/
				}
				FTol.LimitTolerance(shape, tolerance);
				toBeProcessed.Append(shape);
			}
			return hasContent;
		}



		System::String^ XbimGeometryObjectSet::ToBRep::get()
		{
			std::ostringstream oss;
			TopoDS_Compound comp = CreateCompound(geometryObjects);
			BRepTools::Write(comp, oss);
			return gcnew System::String(oss.str().c_str());
		}

		TopoDS_Compound XbimGeometryObjectSet::CreateCompound(System::Collections::Generic::IEnumerable<IXbimGeometryObject^>^ geomObjects)
		{
			BRep_Builder builder;
			TopoDS_Compound bodyCompound;
			builder.MakeCompound(bodyCompound);
			for each (IXbimGeometryObject ^ gObj in geomObjects)
			{
				XbimOccShape^ shape = dynamic_cast<XbimOccShape^>(gObj);
				System::Collections::Generic::IEnumerable<IXbimGeometryObject^>^ geomSet = dynamic_cast<System::Collections::Generic::IEnumerable<IXbimGeometryObject^>^>(gObj);
				if (shape != nullptr)
				{
					builder.Add(bodyCompound, (XbimOccShape^)gObj);
				}
				else if (geomSet != nullptr)
					builder.Add(bodyCompound, CreateCompound(geomSet));
			}
			return bodyCompound;
		}


		IXbimGeometryObjectSet^ XbimGeometryObjectSet::Cut(IXbimSolidSet^ solids, double tolerance, ILogger^ logger)
		{
			TopoDS_ListOfShape arguments;
			TopoDS_ListOfShape tools;
			GetShapeList(arguments);	
			if (arguments.Size() == 0) return gcnew XbimGeometryObjectSet(_modelServices);
			for each (auto solid in solids)
				tools.Append(static_cast<XbimSolid^>(solid));
			if (tolerance > 0)
				return gcnew XbimGeometryObjectSet(_modelServices->GetBooleanFactory()->Cut(arguments, tools, tolerance), _modelServices);
			else
				return gcnew XbimGeometryObjectSet(_modelServices->GetBooleanFactory()->Cut(arguments, tools), _modelServices);
		}

		IXbimGeometryObjectSet^ XbimGeometryObjectSet::Cut(IXbimSolid^ solid, double tolerance, ILogger^ logger)
		{
			TopoDS_ListOfShape arguments;
			TopoDS_ListOfShape tools;
			GetShapeList(arguments);
			tools.Append(static_cast<XbimSolid^>(solid));
			if (tolerance > 0)
				return gcnew XbimGeometryObjectSet(_modelServices->GetBooleanFactory()->Cut(arguments, tools, tolerance), _modelServices);
			else
				return gcnew XbimGeometryObjectSet(_modelServices->GetBooleanFactory()->Cut(arguments, tools), _modelServices);
		}


		IXbimGeometryObjectSet^ XbimGeometryObjectSet::Union(IXbimSolidSet^ solids, double tolerance, ILogger^ logger)
		{
			TopoDS_ListOfShape arguments;
			TopoDS_ListOfShape tools;
			GetShapeList(arguments);
			for each (auto solid in solids)
				tools.Append(static_cast<XbimSolid^>(solid));
			if (tolerance > 0)
				return gcnew XbimGeometryObjectSet(_modelServices->GetBooleanFactory()->Union(arguments, tools, tolerance), _modelServices);
			else
				return gcnew XbimGeometryObjectSet(_modelServices->GetBooleanFactory()->Union(arguments, tools), _modelServices);
		}

		IXbimGeometryObjectSet^ XbimGeometryObjectSet::Union(IXbimSolid^ solid, double tolerance, ILogger^ logger)
		{
			TopoDS_ListOfShape arguments;
			TopoDS_ListOfShape tools;
			GetShapeList(arguments);
			tools.Append(static_cast<XbimSolid^>(solid));
			if (tolerance > 0)
				return gcnew XbimGeometryObjectSet(_modelServices->GetBooleanFactory()->Union(arguments, tools, tolerance), _modelServices);
			else
				return gcnew XbimGeometryObjectSet(_modelServices->GetBooleanFactory()->Union(arguments, tools), _modelServices);
		}

		IXbimGeometryObjectSet^ XbimGeometryObjectSet::Intersection(IXbimSolidSet^ solids, double tolerance, ILogger^ logger)
		{
			TopoDS_ListOfShape arguments;
			TopoDS_ListOfShape tools;
			GetShapeList(arguments);
			for each (auto solid in solids)
				tools.Append(static_cast<XbimSolid^>(solid));
			if (tolerance > 0)
				return gcnew XbimGeometryObjectSet(_modelServices->GetBooleanFactory()->Intersect(arguments, tools, tolerance), _modelServices);
			else
				return gcnew XbimGeometryObjectSet(_modelServices->GetBooleanFactory()->Intersect(arguments, tools), _modelServices);
		}

		IXbimGeometryObjectSet^ XbimGeometryObjectSet::Intersection(IXbimSolid^ solid, double tolerance, ILogger^ logger)
		{
			TopoDS_ListOfShape arguments;
			TopoDS_ListOfShape tools;
			GetShapeList(arguments);
			tools.Append(static_cast<XbimSolid^>(solid));
			if (tolerance > 0)
				return gcnew XbimGeometryObjectSet(_modelServices->GetBooleanFactory()->Intersect(arguments, tools, tolerance), _modelServices);
			else
				return gcnew XbimGeometryObjectSet(_modelServices->GetBooleanFactory()->Intersect(arguments, tools), _modelServices);
		}
	}
}
