#include "XbimEdge.h"
#include "XbimGeometryCreator.h"
#include "XbimVertex.h"
#include "XbimConvert.h"
#include "XbimCurve.h"
#include "XbimCurve2D.h"
#include "XbimConvert.h"
#include "XbimWire.h"
#include "XbimFace.h"
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepBuilderAPI_GTransform.hxx>
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
#include <BRepLib.hxx>
#include <ShapeFix_Shape.hxx>
#include <TColGeom_SequenceOfCurve.hxx>
#include <TopTools_SequenceOfShape.hxx>
#include <TColStd_SequenceOfReal.hxx>
#include <GeomAbs_CurveType.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <ElCLib.hxx>
#include <TColGeom_Array1OfBSplineCurve.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <GeomConvert.hxx>
#include <GeomLProp.hxx>
#include <TColGeom_HArray1OfBSplineCurve.hxx>
#include <TColStd_HArray1OfInteger.hxx>
#include <GeomConvert_CompCurveToBSplineCurve.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <ShapeCustom_BSplineRestriction.hxx>
#include <GC_MakeCircle.hxx>
#include <GeomLib.hxx>
#include <gce_MakeElips.hxx>
#include <ShapeFix_Edge.hxx>
using namespace Xbim::Common;
using namespace System::Linq;
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
		XbimEdge::XbimEdge(const TopoDS_Edge& edge, Object^ tag) : XbimEdge(edge) { Tag = tag; }

		IXbimCurve^ XbimEdge::EdgeGeometry::get()
		{
			if (!IsValid) return nullptr;
			Standard_Real p1, p2;
			Handle(Geom_Curve) curve = BRep_Tool::Curve(*pEdge, p1, p2);
			GC::KeepAlive(this);
			return gcnew XbimCurve(curve);
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

		XbimEdge::XbimEdge(IIfcPcurve^ edge)
		{
			Init(edge);
		}

		XbimEdge::XbimEdge(IIfcConic^ edge)
		{
			Init(edge);
		}

		XbimEdge::XbimEdge(IIfcCurve^ edge)
		{
			Init(edge);
		}

		XbimEdge::XbimEdge(IIfcCircle^ edge)
		{
			Init(edge);
		}

		XbimEdge::XbimEdge(IIfcLine^ edge)
		{
			Init(edge);
		}

		XbimEdge::XbimEdge(IIfcEllipse^ edge)
		{
			Init(edge);
		}
		
		XbimEdge::XbimEdge(IIfcBSplineCurve^ edge)
		{
			Init(edge);
		}

		XbimEdge::XbimEdge(IIfcBSplineCurveWithKnots^ edge)
		{
			Init(edge);
		}
		XbimEdge::XbimEdge(IIfcRationalBSplineCurveWithKnots^ edge)
		{
			Init(edge);
		}

		

		XbimEdge::XbimEdge(XbimVertex^ start, XbimVertex^ midPoint, XbimVertex^ end)
		{
			
			gp_Pnt p1 = BRep_Tool::Pnt(start);
			gp_Pnt p2 = BRep_Tool::Pnt(midPoint);
			gp_Pnt p3 = BRep_Tool::Pnt(end);
			GC_MakeCircle circleMaker(p1, p2, p3);
			if (circleMaker.IsDone())
			{				 
				 Handle(Geom_Circle) curve = circleMaker.Value();
				BRepBuilderAPI_MakeEdge edgeMaker(curve, start, end);
				BRepBuilderAPI_EdgeError edgeErr = edgeMaker.Error();
				if (edgeErr != BRepBuilderAPI_EdgeDone)
				{
					String^ errMsg = XbimEdge::GetBuildEdgeErrorMessage(edgeErr);
					throw gcnew XbimException("WW013: Invalid edge found." + errMsg);
				}
				else
				{
					pEdge = new TopoDS_Edge();
					*pEdge = edgeMaker.Edge();

				}
				
			}
			else //IFC4 documentation says make it a linear edge
			{
				BRepBuilderAPI_MakeEdge edgeMaker(start, end);
				BRepBuilderAPI_EdgeError edgeErr = edgeMaker.Error();
				if (edgeErr != BRepBuilderAPI_EdgeDone)
				{
					String^ errMsg = XbimEdge::GetBuildEdgeErrorMessage(edgeErr);
					throw gcnew XbimException("WW013: Invalid edge found." + errMsg);
				}
				else
				{
					pEdge = new TopoDS_Edge();
					*pEdge = edgeMaker.Edge();
				}
			}

		}
		XbimEdge::XbimEdge(XbimEdge^ edgeCurve, XbimVertex^ start, XbimVertex^ end, double maxTolerance)
		{
			double tolerance = Math::Max(start->Tolerance, end->Tolerance);
			double currentTolerance = tolerance;
			if (start->Equals(end))
			{
				//must be a closed loop or nothing

				
				pEdge = new TopoDS_Edge();
				*pEdge = edgeCurve;
			}
			else
			{
				Standard_Real p1, p2;
				Handle(Geom_Curve) curve = BRep_Tool::Curve(edgeCurve, p1, p2);
				ShapeFix_ShapeTolerance FTol;
			TryMakeEdge:
				BRepBuilderAPI_MakeEdge edgeMaker(curve, start, end);
				BRepBuilderAPI_EdgeError edgeErr = edgeMaker.Error();
				if (edgeErr != BRepBuilderAPI_EdgeDone)
				{
					currentTolerance *= 10;
					if (currentTolerance <= maxTolerance)
					{
						FTol.SetTolerance(start, currentTolerance);
						FTol.SetTolerance(end, currentTolerance);
						goto TryMakeEdge;
					}
				}

				if (edgeErr != BRepBuilderAPI_EdgeDone)
				{
					String^ errMsg = XbimEdge::GetBuildEdgeErrorMessage(edgeErr);
					throw gcnew XbimException("WW013: Invalid edge found." + errMsg);
				}
				else
				{
					pEdge = new TopoDS_Edge();
					*pEdge = edgeMaker.Edge();
				}
			}
		}

		XbimEdge::XbimEdge(IIfcCurve^ edgeCurve, XbimVertex^ start, XbimVertex^ end)
		{
			Init(edgeCurve);
			double tolerance = edgeCurve->Model->ModelFactors->Precision;
			double toleranceMax = edgeCurve->Model->ModelFactors->PrecisionMax;
			double currentTolerance = tolerance;
			if (IsValid &&!start->Equals(end))//must be a closed loop or nothing, no need to trim			
			{
				Standard_Real p1, p2;
				Handle(Geom_Curve)  curve = BRep_Tool::Curve(this, p1, p2);
				curve = BRep_Tool::Curve(this, p1, p2);
				ShapeFix_ShapeTolerance FTol;
				
			TryMakeEdge:				
				BRepBuilderAPI_MakeEdge edgeMaker(curve, start, end);
				/*if(pnt1->Equals(pnt2))
					throw gcnew XbimException("WW013: Invalid edge found.");*/
				BRepBuilderAPI_EdgeError edgeErr = edgeMaker.Error();
				if (edgeErr != BRepBuilderAPI_EdgeDone)
				{
					currentTolerance *= 10;
					if (currentTolerance <= toleranceMax)
					{
						FTol.SetTolerance(start, currentTolerance);
						FTol.SetTolerance(end, currentTolerance);
						goto TryMakeEdge;
					}
				}
				
				if (edgeErr != BRepBuilderAPI_EdgeDone)
				{
					String^ errMsg = XbimEdge::GetBuildEdgeErrorMessage(edgeErr);
					throw gcnew XbimException("WW013: Invalid edge found." + errMsg);
				}
				else
				{
					*pEdge = edgeMaker.Edge();
				}

			}
			
		}


		XbimEdge::XbimEdge(const TopoDS_Wire& aWire, double tolerance, double angleTolerance)
		{
			
			TopoDS_Edge ResEdge;

			BRepLib::BuildCurves3d(aWire);
			ShapeFix_Shape fixer(aWire);
			fixer.SetPrecision(tolerance);
			fixer.SetMaxTolerance(tolerance);
			fixer.Perform();
			TopoDS_Wire theWire = TopoDS::Wire(fixer.Shape());

			TColGeom_SequenceOfCurve CurveSeq;
			TopTools_SequenceOfShape LocSeq;
			TColStd_SequenceOfReal FparSeq;
			TColStd_SequenceOfReal LparSeq;
			TColStd_SequenceOfReal TolSeq;
			GeomAbs_CurveType CurType;
			TopoDS_Vertex FirstVertex, LastVertex;
			Standard_Real aPntShiftDist = 0.;

			BRepTools_WireExplorer wexp(theWire);
			for (; wexp.More(); wexp.Next())
			{
				TopoDS_Edge anEdge = wexp.Current();
				Standard_Real fpar, lpar;
				TopLoc_Location aLoc;
				Handle(Geom_Curve) aCurve = BRep_Tool::Curve(anEdge, aLoc, fpar, lpar);
				if (aCurve.IsNull())
					continue;

				BRepAdaptor_Curve BAcurve(anEdge);
				GeomAbs_CurveType aType = BAcurve.GetType();

				Handle(Geom_Curve) aBasisCurve = BAcurve.Curve().Curve();

				if (aBasisCurve->IsPeriodic())
					ElCLib::AdjustPeriodic(aBasisCurve->FirstParameter(), aBasisCurve->LastParameter(),
					Precision::PConfusion(), fpar, lpar);

				if (CurveSeq.IsEmpty())
				{
					CurveSeq.Append(aCurve);
					TopoDS_Shape aLocShape;
					aLocShape.Location(aLoc);
					aLocShape.Orientation(wexp.Orientation());
					LocSeq.Append(aLocShape);
					FparSeq.Append(fpar);
					LparSeq.Append(lpar);
					CurType = aType;
					FirstVertex = wexp.CurrentVertex();
				}
				else
				{
					Standard_Boolean Done = Standard_False;
					Standard_Real NewFpar, NewLpar;
					GeomAdaptor_Curve GAprevcurve(CurveSeq.Last());
					TopoDS_Vertex CurVertex = wexp.CurrentVertex();
					TopoDS_Vertex CurFirstVer = TopExp::FirstVertex(anEdge);
					TopAbs_Orientation ConnectByOrigin = (CurVertex.IsSame(CurFirstVer)) ? TopAbs_FORWARD : TopAbs_REVERSED;
					if (aCurve == CurveSeq.Last())
					{
						NewFpar = fpar;
						NewLpar = lpar;
						if (aBasisCurve->IsPeriodic())
						{
							if (NewLpar < NewFpar)
								NewLpar += aBasisCurve->Period();
							if (ConnectByOrigin == TopAbs_FORWARD)
								ElCLib::AdjustPeriodic(FparSeq.Last(),
								FparSeq.Last() + aBasisCurve->Period(),
								Precision::PConfusion(), NewFpar, NewLpar);
							else
								ElCLib::AdjustPeriodic(FparSeq.Last() - aBasisCurve->Period(),
								FparSeq.Last(),
								Precision::PConfusion(), NewFpar, NewLpar);
						}
						Done = Standard_True;
					}
					else if (aType == CurType &&
						aType != GeomAbs_BezierCurve &&
						aType != GeomAbs_BSplineCurve &&
						aType != GeomAbs_OtherCurve)
					{
						switch (aType)
						{
						case GeomAbs_Line:
						{
							gp_Lin aLine = BAcurve.Line();
							gp_Lin PrevLine = GAprevcurve.Line();
							if (aLine.Contains(PrevLine.Location(), tolerance) &&
								aLine.Direction().IsParallel(PrevLine.Direction(), angleTolerance))
							{
								gp_Pnt P1 = ElCLib::Value(fpar, aLine);
								gp_Pnt P2 = ElCLib::Value(lpar, aLine);
								NewFpar = ElCLib::Parameter(PrevLine, P1);
								NewLpar = ElCLib::Parameter(PrevLine, P2);

								// Compute shift
								if (ConnectByOrigin == TopAbs_FORWARD) {
									gp_Pnt aNewP2 = ElCLib::Value(NewLpar, PrevLine);

									aPntShiftDist += P2.Distance(aNewP2);
								}
								else {
									gp_Pnt aNewP1 = ElCLib::Value(NewFpar, PrevLine);

									aPntShiftDist += P1.Distance(aNewP1);
								}

								if (NewLpar < NewFpar)
								{
									Standard_Real MemNewFpar = NewFpar;
									NewFpar = NewLpar;
									NewLpar = MemNewFpar;
									ConnectByOrigin = TopAbs::Reverse(ConnectByOrigin);
								}
								Done = Standard_True;
							}
							break;
						}
						case GeomAbs_Circle:
						{
							gp_Circ aCircle = BAcurve.Circle();
							gp_Circ PrevCircle = GAprevcurve.Circle();
							if (aCircle.Location().Distance(PrevCircle.Location()) <= tolerance &&
								Abs(aCircle.Radius() - PrevCircle.Radius()) <= tolerance &&
								aCircle.Axis().IsParallel(PrevCircle.Axis(), angleTolerance))
							{
								const Standard_Boolean isFwd = ConnectByOrigin == TopAbs_FORWARD;

								if (aCircle.Axis().Direction() * PrevCircle.Axis().Direction() < 0.)
								{
									Standard_Real memfpar = fpar;
									fpar = lpar;
									lpar = memfpar;
									ConnectByOrigin = TopAbs::Reverse(ConnectByOrigin);
								}
								gp_Pnt P1 = ElCLib::Value(fpar, aCircle);
								gp_Pnt P2 = ElCLib::Value(lpar, aCircle);
								NewFpar = ElCLib::Parameter(PrevCircle, P1);
								NewLpar = ElCLib::Parameter(PrevCircle, P2);

								// Compute shift
								if (isFwd) {
									gp_Pnt aNewP2 = ElCLib::Value(NewLpar, PrevCircle);

									aPntShiftDist += P2.Distance(aNewP2);
								}
								else {
									gp_Pnt aNewP1 = ElCLib::Value(NewFpar, PrevCircle);

									aPntShiftDist += P1.Distance(aNewP1);
								}

								if (NewLpar < NewFpar)
									NewLpar += 2.*M_PI;
								//Standard_Real MemNewFpar = NewFpar, MemNewLpar =  NewLpar;
								if (ConnectByOrigin == TopAbs_FORWARD)
									ElCLib::AdjustPeriodic(FparSeq.Last(),
									FparSeq.Last() + 2.*M_PI,
									Precision::PConfusion(), NewFpar, NewLpar);
								else
									ElCLib::AdjustPeriodic(FparSeq.Last() - 2.*M_PI,
									FparSeq.Last(),
									Precision::PConfusion(), NewFpar, NewLpar);
								Done = Standard_True;
							}
							break;
						}
						case GeomAbs_Ellipse:
						{
							gp_Elips anEllipse = BAcurve.Ellipse();
							gp_Elips PrevEllipse = GAprevcurve.Ellipse();
							if (anEllipse.Focus1().Distance(PrevEllipse.Focus1()) <= tolerance &&
								anEllipse.Focus2().Distance(PrevEllipse.Focus2()) <= tolerance &&
								Abs(anEllipse.MajorRadius() - PrevEllipse.MajorRadius()) <= tolerance &&
								Abs(anEllipse.MinorRadius() - PrevEllipse.MinorRadius()) <= tolerance &&
								anEllipse.Axis().IsParallel(PrevEllipse.Axis(), angleTolerance))
							{
								const Standard_Boolean isFwd = ConnectByOrigin == TopAbs_FORWARD;

								if (anEllipse.Axis().Direction() * PrevEllipse.Axis().Direction() < 0.)
								{
									Standard_Real memfpar = fpar;
									fpar = lpar;
									lpar = memfpar;
									ConnectByOrigin = TopAbs::Reverse(ConnectByOrigin);
								}
								gp_Pnt P1 = ElCLib::Value(fpar, anEllipse);
								gp_Pnt P2 = ElCLib::Value(lpar, anEllipse);
								NewFpar = ElCLib::Parameter(PrevEllipse, P1);
								NewLpar = ElCLib::Parameter(PrevEllipse, P2);

								// Compute shift
								if (isFwd) {
									gp_Pnt aNewP2 = ElCLib::Value(NewLpar, PrevEllipse);

									aPntShiftDist += P2.Distance(aNewP2);
								}
								else {
									gp_Pnt aNewP1 = ElCLib::Value(NewFpar, PrevEllipse);

									aPntShiftDist += P1.Distance(aNewP1);
								}

								if (NewLpar < NewFpar)
									NewLpar += 2.*M_PI;
								if (ConnectByOrigin == TopAbs_FORWARD)
									ElCLib::AdjustPeriodic(FparSeq.Last(),
									FparSeq.Last() + 2.*M_PI,
									Precision::PConfusion(), NewFpar, NewLpar);
								else
									ElCLib::AdjustPeriodic(FparSeq.Last() - 2.*M_PI,
									FparSeq.Last(),
									Precision::PConfusion(), NewFpar, NewLpar);
								Done = Standard_True;
							}
							break;
						}
						case GeomAbs_Hyperbola:
						{
							gp_Hypr aHypr = BAcurve.Hyperbola();
							gp_Hypr PrevHypr = GAprevcurve.Hyperbola();
							if (aHypr.Focus1().Distance(PrevHypr.Focus1()) <= tolerance &&
								aHypr.Focus2().Distance(PrevHypr.Focus2()) <= tolerance &&
								Abs(aHypr.MajorRadius() - PrevHypr.MajorRadius()) <= tolerance &&
								Abs(aHypr.MinorRadius() - PrevHypr.MinorRadius()) <= tolerance &&
								aHypr.Axis().IsParallel(PrevHypr.Axis(), angleTolerance))
							{
								gp_Pnt P1 = ElCLib::Value(fpar, aHypr);
								gp_Pnt P2 = ElCLib::Value(lpar, aHypr);
								NewFpar = ElCLib::Parameter(PrevHypr, P1);
								NewLpar = ElCLib::Parameter(PrevHypr, P2);

								// Compute shift
								if (ConnectByOrigin == TopAbs_FORWARD) {
									gp_Pnt aNewP2 = ElCLib::Value(NewLpar, PrevHypr);

									aPntShiftDist += P2.Distance(aNewP2);
								}
								else {
									gp_Pnt aNewP1 = ElCLib::Value(NewFpar, PrevHypr);

									aPntShiftDist += P1.Distance(aNewP1);
								}

								if (NewLpar < NewFpar)
								{
									Standard_Real MemNewFpar = NewFpar;
									NewFpar = NewLpar;
									NewLpar = MemNewFpar;
									ConnectByOrigin = TopAbs::Reverse(ConnectByOrigin);
								}
								Done = Standard_True;
							}
							break;
						}
						case GeomAbs_Parabola:
						{
							gp_Parab aParab = BAcurve.Parabola();
							gp_Parab PrevParab = GAprevcurve.Parabola();
							if (aParab.Location().Distance(PrevParab.Location()) <= tolerance &&
								aParab.Focus().Distance(PrevParab.Focus()) <= tolerance &&
								Abs(aParab.Focal() - PrevParab.Focal()) <= tolerance &&
								aParab.Axis().IsParallel(PrevParab.Axis(), angleTolerance))
							{
								gp_Pnt P1 = ElCLib::Value(fpar, aParab);
								gp_Pnt P2 = ElCLib::Value(lpar, aParab);
								NewFpar = ElCLib::Parameter(PrevParab, P1);
								NewLpar = ElCLib::Parameter(PrevParab, P2);

								// Compute shift
								if (ConnectByOrigin == TopAbs_FORWARD) {
									gp_Pnt aNewP2 = ElCLib::Value(NewLpar, PrevParab);

									aPntShiftDist += P2.Distance(aNewP2);
								}
								else {
									gp_Pnt aNewP1 = ElCLib::Value(NewFpar, PrevParab);

									aPntShiftDist += P1.Distance(aNewP1);
								}

								if (NewLpar < NewFpar)
								{
									Standard_Real MemNewFpar = NewFpar;
									NewFpar = NewLpar;
									NewLpar = MemNewFpar;
									ConnectByOrigin = TopAbs::Reverse(ConnectByOrigin);
								}
								Done = Standard_True;
							}
							break;
						}
						} //end of switch (aType)
					} // end of else if (aType == CurType && ...
					if (Done)
					{
						if (NewFpar < FparSeq.Last())
							FparSeq(FparSeq.Length()) = NewFpar;
						else
							LparSeq(LparSeq.Length()) = NewLpar;
					}
					else
					{
						CurveSeq.Append(aCurve);
						TopoDS_Shape aLocShape;
						aLocShape.Location(aLoc);
						aLocShape.Orientation(wexp.Orientation());
						LocSeq.Append(aLocShape);
						FparSeq.Append(fpar);
						LparSeq.Append(lpar);
						TolSeq.Append(aPntShiftDist + BRep_Tool::Tolerance(CurVertex));
						aPntShiftDist = 0.;
						CurType = aType;
					}
				} // end of else (CurveSeq.IsEmpty()) -> not first time
			} // end for (; wexp.More(); wexp.Next())

			LastVertex = wexp.CurrentVertex();
			TolSeq.Append(aPntShiftDist + BRep_Tool::Tolerance(LastVertex));

			FirstVertex.Orientation(TopAbs_FORWARD);
			LastVertex.Orientation(TopAbs_REVERSED);

			if (!CurveSeq.IsEmpty())
			{
				Standard_Integer nb_curve = CurveSeq.Length();   //number of curves
				TColGeom_Array1OfBSplineCurve tab(0, nb_curve - 1);                    //array of the curves
				TColStd_Array1OfReal tabtolvertex(0, nb_curve - 1); //(0,nb_curve-2);  //array of the tolerances

				Standard_Integer i;

				if (nb_curve > 1)
				{
					for (i = 1; i <= nb_curve; i++)
					{
						if (CurveSeq(i)->IsInstance(STANDARD_TYPE(Geom_TrimmedCurve)))
							CurveSeq(i) = (*((Handle(Geom_TrimmedCurve)*)&(CurveSeq(i))))->BasisCurve();

						Handle(Geom_TrimmedCurve) aTrCurve = new Geom_TrimmedCurve(CurveSeq(i), FparSeq(i), LparSeq(i));
						tab(i - 1) = GeomConvert::CurveToBSplineCurve(aTrCurve);
						tab(i - 1)->Transform(LocSeq(i).Location().Transformation());
						GeomConvert::C0BSplineToC1BSplineCurve(tab(i - 1), Precision::Confusion());
						if (LocSeq(i).Orientation() == TopAbs_REVERSED)
							tab(i - 1)->Reverse();

						//Temporary
						//char* name = new char[100];
						//sprintf(name, "c%d", i);
						//DrawTrSurf::Set(name, tab(i-1));

						if (i > 1)
							tabtolvertex(i - 2) = TolSeq(i - 1);
					} // end for (i = 1; i <= nb_curve; i++)
					tabtolvertex(nb_curve - 1) = TolSeq(TolSeq.Length());

					Standard_Boolean closed_flag = Standard_False;
					Standard_Real closed_tolerance = 0.;
					if (FirstVertex.IsSame(LastVertex) &&
						GeomLProp::Continuity(tab(0), tab(nb_curve - 1),
						tab(0)->FirstParameter(),
						tab(nb_curve - 1)->LastParameter(),
						Standard_False, Standard_False, tolerance, angleTolerance) >= GeomAbs_G1)
					{
						closed_flag = Standard_True;
						closed_tolerance = BRep_Tool::Tolerance(FirstVertex);
					}

					Handle(TColGeom_HArray1OfBSplineCurve)  concatcurve;     //array of the concatenated curves
					Handle(TColStd_HArray1OfInteger)        ArrayOfIndices;  //array of the remining Vertex
					GeomConvert::ConcatC1(tab,
						tabtolvertex,
						ArrayOfIndices,
						concatcurve,
						closed_flag,
						closed_tolerance);   //C1 concatenation

					if (concatcurve->Length() > 1)
					{
						GeomConvert_CompCurveToBSplineCurve Concat(concatcurve->Value(concatcurve->Lower()));

						for (i = concatcurve->Lower() + 1; i <= concatcurve->Upper(); i++)
							Concat.Add(concatcurve->Value(i), tolerance, Standard_True);

						concatcurve->SetValue(concatcurve->Lower(), Concat.BSplineCurve());
					}
					// rnc : prevents the driver from building an edge without C1 continuity
					if (concatcurve->Value(concatcurve->Lower())->Continuity() == GeomAbs_C0)
					{
						XbimGeometryCreator::LogInfo(this,"Edge from Wire construction aborted : The given Wire has sharp bends between some Edges, no valid Edge can be built");
						return;
					}

					Standard_Boolean isValidEndVtx = Standard_True;

					if (closed_flag) {
						// Check if closed curve is reordered.
						Handle(Geom_Curve) aCurve = concatcurve->Value(concatcurve->Lower());
						Standard_Real      aFPar = aCurve->FirstParameter();
						gp_Pnt             aPFirst;
						gp_Pnt             aPntVtx = BRep_Tool::Pnt(FirstVertex);
						Standard_Real      aTolVtx = BRep_Tool::Tolerance(FirstVertex);

						aCurve->D0(aFPar, aPFirst);

						if (!aPFirst.IsEqual(aPntVtx, aTolVtx)) {
							// The curve is reordered. Find the new first and last vertices.
							TopTools_IndexedMapOfShape aMapVtx;
							TopExp::MapShapes(theWire, TopAbs_VERTEX, aMapVtx);

							const Standard_Integer aNbVtx = aMapVtx.Extent();
							Standard_Integer       iVtx;

							for (iVtx = 1; iVtx <= aNbVtx; iVtx++) {
								const TopoDS_Vertex aVtx = TopoDS::Vertex(aMapVtx.FindKey(iVtx));
								const gp_Pnt        aPnt = BRep_Tool::Pnt(aVtx);
								const Standard_Real aTol = BRep_Tool::Tolerance(aVtx);

								if (aPFirst.IsEqual(aPnt, aTol)) {
									// The coinsident vertex is found.
									FirstVertex = aVtx;
									LastVertex = aVtx;
									FirstVertex.Orientation(TopAbs_FORWARD);
									LastVertex.Orientation(TopAbs_REVERSED);
									break;
								}
							}

							if (iVtx > aNbVtx) {
								// It is necessary to create new vertices.
								isValidEndVtx = Standard_False;
							}
						}
					}

					if (isValidEndVtx) {
						ResEdge = BRepLib_MakeEdge(concatcurve->Value(concatcurve->Lower()),
							FirstVertex, LastVertex,
							concatcurve->Value(concatcurve->Lower())->FirstParameter(),
							concatcurve->Value(concatcurve->Lower())->LastParameter());
					}
					else {
						ResEdge = BRepLib_MakeEdge(concatcurve->Value(concatcurve->Lower()),
							concatcurve->Value(concatcurve->Lower())->FirstParameter(),
							concatcurve->Value(concatcurve->Lower())->LastParameter());
					}
				}
				else
				{
					if (CurveSeq(1)->IsInstance(STANDARD_TYPE(Geom_TrimmedCurve)))
						CurveSeq(1) = (*((Handle(Geom_TrimmedCurve)*)&(CurveSeq(1))))->BasisCurve();

					Handle(Geom_Curve) aNewCurve =
						Handle(Geom_Curve)::DownCast(CurveSeq(1)->Copy());

					aNewCurve->Transform(LocSeq(1).Location().Transformation());

					if (LocSeq(1).Orientation() == TopAbs_REVERSED) {
						const TopoDS_Vertex aVtxTmp = FirstVertex;

						FirstVertex = LastVertex;
						LastVertex = aVtxTmp;
						FirstVertex.Orientation(TopAbs_FORWARD);
						LastVertex.Orientation(TopAbs_REVERSED);
					}

					ResEdge = BRepLib_MakeEdge(aNewCurve,
						FirstVertex, LastVertex,
						FparSeq(1), LparSeq(1));

					if (LocSeq(1).Orientation() == TopAbs_REVERSED) {
						ResEdge.Reverse();
					}
				}
			}

			pEdge = new TopoDS_Edge();
			*pEdge = ResEdge;
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

		void XbimEdge::Reverse()
		{
			if (!IsValid) return;
			pEdge->Reverse();
		}

		XbimEdge^ XbimEdge::Reversed()
		{
			XbimEdge^ revEdge = gcnew XbimEdge(this);
			revEdge->Reverse();
			return revEdge;
		}

		XbimGeometryObject ^ XbimEdge::Transformed(IIfcCartesianTransformationOperator ^ transformation)
		{
			IIfcCartesianTransformationOperator3DnonUniform^ nonUniform = dynamic_cast<IIfcCartesianTransformationOperator3DnonUniform^>(transformation);
			if (nonUniform != nullptr)
			{
				gp_GTrsf trans = XbimConvert::ToTransform(nonUniform);
				BRepBuilderAPI_GTransform tr(this, trans, Standard_True); //make a copy of underlying shape
				return gcnew XbimEdge(TopoDS::Edge(tr.Shape()), Tag);
			}
			else
			{
				gp_Trsf trans = XbimConvert::ToTransform(transformation);
				BRepBuilderAPI_Transform tr(this, trans, Standard_False); //do not make a copy of underlying shape
				return gcnew XbimEdge(TopoDS::Edge(tr.Shape()), Tag);
			}
		}

		XbimGeometryObject ^ XbimEdge::Moved(IIfcPlacement ^ placement)
		{
			if (!IsValid) return this;
			XbimEdge^ copy = gcnew XbimEdge(this, Tag); //take a copy of the shape
			TopLoc_Location loc = XbimConvert::ToLocation(placement);
			copy->Move(loc);
			return copy;
		}

		XbimGeometryObject ^ XbimEdge::Moved(IIfcObjectPlacement ^ objectPlacement)
		{
			if (!IsValid) return this;
			XbimEdge^ copy = gcnew XbimEdge(this, Tag); //take a copy of the shape
			TopLoc_Location loc = XbimConvert::ToLocation(objectPlacement);
			copy->Move(loc);
			return copy;
		}
		void XbimEdge::Move(TopLoc_Location loc)
		{
			if(IsValid) pEdge->Move(loc);
		}
		void XbimEdge::Mesh(IXbimMeshReceiver ^ mesh, double precision, double deflection, double angle)
		{
			return;//maybe add an implementation for this
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
		XbimPoint3D XbimEdge::EdgeStartPoint::get()
		{
			if (!IsValid) return XbimPoint3D();
			gp_Pnt p = BRep_Tool::Pnt(TopExp::FirstVertex(*pEdge, Standard_True));
			return XbimPoint3D(p.X(), p.Y(), p.Z());
		}

		XbimPoint3D XbimEdge::EdgeEndPoint::get()
		{
			if (!IsValid) return XbimPoint3D();
			gp_Pnt p = BRep_Tool::Pnt(TopExp::LastVertex(*pEdge, Standard_True));
			return XbimPoint3D(p.X(), p.Y(), p.Z());			
		}


		IXbimGeometryObject^ XbimEdge::Transform(XbimMatrix3D matrix3D)
		{
			BRepBuilderAPI_Copy copier(this);
			BRepBuilderAPI_Transform gTran(copier.Shape(), XbimConvert::ToTransform(matrix3D));
			TopoDS_Edge temp = TopoDS::Edge(gTran.Shape());
			return gcnew XbimEdge(temp);
		}
		
		IXbimGeometryObject^ XbimEdge::TransformShallow(XbimMatrix3D matrix3D)
		{
			TopoDS_Edge edge = TopoDS::Edge(pEdge->Moved(XbimConvert::ToTransform(matrix3D)));
			GC::KeepAlive(this);
			return gcnew XbimEdge(edge);
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

		void XbimEdge::Init(IIfcCurve^ curve)
		{
			IIfcLine^ line = dynamic_cast<IIfcLine^>(curve);
			if (line != nullptr) return Init(line);
			IIfcConic^ conic = dynamic_cast<IIfcConic^>(curve);
			if (conic != nullptr) return Init(conic);
			IIfcPolyline^ pline = dynamic_cast<IIfcPolyline^>(curve);
			if (pline != nullptr) return Init(pline);
			IIfcBSplineCurve^ bspline = dynamic_cast<IIfcBSplineCurve^>(curve);
			if (bspline != nullptr) return Init(bspline);
			IIfcPcurve^ pcurve = dynamic_cast<IIfcPcurve^>(curve);
			if (pcurve != nullptr) return Init(pcurve);
			throw gcnew NotImplementedException(String::Format("Curve of Type {0} in entity #{1} is not implemented", curve->GetType()->Name, curve->EntityLabel));
		}

		void XbimEdge::Init(IIfcPcurve^ curve)
		{
			XbimCurve^ occCurve = gcnew XbimCurve(curve);
			if (occCurve->IsValid)
			{
				XbimFace^ faceSurface = gcnew XbimFace(curve->BasisSurface);
				ShapeFix_Edge eFix;
				XbimEdge^ edge = gcnew XbimEdge(occCurve);
				eFix.FixAddPCurve(edge,faceSurface,Standard_False,curve->Model->ModelFactors->Precision);				
				pEdge = new TopoDS_Edge();
				*pEdge = edge;
				ShapeFix_ShapeTolerance FTol;
				FTol.SetTolerance(*pEdge, curve->Model->ModelFactors->Precision, TopAbs_EDGE);
			}
		}

		void XbimEdge::Init(IIfcPolyline^ pline)
		{
			if (Enumerable::Count(pline->Points) == 2)
			{
				IIfcCartesianPoint^ start= Enumerable::First(pline->Points);
				IIfcCartesianPoint^ end = Enumerable::Last(pline->Points);						
				BRepBuilderAPI_MakeEdge edgeMaker(XbimConvert::GetPoint3d(start), XbimConvert::GetPoint3d(end));
				BRepBuilderAPI_EdgeError edgeErr = edgeMaker.Error();
				if (edgeErr != BRepBuilderAPI_EdgeDone)
				{
					String^ errMsg = XbimEdge::GetBuildEdgeErrorMessage(edgeErr);
					XbimGeometryCreator::LogWarning(pline, "Invalid edge found, {0}. It has been ignored", errMsg);
				}
				else
				{
					pEdge = new TopoDS_Edge();
					*pEdge = edgeMaker.Edge();
					// set the tolerance for this shape.
					ShapeFix_ShapeTolerance FTol;
					FTol.SetTolerance(*pEdge, pline->Model->ModelFactors->Precision, TopAbs_EDGE);
				}
			}
			else
			{
				TopoDS_Wire wire = gcnew XbimWire(pline);
				IModelFactors^ mf = pline->Model->ModelFactors;
				XbimEdge^ edge = gcnew XbimEdge(wire, mf->Precision, 0.05);
				pEdge = new TopoDS_Edge();
				*pEdge = edge;
				ShapeFix_ShapeTolerance FTol;
				FTol.SetTolerance(*pEdge, pline->Model->ModelFactors->Precision, TopAbs_EDGE);
			}
		}

		void XbimEdge::Init(IIfcConic^ conic)
		{
			IIfcCircle^ circle = dynamic_cast<IIfcCircle^>(conic);
			if (circle != nullptr) return Init(circle);
			IIfcEllipse^ ellipse = dynamic_cast<IIfcEllipse^>(conic);
			if (ellipse != nullptr) return Init(ellipse);
		}

		void XbimEdge::Init(IIfcCircle^ circle)
		{
			Handle(Geom_Curve) curve;
			if (dynamic_cast<IIfcAxis2Placement2D^>(circle->Position))
			{
				IIfcAxis2Placement2D^ ax2 = (IIfcAxis2Placement2D^)circle->Position;
				gp_Ax2 gpax2(gp_Pnt(ax2->Location->X, ax2->Location->Y, 0), gp_Dir(0, 0, 1), gp_Dir(ax2->P[0].X, ax2->P[0].Y, 0.));
				gp_Circ gc(gpax2, circle->Radius);
				curve = GC_MakeCircle(gc).Value();
			}
			else if (dynamic_cast<IIfcAxis2Placement3D^>(circle->Position))
			{
				IIfcAxis2Placement3D^ ax2 = (IIfcAxis2Placement3D^)circle->Position;
				gp_Ax3 	gpax3 = XbimConvert::ToAx3(ax2);
				gp_Circ gc(gpax3.Ax2(), circle->Radius);
				curve = GC_MakeCircle(gc).Value();
			}
			else
			{
				Type ^ type = circle->Position->GetType();
				XbimGeometryCreator::LogError(circle,"Circle with placement of type {0} is not implemented", type->Name);
				return;
			}
			BRepBuilderAPI_MakeEdge edgeMaker(curve);
			BRepBuilderAPI_EdgeError edgeErr = edgeMaker.Error();
			if (edgeErr != BRepBuilderAPI_EdgeDone)
			{
				String^ errMsg = XbimEdge::GetBuildEdgeErrorMessage(edgeErr);
				XbimGeometryCreator::LogWarning("Invalid edge found in circle, {0}. It has been ignored", errMsg);
			}
			else
			{
				pEdge = new TopoDS_Edge();
				*pEdge = edgeMaker.Edge();
				// set the tolerance for this shape.
				ShapeFix_ShapeTolerance FTol;
				FTol.SetTolerance(*pEdge, circle->Model->ModelFactors->Precision, TopAbs_EDGE);
			}
		}

		void XbimEdge::Init(IIfcLine^ line)
		{
			IIfcCartesianPoint^ cp = line->Pnt;
			IIfcVector^ ifcVec = line->Dir;
			IIfcDirection^ dir = ifcVec->Orientation;
			gp_Pnt pnt(cp->X, cp->Y, cp->Z);
			XbimVector3D v3d(dir->X, dir->Y, dir->Z);
			gp_Vec vec(v3d.X, v3d.Y, v3d.Z);
			BRepBuilderAPI_MakeEdge edgeMaker(GC_MakeLine(pnt, vec).Value(), 0, ifcVec->Magnitude);
			BRepBuilderAPI_EdgeError edgeErr = edgeMaker.Error();
			if (edgeErr != BRepBuilderAPI_EdgeDone)
			{
				String^ errMsg = XbimEdge::GetBuildEdgeErrorMessage(edgeErr);
				XbimGeometryCreator::LogWarning(line,"Invalid edge found, {0}. It has been ignored", errMsg);
			}
			else
			{
				pEdge = new TopoDS_Edge();
				*pEdge = edgeMaker.Edge();
				// set the tolerance for this shape.
				ShapeFix_ShapeTolerance FTol;
				FTol.SetTolerance(*pEdge, line->Model->ModelFactors->Precision, TopAbs_EDGE);
			}
		}

		void XbimEdge::Init(IIfcEllipse^ ellipse)
		{
			XbimCurve^ curve = gcnew XbimCurve(ellipse);
			if(curve->IsValid)
			{
				BRepBuilderAPI_MakeEdge edgeMaker(curve);
				pEdge = new TopoDS_Edge();
				*pEdge = edgeMaker.Edge();
				ShapeFix_ShapeTolerance FTol;
				FTol.SetTolerance(*pEdge, ellipse->Model->ModelFactors->Precision, TopAbs_EDGE);
			}
		}

		void XbimEdge::Init(IIfcBSplineCurve^ bspline)
		{
			IIfcBSplineCurveWithKnots^ bsplineWithKnots = dynamic_cast<IIfcBSplineCurveWithKnots^>(bspline);
			if (bsplineWithKnots != nullptr) return Init(bsplineWithKnots);
			XbimGeometryCreator::LogError(bspline, "Unsupported IfcBSplineCurve type #{0} found. It has been ignored", bspline->GetType()->Name);
		}
		void XbimEdge::Init(IIfcBSplineCurveWithKnots^ bspline)
		{
			IIfcRationalBSplineCurveWithKnots^ ratBez = dynamic_cast<IIfcRationalBSplineCurveWithKnots^>(bspline);
			if (ratBez != nullptr)
				Init(ratBez);
			else
			{

				TColgp_Array1OfPnt poles(1, Enumerable::Count(bspline->ControlPointsList));
				int i = 1;
				for each (IIfcCartesianPoint^ cp in bspline->ControlPointsList)
				{
					poles.SetValue(i, gp_Pnt(cp->X, cp->Y, XbimConvert::GetZValueOrZero(cp)));
					i++;
				}
				TColStd_Array1OfReal knots(1, Enumerable::Count(bspline->Knots));
				TColStd_Array1OfInteger knotMultiplicities(1, Enumerable::Count(bspline->Knots));
				i = 1;
				for each (double knot in bspline->Knots)
				{
					knots.SetValue(i,knot);					
					i++;
				}
				i = 1;
				for each (int multiplicity in bspline->KnotMultiplicities)
				{
					knotMultiplicities.SetValue(i, multiplicity);					
					i++;
				}				
				Handle(Geom_BSplineCurve) hBez(new Geom_BSplineCurve(poles, knots, knotMultiplicities, (Standard_Integer)bspline->Degree));
				BRepBuilderAPI_MakeEdge edgeMaker(hBez);
				pEdge = new TopoDS_Edge();
				*pEdge = edgeMaker.Edge();
				ShapeFix_ShapeTolerance FTol;
				FTol.SetTolerance(*pEdge, bspline->Model->ModelFactors->Precision, TopAbs_EDGE);
			}
		}


		void XbimEdge::Init(IIfcRationalBSplineCurveWithKnots^ bspline)
		{
			TColgp_Array1OfPnt poles(1, Enumerable::Count(bspline->ControlPointsList));
			int i = 1;
			for each (IIfcCartesianPoint^ cp in bspline->ControlPointsList)
			{
				poles.SetValue(i, gp_Pnt(cp->X, cp->Y, XbimConvert::GetZValueOrZero(cp)));
				i++;
			}
			TColStd_Array1OfReal weights(1, Enumerable::Count(bspline->Weights));
		    i = 1;
			for each (double weight in bspline->WeightsData)
			{
				weights.SetValue(i, weight);
				i++;
			}

			TColStd_Array1OfReal knots(1, Enumerable::Count(bspline->Knots));
			TColStd_Array1OfInteger knotMultiplicities(1, Enumerable::Count(bspline->Knots));
			i = 1;
			for each (double knot in bspline->Knots)
			{
				knots.SetValue(i, knot);
				i++;
			}
			i = 1;
			for each (int multiplicity in bspline->KnotMultiplicities)
			{
				knotMultiplicities.SetValue(i, multiplicity);
				i++;
			}
			Handle(Geom_BSplineCurve) hBez(new Geom_BSplineCurve(poles, weights, knots, knotMultiplicities, (Standard_Integer)bspline->Degree));
			BRepBuilderAPI_MakeEdge edgeMaker(hBez);
			pEdge = new TopoDS_Edge();
			*pEdge = edgeMaker.Edge();
			ShapeFix_ShapeTolerance FTol;
			FTol.SetTolerance(*pEdge, bspline->Model->ModelFactors->Precision, TopAbs_EDGE);
		}
		
#pragma endregion
		XbimEdge::XbimEdge(XbimCurve2D^ curve2D)
		{
			XbimCurve^ curve = (XbimCurve^)curve2D->ToCurve3D();
			BRepBuilderAPI_MakeEdge edgeMaker(curve);
			pEdge = new TopoDS_Edge();
			*pEdge = edgeMaker.Edge();
		}

		XbimEdge::XbimEdge(XbimCurve^ curve3D)
		{

			BRepBuilderAPI_MakeEdge edgeMaker(curve3D);
			pEdge = new TopoDS_Edge();
			*pEdge = edgeMaker.Edge();
			
		}
	}
}