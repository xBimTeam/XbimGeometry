#include "XbimEdge.h"
#include "XbimGeometryCreator.h"
#include "XbimVertex.h"
#include "XbimGeomPrim.h"
#include "XbimCurve.h"
#include "XbimGeomPrim.h"

#include <BRepBuilderAPI_Transform.hxx>

#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <gp_Pnt.hxx>
#include <BRep_Builder.hxx>
#include <GC_MakeLine.hxx>
#include <gp_Ax2.hxx>
#include <gp_Ax3.hxx>
#include <gp_Circ.hxx>
#include <GC_MakeCircle.hxx>
#include <gp_Elips.hxx>
#include <GC_MakeEllipse.hxx>
#include <ShapeFix_ShapeTolerance.hxx>
#include <BRep_Tool.hxx>
#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>
#include <TopExp.hxx>
#include <Geom_BezierCurve.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>

using namespace Xbim::Common;

namespace Xbim
{
	namespace Geometry
	{




#pragma region IXbimEdge Interface
		XbimEdge::XbimEdge(const TopoDS_Edge& edge)
		{
			pEdge = new TopoDS_Edge();
			*pEdge = edge;
		}


		IXbimCurve^ XbimEdge::EdgeGeometry::get()
		{
			if (!IsValid) return nullptr;
			Standard_Real p1, p2;
			Handle(Geom_Curve) curve = BRep_Tool::Curve(*pEdge, p1, p2);
			GC::KeepAlive(this);
			return gcnew XbimCurve(curve, p1, p2);
		}

		double XbimEdge::Length::get()
		{
			if (IsValid)
			{
				GProp_GProps gProps;
				BRepGProp::LinearProperties(*pEdge, gProps);
				GC::KeepAlive(this);
				return gProps.Mass();
			}
			else
				return 0;
		}
		XbimEdge::XbimEdge(IXbimVertex^ edgeStart, IXbimVertex^ edgeEnd)
		{

			if (!dynamic_cast<XbimVertex^>(edgeStart))
				throw gcnew ArgumentException("Edge start vertex not created by XbimOCC", "edgeEnd");
			if (!dynamic_cast<XbimVertex^>(edgeEnd))
				throw gcnew ArgumentException("Edge end vertex not created by XbimOCC", "edgeStart");

			BRepBuilderAPI_MakeEdge edgeMaker((XbimVertex^)edgeStart, (XbimVertex^)edgeEnd);
			pEdge = new TopoDS_Edge();
			BRepBuilderAPI_EdgeError edgeErr = edgeMaker.Error();
			if (edgeErr != BRepBuilderAPI_EdgeDone)
			{
				String^ errMsg = XbimEdge::GetBuildEdgeErrorMessage(edgeErr);
				throw gcnew Exception(String::Format("Invalid edge vertices. {0}", errMsg));
			}
			else
				*pEdge = edgeMaker.Edge();
		}
#pragma endregion


#pragma region Constructors

		XbimEdge::XbimEdge(IfcConic^ edge)
		{
			Init(edge);
		}

		XbimEdge::XbimEdge(IfcCurve^ edge)
		{
			Init(edge);
		}

		XbimEdge::XbimEdge(IfcCircle^ edge)
		{
			Init(edge);
		}

		XbimEdge::XbimEdge(IfcLine^ edge)
		{
			Init(edge);
		}

		XbimEdge::XbimEdge(IfcEllipse^ edge)
		{
			Init(edge);
		}
		
		XbimEdge::XbimEdge(IfcBSplineCurve^ edge)
		{
			Init(edge);
		}

		XbimEdge::XbimEdge(IfcBezierCurve^ edge)
		{
			Init(edge);
		}

		XbimEdge::XbimEdge(IfcRationalBezierCurve^ edge)
		{
			Init(edge);
		}
#pragma endregion





		/*Ensures native pointers are deleted and garbage collected*/
		void XbimEdge::InstanceCleanup()
		{
			IntPtr temp = System::Threading::Interlocked::Exchange(ptrContainer, IntPtr::Zero);
			if (temp != IntPtr::Zero)
				delete (TopoDS_Edge*)(temp.ToPointer());
			System::GC::SuppressFinalize(this);
		}


#pragma region Equality Overrides

		bool XbimEdge::Equals(Object^ obj)
		{
			XbimEdge^ e = dynamic_cast< XbimEdge^>(obj);
			// Check for null
			if (e == nullptr) return false;
			return this == e;
		}

		bool XbimEdge::Equals(IXbimEdge^ obj)
		{
			XbimEdge^ e = dynamic_cast< XbimEdge^>(obj);
			// Check for null
			if (e == nullptr) return false;
			return this == e;
		}

		int XbimEdge::GetHashCode()
		{
			if (!IsValid) return 0;
			return pEdge->HashCode(Int32::MaxValue);
		}

		bool XbimEdge::operator ==(XbimEdge^ left, XbimEdge^ right)
		{
			// If both are null, or both are same instance, return true.
			if (System::Object::ReferenceEquals(left, right))
				return true;

			// If one is null, but not both, return false.
			if (((Object^)left == nullptr) || ((Object^)right == nullptr))
				return false;
			//this edge comparer does not conseider orientation
			return  ((const TopoDS_Edge&)left).IsSame(right) == Standard_True;

		}

		bool XbimEdge::operator !=(XbimEdge^ left, XbimEdge^ right)
		{
			return !(left == right);
		}


#pragma endregion


#pragma region Properties

		IXbimVertex^ XbimEdge::EdgeStart::get()
		{
			if (!IsValid) return nullptr;
			return gcnew XbimVertex(TopExp::FirstVertex(*pEdge, Standard_True));
		}

		IXbimVertex^ XbimEdge::EdgeEnd::get()
		{
			if (!IsValid) return nullptr;
			return gcnew XbimVertex(TopExp::LastVertex(*pEdge, Standard_True));
		}

		IXbimGeometryObject^ XbimEdge::Transform(XbimMatrix3D matrix3D)
		{
			BRepBuilderAPI_Copy copier(this);
			BRepBuilderAPI_Transform gTran(copier.Shape(), XbimGeomPrim::ToTransform(matrix3D));
			TopoDS_Edge temp = TopoDS::Edge(gTran.Shape());
			return gcnew XbimEdge(temp);
		}

		XbimRect3D XbimEdge::BoundingBox::get()
		{
			if (pEdge == nullptr)return XbimRect3D::Empty;
			Bnd_Box pBox;
			BRepBndLib::Add(*pEdge, pBox);
			Standard_Real srXmin, srYmin, srZmin, srXmax, srYmax, srZmax;
			if (pBox.IsVoid()) return XbimRect3D::Empty;
			pBox.Get(srXmin, srYmin, srZmin, srXmax, srYmax, srZmax);
			GC::KeepAlive(this);
			return XbimRect3D(srXmin, srYmin, srZmin, (srXmax - srXmin), (srYmax - srYmin), (srZmax - srZmin));
		}

#pragma endregion

		String^ XbimEdge::GetBuildEdgeErrorMessage(BRepBuilderAPI_EdgeError edgeErr)
		{
			switch (edgeErr)
			{
			case BRepBuilderAPI_PointProjectionFailed:
				return "Point Projection Failed";
			case BRepBuilderAPI_ParameterOutOfRange:
				return "Parameter Out Of Range";
			case BRepBuilderAPI_DifferentPointsOnClosedCurve:
				return "Different Points On Closed Curve";
			case BRepBuilderAPI_PointWithInfiniteParameter:
				return "Point With Infinite Parameter";
			case BRepBuilderAPI_DifferentsPointAndParameter:
				return "Differents Point And Parameter";
			case BRepBuilderAPI_LineThroughIdenticPoints:
				return "Line Through Identical Points";
			default:
				return "Unknown Error";
			}
		}

		
#pragma region Initialisers

		void XbimEdge::Init(IfcCurve^ curve)
		{
			IfcLine^ line = dynamic_cast<IfcLine^>(curve);
			if (line != nullptr) return Init(line);
			IfcConic^ conic = dynamic_cast<IfcConic^>(curve);
			if (conic != nullptr) return Init(conic);
			throw gcnew NotImplementedException(String::Format("Curve of Type {0} in entity #{1} is not implemented", curve->GetType()->Name, curve->EntityLabel));
		}

		void XbimEdge::Init(IfcConic^ conic)
		{
			IfcCircle^ circle = dynamic_cast<IfcCircle^>(conic);
			if (circle != nullptr) return Init(circle);
			IfcEllipse^ ellipse = dynamic_cast<IfcEllipse^>(conic);
			if (ellipse != nullptr) return Init(ellipse);
		}

		void XbimEdge::Init(IfcCircle^ circle)
		{
			Handle(Geom_Curve) curve;
			if (dynamic_cast<IfcAxis2Placement2D^>(circle->Position))
			{
				IfcAxis2Placement2D^ ax2 = (IfcAxis2Placement2D^)circle->Position;
				gp_Ax2 gpax2(gp_Pnt(ax2->Location->X, ax2->Location->Y, 0), gp_Dir(0, 0, 1), gp_Dir(ax2->P[0].X, ax2->P[0].Y, 0.));
				gp_Circ gc(gpax2, circle->Radius);
				curve = GC_MakeCircle(gc);
			}
			else if (dynamic_cast<IfcAxis2Placement3D^>(circle->Position))
			{
				IfcAxis2Placement3D^ ax2 = (IfcAxis2Placement3D^)circle->Position;
				gp_Ax3 	gpax3 = XbimGeomPrim::ToAx3(ax2);
				gp_Circ gc(gpax3.Ax2(), circle->Radius);
				curve = GC_MakeCircle(gc);
			}
			else
			{
				Type ^ type = circle->Position->GetType();
				XbimGeometryCreator::logger->ErrorFormat("WE001: Circle #{0} with Placement of type {1} is not implemented", circle->EntityLabel, type->Name);
				return;
			}
			BRepBuilderAPI_MakeEdge edgeMaker(curve);
			BRepBuilderAPI_EdgeError edgeErr = edgeMaker.Error();
			if (edgeErr != BRepBuilderAPI_EdgeDone)
			{
				String^ errMsg = XbimEdge::GetBuildEdgeErrorMessage(edgeErr);
				XbimGeometryCreator::logger->WarnFormat("WE002: Invalid edge found in IfcCircle = #{0}, {1}. It has been ignored", circle->EntityLabel, errMsg);
			}
			else
			{
				pEdge = new TopoDS_Edge();
				*pEdge = edgeMaker.Edge();
				// set the tolerance for this shape.
				ShapeFix_ShapeTolerance FTol;
				FTol.SetTolerance(*pEdge, circle->ModelOf->ModelFactors->Precision, TopAbs_VERTEX);
			}
		}

		void XbimEdge::Init(IfcLine^ line)
		{
			IfcCartesianPoint^ cp = line->Pnt;
			IfcVector^ dir = line->Dir;
			gp_Pnt pnt(cp->X, cp->Y, cp->Z);
			XbimVector3D v3d = dir->XbimVector3D();
			gp_Vec vec(v3d.X, v3d.Y, v3d.Z);
			BRepBuilderAPI_MakeEdge edgeMaker(GC_MakeLine(pnt, vec), 0, dir->Magnitude);
			BRepBuilderAPI_EdgeError edgeErr = edgeMaker.Error();
			if (edgeErr != BRepBuilderAPI_EdgeDone)
			{
				String^ errMsg = XbimEdge::GetBuildEdgeErrorMessage(edgeErr);
				XbimGeometryCreator::logger->WarnFormat("WE003: Invalid edge found in IfcLine = #{0}, {1}. It has been ignored", line->EntityLabel, errMsg);
			}
			else
			{
				pEdge = new TopoDS_Edge();
				*pEdge = edgeMaker.Edge();
				// set the tolerance for this shape.
				ShapeFix_ShapeTolerance FTol;
				FTol.SetTolerance(*pEdge, line->ModelOf->ModelFactors->Precision, TopAbs_VERTEX);
			}
		}

		void XbimEdge::Init(IfcEllipse^ ellipse)
		{
			IfcAxis2Placement2D^ ax2 = (IfcAxis2Placement2D^)ellipse->Position;
			gp_Ax2 gpax2(gp_Pnt(ax2->Location->X, ax2->Location->Y, 0), gp_Dir(0, 0, 1), gp_Dir(ax2->P[0].X, ax2->P[0].Y, 0.));
			double semiAx1 = ellipse->SemiAxis1;
			double semiAx2 = ellipse->SemiAxis2;
			if (semiAx1 <= 0)
			{
				XbimModelFactors^ mf = ellipse->ModelOf->ModelFactors;
				semiAx1 = mf->OneMilliMetre;
				XbimGeometryCreator::logger->WarnFormat("WE004: Illegal Ellipse Semi Axis 1, must be greater than 0, in entity #{0}, it has been set to 1mm.", ellipse->EntityLabel);
			}
			if (semiAx2 <= 0)
			{
				XbimModelFactors^ mf = ellipse->ModelOf->ModelFactors;
				semiAx2 = mf->OneMilliMetre;
				XbimGeometryCreator::logger->WarnFormat("WE005: Illegal Ellipse Semi Axis 2, must be greater than 0, in entity #{0}, it has been set to 1mm.", ellipse->EntityLabel);
			}
			gp_Elips gc(gpax2, semiAx1, semiAx2);
			Handle(Geom_Ellipse) hellipse = GC_MakeEllipse(gc);
			BRepBuilderAPI_MakeEdge edgeMaker(hellipse);
			pEdge = new TopoDS_Edge();
			*pEdge = edgeMaker.Edge();
			ShapeFix_ShapeTolerance FTol;
			FTol.SetTolerance(*pEdge, ellipse->ModelOf->ModelFactors->Precision, TopAbs_VERTEX);
		}

		void XbimEdge::Init(IfcBSplineCurve^ bspline)
		{
			IfcBezierCurve^ bez = dynamic_cast<IfcBezierCurve^>(bspline);
			if (bez != nullptr)
				Init(bez);
			else
				XbimGeometryCreator::logger->WarnFormat("WE006: Unsupported IfcBSplineCurve type #{0} found. Ignored", bspline->EntityLabel);

		}

		void XbimEdge::Init(IfcBezierCurve^ bez)
		{
			IfcRationalBezierCurve^ ratBez = dynamic_cast<IfcRationalBezierCurve^>(bez);
			if (ratBez != nullptr)
				Init(ratBez);
			else
			{
				TColgp_Array1OfPnt pnts(1, bez->ControlPointsList->Count);
				int i = 1;
				for each (IfcCartesianPoint^ cp in bez->ControlPointsList)
				{
					pnts.SetValue(i, gp_Pnt(cp->X, cp->Y, cp->Z));
					i++;
				}
				Handle(Geom_BezierCurve) hBez(new Geom_BezierCurve(pnts));
				BRepBuilderAPI_MakeEdge edgeMaker(hBez);
				pEdge = new TopoDS_Edge();
				*pEdge = edgeMaker.Edge();
				ShapeFix_ShapeTolerance FTol;
				FTol.SetTolerance(*pEdge, bez->ModelOf->ModelFactors->Precision, TopAbs_VERTEX);
			}
		}

		void XbimEdge::Init(IfcRationalBezierCurve^ bez)
		{
			TColgp_Array1OfPnt pnts(1, bez->ControlPointsList->Count);
			int i = 1;
			for each (IfcCartesianPoint^ cp in bez->ControlPointsList)
			{
				pnts.SetValue(i, gp_Pnt(cp->X, cp->Y, cp->Z));
				i++;
			}
			TColStd_Array1OfReal weights(1, bez->WeightsData->Count);
			i = 1;
			for each (double weight in bez->WeightsData)
			{
				weights.SetValue(i, weight);
				i++;
			}

			Handle(Geom_BezierCurve) hBez(new Geom_BezierCurve(pnts, weights));
			BRepBuilderAPI_MakeEdge edgeMaker(hBez);
			pEdge = new TopoDS_Edge();
			*pEdge = edgeMaker.Edge();
			ShapeFix_ShapeTolerance FTol;
			FTol.SetTolerance(*pEdge, bez->ModelOf->ModelFactors->Precision, TopAbs_VERTEX);
		}
#pragma endregion

	}
}