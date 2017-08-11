#include "XbimSolidSet.h"

#include "XbimCompound.h"
#include "XbimGeometryCreator.h"
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopExp.hxx>
#include <BRepTools.hxx>
#include <BRep_Builder.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <ShapeFix_ShapeTolerance.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include "XbimConvert.h"
#include "BRepCheck_Analyzer.hxx"
#include "ShapeFix_Shape.hxx"
#include "ShapeUpgrade_UnifySameDomain.hxx"
using namespace System;
using namespace System::Linq;

namespace Xbim
{
	namespace Geometry
	{

		String^ XbimSolidSet::ToBRep::get()
		{
			if (!IsValid) return String::Empty;
			std::ostringstream oss;
			BRep_Builder b;
			TopoDS_Compound comp;
			b.MakeCompound(comp);

			for each (IXbimSolid^ solid in solids)
			{
				if (dynamic_cast<XbimSolid^>(solid))
				{
					b.Add(comp, (XbimSolid^)solid);
				}
			}
			BRepTools::Write(comp, oss);
			return gcnew String(oss.str().c_str());
		}

		XbimSolidSet::XbimSolidSet(const TopoDS_Shape& shape)
		{
			TopTools_IndexedMapOfShape map;
			TopExp::MapShapes(shape, TopAbs_SOLID, map);
			solids = gcnew  List<IXbimSolid^>(map.Extent());
			for (int i = 1; i <= map.Extent(); i++)
				solids->Add(gcnew XbimSolid(TopoDS::Solid(map(i))));
		}

		XbimSolidSet::XbimSolidSet(XbimCompound^ shape)
		{
			
			if (shape->IsValid)
			{
				TopTools_IndexedMapOfShape map;
				TopExp::MapShapes(shape, TopAbs_SOLID, map);
				solids = gcnew  List<IXbimSolid^>(map.Extent());
				for (int i = 1; i <= map.Extent(); i++)
					solids->Add(gcnew XbimSolid(TopoDS::Solid(map(i))));
			}			
			GC::KeepAlive(shape);
		}

		XbimSolidSet::XbimSolidSet()
		{
			solids = gcnew  List<IXbimSolid^>();	
		}

		XbimSolidSet::XbimSolidSet(IXbimSolid^ solid)
		{
			solids = gcnew  List<IXbimSolid^>(1);
			solids->Add(solid);
		}
		XbimSolidSet::XbimSolidSet(IIfcBooleanResult^ boolOp, ILogger^ logger)
		{
			Init(boolOp, logger);
		}

		XbimSolidSet::XbimSolidSet(IIfcManifoldSolidBrep^ solid, ILogger^ logger)
		{
			XbimCompound^ comp = gcnew XbimCompound(solid, logger);
			Init(comp, solid, logger);
			
		}
		XbimSolidSet::XbimSolidSet(IIfcFacetedBrep^ solid, ILogger^ logger)
		{
			XbimCompound^ comp = gcnew XbimCompound(solid, logger);
			Init(comp, solid, logger);
		}

		XbimSolidSet::XbimSolidSet(IIfcFacetedBrepWithVoids^ solid, ILogger^ logger)
		{
			XbimCompound^ comp = gcnew XbimCompound(solid, logger);
			Init(comp, solid, logger);
		}

		XbimSolidSet::XbimSolidSet(IIfcClosedShell^ solid, ILogger^ logger)
		{
			XbimCompound^ comp = gcnew XbimCompound(solid, logger);
			Init(comp, solid, logger);
		}

		XbimSolidSet::XbimSolidSet(IEnumerable<IXbimSolid^>^ solids)
		{
			this->solids =  gcnew  List<IXbimSolid^>(solids);;
		}
		

		XbimSolidSet::XbimSolidSet(IIfcSweptAreaSolid^ repItem, ILogger^ logger)
		{
			Init(repItem, logger);
		}
		XbimSolidSet::XbimSolidSet(IIfcRevolvedAreaSolid^ solid, ILogger^ logger)
		{
			Init(solid, logger);
		}

		XbimSolidSet::XbimSolidSet(IIfcExtrudedAreaSolid^ repItem, ILogger^ logger)
		{
			Init(repItem, logger);

		}

		XbimSolidSet::XbimSolidSet(IIfcSurfaceCurveSweptAreaSolid^ repItem, ILogger^ logger)
		{
			Init(repItem,logger);
		}

		void XbimSolidSet::Reverse()
		{
			this->solids->Reverse();
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
				//make sure any sewing has been performed if the set is a compound object
				XbimCompound^ compound = dynamic_cast<XbimCompound^>(geomSet);
				
				if (compound != nullptr)
					if (!compound->Sew())
					{
						_isSimplified = true; //set flag true to say the solid set has been simplified and the user should be warned
#ifndef OCC_6_9_SUPPORTED		
						
						return; //don't add it we cannot really make it into a solid and will cause boolean operation errors,
#endif
					}
	
				for each (IXbimGeometryObject^ geom in geomSet)
				{
					
					XbimSolid^ nestedSolid = dynamic_cast<XbimSolid^>(geom);
					XbimCompound^ nestedCompound = dynamic_cast<XbimCompound^>(geom);
					XbimShell^ shell = dynamic_cast<XbimShell^>(geom);
					if (nestedSolid != nullptr && !nestedSolid->IsEmpty)
						solids->Add(nestedSolid );
					else if (nestedCompound != nullptr)
					{
						nestedCompound->Sew();
						for each (IXbimGeometryObject^ nestedGeom in nestedCompound)
						{
							XbimSolid^ subSolid = dynamic_cast<XbimSolid^>(nestedGeom);
							XbimShell^ subShell = dynamic_cast<XbimShell^>(nestedGeom);
							if(subSolid !=nullptr && !subSolid->IsEmpty)
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
			XbimSolidSet^ ss = gcnew XbimSolidSet();
			for (int i = start; i < Math::Min(solids->Count, count); i++)
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
			return solids==nullptr?0:solids->Count;
		}

		double XbimSolidSet::Volume::get()
		{
			double vol = 0;
			if (IsValid)
			{
				for each (XbimSolid^ solid in solids)
				{
					vol += solid->Volume;
				}
			}
			return vol;
		}

		IXbimGeometryObject^ XbimSolidSet::Transform(XbimMatrix3D matrix3D)
		{
			if (!IsValid) return gcnew XbimSolidSet();
			List<IXbimSolid^>^ result = gcnew List<IXbimSolid^>(solids->Count);
			for each (IXbimGeometryObject^ solid in solids)
			{
				result->Add((IXbimSolid^)solid->Transform(matrix3D));
			}
			return gcnew XbimSolidSet(result);
		}

		IXbimGeometryObject^ XbimSolidSet::TransformShallow(XbimMatrix3D matrix3D)
		{
			if (!IsValid) return gcnew XbimSolidSet();
			List<IXbimSolid^>^ result = gcnew List<IXbimSolid^>(solids->Count);
			for each (IXbimGeometryObject^ solid in solids)
			{
				result->Add((IXbimSolid^)((XbimSolid^)solid)->TransformShallow(matrix3D));
			}
			return gcnew XbimSolidSet(result);
		}


		void XbimSolidSet::Move(IIfcAxis2Placement3D^ position)
		{
			if (!IsValid) return;			
			for each (IXbimSolid^ solid in solids)
			{
				((XbimSolid^)solid)->Move(position);
			}
		}

		IXbimGeometryObject ^ XbimSolidSet::Transformed(IIfcCartesianTransformationOperator ^ transformation)
		{
			if (!IsValid) return this;
			XbimSolidSet^ result = gcnew XbimSolidSet();
			for each (XbimSolid^ solid in solids)
				result->Add(solid->Transformed(transformation));
			return result;
		}

		IXbimGeometryObject ^ XbimSolidSet::Moved(IIfcPlacement ^ placement)
		{
			if (!IsValid) return this;
			XbimSolidSet^ result = gcnew XbimSolidSet();
			TopLoc_Location loc = XbimConvert::ToLocation(placement);
			for each (IXbimSolid^ solid in solids)
			{
				XbimSolid^ copy = gcnew XbimSolid((XbimSolid^)solid, Tag);
				copy->Move(loc);
				result->Add(copy);
			}
			return result;
		}

		IXbimGeometryObject ^ XbimSolidSet::Moved(IIfcObjectPlacement ^ objectPlacement, ILogger^ logger)
		{
			if (!IsValid) return this;
			XbimSolidSet^ result = gcnew XbimSolidSet();
			TopLoc_Location loc = XbimConvert::ToLocation(objectPlacement, logger);
			for each (IXbimSolid^ solid in solids)
			{
				XbimSolid^ copy = gcnew XbimSolid((XbimSolid^)solid, Tag);
				copy->Move(loc);
				result->Add(copy);
			}
			return result;
		}

		void XbimSolidSet::Mesh(IXbimMeshReceiver ^ mesh, double precision, double deflection, double angle)
		{
			for each (IXbimSolid^ solid  in solids)
			{
				((XbimSolid^)solid)->Mesh(mesh, precision, deflection, angle);
			}
		}



		XbimRect3D XbimSolidSet::BoundingBox::get()
		{
			XbimRect3D result = XbimRect3D::Empty;
			if (IsValid)
			{
				for each (IXbimGeometryObject^ geomObj in solids)
				{
					XbimRect3D bbox = geomObj->BoundingBox;
					if (result.IsEmpty) result = bbox;
					else
						result.Union(bbox);
				}
			}
			return result;
		}

		IEnumerator<IXbimSolid^>^ XbimSolidSet::GetEnumerator()
		{
			if (solids == nullptr) return Empty->GetEnumerator();
			return solids->GetEnumerator();
		}

		bool XbimSolidSet::IsPolyhedron::get()
		{
			if (!IsValid) return false;
			for each (IXbimSolid^ solid in solids)
			{
				if (!solid->IsPolyhedron) return false;
			}
			return true;
		}

		IXbimSolidSet^ XbimSolidSet::Cut(IXbimSolidSet^ solidsToCut, double tolerance, ILogger^ logger)
		{			
			if (!IsValid) return this;
			String^ err = "";
			try
			{			
				ShapeFix_ShapeTolerance FTol;
				TopTools_ListOfShape shapeTools;
				for each (IXbimSolid^ iSolid in solidsToCut)
				{
					XbimSolid^ solid = dynamic_cast<XbimSolid^>(iSolid);
					if (solid!=nullptr && solid->IsValid)
					{					
						FTol.LimitTolerance(solid, tolerance);
						shapeTools.Append(solid);							
					}
					else
					{
						XbimGeometryCreator::LogWarning(logger, this, "Invalid shape found in Boolean Cut operation. It has been ignored");
					}
				}
				TopTools_ListOfShape shapeObjects;
				for each (IXbimSolid^ iSolid in this)
				{
					XbimSolid^ solid = dynamic_cast<XbimSolid^>(iSolid);
					if (solid != nullptr && solid->IsValid)
					{	
						FTol.LimitTolerance(solid, tolerance);
						shapeObjects.Append(solid);
					}
					else
					{
						XbimGeometryCreator::LogWarning(logger, this, "Invalid shape found in Boolean Cut operation. It has been ignored");
					}
				}
				
				BRepAlgoAPI_Cut boolOp;
				boolOp.SetArguments(shapeObjects);
				boolOp.SetTools(shapeTools);	
				boolOp.SetNonDestructive(Standard_True);
				//boolOp.SetFuzzyValue(tolerance);
				Handle(XbimProgressIndicator) aPI = new XbimProgressIndicator(XbimGeometryCreator::BooleanTimeOut);
				boolOp.SetProgressIndicator(aPI);
				boolOp.Build();
				aPI->StopTimer();

				if (aPI->TimedOut())
				{
					XbimGeometryCreator::LogError(logger, solidsToCut, "Boolean operation timed out after {0} seconds. Try increasing the timeout in the App.config file", (int)aPI->ElapsedTime());
					//throw gcnew XbimException(String::Format("Boolean operation timed out after {0} secs. Try increasing the timeout in the App.config file", (int)aPI->ElapsedTime()));
					return XbimSolidSet::Empty;
				}

				if (boolOp.ErrorStatus() == 0)
				{
					if (BRepCheck_Analyzer(boolOp.Shape(), Standard_False).IsValid() == Standard_False)
					{
						ShapeFix_Shape shapeFixer(boolOp.Shape());
						shapeFixer.SetPrecision(tolerance);
						shapeFixer.SetMinTolerance(tolerance);
						shapeFixer.FixSolidMode() = Standard_True;
						shapeFixer.FixFaceTool()->FixIntersectingWiresMode() = Standard_True;
						shapeFixer.FixFaceTool()->FixOrientationMode() = Standard_True;
						shapeFixer.FixFaceTool()->FixWireTool()->FixAddCurve3dMode() = Standard_True;
						shapeFixer.FixFaceTool()->FixWireTool()->FixIntersectingEdgesMode() = Standard_True;
						if (shapeFixer.Perform())
						{
							ShapeUpgrade_UnifySameDomain unifier(shapeFixer.Shape());
							unifier.SetAngularTolerance(0.00174533); //1 tenth of a degree
							unifier.SetLinearTolerance(tolerance);
							try
							{
								//sometimes unifier crashes
								unifier.Build();
								return gcnew XbimSolidSet(unifier.Shape());
							}
							catch (...)
							{
								//default to what we had
								return gcnew XbimSolidSet(shapeFixer.Shape());
							}
						}

					}
					return gcnew XbimSolidSet(boolOp.Shape());
				}

				err = "Error = " + boolOp.ErrorStatus();
				GC::KeepAlive(solidsToCut);
				GC::KeepAlive(this);
			}
			catch (Standard_Failure e)
			{
				err = gcnew String(Standard_Failure::Caught()->GetMessageString());
				throw gcnew Exception(String::Format("Boolean Cut operation failed. {0}" , err));
			}
			catch (...)
			{
				throw gcnew Exception("General boolean cutting failure");
			}
			
			return XbimSolidSet::Empty;

		}

		IXbimSolidSet^ XbimSolidSet::Union(IXbimSolidSet^ solidSet, double tolerance, ILogger^ logger)
		{
			if (!IsValid) return this;
			IXbimSolidSet^ toUnionSolidSet = solidSet; //just to sort out carve exclusion, they must be all OCC solids if no carve
			IXbimSolidSet^ thisSolidSet = this;

			XbimCompound^ thisSolid = XbimCompound::Merge(thisSolidSet, tolerance,logger);
			XbimCompound^ toUnionSolid = XbimCompound::Merge(toUnionSolidSet, tolerance,logger);
			if (thisSolid == nullptr && toUnionSolid == nullptr) return XbimSolidSet::Empty;
			if (thisSolid != nullptr && toUnionSolid != nullptr)
			{
				XbimCompound^ result = thisSolid->Union(toUnionSolid, tolerance,logger);
				XbimSolidSet^ss = gcnew XbimSolidSet();
				ss->Add(result);
				return ss;
			}
			if (toUnionSolid != nullptr) return solidSet;
			return this;
		}

		

		IXbimSolidSet^ XbimSolidSet::Intersection(IXbimSolidSet^ solidSet, double tolerance, ILogger^ logger)
		{
			if (!IsValid) return this;
			IXbimSolidSet^ toIntersectSolidSet = solidSet; //just to sort out carve exclusion, they must be all OCC solids if no carve
			IXbimSolidSet^ thisSolidSet = this;

			XbimCompound^ thisSolid = XbimCompound::Merge(thisSolidSet, tolerance,logger);
			XbimCompound^ toIntersectSolid = XbimCompound::Merge(toIntersectSolidSet, tolerance,logger);
			if (thisSolid == nullptr || toIntersectSolid == nullptr) return XbimSolidSet::Empty;
			XbimCompound^ result = thisSolid->Intersection(toIntersectSolid, tolerance,logger);
			return gcnew XbimSolidSet(result);
		}


		IXbimSolidSet^ XbimSolidSet::Cut(IXbimSolid^ solid, double tolerance, ILogger^ logger)
		{
			if (Count == 0) return XbimSolidSet::Empty;
			if (Count == 1) return First->Cut(solid, tolerance,logger);
			return Cut(gcnew XbimSolidSet(solid), tolerance,logger);

		}

		IXbimSolidSet^ XbimSolidSet::Union(IXbimSolid^ solid, double tolerance, ILogger^ logger)
		{
			if (Count == 0) return gcnew XbimSolidSet(solid);
			if (Count == 1) return First->Union(solid, tolerance,logger);
			return Union(gcnew XbimSolidSet(solid), tolerance,logger);
		}


		

		IXbimSolidSet^ XbimSolidSet::Intersection(IXbimSolid^ solid, double tolerance, ILogger^ logger)
		{
			if (Count == 0) return XbimSolidSet::Empty;
			if (Count == 1) return First->Intersection(solid, tolerance,logger);
			return Intersection(gcnew XbimSolidSet(solid), tolerance,logger);
		}

		void XbimSolidSet::Init(XbimCompound^ comp, IPersistEntity^ entity, ILogger^ logger)
		{
			solids = gcnew  List<IXbimSolid^>();
			if (!comp->IsValid)
			{
				XbimGeometryCreator::LogWarning(logger,entity, "Empty or invalid solid");
			}
			else
			{
				Add(comp);
			}
		}

		void XbimSolidSet::Init(IIfcSweptAreaSolid^ repItem, ILogger^ logger)
		{
			IIfcCompositeProfileDef^ compProfile = dynamic_cast<IIfcCompositeProfileDef^>(repItem->SweptArea);
			int profileCount = Enumerable::Count(compProfile->Profiles);
			if (compProfile != nullptr) //handle these as composite solids
			{
				if (profileCount == 0)
				{
					XbimGeometryCreator::LogWarning(logger,repItem, "Invalid number of profiles. It must be 2 or more. Profile discarded");
					return;
				}
				if (profileCount == 1)
				{
					XbimGeometryCreator::LogInfo(logger,compProfile, "Invalid number of profiles. It must be 2 or more. A single Profile has been used");
					XbimSolid^ s = gcnew XbimSolid(repItem, logger);
					if (s->IsValid)
					{
						solids = gcnew List<IXbimSolid^>();
						solids->Add(s);
					}
					return;
				}
				solids = gcnew List<IXbimSolid^>();
				for each (IIfcProfileDef^ profile in compProfile->Profiles)
				{
					XbimSolid^ aSolid = gcnew XbimSolid(repItem, profile, logger);
					if (aSolid->IsValid)
						solids->Add(aSolid);
				}
			}
			else
			{
				XbimSolid^ s = gcnew XbimSolid(repItem, logger);
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
			int profileCount = Enumerable::Count(compProfile->Profiles);
			if (compProfile != nullptr) //handle these as composite solids
			{
				if (profileCount == 0)
				{
					XbimGeometryCreator::LogWarning(logger,compProfile,"Invalid number of profiles. It must be 2 or more. Profile discarded");
					return;
				}
				if (profileCount == 1)
				{
					XbimGeometryCreator::LogInfo(logger,compProfile,"Invalid number of profiles. It must be 2 or more. A single Profile has been used");
					XbimSolid^ s = gcnew XbimSolid(repItem, logger);
					if (s->IsValid)
					{
						solids = gcnew List<IXbimSolid^>();
						solids->Add(s);
					}
					return;
				}
				solids = gcnew List<IXbimSolid^>();
				for each (IIfcProfileDef^ profile in compProfile->Profiles)
				{
					XbimSolid^ aSolid = gcnew XbimSolid(repItem, profile, logger);
					if (aSolid->IsValid)
						solids->Add(aSolid);
				}
			}
			else
			{
				XbimSolid^ s = gcnew XbimSolid(repItem, logger);
				if (s->IsValid)
				{
					solids = gcnew List<IXbimSolid^>();
					solids->Add(s);
				}
			}
		}

		void XbimSolidSet::Init(IIfcExtrudedAreaSolid^ repItem, ILogger^ logger)
		{
			IIfcCompositeProfileDef^ compProfile = dynamic_cast<IIfcCompositeProfileDef^>(repItem->SweptArea);
			int profileCount = Enumerable::Count(compProfile->Profiles);
			if (compProfile!=nullptr) //handle these as composite solids
			{
				if (profileCount == 0)
				{
					XbimGeometryCreator::LogWarning(logger,repItem,"Invalid number of profiles. It must be 2 or more. Profile discarded");
					return;
				}
				if (profileCount == 1)
				{
					XbimGeometryCreator::LogInfo(logger,repItem, "Invalid number of profiles in IIfcCompositeProfileDef #{0}. It must be 2 or more. A single Profile has been used");
					XbimSolid^ s = gcnew XbimSolid(repItem, logger);
					if (s->IsValid)
					{
						solids = gcnew List<IXbimSolid^>();
						solids->Add(s);
					}
					return;
				}				
				solids = gcnew List<IXbimSolid^>();
				for each (IIfcProfileDef^ profile in compProfile->Profiles)
				{
					XbimSolid^ aSolid = gcnew XbimSolid(repItem, profile, logger);
					if (aSolid->IsValid)
						solids->Add(aSolid);
				}
			}
			else
			{
				XbimSolid^ s = gcnew XbimSolid(repItem, logger);
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
			int profileCount = Enumerable::Count(compProfile->Profiles);
			if (compProfile != nullptr) //handle these as composite solids
			{
				if (profileCount == 0)
				{
					XbimGeometryCreator::LogWarning(logger,compProfile,"Invalid number of profiles. It must be 2 or more. Profile discarded");
					return;
				}
				if (profileCount == 1)
				{
					XbimGeometryCreator::LogInfo(logger,compProfile,"Invalid number of profiles. It must be 2 or more. A single Profile has been used");
					XbimSolid^ s = gcnew XbimSolid(repItem, logger);
					if (s->IsValid)
					{
						solids = gcnew List<IXbimSolid^>();
						solids->Add(s);
					}
					return;
				}
				solids = gcnew List<IXbimSolid^>();
				for each (IIfcProfileDef^ profile in compProfile->Profiles)
				{
					XbimSolid^ aSolid = gcnew XbimSolid(repItem, profile, logger);
					if (aSolid->IsValid)
						solids->Add(aSolid);
				}
			}
			else
			{
				XbimSolid^ s = gcnew XbimSolid(repItem, logger);
				if (s->IsValid)
				{
					solids = gcnew List<IXbimSolid^>();
					solids->Add(s);
				}
			}
		}



		void XbimSolidSet::Init(IIfcBooleanResult^ boolOp, ILogger^ logger)
		{
			solids = gcnew List<IXbimSolid^>();
			IIfcBooleanOperand^ fOp = boolOp->FirstOperand; //thse must be solids according to the schema
			IIfcBooleanOperand^ sOp = boolOp->SecondOperand;
			XbimSolid^ left = gcnew XbimSolid(fOp, logger);
			XbimSolid^ right = gcnew XbimSolid(sOp, logger);
			if (!left->IsValid)
			{
				if (boolOp->Operator != IfcBooleanOperator::UNION)
				//XbimGeometryCreator::LogWarning(boolOp, "Boolean result has invalid first operand");
				return;
			}

			if (!right->IsValid)
			{
				//XbimGeometryCreator::LogWarning(boolOp, "Boolean result has invalid second operand");
				if(left->IsValid) solids->Add(left); //return the left operand
				return;
			}

			IModelFactors^ mf = boolOp->Model->ModelFactors;
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
			}
			catch (Exception^ xbimE)
			{
				XbimGeometryCreator::LogError(logger,boolOp, "Boolean operation failure, {0}. The operation has been ignored", xbimE->Message);
				solids->Add(left);; //return the left operand
				return;
			}
			solids->AddRange(result);
		}
	}
}
