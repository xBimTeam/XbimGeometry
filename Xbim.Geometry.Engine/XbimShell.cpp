
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
#include "XbimGeometryCreator.h"
#include "XbimGeometryObjectSet.h"
#include "XbimSolidSet.h"
#include "XbimVertexSet.h"
#include "XbimFaceSet.h"
#include "XbimEdgeSet.h"
#include "XbimShellSet.h"
#include "XbimConvert.h"
#include "XbimOccWriter.h"

using namespace Xbim::Common::Exceptions;
namespace Xbim
{
	namespace Geometry
	{
		/*Ensures native pointers are deleted and garbage collected*/
		void XbimShellV5::InstanceCleanup()
		{
			System::IntPtr temp = System::Threading::Interlocked::Exchange(ptrContainer, System::IntPtr::Zero);
			if (temp != System::IntPtr::Zero)
				delete (TopoDS_Shell*)(temp.ToPointer());
			System::GC::SuppressFinalize(this);
		}

#pragma region Constructors

		XbimShellV5::XbimShellV5()
		{
		}

		XbimShellV5::XbimShellV5(IIfcOpenShell^ openShell, ILogger^ logger)
		{
			Init(openShell, logger);
		}

		XbimShellV5::XbimShellV5(IIfcConnectedFaceSet^ fset, ILogger^ logger)
		{
			Init(fset, logger);
		}

		XbimShellV5::XbimShellV5(const TopoDS_Shell& shell)
		{
			pShell = new TopoDS_Shell();
			*pShell = shell;
		}

		XbimShellV5::XbimShellV5(const TopoDS_Shell& shell, Object^ tag) : XbimShellV5(shell)
		{
			Tag = tag;
		}

		XbimShellV5::XbimShellV5(IIfcSurfaceOfLinearExtrusion^ linExt, ILogger^ logger)
		{
			Init(linExt, logger);
		}


#pragma endregion

		//initialisers
		void XbimShellV5::Init(IIfcOpenShell^ openShell, ILogger^ logger)
		{
			XbimCompoundV5^ shapes = gcnew XbimCompoundV5(openShell, logger);
			shapes->Sew(logger);
			pShell = new TopoDS_Shell();
			*pShell = (XbimShellV5^)shapes->MakeShell();
			ShapeFix_ShapeTolerance tolFixer;
			tolFixer.LimitTolerance(*pShell, openShell->Model->ModelFactors->Precision);
		}

		void XbimShellV5::Init(IIfcConnectedFaceSet^ connectedFaceSet, ILogger^ logger)
		{
			XbimCompoundV5^ shapes = gcnew XbimCompoundV5(connectedFaceSet, logger);
			shapes->Sew(logger);
			pShell = new TopoDS_Shell();
			*pShell = (XbimShellV5^)shapes->MakeShell();
			ShapeFix_ShapeTolerance tolFixer;
			tolFixer.LimitTolerance(*pShell, connectedFaceSet->Model->ModelFactors->Precision);
		}

		void XbimShellV5::Init(IIfcSurfaceOfLinearExtrusion^ linExt, ILogger^ logger)
		{
			XbimWireV5^ prof = gcnew XbimWireV5(linExt->SweptCurve, logger, XbimConstraints::None);
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

		bool XbimShellV5::Equals(Object^ obj)
		{
			XbimShellV5^ s = dynamic_cast<XbimShellV5^>(obj);
			// Check for null
			if (s == nullptr) return false;
			return this == s;
		}

		bool XbimShellV5::Equals(IXbimShell^ obj)
		{
			XbimShellV5^ s = dynamic_cast<XbimShellV5^>(obj);
			if (s == nullptr) return false;
			return this == s;
		}

		int XbimShellV5::GetHashCode()
		{
			if (!IsValid) return 0;
			return pShell->HashCode(System::Int32::MaxValue);
		}

		bool XbimShellV5::operator ==(XbimShellV5^ left, XbimShellV5^ right)
		{
			// If both are null, or both are same instance, return true.
			if (System::Object::ReferenceEquals(left, right))
				return true;

			// If one is null, but not both, return false.
			if (((Object^)left == nullptr) || ((Object^)right == nullptr))
				return false;
			return  ((const TopoDS_Shell&)left).IsEqual(right) == Standard_True;

		}

		bool XbimShellV5::operator !=(XbimShellV5^ left, XbimShellV5^ right)
		{
			return !(left == right);
		}


#pragma endregion


#pragma region IXbimShell Interface



		IXbimFaceSet^ XbimShellV5::Faces::get()
		{
			if (!IsValid) return XbimFaceSet::Empty;
			return gcnew XbimFaceSet(*pShell);
		}

		IXbimEdgeSet^ XbimShellV5::Edges::get()
		{
			if (!IsValid) return XbimEdgeSet::Empty;
			return gcnew XbimEdgeSet(*pShell);
		}

		IXbimVertexSet^ XbimShellV5::Vertices::get()
		{
			if (!IsValid) return XbimVertexSet::Empty;
			return gcnew XbimVertexSet(*pShell);
		}

		bool XbimShellV5::IsPolyhedron::get()
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


		XbimRect3D XbimShellV5::BoundingBox::get()
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

		double XbimShellV5::SurfaceArea::get()
		{
			if (!IsValid) return 0;
			GProp_GProps gProps;
			BRepGProp::SurfaceProperties(*pShell, gProps);
			System::GC::KeepAlive(this);
			return gProps.Mass();
		}

		bool XbimShellV5::IsEmpty::get()
		{
			if (!IsValid) return true;
			TopTools_IndexedMapOfShape faceMap;
			TopExp::MapShapes(TopoDS::Shell(*pShell), TopAbs_FACE, faceMap);
			if (faceMap.Extent() > 0) return false; //if we find a face in a shell we have something to work with			
			return true;
		}

		//returns true if the shell is a closed manifold solid
		bool XbimShellV5::IsClosed::get()
		{
			if (!IsValid) return false;
			BRepCheck_Shell checker(*pShell);
			BRepCheck_Status result = checker.Closed();
			System::GC::KeepAlive(this);
			return result == BRepCheck_NoError;

		}

		IXbimGeometryObject^ XbimShellV5::Transform(XbimMatrix3D matrix3D)
		{
			if (!IsValid) return nullptr;
			gp_Trsf trans = XbimConvert::ToTransform(matrix3D);
			BRepBuilderAPI_Transform gTran(this, trans, Standard_True);
			System::GC::KeepAlive(this);
			return gcnew XbimSolidV5(TopoDS::Solid(gTran.Shape()));
		}

		IXbimGeometryObject^ XbimShellV5::TransformShallow(XbimMatrix3D matrix3D)
		{
			if (!IsValid) return nullptr;
			gp_Trsf trans = XbimConvert::ToTransform(matrix3D);
			BRepBuilderAPI_Transform gTran(this, trans, Standard_False);
			System::GC::KeepAlive(this);
			return gcnew XbimSolidV5(TopoDS::Solid(gTran.Shape()));
		}

		IXbimGeometryObjectSet^ XbimShellV5::Cut(IXbimSolidSet^ solids, double tolerance, ILogger^ logger)
		{

			return XbimGeometryObjectSet::PerformBoolean(BOPAlgo_CUT, this, solids, tolerance, logger);
		}


		IXbimGeometryObjectSet^ XbimShellV5::Cut(IXbimSolid^ solid, double tolerance, ILogger^ logger)
		{

			return XbimGeometryObjectSet::PerformBoolean(BOPAlgo_CUT, this, gcnew XbimSolidSet(solid), tolerance, logger);
		}

		IXbimGeometryObjectSet^ XbimShellV5::Union(IXbimSolidSet^ solids, double tolerance, ILogger^ logger)
		{

			return XbimGeometryObjectSet::PerformBoolean(BOPAlgo_FUSE, this, solids, tolerance, logger);
		}

		IXbimGeometryObjectSet^ XbimShellV5::Union(IXbimSolid^ solid, double tolerance, ILogger^ logger)
		{

			return XbimGeometryObjectSet::PerformBoolean(BOPAlgo_FUSE, this, gcnew XbimSolidSet(solid), tolerance, logger);
		}

		IXbimGeometryObjectSet^ XbimShellV5::Intersection(IXbimSolidSet^ solids, double tolerance, ILogger^ logger)
		{

			return XbimGeometryObjectSet::PerformBoolean(BOPAlgo_COMMON, this, solids, tolerance, logger);
		}


		IXbimGeometryObjectSet^ XbimShellV5::Intersection(IXbimSolid^ solid, double tolerance, ILogger^ logger)
		{

			return XbimGeometryObjectSet::PerformBoolean(BOPAlgo_COMMON, this, gcnew XbimSolidSet(solid), tolerance, logger);
		}

		IXbimFaceSet^ XbimShellV5::Section(IXbimFace^ toSection, double tolerance, ILogger^ logger)
		{
			if (!IsValid || !toSection->IsValid) return XbimFaceSet::Empty;
			XbimFaceV5^ faceSection = dynamic_cast<XbimFaceV5^>(toSection);
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
					return gcnew XbimFaceSet(result);
				}
			}
			XbimGeometryCreator::LogWarning(logger, this, "Boolean Section operation has failed to create a section");
			return XbimFaceSet::Empty;
		}



		void XbimShellV5::Orientate()
		{
			if (IsValid)
			{
				BRepClass3d_SolidClassifier class3d(this);
				class3d.PerformInfinitePoint(Precision::Confusion());
				if (class3d.State() == TopAbs_IN)
					this->Reverse();
			}
		}

		bool XbimShellV5::HasValidTopology::get()
		{
			if (!IsValid) return false;
			BRepCheck_Analyzer analyser(*pShell, Standard_True);
			System::GC::KeepAlive(this);
			return analyser.IsValid() == Standard_True;
		}

		void XbimShellV5::FixTopology()
		{
			TopoDS_Shell shell = this;
			std::string errMsg;
			XbimNativeApi::FixShell(shell, 10, errMsg);
			*pShell = shell;
		}

		XbimGeometryObject^ XbimShellV5::Transformed(IIfcCartesianTransformationOperator^ transformation)
		{
			IIfcCartesianTransformationOperator3DnonUniform^ nonUniform = dynamic_cast<IIfcCartesianTransformationOperator3DnonUniform^>(transformation);
			if (nonUniform != nullptr)
			{
				gp_GTrsf trans = XbimConvert::ToTransform(nonUniform);
				BRepBuilderAPI_GTransform tr(this, trans, Standard_True); //make a copy of underlying shape
				System::GC::KeepAlive(this);
				return gcnew XbimShellV5(TopoDS::Shell(tr.Shape()), Tag);
			}
			else
			{
				gp_Trsf trans = XbimConvert::ToTransform(transformation);
				BRepBuilderAPI_Transform tr(this, trans, Standard_False); //do not make a copy of underlying shape
				System::GC::KeepAlive(this);
				return gcnew XbimShellV5(TopoDS::Shell(tr.Shape()), Tag);
			}
		}

		void XbimShellV5::Move(TopLoc_Location loc)
		{
			if (IsValid) pShell->Move(loc);
		}
		XbimGeometryObject^ XbimShellV5::Moved(IIfcPlacement^ placement)
		{
			if (!IsValid) return this;
			XbimShellV5^ copy = gcnew XbimShellV5(this, Tag); //take a copy of the shape
			TopLoc_Location loc = XbimConvert::ToLocation(placement);
			copy->Move(loc);
			return copy;
		}

		XbimGeometryObject^ XbimShellV5::Moved(IIfcObjectPlacement^ objectPlacement, ILogger^ logger)
		{
			if (!IsValid) return this;
			XbimShellV5^ copy = gcnew XbimShellV5(this, Tag); //take a copy of the shape
			TopLoc_Location loc = XbimConvert::ToLocation(objectPlacement, logger);
			copy->Move(loc);
			return copy;
		}

		//makes the shell a solid, note does not check if the shell IsClosed, so can make solids that are not closed or manifold
		IXbimSolid^ XbimShellV5::MakeSolid()
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
					return gcnew XbimSolidV5(solid);
				}
			}
			return gcnew XbimSolidV5(); //return an invalid solid if the shell is not valid
		}

		void XbimShellV5::SaveAsBrep(System::String^ fileName)
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

		void XbimShellV5::Reverse()
		{
			if (!IsValid) return;
			pShell->Reverse();
		}



	}
}