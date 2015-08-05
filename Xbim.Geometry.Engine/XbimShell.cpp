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
#include "XbimGeomPrim.h"


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
#include <BRepPrimAPI_MakePrism.hxx>
#include <ShapeFix_Shell.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepTools.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
using namespace System;
using namespace Xbim::Common::Exceptions;
namespace Xbim
{
	namespace Geometry
	{
		/*Ensures native pointers are deleted and garbage collected*/
		void XbimShell::InstanceCleanup()
		{
			IntPtr temp = System::Threading::Interlocked::Exchange(ptrContainer, IntPtr::Zero);
			if (temp != IntPtr::Zero)
				delete (TopoDS_Shell*)(temp.ToPointer());
			System::GC::SuppressFinalize(this);
		}

#pragma region Constructors

		XbimShell::XbimShell()
		{
		}

		XbimShell::XbimShell(IfcOpenShell^ openShell)
		{
			Init(openShell);
		}

		XbimShell::XbimShell(IfcConnectedFaceSet^ fset)
		{
			Init(fset);
		}

		XbimShell::XbimShell(const TopoDS_Shell& shell)
		{
			pShell = new TopoDS_Shell();
			*pShell = shell;
		}

		XbimShell::XbimShell(IfcSurfaceOfLinearExtrusion^ linExt)
		{
			Init(linExt);
		}


#pragma endregion

		//initialisers
		void XbimShell::Init(IfcOpenShell^ openShell)
		{
			XbimCompound^ shapes = gcnew XbimCompound(openShell);
			shapes->Sew();
			pShell = new TopoDS_Shell();
			*pShell = (XbimShell^)shapes->MakeShell();
		}

		void XbimShell::Init(IfcConnectedFaceSet^ connectedFaceSet)
		{
			XbimCompound^ shapes = gcnew XbimCompound(connectedFaceSet);
			shapes->Sew();
			pShell = new TopoDS_Shell();
			*pShell = (XbimShell^)shapes->MakeShell();
		}

		void XbimShell::Init(IfcSurfaceOfLinearExtrusion ^ linExt)
		{
			XbimWire^ prof = gcnew XbimWire(linExt->SweptCurve);
			if (prof->IsValid && linExt->Depth > 0) //we have a valid wire and extrusion
			{
				IfcDirection^ dir = linExt->ExtrudedDirection;
				gp_Vec vec(dir->X, dir->Y, dir->Z);
				vec *= linExt->Depth;
				BRepPrimAPI_MakePrism shellMaker(prof, vec);
				if (shellMaker.IsDone())
				{
					pShell = new TopoDS_Shell();
					*pShell = TopoDS::Shell(shellMaker.Shape());
					pShell->Move(XbimGeomPrim::ToLocation(linExt->Position));
				}
				else
					XbimGeometryCreator::logger->WarnFormat("WH006: Invalid Surface Extrusion, could not create shell, found in Entity #{0}=IfcSurfaceOfLinearExtrusion.",
					linExt->EntityLabel);
			}
			else if (linExt->Depth <= 0)
			{
				XbimGeometryCreator::logger->WarnFormat("WS007: Invalid Solid Surface, Extrusion Depth must be >0, found in Entity #{0}=IfcSurfaceOfLinearExtrusion.",
					linExt->EntityLabel);
			}
			
		}

#pragma region Equality Overrides

		bool XbimShell::Equals(Object^ obj)
		{
			XbimShell^ s = dynamic_cast< XbimShell^>(obj);
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
			return pShell->HashCode(Int32::MaxValue);
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
			if (!IsValid) return XbimFaceSet::Empty;
			return gcnew XbimFaceSet(*pShell);
		}

		IXbimEdgeSet^ XbimShell::Edges::get()
		{
			if (!IsValid) return XbimEdgeSet::Empty;
			return gcnew XbimEdgeSet(*pShell);
		}

		IXbimVertexSet^ XbimShell::Vertices::get()
		{
			if (!IsValid) return XbimVertexSet::Empty;
			return gcnew XbimVertexSet(*pShell);
		}

		bool XbimShell::IsPolyhedron::get()
		{
			if (!IsValid) return false;
			for (TopExp_Explorer exp(*pShell, TopAbs_EDGE); exp.More(); exp.Next())
			{
				Standard_Real start, end;
				Handle(Geom_Curve) c3d = BRep_Tool::Curve(TopoDS::Edge(exp.Current()),  start, end);
				if (!c3d.IsNull())
				{
					Handle(Standard_Type) cType = c3d->DynamicType();
					if (cType != STANDARD_TYPE(Geom_Line))
					{
						if (cType != STANDARD_TYPE(Geom_TrimmedCurve)) return false;
						Handle(Geom_TrimmedCurve) tc = Handle(Geom_TrimmedCurve)::DownCast(c3d);
						Handle(Standard_Type) tcType = tc->DynamicType();
						if (tcType != STANDARD_TYPE(Geom_Line)) return false;
					}
				}
			}
			GC::KeepAlive(this);
			//all edges are lines
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
			GC::KeepAlive(this);
			return XbimRect3D(srXmin, srYmin, srZmin, (srXmax - srXmin), (srYmax - srYmin), (srZmax - srZmin));
		}
	
		double XbimShell::SurfaceArea::get()
		{
			if (!IsValid) return 0;
			GProp_GProps gProps;
			BRepGProp::SurfaceProperties(*pShell, gProps);
			GC::KeepAlive(this);
			return gProps.Mass();	
		}

		
		//returns true if the shell is a closed manifold solid
		bool XbimShell::IsClosed::get()
		{
			if (!IsValid) return false;
			BRepCheck_Shell checker(*pShell);
			BRepCheck_Status result = checker.Closed();	
			GC::KeepAlive(this);
			return result == BRepCheck_NoError;
			
		}

		IXbimGeometryObject^ XbimShell::Transform(XbimMatrix3D matrix3D)
		{
			BRepBuilderAPI_Copy copier(this);
			BRepBuilderAPI_Transform gTran(copier.Shape(), XbimGeomPrim::ToTransform(matrix3D));
			TopoDS_Shell temp = TopoDS::Shell(gTran.Shape());
			return gcnew XbimShell(temp);
		}

		IXbimGeometryObject^ XbimShell::TransformShallow(XbimMatrix3D matrix3D)
		{
			TopoDS_Shell shell = TopoDS::Shell(pShell->Moved(XbimGeomPrim::ToTransform(matrix3D)));
			GC::KeepAlive(this);
			return gcnew XbimShell(shell);
		}

		IXbimGeometryObjectSet^ XbimShell::Cut(IXbimSolidSet^ solids, double tolerance)
		{

			return XbimGeometryObjectSet::PerformBoolean(BOPAlgo_CUT, this, solids, tolerance);
		}


		IXbimGeometryObjectSet^ XbimShell::Cut(IXbimSolid^ solid, double tolerance)
		{
			
			return XbimGeometryObjectSet::PerformBoolean(BOPAlgo_CUT,this, gcnew XbimSolidSet(solid), tolerance);
		}

		IXbimGeometryObjectSet^ XbimShell::Union(IXbimSolidSet^ solids, double tolerance)
		{

			return XbimGeometryObjectSet::PerformBoolean(BOPAlgo_FUSE,this, solids, tolerance);
		}

		IXbimGeometryObjectSet^ XbimShell::Union(IXbimSolid^ solid, double tolerance)
		{
			
			return XbimGeometryObjectSet::PerformBoolean(BOPAlgo_FUSE, this, gcnew XbimSolidSet(solid), tolerance);
		}

		IXbimGeometryObjectSet^ XbimShell::Intersection(IXbimSolidSet^ solids, double tolerance)
		{

			return XbimGeometryObjectSet::PerformBoolean(BOPAlgo_COMMON,this, solids, tolerance);
		}


		IXbimGeometryObjectSet^ XbimShell::Intersection(IXbimSolid^ solid, double tolerance)
		{
		
			return XbimGeometryObjectSet::PerformBoolean(BOPAlgo_COMMON, this, gcnew XbimSolidSet(solid), tolerance);
		}

		IXbimFaceSet^ XbimShell::Section(IXbimFace^ toSection, double tolerance)
		{
			if (!IsValid || !toSection->IsValid) return XbimFaceSet::Empty;
			XbimFace^ faceSection = dynamic_cast<XbimFace^>(toSection);
			if (faceSection == nullptr)  throw gcnew ArgumentException("Only faces created by Xbim.OCC modules are supported", "toSection");

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
			XbimGeometryCreator::logger->WarnFormat("WS008:Boolean Section operation has failed to create a section");
			return XbimFaceSet::Empty;
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
			GC::KeepAlive(this);
			return analyser.IsValid() == Standard_True;
		}

		void XbimShell::FixTopology()
		{
			ShapeFix_Shell fixer(this);
			fixer.Perform();
			const TopoDS_Shape& fixed = fixer.Shape();
			if (fixed.ShapeType() == TopAbs_SHELL)
				*pShell = TopoDS::Shell(fixed);			
		}

		//makes the shell a solid, note does not check if the shell IsClosed, so can make solids that are not closed or manifold
		IXbimSolid^ XbimShell::MakeSolid()
		{
			if (IsValid)
			{
				BRepClass3d_SolidClassifier class3d(this);
				class3d.PerformInfinitePoint(Precision::Confusion());
				if (class3d.State() == TopAbs_IN) 
					this->Reverse();
				BRepBuilderAPI_MakeSolid solidMaker(this);
				if (solidMaker.IsDone())
					return gcnew XbimSolid(solidMaker.Solid());
			}
			return gcnew XbimSolid(); //return an invalid solid if the shell is not valid
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