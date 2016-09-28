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
		XbimSolidSet::XbimSolidSet(IIfcBooleanResult^ boolOp)
		{
			Init(boolOp);
		}

		XbimSolidSet::XbimSolidSet(IIfcManifoldSolidBrep^ solid)
		{
			XbimCompound^ comp = gcnew XbimCompound(solid);
			Init(comp, solid);
			
		}
		XbimSolidSet::XbimSolidSet(IIfcFacetedBrep^ solid)
		{
			XbimCompound^ comp = gcnew XbimCompound(solid);
			Init(comp, solid);
		}

		XbimSolidSet::XbimSolidSet(IIfcFacetedBrepWithVoids^ solid)
		{
			XbimCompound^ comp = gcnew XbimCompound(solid);
			Init(comp, solid);
		}

		XbimSolidSet::XbimSolidSet(IIfcClosedShell^ solid)
		{
			XbimCompound^ comp = gcnew XbimCompound(solid);
			Init(comp, solid);
		}

		XbimSolidSet::XbimSolidSet(IEnumerable<IXbimSolid^>^ solids)
		{
			this->solids =  gcnew  List<IXbimSolid^>(solids);;
		}
		

		XbimSolidSet::XbimSolidSet(IIfcSweptAreaSolid^ repItem)
		{
			Init(repItem);
		}
		XbimSolidSet::XbimSolidSet(IIfcRevolvedAreaSolid^ solid)
		{
			Init(solid);
		}

		XbimSolidSet::XbimSolidSet(IIfcExtrudedAreaSolid^ repItem)
		{
			Init(repItem);

		}

		XbimSolidSet::XbimSolidSet(IIfcSurfaceCurveSweptAreaSolid^ repItem)
		{
			Init(repItem);
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
			if (solid != nullptr)
			{
				return solids->Add(solid);
			}
			
			IXbimSolidSet^ solidSet = dynamic_cast<IXbimSolidSet^>(shape);
			if (solidSet != nullptr) return solids->AddRange(solidSet);
			IXbimGeometryObjectSet^ geomSet = dynamic_cast<IXbimGeometryObjectSet^>(shape);
			if (geomSet != nullptr)
			{
				//make sure any sewing has been performed
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
					
					IXbimSolid^ compSolid = dynamic_cast<IXbimSolid^>(geom);
					if (compSolid != nullptr) 
						solids->Add(compSolid);
					else
					{
						XbimShell^ shell = dynamic_cast<XbimShell^>(geom);
						if (shell != nullptr)
						{							
							XbimSolid^ s = (XbimSolid^)shell->MakeSolid();								
							solids->Add(s);
						}					
					}
				}
				
				return;
			}
			XbimShell^ shell = dynamic_cast<XbimShell^>(shape);
			if (shell != nullptr/* && shell->IsClosed*/)
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

		IXbimGeometryObject ^ XbimSolidSet::Moved(IIfcObjectPlacement ^ objectPlacement)
		{
			if (!IsValid) return this;
			XbimSolidSet^ result = gcnew XbimSolidSet();
			TopLoc_Location loc = XbimConvert::ToLocation(objectPlacement);
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

		IXbimSolidSet^ XbimSolidSet::Cut(IXbimSolidSet^ solids, double tolerance)
		{
			IXbimSolidSet^ toCutSolidSet = solids; //just to sort out carve exclusion, they must be all OCC solids if no carve
			IXbimSolidSet^ thisSolidSet = this;


#ifdef OCC_6_9_SUPPORTED
			if (!IsValid) return this;
			String^ err = "";
			try
			{
				
				ShapeFix_ShapeTolerance FTol;
				tolerance *= 1.1;
				TopTools_ListOfShape shapeTools;
				for each (IXbimSolid^ iSolid in solids)
				{
					XbimSolid^ solid = dynamic_cast<XbimSolid^>(iSolid);
					if (solid!=nullptr)
					{
						FTol.SetTolerance(solid, tolerance);
						shapeTools.Append(solid);							
					}
				}
				TopTools_ListOfShape shapeObjects;
				for each (IXbimSolid^ iSolid in this)
				{
					XbimSolid^ solid = dynamic_cast<XbimSolid^>(iSolid);
					if (solid != nullptr)
					{
						FTol.SetTolerance(solid, tolerance);
						shapeObjects.Append(solid);
					}
				}
				
				BRepAlgoAPI_Cut boolOp;
				boolOp.SetArguments(shapeObjects);
				boolOp.SetTools(shapeTools);				
				boolOp.SetFuzzyValue(0);
				boolOp.Build();
				
				if (boolOp.ErrorStatus() == 0)
					return gcnew XbimSolidSet(boolOp.Shape());
				err = "Error = " + boolOp.ErrorStatus();
				GC::KeepAlive(solids);
				GC::KeepAlive(this);
			}
			catch (Standard_Failure e)
			{
				err = gcnew String(Standard_Failure::Caught()->GetMessageString());
			}
			XbimGeometryCreator::LogWarning(this,"Boolean Cut operation failed. " + err);
			return XbimSolidSet::Empty;
#else

			if (thisSolidSet->Count >_maxOpeningsToCut) //if the base shape is really complicate just give up trying
			{
				IsSimplified = true;
				return this;
			}
			XbimCompound^ thisSolid = XbimCompound::Merge(thisSolidSet, tolerance);
			
			XbimCompound^ toCutSolid;
			if (thisSolid == nullptr) return XbimSolidSet::Empty;
			bool isSimplified = false;
			if (toCutSolidSet->Count > _maxOpeningsToCut)
			{
				isSimplified = true;
				List<Tuple<double, XbimSolid^>^>^ solidsList = gcnew List<Tuple<double, XbimSolid^>^>(toCutSolidSet->Count);
				for each (XbimSolid^ solid in toCutSolidSet)
				{
					solidsList->Add(gcnew Tuple<double, XbimSolid^>(solid->Volume, solid));
				}
				solidsList->Sort(_volumeComparer);
				TopoDS_Compound subsetToCut;
				BRep_Builder b;
				b.MakeCompound(subsetToCut);
				//int i = 0;
				double totalVolume = this->Volume;
				double minVolume = totalVolume * _maxOpeningVolumePercentage;

				for (int i = 0; i < _maxOpeningsToCut; i++)
				{
					if (solidsList[i]->Item1 < minVolume) break; //give up for small things
					b.Add(subsetToCut, solidsList[i]->Item2);
				}
				
				toCutSolid = gcnew XbimCompound(subsetToCut,true, tolerance);
				
			}
			else
			{
				toCutSolid = XbimCompound::Merge(toCutSolidSet, tolerance);
			}

			if (toCutSolid == nullptr) return this;
			XbimCompound^ result = thisSolid->Cut(toCutSolid, tolerance);
			XbimSolidSet^ ss = gcnew XbimSolidSet(result);
			//BRepTools::Write(result, "d:\\c");
			GC::KeepAlive(result);
			ss->IsSimplified = isSimplified;
			return ss;
#endif
		}

		IXbimSolidSet^ XbimSolidSet::Union(IXbimSolidSet^ solids, double tolerance)
		{
			if (!IsValid) return this;
			IXbimSolidSet^ toUnionSolidSet = solids; //just to sort out carve exclusion, they must be all OCC solids if no carve
			IXbimSolidSet^ thisSolidSet = this;

			XbimCompound^ thisSolid = XbimCompound::Merge(thisSolidSet, tolerance);
			XbimCompound^ toUnionSolid = XbimCompound::Merge(toUnionSolidSet, tolerance);
			if (thisSolid == nullptr && toUnionSolid == nullptr) return XbimSolidSet::Empty;
			if (thisSolid != nullptr && toUnionSolid != nullptr)
			{
				XbimCompound^ result = thisSolid->Union(toUnionSolid, tolerance);
				XbimSolidSet^ss = gcnew XbimSolidSet();
				ss->Add(result);
				return ss;
			}
			if (toUnionSolid != nullptr) return solids;
			return this;
		}

		

		IXbimSolidSet^ XbimSolidSet::Intersection(IXbimSolidSet^ solids, double tolerance)
		{
			if (!IsValid) return this;
			IXbimSolidSet^ toIntersectSolidSet = solids; //just to sort out carve exclusion, they must be all OCC solids if no carve
			IXbimSolidSet^ thisSolidSet = this;

			XbimCompound^ thisSolid = XbimCompound::Merge(thisSolidSet, tolerance);
			XbimCompound^ toIntersectSolid = XbimCompound::Merge(toIntersectSolidSet, tolerance);
			if (thisSolid == nullptr || toIntersectSolid == nullptr) return XbimSolidSet::Empty;
			XbimCompound^ result = thisSolid->Intersection(toIntersectSolid, tolerance);
			return gcnew XbimSolidSet(result);
		}


		IXbimSolidSet^ XbimSolidSet::Cut(IXbimSolid^ solid, double tolerance)
		{
			if (Count == 0) return XbimSolidSet::Empty;
			if (Count == 1) return First->Cut(solid, tolerance);
			return Cut(gcnew XbimSolidSet(solid), tolerance);

		}

		IXbimSolidSet^ XbimSolidSet::Union(IXbimSolid^ solid, double tolerance)
		{
			if (Count == 0) return gcnew XbimSolidSet(solid);
			if (Count == 1) return First->Union(solid, tolerance);
			return Union(gcnew XbimSolidSet(solid), tolerance);
		}


		

		IXbimSolidSet^ XbimSolidSet::Intersection(IXbimSolid^ solid, double tolerance)
		{
			if (Count == 0) return XbimSolidSet::Empty;
			if (Count == 1) return First->Intersection(solid, tolerance);
			return Intersection(gcnew XbimSolidSet(solid), tolerance);
		}

		void XbimSolidSet::Init(XbimCompound^ comp, IPersistEntity^ entity)
		{
			
			if (!comp->IsValid || comp->Count == 0)
			{
				solids = gcnew  List<IXbimSolid^>();
				XbimGeometryCreator::LogWarning(entity, "Empty or invalid solid");
			}
			else
			{
				comp->Sew();
				solids = gcnew  List<IXbimSolid^>(comp->Count);
				for each (IXbimGeometryObject^ geom in comp)
				{
					if (dynamic_cast<XbimShell^>(geom))
					{
						XbimSolid^ s = (XbimSolid ^ )((XbimShell^)geom)->MakeSolid();
						if (s->IsValid)
						{
							if (!s->HasValidTopology)
								s->FixTopology();
							solids->Add(s);
						}		

					}
					else if (dynamic_cast<XbimSolid^>(geom))
						solids->Add((XbimSolid^)geom);
				}
				if (solids->Count == 0)
					XbimGeometryCreator::LogWarning(entity, "Empty solid");
			}
		}

		void XbimSolidSet::Init(IIfcSweptAreaSolid^ repItem)
		{
			IIfcCompositeProfileDef^ compProfile = dynamic_cast<IIfcCompositeProfileDef^>(repItem->SweptArea);
			int profileCount = Enumerable::Count(compProfile->Profiles);
			if (compProfile != nullptr) //handle these as composite solids
			{
				if (profileCount == 0)
				{
					XbimGeometryCreator::LogWarning(repItem, "Invalid number of profiles. It must be 2 or more. Profile discarded");
					return;
				}
				if (profileCount == 1)
				{
					XbimGeometryCreator::LogInfo(compProfile, "Invalid number of profiles. It must be 2 or more. A single Profile has been used");
					XbimSolid^ s = gcnew XbimSolid(repItem);
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
					XbimSolid^ aSolid = gcnew XbimSolid(repItem, profile);
					if (aSolid->IsValid)
						solids->Add(aSolid);
				}
			}
			else
			{
				XbimSolid^ s = gcnew XbimSolid(repItem);
				if (s->IsValid)
				{
					solids = gcnew List<IXbimSolid^>();
					solids->Add(s);
				}
			}
		}
		void XbimSolidSet::Init(IIfcRevolvedAreaSolid^ repItem)
		{
			IIfcCompositeProfileDef^ compProfile = dynamic_cast<IIfcCompositeProfileDef^>(repItem->SweptArea);
			int profileCount = Enumerable::Count(compProfile->Profiles);
			if (compProfile != nullptr) //handle these as composite solids
			{
				if (profileCount == 0)
				{
					XbimGeometryCreator::LogWarning(compProfile,"Invalid number of profiles. It must be 2 or more. Profile discarded");
					return;
				}
				if (profileCount == 1)
				{
					XbimGeometryCreator::LogInfo(compProfile,"Invalid number of profiles. It must be 2 or more. A single Profile has been used");
					XbimSolid^ s = gcnew XbimSolid(repItem);
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
					XbimSolid^ aSolid = gcnew XbimSolid(repItem, profile);
					if (aSolid->IsValid)
						solids->Add(aSolid);
				}
			}
			else
			{
				XbimSolid^ s = gcnew XbimSolid(repItem);
				if (s->IsValid)
				{
					solids = gcnew List<IXbimSolid^>();
					solids->Add(s);
				}
			}
		}

		void XbimSolidSet::Init(IIfcExtrudedAreaSolid^ repItem)
		{
			IIfcCompositeProfileDef^ compProfile = dynamic_cast<IIfcCompositeProfileDef^>(repItem->SweptArea);
			int profileCount = Enumerable::Count(compProfile->Profiles);
			if (compProfile!=nullptr) //handle these as composite solids
			{
				if (profileCount == 0)
				{
					XbimGeometryCreator::LogWarning(repItem,"Invalid number of profiles. It must be 2 or more. Profile discarded");
					return;
				}
				if (profileCount == 1)
				{
					XbimGeometryCreator::LogInfo(repItem, "Invalid number of profiles in IIfcCompositeProfileDef #{0}. It must be 2 or more. A single Profile has been used");
					XbimSolid^ s = gcnew XbimSolid(repItem);
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
					XbimSolid^ aSolid = gcnew XbimSolid(repItem, profile);
					if (aSolid->IsValid)
						solids->Add(aSolid);
				}
			}
			else
			{
				XbimSolid^ s = gcnew XbimSolid(repItem);
				if (s->IsValid) 
				{
					solids = gcnew List<IXbimSolid^>();
					solids->Add(s);
				}
			}
		}

		void XbimSolidSet::Init(IIfcSurfaceCurveSweptAreaSolid^ repItem)
		{
			IIfcCompositeProfileDef^ compProfile = dynamic_cast<IIfcCompositeProfileDef^>(repItem->SweptArea);
			int profileCount = Enumerable::Count(compProfile->Profiles);
			if (compProfile != nullptr) //handle these as composite solids
			{
				if (profileCount == 0)
				{
					XbimGeometryCreator::LogWarning(compProfile,"Invalid number of profiles. It must be 2 or more. Profile discarded");
					return;
				}
				if (profileCount == 1)
				{
					XbimGeometryCreator::LogInfo(compProfile,"Invalid number of profiles. It must be 2 or more. A single Profile has been used");
					XbimSolid^ s = gcnew XbimSolid(repItem);
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
					XbimSolid^ aSolid = gcnew XbimSolid(repItem, profile);
					if (aSolid->IsValid)
						solids->Add(aSolid);
				}
			}
			else
			{
				XbimSolid^ s = gcnew XbimSolid(repItem);
				if (s->IsValid)
				{
					solids = gcnew List<IXbimSolid^>();
					solids->Add(s);
				}
			}
		}



		void XbimSolidSet::Init(IIfcBooleanResult^ boolOp)
		{
			solids = gcnew List<IXbimSolid^>();
			IIfcBooleanOperand^ fOp = boolOp->FirstOperand; //thse must be solids according to the schema
			IIfcBooleanOperand^ sOp = boolOp->SecondOperand;
			XbimSolid^ left = gcnew XbimSolid(fOp);
			XbimSolid^ right = gcnew XbimSolid(sOp);
			if (!left->IsValid)
			{
				XbimGeometryCreator::LogWarning(boolOp, "Boolean result has invalid first operand");
				return;
			}

			if (!right->IsValid)
			{
				XbimGeometryCreator::LogWarning(boolOp, "Boolean result has invalid second operand");
				solids->Add(left); //return the left operand
				return;
			}

			IModelFactors^ mf = boolOp->Model->ModelFactors;
			IXbimSolidSet^ result;
			try
			{
				switch (boolOp->Operator)
				{
				case IfcBooleanOperator::UNION:
					result = left->Union(right, mf->PrecisionBoolean);
					break;
				case IfcBooleanOperator::INTERSECTION:
					result = left->Intersection(right, mf->PrecisionBoolean);
					break;
				case IfcBooleanOperator::DIFFERENCE:
					result = left->Cut(right, mf->PrecisionBoolean);
					break;
				}
			}
			catch (Exception^ xbimE)
			{
				XbimGeometryCreator::LogError(boolOp, "Boolean operation failure, {0}. The operation has been ignored", xbimE->Message);
				solids->Add(left);; //return the left operand
				return;
			}
			solids->AddRange(result);
		}
	}
}
