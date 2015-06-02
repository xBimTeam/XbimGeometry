#include "XbimSolidSet.h"
#include "XbimFacetedSolid.h"
#include "XbimCompound.h"
#include "XbimGeometryCreator.h"
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopExp.hxx>
#include <BRepTools.hxx>
#include <BRep_Builder.hxx>
#include <BRepAlgoAPI_Cut.hxx>
using namespace System;
using namespace Xbim::Common;
namespace Xbim
{
	namespace Geometry
	{
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
		XbimSolidSet::XbimSolidSet(IfcBooleanResult^ boolOp)
		{
			Init(boolOp);
		}

		XbimSolidSet::XbimSolidSet(IfcManifoldSolidBrep^ solid)
		{
			XbimCompound^ comp = gcnew XbimCompound(solid);
			Init(comp, solid->EntityLabel);
			
		}
		XbimSolidSet::XbimSolidSet(IfcFacetedBrep^ solid)
		{
			XbimCompound^ comp = gcnew XbimCompound(solid);
			Init(comp, solid->EntityLabel);
		}

		XbimSolidSet::XbimSolidSet(IfcFacetedBrepWithVoids^ solid)
		{
			XbimCompound^ comp = gcnew XbimCompound(solid);
			Init(comp, solid->EntityLabel);
		}

		XbimSolidSet::XbimSolidSet(IfcClosedShell^ solid)
		{
			XbimCompound^ comp = gcnew XbimCompound(solid);
			Init(comp, solid->EntityLabel);
		}

		XbimSolidSet::XbimSolidSet(IEnumerable<IXbimSolid^>^ solids)
		{
			this->solids =  gcnew  List<IXbimSolid^>(solids);;
		}

		void XbimSolidSet::Reverse()
		{
			this->solids->Reverse();
		}


		///If the shape contains one or more solids these are added to the collection
		void XbimSolidSet::Add(IXbimGeometryObject^ shape)
		{
			IXbimSolid^ solid = dynamic_cast<IXbimSolid^>(shape);
			if (solid != nullptr) return solids->Add(solid);
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
		
		IXbimSolid^ XbimSolidSet::First::get()
		{
			if (solids->Count == 0) return nullptr;
			return solids[0];
		}

		int XbimSolidSet::Count::get()
		{
			return solids==nullptr?0:solids->Count;
		}

		double XbimSolidSet::Volume::get()
		{
			double vol = 0;
			for each (XbimSolid^ solid in solids)
			{
				vol += solid->Volume;
			}
			return vol;
		}

		IXbimGeometryObject^ XbimSolidSet::Transform(XbimMatrix3D matrix3D)
		{
			List<IXbimSolid^>^ result = gcnew List<IXbimSolid^>(solids->Count);
			for each (IXbimGeometryObject^ solid in solids)
			{
				result->Add((IXbimSolid^)solid->Transform(matrix3D));
			}
			return gcnew XbimSolidSet(result);
		}

		XbimRect3D XbimSolidSet::BoundingBox::get()
		{
			XbimRect3D result = XbimRect3D::Empty;
			for each (IXbimGeometryObject^ geomObj in solids)
			{
				XbimRect3D bbox = geomObj->BoundingBox;
				if (result.IsEmpty) result = bbox;
				else
					result.Union(bbox);
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
#ifdef USE_CARVE_CSG
			if (this->IsPolyhedron && solids->IsPolyhedron) //downgrade everything to faceted representation
			{
				IXbimSolidSet^ thisFacetationSet = gcnew XbimSolidSet();
				for each (IXbimSolid^ thisSolid in this)
				{
					XbimFacetedSolid^ fSolid = dynamic_cast<XbimFacetedSolid^>(thisSolid);
					if (fSolid != nullptr)
						thisFacetationSet->Add(fSolid);
					else
					{
						XbimSolid^ occSolid = dynamic_cast<XbimSolid^>(thisSolid);
						if (occSolid != nullptr)
							thisFacetationSet->Add(gcnew XbimFacetedSolid(occSolid, tolerance)); //convert it
						else
							XbimGeometryCreator::logger->WarnFormat("WSS01: Invalid operation. Only solid shapes can be cut from another solid");
					}
				}
				IXbimSolidSet^ toCutFacetationSet = gcnew XbimSolidSet();
				for each (IXbimSolid^ solid in solids)
				{
					XbimFacetedSolid^ fSolid = dynamic_cast<XbimFacetedSolid^>(solid);
					if (fSolid != nullptr)
						toCutFacetationSet->Add(fSolid);
					else
					{
						XbimSolid^ occSolid = dynamic_cast<XbimSolid^>(solid);
						if (occSolid != nullptr)
							toCutFacetationSet->Add(gcnew XbimFacetedSolid(occSolid, tolerance)); //convert it
						else
							XbimGeometryCreator::logger->WarnFormat("WSS02: Invalid operation. Only solid shapes can be cut from another solid");
					}
				}
				XbimFacetedSolid^ thisFacetation = XbimFacetedSolid::Merge(thisFacetationSet, tolerance);
				XbimFacetedSolid^ toCutFacetation = XbimFacetedSolid::Merge(toCutFacetationSet, tolerance);
				if (thisFacetation == nullptr) return XbimSolidSet::Empty;
				if (toCutFacetation == nullptr) return this;

				if ((thisFacetation == nullptr || !thisFacetation->IsValid) && (toCutFacetation == nullptr || !toCutFacetation->IsValid))
					return XbimSolidSet::Empty;
				if (thisFacetation != nullptr && thisFacetation->IsValid  && toCutFacetation != nullptr && toCutFacetation->IsValid)
				{
					return (IXbimSolidSet^)thisFacetation->Cut(toCutFacetation, tolerance);
				}
				if (toCutFacetation != nullptr && toCutFacetation->IsValid)
					return solids;
				return this;
			}
			else //upgrade everything to OCC  
			{
				thisSolidSet = gcnew XbimSolidSet();
				for each (IXbimSolid^ thisSolid in this)
				{

					XbimSolid^ solid = dynamic_cast<XbimSolid^>(thisSolid);
					if (solid != nullptr)
						thisSolidSet->Add(solid);
					else
					{
						XbimFacetedSolid^ fSolid = dynamic_cast<XbimFacetedSolid^>(thisSolid);
						if (fSolid != nullptr)
							thisSolidSet->Add(fSolid->ConvertToXbimSolid()); //convert it
						else
							XbimGeometryCreator::logger->WarnFormat("WSS03: Invalid operation. Only solid shapes can be cut from another solid");
					}
				}
				toCutSolidSet = gcnew XbimSolidSet();
				for each (IXbimSolid^ isolid in solids)
				{
					XbimSolid^ solid = dynamic_cast<XbimSolid^>(isolid);
					if (solid != nullptr)
						toCutSolidSet->Add(solid);
					else
					{
						XbimFacetedSolid^ fSolid = dynamic_cast<XbimFacetedSolid^>(isolid);
						if (fSolid != nullptr)
							toCutSolidSet->Add(fSolid->ConvertToXbimSolid()); //convert it
						else
							XbimGeometryCreator::logger->WarnFormat("WSS04: Invalid operation. Only solid shapes can be cut from another solid");
					}
				}
		}

#endif // USE_CARVE_CSG

#ifdef OCC_6_9_SUPPORTED
			
			String^ err = "";
			try
			{
				
				TopTools_ListOfShape shapeTools;
				for each (IXbimSolid^ iSolid in solids)
				{
					XbimSolid^ solid = dynamic_cast<XbimSolid^>(iSolid);
					if (solid!=nullptr)
					{
						shapeTools.Append(solid);	
					}
				}
				TopTools_ListOfShape shapeObjects;
				for each (IXbimSolid^ iSolid in this)
				{
					XbimSolid^ solid = dynamic_cast<XbimSolid^>(iSolid);
					if (solid != nullptr)
					{
						shapeObjects.Append(solid);
					}
				}
				BRepAlgoAPI_Cut boolOp;
				boolOp.SetArguments(shapeObjects);
				boolOp.SetTools(shapeTools);
				boolOp.SetFuzzyValue(tolerance);
				boolOp.Build();
				
				if (boolOp.ErrorStatus() == 0)
					return gcnew XbimSolidSet(boolOp.Shape());
				err = "Error = " + boolOp.ErrorStatus();
			}
			catch (Standard_Failure e)
			{
				err = gcnew String(Standard_Failure::Caught()->GetMessageString());
			}
			XbimGeometryCreator::logger->WarnFormat("WS032: Boolean Cut operation failed. " + err);
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
			IXbimSolidSet^ toUnionSolidSet = solids; //just to sort out carve exclusion, they must be all OCC solids if no carve
			IXbimSolidSet^ thisSolidSet = this;
#ifdef USE_CARVE_CSG
			if (this->IsPolyhedron && solids->IsPolyhedron) //downgrade everything to faceted representation
			{
				IXbimSolidSet^ thisFacetationSet = gcnew XbimSolidSet();
				for each (IXbimSolid^ thisSolid in this)
				{
					XbimFacetedSolid^ fSolid = dynamic_cast<XbimFacetedSolid^>(thisSolid);
					if (fSolid != nullptr)
						thisFacetationSet->Add(fSolid);
					else
					{
						XbimSolid^ occSolid = dynamic_cast<XbimSolid^>(thisSolid);
						if (occSolid != nullptr)
							thisFacetationSet->Add(gcnew XbimFacetedSolid(occSolid, tolerance)); //convert it
						else
							XbimGeometryCreator::logger->WarnFormat("WSS05: Invalid operation. Only solid shapes can be unioned from another solid");
					}
				}
				IXbimSolidSet^ toUnionFacetationSet = gcnew XbimSolidSet();
				for each (IXbimSolid^ solid in solids)
				{
					XbimFacetedSolid^ fSolid = dynamic_cast<XbimFacetedSolid^>(solid);
					if (fSolid != nullptr)
						toUnionFacetationSet->Add(fSolid);
					else
					{
						XbimSolid^ occSolid = dynamic_cast<XbimSolid^>(solid);
						if (occSolid != nullptr)
							toUnionFacetationSet->Add(gcnew XbimFacetedSolid(occSolid, tolerance)); //convert it
						else
							XbimGeometryCreator::logger->WarnFormat("WSS06: Invalid operation. Only solid shapes can be unioned from another solid");
					}
				}
				XbimFacetedSolid^ thisFacetation = XbimFacetedSolid::Merge(thisFacetationSet, tolerance);

				XbimFacetedSolid^ toUnionFacetation = XbimFacetedSolid::Merge(toUnionFacetationSet, tolerance);
				if ((thisFacetation == nullptr || !thisFacetation->IsValid) && (toUnionFacetation == nullptr || !toUnionFacetation->IsValid))
					return XbimSolidSet::Empty;
				if (thisFacetation != nullptr && thisFacetation->IsValid  && toUnionFacetation != nullptr && toUnionFacetation->IsValid)
				{
					return (IXbimSolidSet^)thisFacetation->Union(toUnionFacetation, tolerance);
				}
				if (toUnionFacetation != nullptr && toUnionFacetation->IsValid)
					return solids;
				return this;
			}
			else //upgrade everything to OCC
			{
			    thisSolidSet = gcnew XbimSolidSet();
				for each (IXbimSolid^ thisSolid in this)
				{
					XbimSolid^ solid = dynamic_cast<XbimSolid^>(thisSolid);
					if (solid != nullptr)
						thisSolidSet->Add(solid);
					else
					{
						XbimFacetedSolid^ fSolid = dynamic_cast<XbimFacetedSolid^>(thisSolid);
						if (fSolid != nullptr)
							thisSolidSet->Add(fSolid->ConvertToXbimSolid()); //convert it
						else
							XbimGeometryCreator::logger->WarnFormat("WSS07: Invalid operation. Only solid shapes can be unioned from another solid");
					}
				}
				toUnionSolidSet = gcnew XbimSolidSet();
				for each (IXbimSolid^ isolid in solids)
				{
					XbimSolid^ solid = dynamic_cast<XbimSolid^>(isolid);
					if (solid != nullptr)
						toUnionSolidSet->Add(solid);
					else
					{
						XbimFacetedSolid^ fSolid = dynamic_cast<XbimFacetedSolid^>(isolid);
						if (fSolid != nullptr)
							toUnionSolidSet->Add(fSolid->ConvertToXbimSolid()); //convert it
						else
							XbimGeometryCreator::logger->WarnFormat("WSS08: Invalid operation. Only solid shapes can be unioned from another solid");
					}
				}
		}

#endif // USE_CARVE_CSG
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
			IXbimSolidSet^ toIntersectSolidSet = solids; //just to sort out carve exclusion, they must be all OCC solids if no carve
			IXbimSolidSet^ thisSolidSet = this;
#ifdef USE_CARVE_CSG
			if (this->IsPolyhedron && solids->IsPolyhedron) //downgrade everything to faceted representation
			{
				IXbimSolidSet^ thisFacetationSet = gcnew XbimSolidSet();
				for each (IXbimSolid^ thisSolid in this)
				{
					XbimFacetedSolid^ fSolid = dynamic_cast<XbimFacetedSolid^>(thisSolid);
					if (fSolid != nullptr)
						thisFacetationSet->Add(fSolid);
					else
					{
						XbimSolid^ occSolid = dynamic_cast<XbimSolid^>(thisSolid);
						if (occSolid != nullptr)
							thisFacetationSet->Add(gcnew XbimFacetedSolid(occSolid, tolerance)); //convert it
						else
							XbimGeometryCreator::logger->WarnFormat("WSS09: Invalid operation. Only solid shapes can be intersected with another solid");
					}
				}
				IXbimSolidSet^ toIntersectFacetationSet = gcnew XbimSolidSet();
				for each (IXbimSolid^ solid in solids)
				{
					XbimFacetedSolid^ fSolid = dynamic_cast<XbimFacetedSolid^>(solid);
					if (fSolid != nullptr)
						toIntersectFacetationSet->Add(fSolid);
					else
					{
						XbimSolid^ occSolid = dynamic_cast<XbimSolid^>(solid);
						if (occSolid != nullptr)
							toIntersectFacetationSet->Add(gcnew XbimFacetedSolid(occSolid, tolerance)); //convert it
						else
							XbimGeometryCreator::logger->WarnFormat("WSS10: Invalid operation. Only solid shapes can be intersected with another solid");
					}
				}
				XbimFacetedSolid^ thisFacetation = XbimFacetedSolid::Merge(thisFacetationSet, tolerance);
				XbimFacetedSolid^ toIntersectFacetation = XbimFacetedSolid::Merge(toIntersectFacetationSet, tolerance);
				if (thisFacetation == nullptr || toIntersectFacetation == nullptr) return XbimSolidSet::Empty;
				return (IXbimSolidSet^)thisFacetation->Union(toIntersectFacetation, tolerance);
			}
			else //upgrade everything to OCC
			{
				thisSolidSet = gcnew XbimSolidSet();
				for each (IXbimSolid^ thisSolid in this)
				{
					XbimSolid^ solid = dynamic_cast<XbimSolid^>(thisSolid);
					if (solid != nullptr)
						thisSolidSet->Add(solid);
					else
					{
						XbimFacetedSolid^ fSolid = dynamic_cast<XbimFacetedSolid^>(thisSolid);
						if (fSolid != nullptr)
							thisSolidSet->Add(fSolid->ConvertToXbimSolid()); //convert it
						else
							XbimGeometryCreator::logger->WarnFormat("WSS11: Invalid operation. Only solid shapes can be intersected with another solid");
					}
				}
				toIntersectSolidSet = gcnew XbimSolidSet();
				for each (IXbimSolid^ isolid in solids)
				{
					XbimSolid^ solid = dynamic_cast<XbimSolid^>(isolid);
					if (solid != nullptr)
						toIntersectSolidSet->Add(solid);
					else
					{
						XbimFacetedSolid^ fSolid = dynamic_cast<XbimFacetedSolid^>(isolid);
						if (fSolid != nullptr)
							toIntersectSolidSet->Add(fSolid->ConvertToXbimSolid()); //convert it
						else
							XbimGeometryCreator::logger->WarnFormat("WSS12: Invalid operation. Only solid shapes can be intersected with another solid");
					}
				}
		}
#endif // USE_CARVE
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
		void XbimSolidSet::Init(XbimCompound^ comp, int label)
		{
			
			if (!comp->IsValid || comp->Count == 0)
			{
				solids = gcnew  List<IXbimSolid^>();
				XbimGeometryCreator::logger->WarnFormat("WSS13: Invalid IfcManifoldSolidBrep #{0}", label);
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
					XbimGeometryCreator::logger->WarnFormat("WSS14: Invalid IfcManifoldSolidBrep #{0}", label);

			}

		}
		void XbimSolidSet::Init(IfcBooleanResult^ boolOp)
		{
			solids = gcnew List<IXbimSolid^>();
			IfcBooleanOperand^ fOp = boolOp->FirstOperand; //thse must be solids according to the schema
			IfcBooleanOperand^ sOp = boolOp->SecondOperand;
			XbimSolid^ left = gcnew XbimSolid(fOp);
			XbimSolid^ right = gcnew XbimSolid(sOp);
			if (!left->IsValid)
			{
				XbimGeometryCreator::logger->WarnFormat("WS006: IfcBooleanResult #{0} with invalid first operand", boolOp->EntityLabel);
				return;
			}

			if (!right->IsValid)
			{
				XbimGeometryCreator::logger->WarnFormat("WS007: IfcBooleanResult #{0} with invalid second operand", boolOp->EntityLabel);
				solids->Add(left); //return the left operand
				return;
			}

			XbimModelFactors^ mf = boolOp->ModelOf->ModelFactors;
			IXbimSolidSet^ result;
			try
			{
				switch (boolOp->Operator)
				{
				case IfcBooleanOperator::Union:
					result = left->Union(right, mf->PrecisionBoolean);
					break;
				case IfcBooleanOperator::Intersection:
					result = left->Intersection(right, mf->PrecisionBoolean);
					break;
				case IfcBooleanOperator::Difference:
					result = left->Cut(right, mf->PrecisionBoolean);
					break;
				}
			}
			catch (Exception^ xbimE)
			{
				XbimGeometryCreator::logger->ErrorFormat("ES001: Error performing boolean operation for entity #{0}={1}\n{2}. The operation has been ignored", boolOp->EntityLabel, boolOp->GetType()->Name, xbimE->Message);
				solids->Add(left);; //return the left operand
				return;
			}
			solids->AddRange(result);
		}
	}
}
