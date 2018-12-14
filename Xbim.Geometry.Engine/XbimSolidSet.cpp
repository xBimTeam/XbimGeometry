#include "XbimSolidSet.h"
#include "XbimShellSet.h"
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
#include "BRepBuilderAPI_MakeSolid.hxx"
#include "BOPAlgo_PaveFiller.hxx"
#include "BOPAlgo_BOP.hxx"

using namespace System::Linq;
using namespace System::Threading;
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
		XbimSolidSet::XbimSolidSet(IIfcCsgSolid^ repItem, ILogger^ logger)
		{
			Init(repItem, logger);
		}

		XbimSolidSet::XbimSolidSet(IXbimSolid^ solid)
		{
			solids = gcnew  List<IXbimSolid^>(1);
			solids->Add(solid);
		}
		XbimSolidSet::XbimSolidSet(IIfcBooleanClippingResult ^ solid, ILogger ^ logger)
		{
			Init(solid, logger);
		}

		XbimSolidSet::XbimSolidSet(IIfcBooleanResult^ boolOp, ILogger^ logger)
		{
			Init(boolOp, logger);
		}

		XbimSolidSet::XbimSolidSet(IIfcBooleanOperand ^ boolOp, ILogger ^ logger)
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
			this->solids = gcnew  List<IXbimSolid^>(solids);;
		}


		XbimSolidSet::XbimSolidSet(IIfcSweptAreaSolid^ repItem, ILogger^ logger)
		{
			Init(repItem, logger);
		}
		XbimSolidSet::XbimSolidSet(IIfcRevolvedAreaSolid^ solid, ILogger^ logger)
		{
			Init(solid, logger);
		}

		XbimSolidSet::XbimSolidSet(IIfcTriangulatedFaceSet ^ IIfcSolid, ILogger ^ logger)
		{
			Init(IIfcSolid, logger);
		}

		XbimSolidSet::XbimSolidSet(IIfcFaceBasedSurfaceModel ^ solid, ILogger ^ logger)
		{
			Init(solid, logger);
		}

		XbimSolidSet::XbimSolidSet(IIfcShellBasedSurfaceModel ^ solid, ILogger ^ logger)
		{
			Init(solid, logger);
		}

		XbimSolidSet::XbimSolidSet(IIfcExtrudedAreaSolid^ repItem, ILogger^ logger)
		{
			Init(repItem, logger);

		}

		XbimSolidSet::XbimSolidSet(IIfcSurfaceCurveSweptAreaSolid^ repItem, ILogger^ logger)
		{
			Init(repItem, logger);
		}

		void XbimSolidSet::Reverse()
		{
			this->solids->Reverse();
		}

		XbimSolidSet^ XbimSolidSet::BuildClippingList(IIfcBooleanClippingResult^ solid, List<IIfcBooleanOperand^>^ clipList, ILogger^ logger)
		{
			IIfcBooleanOperand^ fOp = solid->FirstOperand;
			IIfcBooleanOperand^ sOp = solid->SecondOperand;

			IIfcBooleanClippingResult^ boolClip = dynamic_cast<IIfcBooleanClippingResult^>(fOp);
			if (boolClip != nullptr)
			{
				clipList->Add(sOp);
				return XbimSolidSet::BuildClippingList(boolClip, clipList, logger);
			}
			else //we need to build the solid
			{
				clipList->Add(sOp);
				clipList->Reverse();
				return gcnew XbimSolidSet(fOp, logger);
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
						solids->Add(nestedSolid);
					else if (nestedCompound != nullptr)
					{
						nestedCompound->Sew();
						for each (IXbimGeometryObject^ nestedGeom in nestedCompound)
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
			return solids == nullptr ? 0 : solids->Count;
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

		static void ThreadProc(Object^ params)
		{
			
			ShapeAnalysis_Wire tolFixer;

			BRep_Builder builder;
			TopoDS_Compound cutCompound;
			builder.MakeCompound(cutCompound);
			XbimSolidSetBoolOpParams^ boolParams = dynamic_cast<XbimSolidSetBoolOpParams^>(params);
			TopTools_ListOfShape shapeObjects;
			shapeObjects.Append(boolParams->Body);

			TopTools_ListOfShape shapeTools;


			// new approach
			TopTools_ListOfShape aLC;
			aLC.Append(boolParams->Body);

			for each (IXbimSolid^ iSolid in boolParams->Ops)
			{
				XbimSolid^ solid = dynamic_cast<XbimSolid^>(iSolid);
				if (solid != nullptr && solid->IsValid)
				{
					aLC.Append(solid);
					shapeTools.Append(solid);
					builder.Add(cutCompound, solid);
				}
			}

			try
			{
				Handle(NCollection_BaseAllocator)aAL =
					NCollection_BaseAllocator::CommonBaseAllocator();
				BOPAlgo_PaveFiller aPF(aAL);

				aPF.SetArguments(aLC);
				aPF.SetFuzzyValue(5 * boolParams->Tolerance); //this seems about right
				aPF.SetRunParallel(false);
				aPF.SetNonDestructive(false);
				aPF.Perform();
				int iErr = aPF.HasErrors();
				if (iErr)
				{
					boolParams->Success = false;
					return;
				}
				BOPAlgo_BOP aBOP(aAL);
				//
				aBOP.AddArgument(boolParams->Body);
				aBOP.AddTool(cutCompound);
				aBOP.SetOperation(boolParams->Operation);
				aBOP.SetRunParallel(false);
				aBOP.PerformWithFiller(aPF);
				const TopoDS_Shape& aR = aBOP.Shape();
				if (aR.IsNull()) {
					boolParams->Success = false;
					return;
				}
				boolParams->Success = true;
				//have one go at fixing if it is not right
				if (BRepCheck_Analyzer(aR, Standard_True).IsValid() == Standard_False)
				{
					//try and fix if we can
					ShapeFix_Shape fixer(aR);
					fixer.Perform();
					boolParams->Result = gcnew XbimSolidSet(fixer.Shape());
				}
				else
				{				
					boolParams->Result = gcnew XbimSolidSet(aR);
				}
			}
			catch (const std::exception &exc)
			{
				String^ err = gcnew String(exc.what());
				XbimGeometryCreator::LogError(boolParams->Logger, boolParams->Body, "Boolean Cut operation failed, no holes have been cut. {0}", err);
				boolParams->Success = false;
			}
			catch (...)
			{
				XbimGeometryCreator::LogError(boolParams->Logger, boolParams->Body, "General boolean cutting failure, no holes have been cut.");
				boolParams->Success = false;
			}
		}
		IXbimSolidSet^ XbimSolidSet::DoBoolean(IXbimSolidSet^ arguments, BOPAlgo_Operation operation,  double tolerance, ILogger^ logger)
		{
			if (!IsValid) return this;

			List<Thread^>^ threads = gcnew List<Thread^>(this->Count);
			List<XbimSolidSetBoolOpParams^>^ params = gcnew List<XbimSolidSetBoolOpParams^>(this->Count);
			for (int i = 0; i < this->Count; i++)
			{
				if (!solids[i]->IsValid) continue;
				Thread^ oThread = gcnew Thread(gcnew ParameterizedThreadStart(Xbim::Geometry::ThreadProc));
				threads->Add(oThread);

				XbimSolidSet^ copyOfCuts = gcnew XbimSolidSet();
				for each (IXbimSolid^ iSolid in arguments)
				{
					XbimSolid^ solid = dynamic_cast<XbimSolid^>(iSolid);
					if (solid != nullptr && solid->IsValid)
					{

						BRepBuilderAPI_Copy cutCopier(solid);
						copyOfCuts->Add(gcnew XbimSolid(TopoDS::Solid(cutCopier.Shape())));
					}
					else
					{
						XbimGeometryCreator::LogWarning(logger, this, "Invalid shape found in Boolean Cut operation. It has been ignored");
					}
				}
				BRepBuilderAPI_Copy bodyCopier(dynamic_cast<XbimSolid^>(solids[i]));
				XbimSolidSetBoolOpParams^ param = gcnew XbimSolidSetBoolOpParams(gcnew XbimSolid(TopoDS::Solid(bodyCopier.Shape())), copyOfCuts, tolerance, logger);
				param->Operation = operation;
				params->Add(param);
				oThread->Start(param);
			}
			for (int i = 0; i < threads->Count; i++)
			{
				Thread^ oThread = threads[i];

				if (!oThread->Join((int)(XbimGeometryCreator::BooleanTimeOut * 1000)))
				{
					XbimGeometryCreator::LogError(logger, nullptr,
						"Boolean operation timed out after {0} seconds.\nCutting Entity #{1} from #{2}.\nTry increasing the timeout in the App.config file\nUncut shape is used",
						XbimGeometryCreator::BooleanTimeOut,
						((XbimSolidSet^)arguments)->IfcEntityLabel,
						this->IfcEntityLabel);
					// we have identified now continue with the uncut  left operand
					params[i]->UseBody = true; //stop further error reporting
				}
			}

			XbimSolidSet^ result = gcnew XbimSolidSet();
			for each (XbimSolidSetBoolOpParams^ param in params)
			{
				if (!param->Success)
				{
					if (param->UseBody)
					{
						result->Add(param->Body);
					}
					else
					{
						XbimGeometryCreator::LogError(logger, nullptr,
							"Boolean operation failed.\nCutting Entity #{0} from #{1}.",
							((XbimSolidSet^)arguments)->IfcEntityLabel,
							this->IfcEntityLabel);
					}
				}
				else
				{
					if (param->Result != nullptr && param->Result->IsValid)
					{
						result->Add(param->Result);
					}
				}
			}
			return result;
		}

		IXbimSolidSet^ XbimSolidSet::Cut(IXbimSolidSet^ solidsToCut, double tolerance, ILogger^ logger)
		{
			
			return DoBoolean(solidsToCut,BOPAlgo_CUT,tolerance,logger);
		}

		IXbimSolidSet^ XbimSolidSet::Union(IXbimSolidSet^ solidsToUnion, double tolerance, ILogger^ logger)
		{
			return DoBoolean(solidsToUnion, BOPAlgo_FUSE, tolerance, logger);
		}



		IXbimSolidSet^ XbimSolidSet::Intersection(IXbimSolidSet^ solidSet, double tolerance, ILogger^ logger)
		{
			if (!IsValid) return this;
			IXbimSolidSet^ toIntersectSolidSet = solidSet; //just to sort out carve exclusion, they must be all OCC solids if no carve
			IXbimSolidSet^ thisSolidSet = this;

			XbimCompound^ thisSolid = XbimCompound::Merge(thisSolidSet, tolerance, logger);
			XbimCompound^ toIntersectSolid = XbimCompound::Merge(toIntersectSolidSet, tolerance, logger);
			if (thisSolid == nullptr || toIntersectSolid == nullptr) return XbimSolidSet::Empty;
			XbimCompound^ result = thisSolid->Intersection(toIntersectSolid, tolerance, logger);
			return gcnew XbimSolidSet(result);
		}


		IXbimSolidSet^ XbimSolidSet::Cut(IXbimSolid^ solid, double tolerance, ILogger^ logger)
		{
			if (Count == 0) return XbimSolidSet::Empty;
			if (Count == 1) return First->Cut(solid, tolerance, logger);
			return Cut(gcnew XbimSolidSet(solid), tolerance, logger);

		}

		IXbimSolidSet^ XbimSolidSet::Union(IXbimSolid^ solid, double tolerance, ILogger^ logger)
		{
			if (Count == 0) return gcnew XbimSolidSet(solid);
			if (Count == 1) return First->Union(solid, tolerance, logger);
			return Union(gcnew XbimSolidSet(solid), tolerance, logger);
		}




		IXbimSolidSet^ XbimSolidSet::Intersection(IXbimSolid^ solid, double tolerance, ILogger^ logger)
		{
			if (Count == 0) return XbimSolidSet::Empty;
			if (Count == 1) return First->Intersection(solid, tolerance, logger);
			return Intersection(gcnew XbimSolidSet(solid), tolerance, logger);
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

		void XbimSolidSet::Init(IIfcTriangulatedFaceSet ^ IIfcSolid, ILogger ^ logger)
		{
			XbimCompound^ comp = gcnew XbimCompound(IIfcSolid, logger);
			solids = gcnew List<IXbimSolid^>();
			for each (IXbimSolid^ xbimSolid in comp->Solids)
			{
				if (xbimSolid->IsValid)
					solids->Add(xbimSolid);
			}
		}

		void XbimSolidSet::Init(IIfcFaceBasedSurfaceModel ^ solid, ILogger ^ logger)
		{
			XbimCompound^ comp = gcnew XbimCompound(solid, logger);
			solids = gcnew List<IXbimSolid^>();
			for each (IXbimSolid^ xbimSolid in comp->Solids)
			{
				if (xbimSolid->IsValid)
					solids->Add(xbimSolid);
			}
		}

		void XbimSolidSet::Init(IIfcShellBasedSurfaceModel ^ solid, ILogger ^ logger)
		{
			XbimCompound^ comp = gcnew XbimCompound(solid, logger);
			solids = gcnew List<IXbimSolid^>();
			for each (IXbimSolid^ xbimSolid in comp->Solids)
			{
				if (xbimSolid->IsValid)
					solids->Add(xbimSolid);
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
				solids->Add(gcnew XbimSolid(csgPrim, logger));
			}
			else
			{
				IIfcBooleanResult^ booleanResult = dynamic_cast<IIfcBooleanResult^>(IIfcSolid->TreeRootExpression);
				if (booleanResult != nullptr) return Init(booleanResult, logger);
				throw gcnew NotImplementedException(String::Format("IIfcCsgSolid of Type {0} in entity #{1} is not implemented", IIfcSolid->GetType()->Name, IIfcSolid->EntityLabel));

			}
		}



		//Booleans
		void XbimSolidSet::Init(IIfcBooleanClippingResult^ solid, ILogger^ logger)
		{
			solids = gcnew List<IXbimSolid^>();
			IModelFactors^ mf = solid->Model->ModelFactors;
			IIfcBooleanOperand^ fOp = solid->FirstOperand;

			IIfcBooleanClippingResult^ boolClip = dynamic_cast<IIfcBooleanClippingResult^>(fOp);
			if (boolClip != nullptr)
			{
				List<IIfcBooleanOperand^>^ clips = gcnew List<IIfcBooleanOperand^>();

				XbimSolidSet^ solidSet = gcnew XbimSolidSet();
				solidSet->IfcEntityLabel = solid->EntityLabel;
				XbimSolidSet^ bodySet = XbimSolidSet::BuildClippingList(boolClip, clips, logger);
				bodySet->IfcEntityLabel = boolClip->EntityLabel;
				double maxLen = bodySet->BoundingBox.Length();
				for each (IIfcBooleanOperand^ bOp in clips)
				{
					IIfcPolygonalBoundedHalfSpace^ pbhs = dynamic_cast<IIfcPolygonalBoundedHalfSpace^>(bOp);
					if (pbhs != nullptr) //special case for IIfcPolygonalBoundedHalfSpace to keep extrusion to the minimum
					{
						XbimSolid^ s = gcnew XbimSolid(pbhs, maxLen, logger);
						if (s->IsValid) solidSet->Add(s);
					}
					else
					{
						XbimSolidSet^ s = gcnew XbimSolidSet(bOp, logger);
						if (s->IsValid) solidSet->Add(s);
					}
				}


				IXbimSolidSet^ xbimSolidSet = bodySet->Cut(solidSet, mf->Precision, logger);
				if (xbimSolidSet != nullptr && xbimSolidSet->IsValid)
				{
					solids->AddRange(xbimSolidSet);
				}

			}
			else
			{

				IIfcBooleanOperand^ sOp = solid->SecondOperand;
				XbimSolidSet^ left = gcnew XbimSolidSet(fOp, logger);
				left->IfcEntityLabel = fOp->EntityLabel;
				XbimSolidSet^ right = gcnew XbimSolidSet(sOp, logger);
				right->IfcEntityLabel = sOp->EntityLabel;
				if (!left->IsValid)
				{
					return; //nothing happening here
				}

				if (!right->IsValid)
				{
					// XbimGeometryCreator::LogError(logger, sOp, "Error performing boolean operation, Invalid Second Operand in IfcBooleanClippingResult #{0}", solid->EntityLabel);
					solids->AddRange(left); // no change
					return;
				}


				try
				{
					IXbimSolidSet^ result = left->Cut(right, mf->Precision, logger);
					if (result->IsValid) solids->AddRange(result);
				}
				catch (...)
				{
					XbimGeometryCreator::LogError(logger, solid, "Error performing boolean operation, {0}. The operation has been ignored", solid->EntityLabel);
					solids->AddRange(left); // no change
					return;
				}

			}
		}


		void XbimSolidSet::Init(IIfcBooleanOperand ^ boolOp, ILogger ^ logger)
		{
			IIfcBooleanResult^ boolRes = dynamic_cast<IIfcBooleanResult^>(boolOp);
			IIfcCsgSolid^ csgOp = dynamic_cast<IIfcCsgSolid^>(boolOp);
			IIfcHalfSpaceSolid^ hs = dynamic_cast<IIfcHalfSpaceSolid^>(boolOp);
			IIfcCsgPrimitive3D^ csgPrim = dynamic_cast<IIfcCsgPrimitive3D^>(boolOp);
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
			else if (hs != nullptr)
			{
				solids->Add(gcnew XbimSolid(hs, logger));
			}
			else if (csgPrim != nullptr)
			{
				solids->Add(gcnew XbimSolid(csgPrim, logger));
			}
			else if (sm != nullptr)
			{
				solids->Add(gcnew XbimSolid(sm, logger)); // otherwise create a  solid model
			}
			else
			{
				XbimGeometryCreator::LogError(logger, boolOp, "Not Implemented boolean operand {0})", boolOp->GetType()->Name);
			}
			/*if (IsValid)
			{
				for each (XbimSolid^ solid in solids)
				{
					if (!solid->IsValid)
						XbimGeometryCreator::LogWarning(logger, boolOp, "Partially invalid boolean operand result (solid volume {0})", solid->Volume);
				}
			}
			else
			{
				XbimGeometryCreator::LogWarning(logger, boolOp, "Invalid boolean operand result");
			}*/
		}
		XbimSolidSet^ XbimSolidSet::BuildBooleanResult(IIfcBooleanResult^ boolRes, IfcBooleanOperator operatorType,  XbimSolidSet^ ops, ILogger^ logger)
		{
			XbimSolidSet^ right = gcnew XbimSolidSet(boolRes->SecondOperand, logger);
			if (Math::Abs(right->Volume) > Precision::Confusion())
			{
				right->IfcEntityLabel = boolRes->SecondOperand->EntityLabel;
				ops->Add(right);
			}
			//if we are the same operator type just aggregate them into a single solid set
			if (boolRes->Operator == operatorType && dynamic_cast<IIfcBooleanResult^>(boolRes->FirstOperand) && !dynamic_cast<IIfcBooleanClippingResult^>(boolRes->FirstOperand))
			{								
				return BuildBooleanResult((IIfcBooleanResult^)(boolRes->FirstOperand), operatorType, ops, logger);
			}
			else
			{
				XbimSolidSet^ left = gcnew XbimSolidSet(boolRes->FirstOperand, logger);

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
			XbimSolidSet^ right = gcnew XbimSolidSet();
			right->IfcEntityLabel = boolOp->SecondOperand->EntityLabel;
			XbimSolidSet^ left = BuildBooleanResult(boolOp, boolOp->Operator, right, logger);
			solids = gcnew List<IXbimSolid^>();

			if (!left->IsValid)
			{
				if (boolOp->Operator != IfcBooleanOperator::UNION)
					XbimGeometryCreator::LogWarning(logger, boolOp, "Boolean result has invalid first operand");
				return;
			}
			if (Math::Abs(left->Volume) <= Precision::Confusion()) //nothing to do						
				return;
			if (!right->IsValid)
			{
				solids->AddRange(left);
				XbimGeometryCreator::LogWarning(logger, boolOp, "Boolean result has invalid second operand");
				return;
			}

			if (Math::Abs(right->Volume) <= Precision::Confusion()) //nothing to do
			{
				solids->AddRange(left);
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
				XbimGeometryCreator::LogError(logger, boolOp, "Boolean operation failure, {0}. The operation has been ignored", xbimE->Message);
				solids->AddRange(left);; //return the left operand
				return;
			}
			solids->AddRange(result);
		}
	}
}
