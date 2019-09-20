#include "XbimCurve.h"
#include "XbimCurve2D.h"
#include "XbimFace.h"
#include "XbimConvert.h"
#include "XbimGeometryCreator.h"
#include <gce_MakeLin.hxx>
#include <GC_MakeLine.hxx>
#include <GC_MakeCircle.hxx>
#include <GC_MakeEllipse.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TColStd_Array1OfInteger.hxx>
#include <Geom_BSplineCurve.hxx>
#include <GeomLib_Tool.hxx>
#include <GeomAPI_ExtremaCurveCurve.hxx>
#include <Geom_OffsetCurve.hxx>
#include <ShapeConstruct_ProjectCurveOnSurface.hxx>
#include <ShapeFix_Edge.hxx>
#include <BndLib_Add3dCurve.hxx>
#include <Bnd_Box.hxx>
#include <Extrema_ExtPC.hxx>
#include <ShapeFix_ShapeTolerance.hxx>
#include <GeomConvert_CompCurveToBSplineCurve.hxx>
#include <GCPnts_AbscissaPoint.hxx>

using namespace System;
using namespace System::Linq;
namespace Xbim
{
	namespace Geometry
	{
		/*Ensures native pointers are deleted and garbage collected*/
		void XbimCurve::InstanceCleanup()
		{
			IntPtr temp = System::Threading::Interlocked::Exchange(ptrContainer, IntPtr::Zero);
			if (temp != IntPtr::Zero)
				delete (Handle(Geom_Curve)*)(temp.ToPointer());
			System::GC::SuppressFinalize(this);
		}

		XbimCurve::XbimCurve(const Handle(Geom_Curve)& curve)
		{
			this->pCurve = new Handle(Geom_Curve);
			*pCurve = curve;
		}

		XbimRect3D XbimCurve::BoundingBox::get()
		{
			if (!IsValid) return XbimRect3D::Empty;
			Bnd_Box b1;
			GeomAdaptor_Curve myAdpSection;
			myAdpSection.Load(*pCurve);
			BndLib_Add3dCurve::Add(myAdpSection, 0., b1);
			Standard_Real srXmin, srYmin, srZmin, srXmax, srYmax, srZmax;
			b1.Get(srXmin, srYmin, srZmin, srXmax, srYmax, srZmax);
			GC::KeepAlive(this);
			return XbimRect3D(srXmin, srYmin, srZmin, (srXmax - srXmin), (srYmax - srYmin), (srZmax - srZmin));
		}

		XbimPoint3D XbimCurve::Start::get()
		{
			if (!IsValid) return XbimPoint3D();
			Standard_Real fp = (*pCurve)->FirstParameter();
			return GetPoint(fp);
		}

		XbimPoint3D XbimCurve::End::get()
		{
			if (!IsValid) return XbimPoint3D();
			Standard_Real lp = (*pCurve)->LastParameter();
			return GetPoint(lp);
		}

		double XbimCurve::Length::get()
		{
			return  GCPnts_AbscissaPoint::Length(GeomAdaptor_Curve(*pCurve));
		}
		bool XbimCurve::IsClosed::get()
		{
			if (!IsValid) return false;
			return (*pCurve)->IsClosed() == Standard_True;
		}


		double XbimCurve::GetParameter(XbimPoint3D point, double tolerance)
		{
			if (!IsValid) return 0;
			double u1;
			gp_Pnt p1(point.X, point.Y, point.Z);
			GeomLib_Tool::Parameter(*pCurve, p1, tolerance, u1);
			return u1;
		}

		XbimPoint3D XbimCurve::GetPoint(double parameter)
		{
			if (!IsValid) return XbimPoint3D();
			gp_Pnt pt = (*pCurve)->Value(parameter);
			return XbimPoint3D(pt.X(), pt.Y(), pt.Z());
		}



		IXbimGeometryObject^ XbimCurve::Transform(XbimMatrix3D /*matrix3D*/)
		{
			throw gcnew Exception("Tranformation of curves is not currently supported");
		}

		IXbimGeometryObject^ XbimCurve::TransformShallow(XbimMatrix3D /*matrix3D*/)
		{
			throw gcnew Exception("TransformShallow of curves is not currently supported");
		}

		IEnumerable<XbimPoint3D>^ XbimCurve::Intersections(IXbimCurve^ intersector, double tolerance, ILogger^ /*logger*/)
		{
			List<XbimPoint3D>^ intersects = gcnew List<XbimPoint3D>();
			if (!intersector->Is3D) intersector = ((XbimCurve2D^)intersector)->ToCurve3D();
			if (IsValid && intersector->IsValid)
			{
				GeomAPI_ExtremaCurveCurve extrema(*pCurve, *((XbimCurve^)intersector)->pCurve);
				for (Standard_Integer i = 0; i < extrema.NbExtrema(); i++)
				{
					gp_Pnt p1;
					gp_Pnt p2;
					extrema.Points(i + 1, p1, p2);
					if (p1.IsEqual(p2, tolerance))
						intersects->Add(XbimPoint3D(p1.X(), p1.Y(), p1.Z()));
				}
			}
			return intersects;
		}
#pragma region IfcCurve Initialisers

		void XbimCurve::Init(IIfcCurve^ curve, ILogger^ logger)
		{
			if (dynamic_cast<IIfcPolyline^>(curve)) Init((IIfcPolyline^)curve, logger);
			else if (dynamic_cast<IIfcCompositeCurve^>(curve)) Init((IIfcCompositeCurve^)curve, logger);
			else if (dynamic_cast<IIfcCircle^>(curve)) Init((IIfcCircle^)curve, logger);
			else if (dynamic_cast<IIfcEllipse^>(curve)) Init((IIfcEllipse^)curve, logger);
			else if (dynamic_cast<IIfcTrimmedCurve^>(curve)) Init((IIfcTrimmedCurve^)curve, logger);
			else if (dynamic_cast<IIfcLine^>(curve)) Init((IIfcLine^)curve, logger);
			else if (dynamic_cast<IIfcRationalBSplineCurveWithKnots^>(curve)) Init((IIfcRationalBSplineCurveWithKnots^)curve, logger);
			else if (dynamic_cast<IIfcBSplineCurveWithKnots^>(curve)) Init((IIfcBSplineCurveWithKnots^)curve, logger);
			else if (dynamic_cast<IIfcOffsetCurve3D^>(curve)) Init((IIfcOffsetCurve3D^)curve, logger);
			else if (dynamic_cast<IIfcOffsetCurve2D^>(curve)) Init((IIfcOffsetCurve2D^)curve, logger);
			else if (dynamic_cast<IIfcIndexedPolyCurve^>(curve)) Init((IIfcIndexedPolyCurve^)curve, logger);
			else if (dynamic_cast<IIfcPcurve^>(curve)) Init((IIfcPcurve^)curve, logger);
			else if (dynamic_cast<IIfcSurfaceCurve^>(curve)) Init((IIfcSurfaceCurve^)curve, logger);
			else if(curve == nullptr)
				XbimGeometryCreator::LogWarning(logger, curve, "Curve is null");
			else
				throw gcnew Exception(String::Format("Unsupported Curve Type {0}", curve->GetType()->Name));
		}

#pragma region IfcBoundedCurve

		void XbimCurve::Init(IIfcBSplineCurveWithKnots^ bspline, ILogger^ /*logger*/)
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
				knots.SetValue(i, knot);
				i++;
			}
			i = 1;
			for each (int multiplicity in bspline->KnotMultiplicities)
			{
				knotMultiplicities.SetValue(i, multiplicity);
				i++;
			}
			pCurve = new Handle(Geom_Curve);
			*pCurve = new Geom_BSplineCurve(poles, knots, knotMultiplicities, (Standard_Integer)bspline->Degree);
		}

		void XbimCurve::Init(IIfcRationalBSplineCurveWithKnots^ bspline, ILogger^ /*logger*/)
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
			pCurve = new Handle(Geom_Curve);
			*pCurve = new Geom_BSplineCurve(poles, weights, knots, knotMultiplicities, (Standard_Integer)bspline->Degree);

		}

		void XbimCurve::Init(IIfcCompositeCurve^ cCurve, ILogger^ logger)
		{

			double tolerance = cCurve->Model->ModelFactors->Precision;
			GeomConvert_CompCurveToBSplineCurve converter;

			gp_Pnt lastVertex;
			gp_Pnt startVertex;
			bool firstPass = true;
			bool isContinuous = true; //assume continuous or clsoed unless last segment is discontinuous
			int segCount = cCurve->Segments->Count;
			int segIdx = 1;
			XbimPoint3D startPnt;

			for each(IIfcCompositeCurveSegment^ seg in cCurve->Segments) //every segment shall be a bounded curve
			{
				bool lastSeg = (segIdx == segCount);

				if (!dynamic_cast<IIfcBoundedCurve^>(seg->ParentCurve))
				{
					XbimGeometryCreator::LogError(logger, seg, "Composite curve contains a segment whih is not a bounded curve. It has been ignored");
					return;
				}
				XbimCurve^ curve = gcnew XbimCurve(seg->ParentCurve, logger);
				if (dynamic_cast<IIfcTrimmedCurve^>(seg->ParentCurve)) //we have to treat sense agreement differently
				{
					IIfcTrimmedCurve^ tc = ((IIfcTrimmedCurve^)seg->ParentCurve);
					if (curve->IsValid)
					{
						if (!seg->SameSense)
						{
							if (tc->SenseAgreement)
							{
								curve->Reverse();
							}
						}
						else
						{
							if (!tc->SenseAgreement)
							{
								curve->Reverse();
							}
						}
					}

				}
				else
				{
					if (!seg->SameSense && curve->IsValid)
						curve->Reverse();
				}

				if (lastSeg && seg->Transition == IfcTransitionCode::DISCONTINUOUS) isContinuous = false;
				if (curve->IsValid)
				{
					gp_Pnt nextVertex = curve->StartPoint();
					startPnt = curve->Start;
					if (firstPass)
					{
						startVertex = nextVertex;
					}
					double actualTolerance = tolerance; //reset for each segment

					if (!firstPass)
					{
						double actualGap = nextVertex.Distance(lastVertex);
						if (actualGap > tolerance)
						{
							double fiveMilli = 5 * cCurve->Model->ModelFactors->OneMilliMeter; //we are going to accept that a gap of 5mm is not a gap
							if (actualGap > fiveMilli)
							{
								XbimGeometryCreator::LogError(logger, seg, "Failed to join composite curve segment. It has been ignored");
								return;
							}
							actualTolerance = actualGap + tolerance;
						}
					}
					firstPass = false;
					bool ok = false;
					try
					{
						ok = converter.Add(curve->AsBoundedCurve(), actualTolerance);
					}
					catch (const std::exception&)
					{
						ok = false;
					}
					if (!ok)
					{
						XbimGeometryCreator::LogError(logger, seg, "Failed to join composite curve segment. It has been ignored");
						return;
					}
					lastVertex = curve->EndPoint();
				}
				else
				{
					XbimGeometryCreator::LogWarning(logger, seg, "Invalid edge of a composite curve found. It could not be created");
				}
				segIdx++;
			}
			Handle(Geom_BSplineCurve) bspline = converter.BSplineCurve();
			pCurve = new Handle(Geom_Curve);
			*pCurve = bspline;
		}

		void  XbimCurve::Init(IIfcIndexedPolyCurve ^ polyCurve, ILogger ^ logger)
		{
			double tolerance = polyCurve->Model->ModelFactors->Precision;

			IItemSet<IItemSet<Ifc4::MeasureResource::IfcLengthMeasure>^>^ coordList;
			IIfcCartesianPointList3D^ points3D = dynamic_cast<IIfcCartesianPointList3D^>(polyCurve->Points);
			IIfcCartesianPointList2D^ points2D = dynamic_cast<IIfcCartesianPointList2D^>(polyCurve->Points);
			int dim;
			if (points3D != nullptr)
			{
				coordList = points3D->CoordList;
				dim = 3;
			}
			else if (points2D != nullptr)
			{
				coordList = points2D->CoordList;
				dim = 2;
			}
			else
			{
				XbimGeometryCreator::LogError(logger, polyCurve, "Unsupported type of Coordinate List");
				return;
			}

			//get a index of all the points
			int pointCount = coordList->Count;
			TColgp_Array1OfPnt poles(1, pointCount);
			int i = 1;
			for each (IItemSet<Ifc4::MeasureResource::IfcLengthMeasure>^ coll in coordList)
			{
				IEnumerator<Ifc4::MeasureResource::IfcLengthMeasure>^ enumer = coll->GetEnumerator();
				enumer->MoveNext();
				gp_Pnt p;
				p.SetX((double)enumer->Current);
				enumer->MoveNext();
				p.SetY((double)enumer->Current);
				if (dim == 3)
				{
					enumer->MoveNext();
					p.SetZ((double)enumer->Current);

				}
				else
					p.SetZ(0);
				poles.SetValue(i, p);
				i++;
			}

			if (Enumerable::Any(polyCurve->Segments))
			{
				GeomConvert_CompCurveToBSplineCurve converter;
				for each (IIfcSegmentIndexSelect^ segment in  polyCurve->Segments)
				{
					Ifc4::GeometryResource::IfcArcIndex^ arcIndex = dynamic_cast<Ifc4::GeometryResource::IfcArcIndex^>(segment);
					Ifc4::GeometryResource::IfcLineIndex^ lineIndex = dynamic_cast<Ifc4::GeometryResource::IfcLineIndex^>(segment);
					if (arcIndex != nullptr)
					{

						List<Ifc4::MeasureResource::IfcPositiveInteger>^ indices = (List<Ifc4::MeasureResource::IfcPositiveInteger>^)arcIndex->Value;
						if (indices->Count != 3)
						{
							XbimGeometryCreator::LogError(logger, segment, "There should be three indices in an arc segment");
							return;
						}
						gp_Pnt start = poles.Value((int)indices[0]);
						gp_Pnt mid = poles.Value((int)indices[1]);
						gp_Pnt end = poles.Value((int)indices[2]);
						GC_MakeCircle circleMaker(start, mid, end);
						if (circleMaker.IsDone()) //it is a valid arc
						{
							const Handle(Geom_Circle)& curve = circleMaker.Value();
							double u1, u2;
							GeomLib_Tool::Parameter(curve, start, tolerance, u1);
							GeomLib_Tool::Parameter(curve, end, tolerance, u2);
							Handle(Geom_TrimmedCurve) trimmed = new Geom_TrimmedCurve(curve, u1, u2);
							if (!converter.Add(trimmed, tolerance))
							{
								XbimGeometryCreator::LogError(logger, segment, "Could not add arc segment to IfcIndexedPolyCurve");
								return;
							}
						}
						else //most likley the three points are in a line it should be treated as a polyline segment according the the docs
						{
							GC_MakeLine lineMaker(start, end);
							if (lineMaker.IsDone()) //it is a valid line
							{
								const Handle(Geom_Line)& line = lineMaker.Value();
								double u1, u2;
								GeomLib_Tool::Parameter(line, start, tolerance, u1);
								GeomLib_Tool::Parameter(line, end, tolerance, u2);
								Handle(Geom_TrimmedCurve) trimmed = new Geom_TrimmedCurve(line, u1, u2);
								if (!converter.Add(trimmed, tolerance))
								{
									XbimGeometryCreator::LogError(logger, segment, "Could not add arc segment as polyline to IfcIndexedPolyCurve");
									return;
								}
							}
							else
							{
								//most probably the start and end are the same point
								XbimGeometryCreator::LogWarning(logger, segment, "Could not create arc segment as a polyline to IfcIndexedPolyCurve");
							}
						}
					}
					else if (lineIndex != nullptr)
					{
						List<Ifc4::MeasureResource::IfcPositiveInteger>^ indices = (List<Ifc4::MeasureResource::IfcPositiveInteger>^)lineIndex->Value;
						if (indices->Count < 2)
						{
							XbimGeometryCreator::LogError(logger, segment, "There should be at least two indices in an line index segment");
							return;
						}
						int linePointCount = indices->Count;
						TColgp_Array1OfPnt linePoles(1, linePointCount);
						TColStd_Array1OfReal lineKnots(1, linePointCount);
						TColStd_Array1OfInteger lineMults(1, linePointCount);

						for (Standard_Integer p = 1; p <= linePointCount; p++)
						{
							linePoles.SetValue(p, poles.Value((int)indices[p - 1]));
							lineKnots.SetValue(p, Standard_Real(p - 1));
							lineMults.SetValue(p, 1);
						}
						lineMults.SetValue(1, 2);
						lineMults.SetValue(linePointCount, 2);

						Handle(Geom_BSplineCurve) spline = new Geom_BSplineCurve(linePoles, lineKnots, lineMults, 1);

						if (!converter.Add(spline, tolerance))
						{
							XbimGeometryCreator::LogError(logger, segment, "Could not add line index segment as polyline to IfcIndexedPolyCurve");
							return;
						}
					}
					else
					{
						//most probably the start and end are the same point
						XbimGeometryCreator::LogWarning(logger, segment, "Could not create line index segment as a polyline to IfcIndexedPolyCurve");
					}
				}
				pCurve = new Handle(Geom_Curve);
				*pCurve = converter.BSplineCurve();
			}
			else
			{
				// To be compliant with:
				// "In the case that the list of Segments is not provided, all points in the IfcCartesianPointList are connected by straight line segments in the order they appear in the IfcCartesianPointList."
				// http://www.buildingsmart-tech.org/ifc/IFC4/Add1/html/schema/ifcgeometryresource/lexical/ifcindexedpolycurve.htm
				TColStd_Array1OfReal knots(1, pointCount);
				TColStd_Array1OfInteger mults(1, pointCount);

				for (Standard_Integer p = 1; p <= pointCount; p++)
				{
					knots.SetValue(p, Standard_Real(p - 1));
					mults.SetValue(p, 1);
				}
				mults.SetValue(1, 2);
				mults.SetValue(pointCount, 2);
				pCurve = new Handle(Geom_Curve);
				*pCurve = new Geom_BSplineCurve(poles, knots, mults, 1);
			}

		}

		void XbimCurve::Init(IIfcPolyline^ pline, ILogger^ logger)
		{

			int pointCount = pline->Points->Count;
			if (pointCount < 2)
			{
				XbimGeometryCreator::LogError(logger, pline, "Polyline with less than 2 points is not a line. It has been ignored");
				return;
			}
			TColgp_Array1OfPnt poles(1, pointCount);
			TColStd_Array1OfReal knots(1, pointCount);
			TColStd_Array1OfInteger mults(1, pointCount);

			for (Standard_Integer i = 1; i <= pointCount; i++)
			{
				IIfcCartesianPoint^ cp = pline->Points[i - 1];
				gp_Pnt pnt(cp->X, cp->Y, XbimConvert::GetZValueOrZero(cp));
				poles.SetValue(i, pnt);
				knots.SetValue(i, Standard_Real(i - 1));
				mults.SetValue(i, 1);
			}
			mults.SetValue(1, 2);
			mults.SetValue(pointCount, 2);
			pCurve = new Handle(Geom_Curve);
			*pCurve = new Geom_BSplineCurve(poles, knots, mults, 1);

		}

		void XbimCurve::Init(IIfcTrimmedCurve^ curve, ILogger^ logger)
		{
			Init(curve->BasisCurve, logger);
			if (IsValid)
			{
				//check if we have an ellipse in case we have to correct axis


				bool isConic = (dynamic_cast<IIfcConic^>(curve->BasisCurve) != nullptr);
				bool isLine = (dynamic_cast<IIfcLine^>(curve->BasisCurve) != nullptr);
				bool isEllipse = (dynamic_cast<IIfcEllipse^>(curve->BasisCurve) != nullptr);

				double parameterFactor = isConic ? curve->Model->ModelFactors->AngleToRadiansConversionFactor : 1;
				double precision = curve->Model->ModelFactors->Precision;
				bool trim_cartesian = (curve->MasterRepresentation == IfcTrimmingPreference::CARTESIAN);

				double u1;
				gp_Pnt p1;
				bool u1Found, u2Found, p1Found, p2Found;
				double u2;
				gp_Pnt p2;
				for each (IIfcTrimmingSelect^ trim in curve->Trim1)
				{
					if (dynamic_cast<IIfcCartesianPoint^>(trim))
					{
						p1 = XbimConvert::GetPoint3d((IIfcCartesianPoint^)trim);
						p1Found = true;
					}
					else if (dynamic_cast<Xbim::Ifc4::MeasureResource::IfcParameterValue^>(trim))
					{
						u1 = (Xbim::Ifc4::MeasureResource::IfcParameterValue)trim;
						if (isConic) u1 *= parameterFactor; //correct to radians
						else if (isLine) u1 *= ((IIfcLine^)curve->BasisCurve)->Dir->Magnitude;
						u1Found = true;
					}
				}
				for each (IIfcTrimmingSelect^ trim in curve->Trim2)
				{
					if (dynamic_cast<IIfcCartesianPoint^>(trim))
					{
						p2 = XbimConvert::GetPoint3d((IIfcCartesianPoint^)trim);
						p2Found = true;
					}
					else if (dynamic_cast<Xbim::Ifc4::MeasureResource::IfcParameterValue^>(trim))
					{
						u2 = (Xbim::Ifc4::MeasureResource::IfcParameterValue)trim;
						if (isConic) u2 *= parameterFactor; //correct to radians
						else if (isLine) u2 *= ((IIfcLine^)curve->BasisCurve)->Dir->Magnitude;
						u2Found = true;
					}
				}



				if (trim_cartesian) //if we prefer cartesian and we have the points override the parameters
				{

					double u;
					if (p1Found)
						if (GeomLib_Tool::Parameter(*pCurve, p1, precision * 10, u))
							u1 = u;
					if (p2Found)
						if (GeomLib_Tool::Parameter(*pCurve, p2, precision * 10, u))
							u2 = u;

				}
				else //if we prefer parameters or don't care, use u1 nad u2 unless we don't have them
				{
					if (!u1Found)  GeomLib_Tool::Parameter(*pCurve, p1, precision * 10, u1);
					if (!u2Found)  GeomLib_Tool::Parameter(*pCurve, p2, precision * 10, u2);
				}
				if (u1 == u2)
				{
					pCurve->Nullify();
					pCurve = nullptr;
					return;// zero length curve;
				}
				if (isEllipse)
				{
					IIfcEllipse^ ellipse = (IIfcEllipse^)curve->BasisCurve;
					if (ellipse->SemiAxis1 < ellipse->SemiAxis2)
					{
						u1 -= Math::PI / 2;
						u2 -= Math::PI / 2;
					}
				}
				//now just go with
				bool sameSense = curve->SenseAgreement;
				*pCurve = new Geom_TrimmedCurve(*pCurve, sameSense ? u1 : u2, sameSense ? u2 : u1);
			}
		}

#pragma endregion

#pragma region IfcConic

		void XbimCurve::Init(IIfcCircle^ circle, ILogger^ logger)
		{
			double radius = circle->Radius;

			if (dynamic_cast<IIfcAxis2Placement2D^>(circle->Position))
			{
				IIfcAxis2Placement2D^ ax2 = (IIfcAxis2Placement2D^)circle->Position;
				gp_Ax2 gpax2(gp_Pnt(ax2->Location->X, ax2->Location->Y, 0), gp_Dir(0, 0, 1), gp_Dir(ax2->P[0].X, ax2->P[0].Y, 0.));
				GC_MakeCircle maker(gpax2, radius);
				pCurve = new Handle(Geom_Curve)(maker.Value());
			}
			else if (dynamic_cast<IIfcAxis2Placement3D^>(circle->Position))
			{
				IIfcAxis2Placement3D^ ax2 = (IIfcAxis2Placement3D^)circle->Position;
				gp_Ax3 	gpax3 = XbimConvert::ToAx3(ax2);
				GC_MakeCircle maker(gpax3.Ax2(), radius);
				pCurve = new Handle(Geom_Curve)(maker.Value());
			}
			else
			{
				Type ^ type = circle->Position->GetType();
				XbimGeometryCreator::LogError(logger, circle, "Placement of type {0} is not implemented", type->Name);
				return;
			}

		}

		void XbimCurve::Init(IIfcEllipse^ ellipse, ILogger^ logger)
		{
			double semiAx1 = ellipse->SemiAxis1;
			double semiAx2 = ellipse->SemiAxis2;
			if (semiAx1 <= 0)
			{
				XbimGeometryCreator::LogError(logger, ellipse, "Illegal Ellipse Semi Axis 1, must be greater than 0");
				return;
			}
			if (semiAx2 <= 0)
			{
				XbimGeometryCreator::LogError(logger, ellipse, "Illegal Ellipse Semi Axis 2, must be greater than 0");
				return;
			}
			gp_Ax3 ax3 = XbimConvert::ToAx3(ellipse->Position);
			if (Math::Abs(semiAx1 - semiAx2) < gp::Resolution()) //its a circle
			{
				GC_MakeCircle maker(ax3.Ax2(), semiAx1);
				pCurve = new Handle(Geom_Curve)(maker.Value());
			}
			else //it really is an ellipse
			{
				gp_Trsf trsf;
				trsf.SetTransformation(ax3, gp::XOY());
				gp_Ax2 ax = gp_Ax2();

				if (semiAx1 <= semiAx2)//major and minor axis are in the wrong order for opencascade			 
				{
					semiAx1 = ellipse->SemiAxis2;
					semiAx2 = ellipse->SemiAxis1;
					ax.Rotate(ax.Axis(), M_PI / 2.);
				}
				ax.Transform(trsf);
				GC_MakeEllipse maker(ax, semiAx1, semiAx2);
				pCurve = new Handle(Geom_Curve)(maker.Value());
			}
		}

#pragma endregion

		void XbimCurve::Init(IIfcLine^ line, ILogger^ /*logger*/)
		{
			IIfcCartesianPoint^ cp = line->Pnt;
			IIfcVector^ ifcVec = line->Dir;
			IIfcDirection^ dir = ifcVec->Orientation;
			gp_Pnt pnt(cp->X, cp->Y, XbimConvert::GetZValueOrZero(cp));
			gp_Dir vec(dir->X, dir->Y, XbimConvert::GetZValueOrZero(dir));
			GC_MakeLine maker(pnt, vec);
			pCurve = new Handle(Geom_Curve)(maker.Value());
		}

		void XbimCurve::Init(IIfcOffsetCurve3D^ offset, ILogger^ logger)
		{
			Init(offset->BasisCurve, logger);
			if (IsValid)
			{
				gp_Dir dir = XbimConvert::GetDir3d(offset->RefDirection);
				*pCurve = new Geom_OffsetCurve(*pCurve, offset->Distance, dir);
			}
		}

		void XbimCurve::Init(IIfcOffsetCurve2D^ offset, ILogger^ logger)
		{
			XbimCurve2D^ c2d = gcnew XbimCurve2D(offset, logger);
			if (c2d->IsValid)
			{
				pCurve = new Handle(Geom_Curve)();
				*pCurve = (XbimCurve^)(c2d->ToCurve3D());
			}
		}

		void XbimCurve::Init(IIfcPcurve^ curve, ILogger^ logger)
		{
			XbimFace^ face = gcnew XbimFace(curve->BasisSurface, logger);
			if (face->IsValid)
			{
				ShapeConstruct_ProjectCurveOnSurface projector;
				projector.Init(face->GetSurface(), curve->Model->ModelFactors->Precision);
				XbimCurve^ baseCurve = gcnew XbimCurve(curve->ReferenceCurve, logger);
				Standard_Real first = baseCurve->FirstParameter;
				Standard_Real last = baseCurve->LastParameter;
				Handle(Geom2d_Curve) c2d;
				Handle(Geom_Curve) cBase = baseCurve;
				if (projector.Perform(cBase, first, last, c2d))
				{
					pCurve = new Handle(Geom_Curve);
					*pCurve = cBase;
				}
			}
		}

		void XbimCurve::Init(IIfcSurfaceCurve ^ /*curve*/, ILogger ^ /*logger*/)
		{
			throw gcnew NotImplementedException("IIfcSurfaceCurve is not yet implemented");
		}

		void XbimCurve::Reverse()
		{
			(*pCurve)->Reverse();
		}
		gp_Pnt XbimCurve::StartPoint()
		{
			gp_Pnt p;
			(*pCurve)->D0((*pCurve)->FirstParameter(), p);
			return p;
		}
		gp_Pnt XbimCurve::EndPoint()
		{
			gp_Pnt p;
			(*pCurve)->D0((*pCurve)->LastParameter(), p);
			return p;
		}
	}


#pragma endregion
}


