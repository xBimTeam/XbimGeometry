
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopExp.hxx>
#include <BRepTools.hxx>
#include <BRep_Builder.hxx>
#include <Standard_NotImplemented.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <ShapeFix_ShapeTolerance.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <ShapeFix_Shape.hxx>
#include <ShapeUpgrade_UnifySameDomain.hxx>

#include <BRepBuilderAPI_MakeSolid.hxx>
#include <BOPAlgo_PaveFiller.hxx>
#include <BOPAlgo_BOP.hxx>
#include <OSD_OpenFile.hxx>
#include <algorithm>
#include "XbimConvert.h"
#include "XbimSolidSet.h"
#include "XbimShellSet.h"
#include "XbimCompound.h"
#include "XbimGeometryCreator.h"
#include "XbimOccWriter.h"
#include "XbimProgressMonitor.h"
#include "./Factories/BooleanFactory.h"

using namespace System::Linq;
using namespace System::Threading;
#include "XbimGeometryObjectSet.h"
#include "./BRep/XCompound.h"
namespace Xbim
{
	namespace Geometry
	{
		IXCompound^ XbimSolidSet::ToXCompound()
		{
			return gcnew Xbim::Geometry::BRep::XCompound(XbimGeometryObjectSet::CreateCompound(Enumerable::Cast<IXbimGeometryObject^>(solids)));
		}


		System::String^ XbimSolidSet::ToBRep::get()
		{
			if (!IsValid) return System::String::Empty;
			std::ostringstream oss;
			BRep_Builder b;
			TopoDS_Compound comp;
			b.MakeCompound(comp);

			for each (IXbimSolid ^ solid in solids)
			{
				if (dynamic_cast<XbimSolid^>(solid))
				{
					b.Add(comp, (XbimSolid^)solid);
				}
			}
			BRepTools::Write(comp, oss);
			return gcnew System::String(oss.str().c_str());
		}

		XbimSolidSet::XbimSolidSet(const TopoDS_Shape& shape, ModelGeometryService^ modelService) : XbimSetObject(modelService)
		{
			InitSolidsFromShape(shape);

		}

		void XbimSolidSet::InitSolidsFromShape(const TopoDS_Shape& shape)
		{
			TopTools_IndexedMapOfShape map;
			TopExp::MapShapes(shape, TopAbs_SOLID, map);
			solids = gcnew  List<IXbimSolid^>(map.Extent());
			for (int i = 1; i <= map.Extent(); i++)
				solids->Add(gcnew XbimSolid(TopoDS::Solid(map(i)), _modelServices));
		}

		XbimSolidSet::XbimSolidSet(XbimCompound^ shape, ModelGeometryService^ modelService) : XbimSetObject(modelService)
		{

			if (shape->IsValid)
			{
				TopTools_IndexedMapOfShape solidMap;

				TopExp::MapShapes(shape, TopAbs_SOLID, solidMap);
				solids = gcnew  List<IXbimSolid^>(solidMap.Extent());
				for (int i = 1; i <= solidMap.Extent(); i++)
					solids->Add(gcnew XbimSolid(TopoDS::Solid(solidMap(i)), _modelServices));
				//srl let shells through to improve final content as some geometries are badly formed and don't make a solid, even though they are supposed to
				/*TopExp_Explorer te(shape, TopAbs_SHELL, TopAbs_SOLID);
				while (te.More())
				{
					const TopoDS_Shell& shell = TopoDS::Shell(te.Current());
					ShapeFix_Solid sf;
					TopoDS_Solid s = sf.SolidFromShell(shell);
					solids->Add(gcnew XbimSolid(s));
				}*/
			}
			System::GC::KeepAlive(shape);
		}

		XbimSolidSet::XbimSolidSet(ModelGeometryService^ modelService) : XbimSetObject(modelService)
		{
			;
			solids = gcnew  List<IXbimSolid^>();
		}
		XbimSolidSet::XbimSolidSet(IIfcCsgSolid^ repItem, ILogger^ logger, ModelGeometryService^ modelService) : XbimSetObject(modelService)
		{
			Init(repItem, logger);
		}

		XbimSolidSet::XbimSolidSet(IXbimSolid^ solid, ModelGeometryService^ modelService) : XbimSetObject(modelService)
		{
			solids = gcnew  List<IXbimSolid^>(1);
			solids->Add(solid);
		}
		XbimSolidSet::XbimSolidSet(IIfcBooleanClippingResult^ solid, ILogger^ logger, ModelGeometryService^ modelService) : XbimSetObject(modelService)
		{
			Init(solid, logger);
		}

		XbimSolidSet::XbimSolidSet(IIfcBooleanResult^ boolOp, ILogger^ logger, ModelGeometryService^ modelService) : XbimSetObject(modelService)
		{

			Init(boolOp, logger);
		}

		XbimSolidSet::XbimSolidSet(IIfcBooleanOperand^ boolOp, ILogger^ logger, ModelGeometryService^ modelService) : XbimSetObject(modelService)
		{
			Init(boolOp, logger);
		}

		XbimSolidSet::XbimSolidSet(IIfcManifoldSolidBrep^ solid, ILogger^ logger, ModelGeometryService^ modelService) : XbimSetObject(modelService)
		{
			XbimCompound^ comp = gcnew XbimCompound(solid, logger, _modelServices);
			Init(comp, solid, logger);

		}
		XbimSolidSet::XbimSolidSet(IIfcFacetedBrep^ solid, ILogger^ logger, ModelGeometryService^ modelService) : XbimSetObject(modelService)
		{
			XbimCompound^ comp = gcnew XbimCompound(solid, logger, _modelServices);
			Init(comp, solid, logger);
		}

		XbimSolidSet::XbimSolidSet(IIfcFacetedBrepWithVoids^ solid, ILogger^ logger, ModelGeometryService^ modelService) : XbimSetObject(modelService)
		{
			XbimCompound^ comp = gcnew XbimCompound(solid, logger, _modelServices);
			Init(comp, solid, logger);
		}

		XbimSolidSet::XbimSolidSet(IIfcClosedShell^ solid, ILogger^ logger, ModelGeometryService^ modelService) : XbimSetObject(modelService)
		{
			XbimCompound^ comp = gcnew XbimCompound(solid, logger, _modelServices);
			Init(comp, solid, logger);
		}

		XbimSolidSet::XbimSolidSet(System::Collections::Generic::IEnumerable<IXbimSolid^>^ solids, ModelGeometryService^ modelService) : XbimSetObject(modelService)
		{
			this->solids = gcnew  List<IXbimSolid^>(solids);;
		}


		XbimSolidSet::XbimSolidSet(IIfcSweptAreaSolid^ repItem, ILogger^ logger, ModelGeometryService^ modelService) : XbimSetObject(modelService)
		{
			Init(repItem, logger);
		}
		XbimSolidSet::XbimSolidSet(IIfcRevolvedAreaSolid^ solid, ILogger^ logger, ModelGeometryService^ modelService) : XbimSetObject(modelService)
		{
			Init(solid, logger);
		}

		XbimSolidSet::XbimSolidSet(IIfcTriangulatedFaceSet^ IIfcSolid, ILogger^ logger, ModelGeometryService^ modelService) : XbimSetObject(modelService)
		{
			Init(IIfcSolid, logger);
		}
		XbimSolidSet::XbimSolidSet(IIfcPolygonalFaceSet^ IIfcSolid, ILogger^ logger, ModelGeometryService^ modelService) : XbimSetObject(modelService)
		{
			Init(IIfcSolid, logger);
		}

		XbimSolidSet::XbimSolidSet(IIfcFaceBasedSurfaceModel^ solid, ILogger^ logger, ModelGeometryService^ modelService) : XbimSetObject(modelService)
		{
			Init(solid, logger);
		}

		XbimSolidSet::XbimSolidSet(IIfcShellBasedSurfaceModel^ solid, ILogger^ logger, ModelGeometryService^ modelService) : XbimSetObject(modelService)
		{
			Init(solid, logger);
		}

		XbimSolidSet::XbimSolidSet(IIfcExtrudedAreaSolid^ repItem, ILogger^ logger, ModelGeometryService^ modelService) : XbimSetObject(modelService)
		{
			Init(repItem, logger);

		}

		XbimSolidSet::XbimSolidSet(IIfcSurfaceCurveSweptAreaSolid^ repItem, ILogger^ logger, ModelGeometryService^ modelService) : XbimSetObject(modelService)
		{
			Init(repItem, logger);
		}

		void XbimSolidSet::Reverse()
		{
			this->solids->Reverse();
		}

		XbimSolidSet^ XbimSolidSet::BuildClippingList(IIfcBooleanClippingResult^ solid, List<IIfcBooleanOperand^>^ clipList, ILogger^ logger, ModelGeometryService^ modelServices)
		{
			IIfcBooleanOperand^ fOp = solid->FirstOperand;
			IIfcBooleanOperand^ sOp = solid->SecondOperand;

			IIfcBooleanClippingResult^ boolClip = dynamic_cast<IIfcBooleanClippingResult^>(fOp);
			if (boolClip != nullptr)
			{
				clipList->Add(sOp);
				return XbimSolidSet::BuildClippingList(boolClip, clipList, logger, modelServices);
			}
			else //we need to build the solid
			{
				clipList->Add(sOp);
				clipList->Reverse();
				return gcnew XbimSolidSet(fOp, logger, modelServices);
			}
		}

		///If the shape contains one or more solids these are added to the collection
		void XbimSolidSet::Add(IXbimGeometryObject^ shape)
		{
			if (solids == nullptr) solids = gcnew List<IXbimSolid^>();
			IXbimSolid^ solid = dynamic_cast<IXbimSolid^>(shape);
			if (solid != nullptr && solid->IsValid)
			{
				return solids->Add(solid);
			}

			IXbimSolidSet^ solidSet = dynamic_cast<IXbimSolidSet^>(shape);
			if (solidSet != nullptr) return solids->AddRange(solidSet);
			IXbimGeometryObjectSet^ geomSet = dynamic_cast<IXbimGeometryObjectSet^>(shape);
			if (geomSet != nullptr)
			{
				//				//make sure any sewing has been performed if the set is a compound object
				//				XbimCompound^ compound = dynamic_cast<XbimCompound^>(geomSet);
				//
				//				if (compound != nullptr)
				//					if (!compound->Sew())
				//					{
				//						_isSimplified = true; //set flag true to say the solid set has been simplified and the user should be warned
				//#ifndef OCC_6_9_SUPPORTED		
				//
				//						return; //don't add it we cannot really make it into a solid and will cause boolean operation errors,
				//#endif
				//					}

				for each (IXbimGeometryObject ^ geom in geomSet)
				{

					XbimSolid^ nestedSolid = dynamic_cast<XbimSolid^>(geom);
					XbimCompound^ nestedCompound = dynamic_cast<XbimCompound^>(geom);
					XbimShell^ shell = dynamic_cast<XbimShell^>(geom);
					if (nestedSolid != nullptr && !nestedSolid->IsEmpty)
						solids->Add(nestedSolid);
					else if (nestedCompound != nullptr)
					{
						nestedCompound->Sew();
						for each (IXbimGeometryObject ^ nestedGeom in nestedCompound)
						{
							XbimSolid^ subSolid = dynamic_cast<XbimSolid^>(nestedGeom);
							XbimShell^ subShell = dynamic_cast<XbimShell^>(nestedGeom);
							if (subSolid != nullptr && !subSolid->IsEmpty)
								solids->Add(subSolid);
							else if (subShell != nullptr && subShell->IsValid)
								solids->Add(subShell->MakeSolid());
						}
					}
					else if (shell != nullptr && shell->IsValid)
					{
						XbimSolid^ s = (XbimSolid^)shell->MakeSolid();
						solids->Add(s);
					}

				}

				return;
			}
			XbimShell^ shell = dynamic_cast<XbimShell^>(shape);
			if (shell != nullptr && shell->IsValid/* && shell->IsClosed*/)
				return solids->Add(shell->MakeSolid());

		}

		IXbimSolidSet^ XbimSolidSet::Range(int start, int count)
		{
			XbimSolidSet^ ss = gcnew XbimSolidSet(_modelServices);
			for (int i = start; i < System::Math::Min(solids->Count, count); i++)
			{
				ss->Add(solids[i]);
			}
			return ss;
		}


		IXbimSolid^ XbimSolidSet::First::get()
		{
			if (Count == 0) return nullptr;
			return solids[0];
		}

		int XbimSolidSet::Count::get()
		{
			return solids == nullptr ? 0 : solids->Count;
		}

		double XbimSolidSet::Volume::get()
		{
			double vol = 0;
			if (IsValid)
			{
				for each (XbimSolid ^ solid in solids)
				{
					vol += solid->Volume;
				}
			}
			return vol;
		}

		IXbimGeometryObject^ XbimSolidSet::Transform(XbimMatrix3D matrix3D)
		{
			if (!IsValid) return gcnew XbimSolidSet(_modelServices);
			List<IXbimSolid^>^ result = gcnew List<IXbimSolid^>(solids->Count);
			for each (IXbimGeometryObject ^ solid in solids)
			{
				result->Add((IXbimSolid^)solid->Transform(matrix3D));
			}
			return gcnew XbimSolidSet(result, _modelServices);
		}

		IXbimGeometryObject^ XbimSolidSet::TransformShallow(XbimMatrix3D matrix3D)
		{
			if (!IsValid) return gcnew XbimSolidSet(_modelServices);
			List<IXbimSolid^>^ result = gcnew List<IXbimSolid^>(solids->Count);
			for each (IXbimGeometryObject ^ solid in solids)
			{
				result->Add((IXbimSolid^)((XbimSolid^)solid)->TransformShallow(matrix3D));
			}
			return gcnew XbimSolidSet(result, _modelServices);
		}


		void XbimSolidSet::Move(IIfcAxis2Placement3D^ position)
		{
			if (!IsValid) return;
			for each (IXbimSolid ^ solid in solids)
			{
				((XbimSolid^)solid)->Move(position);
			}
		}

		IXbimGeometryObject^ XbimSolidSet::Transformed(IIfcCartesianTransformationOperator^ transformation)
		{
			if (!IsValid) return this;
			XbimSolidSet^ result = gcnew XbimSolidSet(_modelServices);
			for each (XbimSolid ^ solid in solids)
				result->Add(solid->Transformed(transformation));
			return result;
		}

		IXbimGeometryObject^ XbimSolidSet::Moved(IIfcPlacement^ placement)
		{
			if (!IsValid) return this;
			XbimSolidSet^ result = gcnew XbimSolidSet(_modelServices);
			TopLoc_Location loc = XbimConvert::ToLocation(placement);
			for each (IXbimSolid ^ solid in solids)
			{
				XbimSolid^ copy = gcnew XbimSolid((XbimSolid^)solid, Tag, _modelServices);
				copy->Move(loc);
				result->Add(copy);
			}
			return result;
		}

		IXbimGeometryObject^ XbimSolidSet::Moved(IIfcObjectPlacement^ objectPlacement, ILogger^ logger)
		{
			if (!IsValid) return this;
			XbimSolidSet^ result = gcnew XbimSolidSet(_modelServices);
			TopLoc_Location loc = XbimConvert::ToLocation(objectPlacement, logger, _modelServices);
			for each (IXbimSolid ^ solid in solids)
			{
				XbimSolid^ copy = gcnew XbimSolid((XbimSolid^)solid, Tag, _modelServices);
				copy->Move(loc);
				result->Add(copy);
			}
			return result;
		}

		void XbimSolidSet::Mesh(IXbimMeshReceiver^ mesh, double precision, double deflection, double angle)
		{
			for each (IXbimSolid ^ solid  in solids)
			{
				((XbimSolid^)solid)->Mesh(mesh, precision, deflection, angle);
			}
		}

		XbimSolidSet::operator TopoDS_Shape()
		{
			BRep_Builder builder;
			TopoDS_Compound bodyCompound;
			builder.MakeCompound(bodyCompound);
			for each (IXbimSolid ^ solid in solids)
			{
				builder.Add(bodyCompound, (XbimSolid^)solid);
			}
			return bodyCompound;
		}



		XbimRect3D XbimSolidSet::BoundingBox::get()
		{
			XbimRect3D result = XbimRect3D::Empty;
			if (IsValid)
			{
				for each (IXbimGeometryObject ^ geomObj in solids)
				{
					XbimRect3D bbox = geomObj->BoundingBox;
					if (result.IsEmpty) result = bbox;
					else
						result.Union(bbox);
				}
			}
			return result;
		}

		System::Collections::Generic::IEnumerator<IXbimSolid^>^ XbimSolidSet::GetEnumerator()
		{
			if (solids == nullptr) return System::Linq::Enumerable::Empty<IXbimSolid^>()->GetEnumerator();
			return solids->GetEnumerator();
		}

		bool XbimSolidSet::IsPolyhedron::get()
		{
			if (!IsValid) return false;
			for each (IXbimSolid ^ solid in solids)
			{
				if (!solid->IsPolyhedron) return false;
			}
			return true;
		}



		IXbimSolidSet^ XbimSolidSet::Cut(IXbimSolidSet^ solidTools, double tolerance, ILogger^ logger)
		{
			TopoDS_ListOfShape arguments;
			TopoDS_ListOfShape tools;
			for each (auto solid in solids)
				arguments.Append(static_cast<XbimSolid^>(solid));
			for each (auto solid in solidTools)
				tools.Append(static_cast<XbimSolid^>(solid));
			if (tolerance > 0)
				return gcnew XbimSolidSet(_modelServices->GetBooleanFactory()->Cut(arguments, tools, tolerance), _modelServices);
			else
				return gcnew XbimSolidSet(_modelServices->GetBooleanFactory()->Cut(arguments, tools), _modelServices);
		}
		
		IXbimSolidSet^ XbimSolidSet::Union(IXbimSolidSet^ solidTools, double tolerance, ILogger^ logger)
		{
			TopoDS_ListOfShape arguments;
			TopoDS_ListOfShape tools;
			for each (auto solid in solids)
				arguments.Append(static_cast<XbimSolid^>(solid));
			for each (auto solid in solidTools)
				tools.Append(static_cast<XbimSolid^>(solid));
			if (tolerance > 0)
				return gcnew XbimSolidSet(_modelServices->GetBooleanFactory()->Union(arguments, tools, tolerance), _modelServices);
			else
				return gcnew XbimSolidSet(_modelServices->GetBooleanFactory()->Union(arguments, tools), _modelServices);
		}



		IXbimSolidSet^ XbimSolidSet::Intersection(IXbimSolidSet^ solidTools, double tolerance, ILogger^ logger)
		{
			TopoDS_ListOfShape arguments;
			TopoDS_ListOfShape tools;
			for each (auto solid in solids)
				arguments.Append(static_cast<XbimSolid^>(solid));
			for each (auto solid in solidTools)
				tools.Append(static_cast<XbimSolid^>(solid));
			if (tolerance > 0)
				return gcnew XbimSolidSet(_modelServices->GetBooleanFactory()->Intersect(arguments, tools, tolerance), _modelServices);
			else
				return gcnew XbimSolidSet(_modelServices->GetBooleanFactory()->Intersect(arguments, tools), _modelServices);
		}


		IXbimSolidSet^ XbimSolidSet::Cut(IXbimSolid^ solid, double tolerance, ILogger^ logger)
		{
			TopoDS_ListOfShape arguments;
			TopoDS_ListOfShape tools;
			for each (auto solid in solids)
				arguments.Append(static_cast<XbimSolid^>(solid));
			tools.Append(static_cast<XbimSolid^>(solid));
			if (tolerance > 0)
				return gcnew XbimSolidSet(_modelServices->GetBooleanFactory()->Cut(arguments, tools, tolerance), _modelServices);
			else
				return gcnew XbimSolidSet(_modelServices->GetBooleanFactory()->Cut(arguments, tools), _modelServices);
		}

		IXbimSolidSet^ XbimSolidSet::Union(IXbimSolid^ solid, double tolerance, ILogger^ logger)
		{
			TopoDS_ListOfShape arguments;
			TopoDS_ListOfShape tools;
			for each (auto solid in solids)
				arguments.Append(static_cast<XbimSolid^>(solid));
			tools.Append(static_cast<XbimSolid^>(solid));
			if (tolerance > 0)
				return gcnew XbimSolidSet(_modelServices->GetBooleanFactory()->Union(arguments, tools, tolerance), _modelServices);
			else
				return gcnew XbimSolidSet(_modelServices->GetBooleanFactory()->Union(arguments, tools), _modelServices);
		}

		IXbimSolidSet^ XbimSolidSet::Intersection(IXbimSolid^ solid, double tolerance, ILogger^ logger)
		{
			TopoDS_ListOfShape arguments;
			TopoDS_ListOfShape tools;
			for each (auto solid in solids)
				arguments.Append(static_cast<XbimSolid^>(solid));
			tools.Append(static_cast<XbimSolid^>(solid));
			if (tolerance > 0)
				return gcnew XbimSolidSet(_modelServices->GetBooleanFactory()->Intersect(arguments, tools, tolerance), _modelServices);
			else
				return gcnew XbimSolidSet(_modelServices->GetBooleanFactory()->Intersect(arguments, tools), _modelServices);
		}

		void XbimSolidSet::Init(XbimCompound^ comp, IPersistEntity^ entity, ILogger^ logger)
		{
			solids = gcnew  List<IXbimSolid^>();
			if (!comp->IsValid)
			{
				XbimGeometryCreator::LogWarning(logger, entity, "Empty or invalid solid");
			}
			else
			{
				Add(comp);
			}
		}

		void XbimSolidSet::Init(IIfcSweptAreaSolid^ repItem, ILogger^ logger)
		{
			IIfcCompositeProfileDef^ compProfile = dynamic_cast<IIfcCompositeProfileDef^>(repItem->SweptArea);

			if (compProfile != nullptr) //handle these as composite solids
			{
				int profileCount = Enumerable::Count(compProfile->Profiles);
				if (profileCount == 0)
				{
					XbimGeometryCreator::LogWarning(logger, repItem, "Invalid number of profiles. It must be 2 or more. Profile discarded");
					return;
				}
				if (profileCount == 1)
				{
					XbimGeometryCreator::LogInfo(logger, compProfile, "Invalid number of profiles. It must be 2 or more. A single Profile has been used");
					XbimSolid^ s = gcnew XbimSolid(repItem, logger, _modelServices);
					if (s->IsValid)
					{
						solids = gcnew List<IXbimSolid^>();
						solids->Add(s);
					}
					return;
				}
				solids = gcnew List<IXbimSolid^>();
				for each (IIfcProfileDef ^ profile in compProfile->Profiles)
				{
					XbimSolid^ aSolid = gcnew XbimSolid(repItem, profile, logger, _modelServices);
					if (aSolid->IsValid)
						solids->Add(aSolid);
				}
			}
			else
			{
				XbimSolid^ s = gcnew XbimSolid(repItem, logger, _modelServices);
				if (s->IsValid)
				{
					solids = gcnew List<IXbimSolid^>();
					solids->Add(s);
				}
			}
		}
		void XbimSolidSet::Init(IIfcRevolvedAreaSolid^ repItem, ILogger^ logger)
		{
			IIfcCompositeProfileDef^ compProfile = dynamic_cast<IIfcCompositeProfileDef^>(repItem->SweptArea);

			if (compProfile != nullptr) //handle these as composite solids
			{
				int profileCount = Enumerable::Count(compProfile->Profiles);
				if (profileCount == 0)
				{
					XbimGeometryCreator::LogWarning(logger, compProfile, "Invalid number of profiles. It must be 2 or more. Profile discarded");
					return;
				}
				if (profileCount == 1)
				{
					XbimGeometryCreator::LogInfo(logger, compProfile, "Invalid number of profiles. It must be 2 or more. A single Profile has been used");
					XbimSolid^ s = gcnew XbimSolid(repItem, logger, _modelServices);
					if (s->IsValid)
					{
						solids = gcnew List<IXbimSolid^>();
						solids->Add(s);
					}
					return;
				}
				solids = gcnew List<IXbimSolid^>();
				for each (IIfcProfileDef ^ profile in compProfile->Profiles)
				{
					XbimSolid^ aSolid = gcnew XbimSolid(repItem, profile, logger, _modelServices);
					if (aSolid->IsValid)
						solids->Add(aSolid);
				}
			}
			else
			{
				XbimSolid^ s = gcnew XbimSolid(repItem, logger, _modelServices);
				if (s->IsValid)
				{
					solids = gcnew List<IXbimSolid^>();
					solids->Add(s);
				}
			}
		}

		void XbimSolidSet::Init(IIfcTriangulatedFaceSet^ IIfcSolid, ILogger^ logger)
		{
			XbimCompound^ comp = gcnew XbimCompound(IIfcSolid, logger, _modelServices);
			solids = gcnew List<IXbimSolid^>();
			for each (IXbimSolid ^ xbimSolid in comp->Solids)
			{
				if (xbimSolid->IsValid)
					solids->Add(xbimSolid);
			}
		}
		void XbimSolidSet::Init(IIfcPolygonalFaceSet^ IIfcSolid, ILogger^ logger)
		{
			XbimCompound^ comp = gcnew XbimCompound(IIfcSolid, logger, _modelServices);
			solids = gcnew List<IXbimSolid^>();
			for each (IXbimSolid ^ xbimSolid in comp->Solids)
			{
				if (xbimSolid->IsValid)
					solids->Add(xbimSolid);
			}
			//upgrade any shells that are closed to solids
			TopExp_Explorer te(comp, TopAbs_SHELL, TopAbs_SOLID);
			while (te.More())
			{
				const TopoDS_Shell& shell = TopoDS::Shell(te.Current());
				if (shell.Closed())
				{
					BRepBuilderAPI_MakeSolid solidMaker(shell);
					if (solidMaker.IsDone())
					{
						solids->Add(gcnew XbimSolid(solidMaker.Solid(), _modelServices));
					}
				}
				te.Next();
			}
		}


		void XbimSolidSet::Init(IIfcFaceBasedSurfaceModel^ solid, ILogger^ logger)
		{
			XbimCompound^ comp = gcnew XbimCompound(solid, logger, _modelServices);
			solids = gcnew List<IXbimSolid^>();
			for each (IXbimSolid ^ xbimSolid in comp->Solids)
			{
				if (xbimSolid->IsValid)
					solids->Add(xbimSolid);
			}
			//upgrade any shells that are closed to solids
			TopExp_Explorer te(comp, TopAbs_SHELL, TopAbs_SOLID);
			while (te.More())
			{
				const TopoDS_Shell& shell = TopoDS::Shell(te.Current());
				if (shell.Closed())
				{
					BRepBuilderAPI_MakeSolid solidMaker(shell);
					if (solidMaker.IsDone())
					{
						solids->Add(gcnew XbimSolid(solidMaker.Solid(), _modelServices));
					}
				}
				te.Next();
			}

		}

		void XbimSolidSet::Init(IIfcShellBasedSurfaceModel^ solid, ILogger^ logger)
		{
			XbimCompound^ comp = gcnew XbimCompound(solid, logger, _modelServices);
			solids = gcnew List<IXbimSolid^>();
			for each (IXbimSolid ^ xbimSolid in comp->Solids)
			{
				if (xbimSolid->IsValid)
					solids->Add(xbimSolid);
			}
			//upgrade any shells that are closed to solids
			TopExp_Explorer te(comp, TopAbs_SHELL, TopAbs_SOLID);
			while (te.More())
			{
				const TopoDS_Shell& shell = TopoDS::Shell(te.Current());
				if (shell.Closed())
				{
					BRepBuilderAPI_MakeSolid solidMaker(shell);
					if (solidMaker.IsDone())
					{
						solids->Add(gcnew XbimSolid(solidMaker.Solid(), _modelServices));
					}
				}
				te.Next();
			}
		}

		void XbimSolidSet::Init(IIfcExtrudedAreaSolid^ repItem, ILogger^ logger)
		{
			IIfcCompositeProfileDef^ compProfile = dynamic_cast<IIfcCompositeProfileDef^>(repItem->SweptArea);

			if (compProfile != nullptr) //handle these as composite solids
			{
				int profileCount = Enumerable::Count(compProfile->Profiles);
				if (profileCount == 0)
				{
					XbimGeometryCreator::LogWarning(logger, repItem, "Invalid number of profiles. It must be 2 or more. Profile discarded");
					return;
				}
				if (profileCount == 1)
				{
					XbimGeometryCreator::LogInfo(logger, repItem, "Invalid number of profiles in IIfcCompositeProfileDef #{0}. It must be 2 or more. A single Profile has been used");
					XbimSolid^ s = gcnew XbimSolid(repItem, logger, _modelServices);
					if (s->IsValid)
					{
						solids = gcnew List<IXbimSolid^>();
						solids->Add(s);
					}
					return;
				}
				solids = gcnew List<IXbimSolid^>();
				for each (IIfcProfileDef ^ profile in compProfile->Profiles)
				{
					XbimSolid^ aSolid = gcnew XbimSolid(repItem, profile, logger, _modelServices);
					if (aSolid->IsValid)
						solids->Add(aSolid);
				}
			}
			else
			{
				XbimSolid^ s = gcnew XbimSolid(repItem, logger, _modelServices);
				if (s->IsValid)
				{
					solids = gcnew List<IXbimSolid^>();
					solids->Add(s);
				}
			}
		}

		void XbimSolidSet::Init(IIfcSurfaceCurveSweptAreaSolid^ repItem, ILogger^ logger)
		{
			IIfcCompositeProfileDef^ compProfile = dynamic_cast<IIfcCompositeProfileDef^>(repItem->SweptArea);

			if (compProfile != nullptr) //handle these as composite solids
			{
				int profileCount = Enumerable::Count(compProfile->Profiles);
				if (profileCount == 0)
				{
					XbimGeometryCreator::LogWarning(logger, compProfile, "Invalid number of profiles. It must be 2 or more. Profile discarded");
					return;
				}
				if (profileCount == 1)
				{
					XbimGeometryCreator::LogInfo(logger, compProfile, "Invalid number of profiles. It must be 2 or more. A single Profile has been used");
					XbimSolid^ s = gcnew XbimSolid(repItem, logger, _modelServices);
					if (s->IsValid)
					{
						solids = gcnew List<IXbimSolid^>();
						solids->Add(s);
					}
					return;
				}
				solids = gcnew List<IXbimSolid^>();
				for each (IIfcProfileDef ^ profile in compProfile->Profiles)
				{
					XbimSolid^ aSolid = gcnew XbimSolid(repItem, profile, logger, _modelServices);
					if (aSolid->IsValid)
						solids->Add(aSolid);
				}
			}
			else
			{
				XbimSolid^ s = gcnew XbimSolid(repItem, logger, _modelServices);
				if (s->IsValid)
				{
					solids = gcnew List<IXbimSolid^>();
					solids->Add(s);
				}
			}
		}

		double VolumeOf(IXbimSolidSet^ set) {
			double ret = -1;
			XbimSolidSet^ basic = dynamic_cast<XbimSolidSet^>(set);
			if (basic != nullptr)
			{
				return basic->Volume;
			}
			return ret;
		}

		void XbimSolidSet::Init(IIfcCsgSolid^ IIfcSolid, ILogger^ logger)
		{
			solids = gcnew List<IXbimSolid^>();
			IIfcCsgPrimitive3D^ csgPrim = dynamic_cast<IIfcCsgPrimitive3D^>(IIfcSolid->TreeRootExpression);
			if (csgPrim != nullptr)
			{
				solids->Add(gcnew XbimSolid(csgPrim, logger, _modelServices));
			}
			else
			{
				IIfcBooleanResult^ booleanResult = dynamic_cast<IIfcBooleanResult^>(IIfcSolid->TreeRootExpression);
				if (booleanResult != nullptr) return Init(booleanResult, logger);
				throw gcnew System::NotImplementedException(System::String::Format("IIfcCsgSolid of Type {0} in entity #{1} is not implemented", IIfcSolid->GetType()->Name, IIfcSolid->EntityLabel));

			}
		}


		//Booleans
		void XbimSolidSet::Init(IIfcBooleanClippingResult^ solid, ILogger^ logger)
		{
			ModelGeometryService^ ms = XbimConvert::ModelGeometryService(solid);
			auto shape = ms->GetBooleanFactory()->BuildBooleanResult(solid);
			InitSolidsFromShape(shape);
		}


		void XbimSolidSet::Init(IIfcBooleanOperand^ boolOp, ILogger^ logger)
		{
			IIfcBooleanResult^ boolRes = dynamic_cast<IIfcBooleanResult^>(boolOp);
			IIfcCsgSolid^ csgOp = dynamic_cast<IIfcCsgSolid^>(boolOp);
			IIfcHalfSpaceSolid^ hs = dynamic_cast<IIfcHalfSpaceSolid^>(boolOp);
			IIfcCsgPrimitive3D^ csgPrim = dynamic_cast<IIfcCsgPrimitive3D^>(boolOp);

			IIfcSweptAreaSolid^ sa = dynamic_cast<IIfcSweptAreaSolid^>(boolOp);
			IIfcManifoldSolidBrep^ ms = dynamic_cast<IIfcManifoldSolidBrep^>(boolOp);
			IIfcSolidModel^ sm = dynamic_cast<IIfcSolidModel^>(boolOp);
			solids = gcnew List<IXbimSolid^>();
			if (boolRes != nullptr)
			{
				Init(boolRes, logger); // dispatch for boolean result
			}
			else if (csgOp != nullptr)
			{
				Init(csgOp, logger); // dispatch for IIfcCsgSolid result
			}
			else if (sa != nullptr)
			{
				Init(sa, logger);
			}
			else if (ms != nullptr)
			{
				XbimCompound^ comp = gcnew XbimCompound(ms, logger, _modelServices);
				Init(comp, ms, logger);
			}
			else if (hs != nullptr)
			{
				XbimSolid^ s = gcnew XbimSolid(hs, logger, _modelServices);
				if (s->IsValid) solids->Add(s);
			}
			else if (csgPrim != nullptr)
			{
				XbimSolid^ s = gcnew XbimSolid(csgPrim, logger, _modelServices);
				if (s->IsValid)solids->Add(s);
			}
			else if (sm != nullptr)
			{
				XbimSolid^ s = gcnew XbimSolid(sm, logger, _modelServices);
				if (s->IsValid)solids->Add(s); // otherwise create a  solid model
			}
			else
			{
				XbimGeometryCreator::LogError(logger, boolOp, "Not Implemented boolean operand {0})", boolOp->GetType()->Name);
			}
		}

		XbimSolidSet^ XbimSolidSet::BuildBooleanResult(IIfcBooleanResult^ boolRes, IfcBooleanOperator operatorType, XbimSolidSet^ ops, ILogger^ logger, ModelGeometryService^ modelServices)
		{


			XbimSolidSet^ right = gcnew XbimSolidSet(boolRes->SecondOperand, logger, modelServices);

			if (right->IsValid)
			{
				right->IfcEntityLabel = boolRes->SecondOperand->EntityLabel;
				ops->Add(right);
			}

			//if we are the same operator type for the first operand just aggregate them into a single solid set, otherwise execute, 
			//we cannot execute halfspace solid cuts in batch either as OCC blows
			IIfcBooleanResult^ booleanResult = dynamic_cast<IIfcBooleanResult^>(boolRes->FirstOperand);
			IIfcBooleanClippingResult^ clippingResult = dynamic_cast<IIfcBooleanClippingResult^>(boolRes->FirstOperand);
			IIfcHalfSpaceSolid^ secondOpResult = nullptr;
			if (booleanResult != nullptr)
				secondOpResult = dynamic_cast<IIfcHalfSpaceSolid^>(booleanResult->SecondOperand);
			if (booleanResult != nullptr && clippingResult == nullptr && booleanResult->Operator == operatorType && secondOpResult == nullptr)
			{
				return BuildBooleanResult((IIfcBooleanResult^)(boolRes->FirstOperand), operatorType, ops, logger, modelServices);
			}
			else
			{
				XbimSolidSet^ left = gcnew XbimSolidSet(boolRes->FirstOperand, logger, modelServices);
				left->IfcEntityLabel = boolRes->FirstOperand->EntityLabel;
				ops->Reverse();
				return left;
			}
		}

		void XbimSolidSet::Init(IIfcBooleanResult^ boolOp, ILogger^ logger)
		{

			if (dynamic_cast<IIfcBooleanClippingResult^>(boolOp))
			{
				Init((IIfcBooleanClippingResult^)boolOp, logger);
				return;
			}
			//XbimSolidSet^ right = gcnew XbimSolidSet(boolOp->SecondOperand,logger);
			//right->IfcEntityLabel = boolOp->SecondOperand->EntityLabel;
			////XbimSolidSet^ left = BuildBooleanResult(boolOp, boolOp->Operator, right, logger);
			//XbimSolidSet^ left = gcnew XbimSolidSet(boolOp->FirstOperand, logger);
			//left->IfcEntityLabel = boolOp->FirstOperand->EntityLabel;
			XbimSolidSet^ right = gcnew XbimSolidSet(_modelServices);
			right->IfcEntityLabel = boolOp->SecondOperand->EntityLabel;
			XbimSolidSet^ left = BuildBooleanResult(boolOp, boolOp->Operator, right, logger, _modelServices);

			solids = gcnew List<IXbimSolid^>();

			if (!left->IsValid)
			{
				if (boolOp->Operator != IfcBooleanOperator::UNION)
					XbimGeometryCreator::LogWarning(logger, boolOp, "Boolean result has invalid first operand");
				return;
			}

			if (!right->IsValid)
			{
				solids->AddRange(left);
				XbimGeometryCreator::LogWarning(logger, boolOp, "Boolean result has invalid second operand");
				return;
			}

			ModelGeometryService^ mf = XbimConvert::ModelGeometryService(boolOp);

			IXbimSolidSet^ result;
			try
			{

				switch (boolOp->Operator)
				{
				case IfcBooleanOperator::UNION:
					result = left->Union(right, mf->Precision, logger);
					break;
				case IfcBooleanOperator::INTERSECTION:
					result = left->Intersection(right, mf->Precision, logger);
					break;
				case IfcBooleanOperator::DIFFERENCE:
					result = left->Cut(right, mf->Precision, logger);
					break;
				}
				//XbimOccWriter::Write(result, "c:/tmp/bop" + boolOp->ToString()->Replace(";","") +".txt");
			}
			catch (System::Exception^ e)
			{
				XbimGeometryCreator::LogError(logger, boolOp, "Failed to perform boolean result from #{0} with #{1}. {2}", boolOp->FirstOperand->EntityLabel, boolOp->SecondOperand->EntityLabel, e->Message);
				solids->AddRange(left);; //return the left operand
				return;
			}

			solids->AddRange(result);
		}
	}
}
