
#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <BRep_Builder.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <TopExp.hxx>
#include <TopoDS_Edge.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepCheck_Shell.hxx>
#include <BRepBuilderAPI_MakeSolid.hxx>
#include <BRepClass3d_SolidClassifier.hxx>
#include <Precision.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Section.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <ShapeFix_ShapeTolerance.hxx>
#include <TopTools_HSequenceOfShape.hxx>
#include <ShapeAnalysis_FreeBounds.hxx>
#include <BRepAlgo_FaceRestrictor.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Line.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepBuilderAPI_GTransform.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <ShapeFix_Shell.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepTools.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <GeomLib_IsPlanarSurface.hxx>
#include <Standard_CString.hxx>
#include "XbimNativeApi.h"
#include "XbimCompound.h"
#include "XbimSolid.h"
#include "XbimShell.h"
#include "XbimFace.h"

#include "XbimGeometryObjectSet.h"
#include "XbimSolidSet.h"
#include "XbimVertexSet.h"
#include "XbimFaceSet.h"
#include "XbimEdgeSet.h"
#include "XbimShellSet.h"
#include "XbimConvert.h"
#include "XbimOccWriter.h"
#include "./Factories/BooleanFactory.h"

using namespace Xbim::Common::Exceptions;
namespace Xbim
{
	namespace Geometry
	{
		/*Ensures native pointers are deleted and garbage collected*/
		void XbimShell::InstanceCleanup()
		{
			System::IntPtr temp = System::Threading::Interlocked::Exchange(ptrContainer, System::IntPtr::Zero);
			if (temp != System::IntPtr::Zero)
				delete (TopoDS_Shell*)(temp.ToPointer());
			System::GC::SuppressFinalize(this);
		}

#pragma region Constructors



		XbimShell::XbimShell(IIfcOpenShell^ openShell, ILogger^ logger, ModelGeometryService^ modelService) :XbimOccShape(modelService)
		{
			Init(openShell, logger);
		}

		XbimShell::XbimShell(IIfcConnectedFaceSet^ fset, ILogger^ logger, ModelGeometryService^ modelService) :XbimOccShape(modelService)
		{
			Init(fset, logger);
		}

		XbimShell::XbimShell(const TopoDS_Shell& shell, ModelGeometryService^ modelService) :XbimOccShape(modelService)
		{
			pShell = new TopoDS_Shell();
			*pShell = shell;
		}

		XbimShell::XbimShell(const TopoDS_Shell& shell, Object^ tag, ModelGeometryService^ modelService) : XbimShell(shell, modelService)
		{
			Tag = tag;
		}

		XbimShell::XbimShell(IIfcSurfaceOfLinearExtrusion^ linExt, ILogger^ logger, ModelGeometryService^ modelService) :XbimOccShape(modelService)
		{
			Init(linExt, logger);
		}


#pragma endregion

		//initialisers
		void XbimShell::Init(IIfcOpenShell^ openShell, ILogger^ logger)
		{
			XbimCompound^ shapes = gcnew XbimCompound(openShell, logger, _modelServices);
			shapes->Sew(logger);
			pShell = new TopoDS_Shell();
			*pShell = (XbimShell^)shapes->MakeShell();
			ShapeFix_ShapeTolerance tolFixer;
			tolFixer.LimitTolerance(*pShell, openShell->Model->ModelFactors->Precision);
		}

		void XbimShell::Init(IIfcConnectedFaceSet^ connectedFaceSet, ILogger^ logger)
		{
			XbimCompound^ shapes = gcnew XbimCompound(connectedFaceSet, logger, _modelServices);
			shapes->Sew(logger);
			pShell = new TopoDS_Shell();
			*pShell = (XbimShell^)shapes->MakeShell();
			ShapeFix_ShapeTolerance tolFixer;
			tolFixer.LimitTolerance(*pShell, connectedFaceSet->Model->ModelFactors->Precision);
		}

		void XbimShell::Init(IIfcSurfaceOfLinearExtrusion^ linExt, ILogger^ logger)
		{
			XbimWire^ prof = gcnew XbimWire(linExt->SweptCurve, logger, _modelServices);
			if (prof->IsValid && linExt->Depth > 0) //we have a valid wire and extrusion
			{
				IIfcDirection^ dir = linExt->ExtrudedDirection;
				gp_Vec vec(dir->X, dir->Y, dir->Z);
				vec.Normalize();
				vec *= linExt->Depth;
				BRepPrimAPI_MakePrism shellMaker(prof, vec);
				if (shellMaker.IsDone())
				{
					pShell = new TopoDS_Shell();
					*pShell = TopoDS::Shell(shellMaker.Shape());
					if (linExt->Position != nullptr)
						pShell->Move(XbimConvert::ToLocation(linExt->Position));
					ShapeFix_ShapeTolerance tolFixer;
					tolFixer.LimitTolerance(*pShell, linExt->Model->ModelFactors->Precision);
				}
				else
					XbimGeometryCreator::LogWarning(logger, linExt, "Invalid Surface Extrusion, could not create shell");
			}
			else if (linExt->Depth <= 0)
			{
				XbimGeometryCreator::LogWarning(logger, linExt, "Invalid shell surface, Extrusion Depth must be >0");
			}

		}

#pragma region Equality Overrides

		bool XbimShell::Equals(Object^ obj)
		{
			XbimShell^ s = dynamic_cast<XbimShell^>(obj);
			// Check for null
			if (s == nullptr) return false;
			return this == s;
		}

		bool XbimShell::Equals(IXbimShell^ obj)
		{
			XbimShell^ s = dynamic_cast<XbimShell^>(obj);
			if (s == nullptr) return false;
			return this == s;
		}

		int XbimShell::GetHashCode()
		{
			if (!IsValid) return 0;
			return pShell->HashCode(System::Int32::MaxValue);
		}

		bool XbimShell::operator ==(XbimShell^ left, XbimShell^ right)
		{
			// If both are null, or both are same instance, return true.
			if (System::Object::ReferenceEquals(left, right))
				return true;

			// If one is null, but not both, return false.
			if (((Object^)left == nullptr) || ((Object^)right == nullptr))
				return false;
			return  ((const TopoDS_Shell&)left).IsEqual(right) == Standard_True;

		}

		bool XbimShell::operator !=(XbimShell^ left, XbimShell^ right)
		{
			return !(left == right);
		}


#pragma endregion


#pragma region IXbimShell Interface



		IXbimFaceSet^ XbimShell::Faces::get()
		{
			if (!IsValid) return gcnew XbimFaceSet(_modelServices);
			return gcnew XbimFaceSet(*pShell, _modelServices);
		}

		IXbimEdgeSet^ XbimShell::Edges::get()
		{
			if (!IsValid) return gcnew XbimEdgeSet(_modelServices);
			return gcnew XbimEdgeSet(*pShell, _modelServices);
		}

		IXbimVertexSet^ XbimShell::Vertices::get()
		{
			if (!IsValid) return gcnew XbimVertexSet(_modelServices);
			return gcnew XbimVertexSet(*pShell, _modelServices);
		}

		bool XbimShell::IsPolyhedron::get()
		{
			if (!IsValid) return false;
			for (TopExp_Explorer exp(*pShell, TopAbs_FACE); exp.More(); exp.Next())
			{
				Handle(Geom_Surface) s = BRep_Tool::Surface(TopoDS::Face(exp.Current()));
				GeomLib_IsPlanarSurface tester(s);
				if (!tester.IsPlanar())
					return false;
			}
			System::GC::KeepAlive(this);
			//all faces are planar
			return true;
		}


		XbimRect3D XbimShell::BoundingBox::get()
		{
			if (pShell == nullptr)return XbimRect3D::Empty;
			Bnd_Box pBox;
			if (IsPolyhedron)
				BRepBndLib::AddClose(*pShell, pBox);
			else
				BRepBndLib::Add(*pShell, pBox);
			Standard_Real srXmin, srYmin, srZmin, srXmax, srYmax, srZmax;
			if (pBox.IsVoid()) return XbimRect3D::Empty;
			pBox.Get(srXmin, srYmin, srZmin, srXmax, srYmax, srZmax);
			System::GC::KeepAlive(this);
			return XbimRect3D(srXmin, srYmin, srZmin, (srXmax - srXmin), (srYmax - srYmin), (srZmax - srZmin));
		}

		double XbimShell::SurfaceArea::get()
		{
			if (!IsValid) return 0;
			GProp_GProps gProps;
			BRepGProp::SurfaceProperties(*pShell, gProps);
			System::GC::KeepAlive(this);
			return gProps.Mass();
		}

		bool XbimShell::IsEmpty::get()
		{
			if (!IsValid) return true;
			TopTools_IndexedMapOfShape faceMap;
			TopExp::MapShapes(TopoDS::Shell(*pShell), TopAbs_FACE, faceMap);
			if (faceMap.Extent() > 0) return false; //if we find a face in a shell we have something to work with			
			return true;
		}

		//returns true if the shell is a closed manifold solid
		bool XbimShell::IsClosed::get()
		{
			if (!IsValid) return false;
			BRepCheck_Shell checker(*pShell);
			BRepCheck_Status result = checker.Closed();
			System::GC::KeepAlive(this);
			return result == BRepCheck_NoError;

		}

		IXbimGeometryObject^ XbimShell::Transform(XbimMatrix3D matrix3D)
		{
			if (!IsValid) return nullptr;
			gp_Trsf trans = XbimConvert::ToTransform(matrix3D);
			BRepBuilderAPI_Transform gTran(this, trans, Standard_True);
			System::GC::KeepAlive(this);
			return gcnew XbimSolid(TopoDS::Solid(gTran.Shape()), _modelServices);
		}

		IXbimGeometryObject^ XbimShell::TransformShallow(XbimMatrix3D matrix3D)
		{
			if (!IsValid) return nullptr;
			gp_Trsf trans = XbimConvert::ToTransform(matrix3D);
			BRepBuilderAPI_Transform gTran(this, trans, Standard_False);
			System::GC::KeepAlive(this);
			return gcnew XbimSolid(TopoDS::Solid(gTran.Shape()), _modelServices);
		}

		IXbimGeometryObjectSet^ XbimShell::Cut(IXbimSolidSet^ solidTools, double tolerance, ILogger^ logger)
		{
			TopoDS_ListOfShape arguments;
			TopoDS_ListOfShape tools;
			arguments.Append(this);
			for each (auto solid in solidTools)
				tools.Append(static_cast<XbimSolid^>(solid));
			if (tolerance > 0)
				return gcnew XbimGeometryObjectSet(_modelServices->GetBooleanFactory()->Cut(arguments, tools, tolerance), _modelServices);
			else
				return gcnew XbimGeometryObjectSet(_modelServices->GetBooleanFactory()->Cut(arguments, tools), _modelServices);
		}

		IXbimGeometryObjectSet^ XbimShell::Union(IXbimSolidSet^ solidTools, double tolerance, ILogger^ logger)
		{
			TopoDS_ListOfShape arguments;
			TopoDS_ListOfShape tools;
			arguments.Append(this);
			for each (auto solid in solidTools)
				tools.Append(static_cast<XbimSolid^>(solid));
			if (tolerance > 0)
				return gcnew XbimGeometryObjectSet(_modelServices->GetBooleanFactory()->Union(arguments, tools, tolerance), _modelServices);
			else
				return gcnew XbimGeometryObjectSet(_modelServices->GetBooleanFactory()->Union(arguments, tools), _modelServices);
		}



		IXbimGeometryObjectSet^ XbimShell::Intersection(IXbimSolidSet^ solidTools, double tolerance, ILogger^ logger)
		{
			TopoDS_ListOfShape arguments;
			TopoDS_ListOfShape tools;
			arguments.Append(this);
			for each (auto solid in solidTools)
				tools.Append(static_cast<XbimSolid^>(solid));
			if (tolerance > 0)
				return gcnew XbimGeometryObjectSet(_modelServices->GetBooleanFactory()->Intersect(arguments, tools, tolerance), _modelServices);
			else
				return gcnew XbimGeometryObjectSet(_modelServices->GetBooleanFactory()->Intersect(arguments, tools), _modelServices);
		}


		IXbimGeometryObjectSet^ XbimShell::Cut(IXbimSolid^ solid, double tolerance, ILogger^ logger)
		{
			TopoDS_ListOfShape arguments;
			TopoDS_ListOfShape tools;
			arguments.Append(this);
			tools.Append(static_cast<XbimSolid^>(solid));
			if (tolerance > 0)
				return gcnew XbimGeometryObjectSet(_modelServices->GetBooleanFactory()->Cut(arguments, tools, tolerance), _modelServices);
			else
				return gcnew XbimGeometryObjectSet(_modelServices->GetBooleanFactory()->Cut(arguments, tools), _modelServices);
		}

		IXbimGeometryObjectSet^ XbimShell::Union(IXbimSolid^ solid, double tolerance, ILogger^ logger)
		{
			TopoDS_ListOfShape arguments;
			TopoDS_ListOfShape tools;
			arguments.Append(this);
			tools.Append(static_cast<XbimSolid^>(solid));
			if (tolerance > 0)
				return gcnew XbimGeometryObjectSet(_modelServices->GetBooleanFactory()->Union(arguments, tools, tolerance), _modelServices);
			else
				return gcnew XbimGeometryObjectSet(_modelServices->GetBooleanFactory()->Union(arguments, tools), _modelServices);
		}

		IXbimGeometryObjectSet^ XbimShell::Intersection(IXbimSolid^ solid, double tolerance, ILogger^ logger)
		{
			TopoDS_ListOfShape arguments;
			TopoDS_ListOfShape tools;
			arguments.Append(this);
			tools.Append(static_cast<XbimSolid^>(solid));
			if (tolerance > 0)
				return gcnew XbimGeometryObjectSet(_modelServices->GetBooleanFactory()->Intersect(arguments, tools, tolerance), _modelServices);
			else
				return gcnew XbimGeometryObjectSet(_modelServices->GetBooleanFactory()->Intersect(arguments, tools), _modelServices);
		}

		IXbimFaceSet^ XbimShell::Section(IXbimFace^ toSection, double tolerance, ILogger^ logger)
		{
			if (!IsValid || !toSection->IsValid) return gcnew XbimFaceSet(_modelServices);
			XbimFace^ faceSection = dynamic_cast<XbimFace^>(toSection);
			if (faceSection == nullptr)  throw gcnew System::ArgumentException("Only faces created by Xbim.OCC modules are supported", "toSection");

			ShapeFix_ShapeTolerance fixTol;
			fixTol.SetTolerance(faceSection, tolerance);
			fixTol.SetTolerance(this, tolerance);
			BRepAlgoAPI_Section boolOp(this, faceSection, false);
			boolOp.ComputePCurveOn2(Standard_True);
			boolOp.Build();
			if (boolOp.IsDone())
			{
				Handle(TopTools_HSequenceOfShape) edges = new TopTools_HSequenceOfShape();
				Handle(TopTools_HSequenceOfShape) wires = new TopTools_HSequenceOfShape();
				for (TopExp_Explorer expl(boolOp.Shape(), TopAbs_EDGE); expl.More(); expl.Next())
					edges->Append(TopoDS::Edge(expl.Current()));

				ShapeAnalysis_FreeBounds::ConnectEdgesToWires(edges, tolerance, false, wires);
				TopoDS_Compound open;
				TopoDS_Compound closed;
				BRep_Builder b;
				b.MakeCompound(open);
				b.MakeCompound(closed);
				ShapeAnalysis_FreeBounds::DispatchWires(wires, closed, open);
				BRepAlgo_FaceRestrictor fr;
				TopoDS_Shape aLocalS = boolOp.Shape2().Oriented(TopAbs_FORWARD);
				fr.Init(TopoDS::Face(aLocalS), Standard_False, Standard_True);
				for (TopExp_Explorer exp(closed, TopAbs_WIRE); exp.More(); exp.Next())
				{
					TopoDS_Wire wireLocal = TopoDS::Wire(exp.Current());
					fr.Add(wireLocal);
				}
				fr.Perform();
				if (fr.IsDone())
				{
					TopTools_ListOfShape result;
					TopAbs_Orientation orientationOfFace = boolOp.Shape2().Orientation();
					for (; fr.More(); fr.Next()) {
						result.Append(fr.Current().Oriented(orientationOfFace));
					}
					return gcnew XbimFaceSet(result, _modelServices);
				}
			}
			XbimGeometryCreator::LogWarning(logger, this, "Boolean Section operation has failed to create a section");
			return gcnew XbimFaceSet(_modelServices);
		}



		void XbimShell::Orientate()
		{
			if (IsValid)
			{
				BRepClass3d_SolidClassifier class3d(this);
				class3d.PerformInfinitePoint(Precision::Confusion());
				if (class3d.State() == TopAbs_IN)
					this->Reverse();
			}
		}

		bool XbimShell::HasValidTopology::get()
		{
			if (!IsValid) return false;
			BRepCheck_Analyzer analyser(*pShell, Standard_True);
			System::GC::KeepAlive(this);
			return analyser.IsValid() == Standard_True;
		}

		void XbimShell::FixTopology()
		{
			TopoDS_Shell shell = this;
			std::string errMsg;
			XbimNativeApi::FixShell(shell, 10, errMsg);
			*pShell = shell;
		}

		XbimGeometryObject^ XbimShell::Transformed(IIfcCartesianTransformationOperator^ transformation)
		{
			IIfcCartesianTransformationOperator3DnonUniform^ nonUniform = dynamic_cast<IIfcCartesianTransformationOperator3DnonUniform^>(transformation);
			if (nonUniform != nullptr)
			{
				gp_GTrsf trans = XbimConvert::ToTransform(nonUniform);
				BRepBuilderAPI_GTransform tr(this, trans, Standard_True); //make a copy of underlying shape
				System::GC::KeepAlive(this);
				return gcnew XbimShell(TopoDS::Shell(tr.Shape()), Tag, _modelServices);
			}
			else
			{
				gp_Trsf trans = XbimConvert::ToTransform(transformation);
				BRepBuilderAPI_Transform tr(this, trans, Standard_False); //do not make a copy of underlying shape
				System::GC::KeepAlive(this);
				return gcnew XbimShell(TopoDS::Shell(tr.Shape()), Tag, _modelServices);
			}
		}

		void XbimShell::Move(IIfcAxis2Placement3D^ position)
		{
			if (!IsValid) return;
			gp_Trsf toPos = XbimConvert::ToTransform(position);
			pShell->Move(toPos);
		}

		void XbimShell::Move(TopLoc_Location loc)
		{
			if (IsValid) pShell->Move(loc);
		}
		XbimGeometryObject^ XbimShell::Moved(IIfcPlacement^ placement)
		{
			if (!IsValid) return this;
			XbimShell^ copy = gcnew XbimShell(this, Tag, _modelServices); //take a copy of the shape
			TopLoc_Location loc = XbimConvert::ToLocation(placement);
			copy->Move(loc);
			return copy;
		}

		XbimGeometryObject^ XbimShell::Moved(IIfcObjectPlacement^ objectPlacement, ILogger^ logger)
		{
			if (!IsValid) return this;
			XbimShell^ copy = gcnew XbimShell(this, Tag, _modelServices); //take a copy of the shape
			TopLoc_Location loc = XbimConvert::ToLocation(objectPlacement, logger, _modelServices);
			copy->Move(loc);
			return copy;
		}

		//makes the shell a solid, note does not check if the shell IsClosed, so can make solids that are not closed or manifold
		IXbimSolid^ XbimShell::MakeSolid()
		{
			if (IsValid)
			{
				BRepBuilderAPI_MakeSolid solidMaker(this);
				if (solidMaker.IsDone())
				{
					TopoDS_Solid solid = solidMaker.Solid();
					try
					{
						BRepClass3d_SolidClassifier class3d(solid);
						class3d.PerformInfinitePoint(Precision::Confusion());
						if (class3d.State() == TopAbs_IN)
							solid.Reverse();
					}
					catch (const Standard_Failure& /*sf*/)
					{
						//String^ err = gcnew String(sf.GetMessageString());	
						//XbimGeometryCreator::LogWarning(logger, this, "Could not build a correct solid from the shell: " + err);
					}
					return gcnew XbimSolid(solid, _modelServices);
				}
			}
			return gcnew XbimSolid(_modelServices); //return an invalid solid if the shell is not valid
		}

		void XbimShell::SaveAsBrep(System::String^ fileName)
		{
			if (IsValid)
			{
				XbimOccWriter^ occWriter = gcnew XbimOccWriter();
				occWriter->Write(this, fileName);
			}
		}

#pragma endregion

#pragma region Initialisers

#pragma endregion

		void XbimShell::Reverse()
		{
			if (!IsValid) return;
			pShell->Reverse();
		}



	}
}