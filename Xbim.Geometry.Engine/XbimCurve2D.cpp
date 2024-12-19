
#include <gp_Pnt2d.hxx>
#include <gp_Ax2.hxx>
#include <GeomLib_Tool.hxx>

#include <Geom2dAPI_InterCurveCurve.hxx>
#include <Geom2d_OffsetCurve.hxx>
#include <GeomLib.hxx>
#include <GCE2d_MakeLine.hxx>
#include <GCE2d_MakeCircle.hxx>
#include <GCE2d_MakeEllipse.hxx>
#include <Geom2d_BSplineCurve.hxx>
#include <BndLib_Add2dCurve.hxx>
#include <Geom2dAdaptor_Curve.hxx>
#include <Bnd_Box2d.hxx>
#include <Geom2dConvert_CompCurveToBSplineCurve.hxx>
#include <ShapeConstruct_ProjectCurveOnSurface.hxx>
#include <Precision.hxx>
#include "XbimCurve2D.h"
#include "XbimCurve.h"
#include "XbimConvert.h"
#include "XbimFace.h"
#include "XbimGeometryCreator.h"

namespace Xbim
{
	namespace Geometry
	{
		/*Ensures native pointers are deleted and garbage collected*/
		void XbimCurve2D::InstanceCleanup()
		{
			System::IntPtr temp = System::Threading::Interlocked::Exchange(ptrContainer, System::IntPtr::Zero);
			if (temp != System::IntPtr::Zero)
				delete (Handle(Geom2d_Curve)*)(temp.ToPointer());
			System::GC::SuppressFinalize(this);
		}




		XbimRect3D XbimCurve2D::BoundingBox::get()
		{
			if (!IsValid) return XbimRect3D::Empty;
			Standard_Real lp = (*pCurve2D)->LastParameter();
			Standard_Real fp = (*pCurve2D)->FirstParameter();
			Bnd_Box2d b1;
			BndLib_Add2dCurve::Add(*pCurve2D, fp, lp, 0., b1);
			Standard_Real srXmin, srYmin, srXmax, srYmax;
			b1.Get(srXmin, srYmin, srXmax, srYmax);
			return XbimRect3D(srXmin, srYmin, 0., (srXmax - srXmin), (srYmax - srYmin), 0.);
		}

		XbimPoint3D XbimCurve2D::Start::get()
		{
			if (!IsValid) return XbimPoint3D();
			Standard_Real fp = (*pCurve2D)->FirstParameter();
			return GetPoint(fp);
		}

		XbimPoint3D XbimCurve2D::End::get()
		{
			if (!IsValid) return XbimPoint3D();
			Standard_Real lp = (*pCurve2D)->LastParameter();
			return GetPoint(lp);
		}

		double XbimCurve2D::Length::get()
		{
			Standard_Real fp = (*pCurve2D)->FirstParameter();
			Standard_Real lp = (*pCurve2D)->LastParameter();
			return  lp - fp;
		}
		bool XbimCurve2D::IsClosed::get()
		{
			if (!IsValid) return false;
			return (*pCurve2D)->IsClosed() == Standard_True;
		}


		double XbimCurve2D::GetParameter(XbimPoint3D point, double tolerance)
		{
			if (!IsValid) return 0;
			double u1;
			gp_Pnt2d p1(point.X, point.Y);
			GeomLib_Tool::Parameter(*pCurve2D, p1, tolerance, u1);
			return u1;
		}

		XbimPoint3D XbimCurve2D::GetPoint(double parameter)
		{
			if (!IsValid) return XbimPoint3D();
			gp_Pnt2d pt = (*pCurve2D)->Value(parameter);
			return XbimPoint3D(pt.X(), pt.Y(), 0);
		}

		IXbimCurve^ XbimCurve2D::ToCurve3D()
		{
			if (!IsValid) return nullptr;
			return gcnew XbimCurve(GeomLib::To3d(gp_Ax2(), *pCurve2D), _modelServices);
		}

		XbimVector3D XbimCurve2D::TangentAt(double parameter)
		{
			if (!IsValid) return XbimVector3D();
			gp_Pnt2d p;
			gp_Vec2d v;
			(*pCurve2D)->D1(parameter, p, v);
			return XbimVector3D(v.X(), v.Y(), 0.);
		}

		IXbimGeometryObject^ XbimCurve2D::Transform(XbimMatrix3D /*matrix3D*/)
		{
			throw gcnew System::Exception("Tranformation of curves is not currently supported");
		}

		IXbimGeometryObject^ XbimCurve2D::TransformShallow(XbimMatrix3D /*matrix3D*/)
		{
			throw gcnew System::Exception("TransformShallow of curves is not currently supported");
		}

		System::Collections::Generic::IEnumerable<XbimPoint3D>^ XbimCurve2D::Intersections(IXbimCurve^ intersector, double tolerance, ILogger^ /*logger*/)
		{
			Geom2dAPI_InterCurveCurve extrema(*pCurve2D, *((XbimCurve2D^)intersector)->pCurve2D, tolerance);
			List<XbimPoint3D>^ intersects = gcnew List<XbimPoint3D>();
			for (Standard_Integer i = 0; i < extrema.NbPoints(); i++)
			{
				gp_Pnt2d p1 = extrema.Point(i + 1);
				intersects->Add(XbimPoint3D(p1.X(), p1.Y(), 0));
			}
			return intersects;
		}

		void XbimCurve2D::Init(IIfcGridAxis^ axis, ILogger^ logger)
		{
			Init(axis->AxisCurve, logger);
			if (IsValid && !axis->SameSense) (*pCurve2D)->Reverse();
		}

		void XbimCurve2D::Init(IIfcCurve^ curve, ILogger^ logger)
		{
			if (dynamic_cast<IIfcPolyline^>(curve)) Init((IIfcPolyline^)curve, logger);
			else if (dynamic_cast<IIfcCircle^>(curve)) Init((IIfcCircle^)curve, logger);
			else if (dynamic_cast<IIfcEllipse^>(curve)) Init((IIfcEllipse^)curve, logger);
			else if (dynamic_cast<IIfcTrimmedCurve^>(curve)) Init((IIfcTrimmedCurve^)curve, logger);
			else if (dynamic_cast<IIfcLine^>(curve)) Init((IIfcLine^)curve, logger);
			else if (dynamic_cast<IIfcRationalBSplineCurveWithKnots^>(curve)) Init((IIfcRationalBSplineCurveWithKnots^)curve, logger);
			else if (dynamic_cast<IIfcBSplineCurveWithKnots^>(curve)) Init((IIfcBSplineCurveWithKnots^)curve, logger);
			else if (dynamic_cast<IIfcOffsetCurve2D^>(curve)) Init((IIfcOffsetCurve2D^)curve, logger);
			else if (dynamic_cast<IIfcIndexedPolyCurve^>(curve)) Init((IIfcIndexedPolyCurve^)curve, logger);
			else if (dynamic_cast<IIfcPcurve^>(curve)) Init((IIfcPcurve^)curve, logger);
			else throw gcnew System::Exception(System::String::Format("Unsupported Curve Type {0}", curve->GetType()->Name));
		}

		void XbimCurve2D::Init(IIfcPcurve^ curve, ILogger^ logger)
		{
			XbimFace^ face = gcnew XbimFace(curve->BasisSurface, logger, _modelServices);

			if (face->IsValid)
			{
				ShapeConstruct_ProjectCurveOnSurface projector;
				projector.Init(face->GetSurface(), _modelServices->MinimumGap);
				XbimCurve^ baseCurve = gcnew XbimCurve(curve->ReferenceCurve, logger, _modelServices);
				Standard_Real first = baseCurve->FirstParameter;
				Standard_Real last = baseCurve->LastParameter;
				Handle(Geom2d_Curve) c2d;
				Handle(Geom_Curve) cBase = baseCurve;
				if (projector.Perform(cBase, first, last, c2d))
				{
					pCurve2D = new Handle(Geom2d_Curve);
					*pCurve2D = c2d;
				}
			}
		}
		void XbimCurve2D::Init(IIfcIndexedPolyCurve^ polyCurve, ILogger^ logger)
		{
			double tolerance = _modelServices->MinimumGap;

			IItemSet<IItemSet<Ifc4::MeasureResource::IfcLengthMeasure>^>^ coordList;

			IIfcCartesianPointList2D^ points2D = dynamic_cast<IIfcCartesianPointList2D^>(polyCurve->Points);
			int dim;
			if (points2D != nullptr)
			{
				coordList = points2D->CoordList;
				dim = 2;
			}
			else
			{
				XbimGeometryCreator::LogError(logger, polyCurve, "Unsupported type of 2d Coordinate List");
				return;
			}

			//get a index of all the points
			int pointCount = coordList->Count;
			TColgp_Array1OfPnt2d poles(1, pointCount);
			int i = 1;
			for each (IItemSet<Ifc4::MeasureResource::IfcLengthMeasure> ^ coll in coordList)
			{
				System::Collections::Generic::IEnumerator<Ifc4::MeasureResource::IfcLengthMeasure>^ enumer = coll->GetEnumerator();
				enumer->MoveNext();
				gp_Pnt2d p;
				p.SetX((double)enumer->Current);
				enumer->MoveNext();
				p.SetY((double)enumer->Current);
				poles.SetValue(i, p);
				i++;
			}

			if (System::Linq::Enumerable::Any(polyCurve->Segments))
			{
				Geom2dConvert_CompCurveToBSplineCurve converter;
				for each (IIfcSegmentIndexSelect ^ segment in  polyCurve->Segments)
				{
					Ifc4::GeometryResource::IfcArcIndex^ arcIndex = dynamic_cast<Ifc4::GeometryResource::IfcArcIndex^>(segment);
					Ifc4::GeometryResource::IfcLineIndex^ lineIndex = dynamic_cast<Ifc4::GeometryResource::IfcLineIndex^>(segment);
					if (arcIndex != nullptr)
					{

						List<Ifc4::MeasureResource::IfcPositiveInteger>^ indices = (List<Ifc4::MeasureResource::IfcPositiveInteger>^)arcIndex->Value;
						if (indices->Count != 3)
						{
							XbimGeometryCreator::LogWarning(logger, segment, "There should be three indices in an arc segment");
							return;
						}
						gp_Pnt2d start = poles.Value((int)indices[0]);
						gp_Pnt2d mid = poles.Value((int)indices[1]);
						gp_Pnt2d end = poles.Value((int)indices[2]);
						GCE2d_MakeCircle circleMaker(start, mid, end);
						if (circleMaker.IsDone()) //it is a valid arc
						{
							const Handle(Geom2d_Circle)& curve = circleMaker.Value();
							double u1, u2;
							GeomLib_Tool::Parameter(curve, start, tolerance, u1);
							GeomLib_Tool::Parameter(curve, end, tolerance, u2);
							Handle(Geom2d_TrimmedCurve) trimmed = new Geom2d_TrimmedCurve(curve, u1, u2);
							if (!converter.Add(trimmed, tolerance))
							{
								XbimGeometryCreator::LogWarning(logger, segment, "Could not add arc segment to IfcIndexedPolyCurve");
								return;
							}
						}
						else //most likley the three points are in a line it should be treated as a polyline segment according the the docs
						{
							GCE2d_MakeLine lineMaker(start, end);
							if (lineMaker.IsDone()) //it is a valid line
							{
								const Handle(Geom2d_Line)& line = lineMaker.Value();
								double u1, u2;
								GeomLib_Tool::Parameter(line, start, tolerance, u1);
								GeomLib_Tool::Parameter(line, end, tolerance, u2);
								Handle(Geom2d_TrimmedCurve) trimmed = new Geom2d_TrimmedCurve(line, u1, u2);
								if (!converter.Add(trimmed, tolerance))
								{
									XbimGeometryCreator::LogWarning(logger, segment, "Could not add arc segment as polyline to IfcIndexedPolyCurve");
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
							XbimGeometryCreator::LogWarning(logger, segment, "There should be at least two indices in an line index segment");
							return;
						}
						int linePointCount = indices->Count;
						TColgp_Array1OfPnt2d linePoles(1, linePointCount);
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

						Handle(Geom2d_BSplineCurve) spline = new Geom2d_BSplineCurve(linePoles, lineKnots, lineMults, 1);

						if (!converter.Add(spline, tolerance))
						{
							XbimGeometryCreator::LogWarning(logger, segment, "Could not add line index segment as polyline to IfcIndexedPolyCurve");
							return;
						}
					}
					else
					{
						//most probably the start and end are the same point
						XbimGeometryCreator::LogWarning(logger, segment, "Could not create line index segment as a polyline to IfcIndexedPolyCurve");
					}
				}
				pCurve2D = new Handle(Geom2d_Curve);
				*pCurve2D = converter.BSplineCurve();
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
				pCurve2D = new Handle(Geom2d_Curve);
				*pCurve2D = new Geom2d_BSplineCurve(poles, knots, mults, 1);
			}

		}

		void XbimCurve2D::Init(IIfcOffsetCurve2D^ offset, ILogger^ logger)
		{
			Init(offset->BasisCurve, logger);
			if (IsValid)
			{
				*pCurve2D = new Geom2d_OffsetCurve(*pCurve2D, offset->Distance);
			}
		}

		void XbimCurve2D::Init(IIfcPolyline^ pline, ILogger^ logger)
		{
			int pointCount = pline->Points->Count;
			if (pointCount < 2)
			{
				XbimGeometryCreator::LogWarning(logger, pline, "Polyline with less than 2 points is not a line. It has been ignored");
				return;
			}
			if (pointCount == 2) //just trim a line
			{
				IIfcCartesianPoint^ cp1 = pline->Points[0];
				IIfcCartesianPoint^ cp2 = pline->Points[1];
				gp_Pnt2d pnt1(cp1->X, cp1->Y);
				gp_Pnt2d pnt2(cp2->X, cp2->Y);
				double len = pnt1.Distance(pnt2);
				if (std::abs(len) < Precision::Confusion())
				{
					XbimGeometryCreator::LogWarning(logger, pline, "Polyline segment has zero length. It has been ignored");
					return;
				}
				GCE2d_MakeLine maker(pnt1, pnt2);
				if (!maker.IsDone())
				{
					XbimGeometryCreator::LogWarning(logger, pline, "Polyline does not define a valid line segment. It has been ignored");
					return;

				}
				else
				{
					pCurve2D = new Handle(Geom2d_Curve);
					*pCurve2D = new Geom2d_TrimmedCurve(maker.Value(), 0, len);
					return;
				}
			}
			TColgp_Array1OfPnt2d poles(1, pointCount);
			TColStd_Array1OfReal knots(1, pointCount);
			TColStd_Array1OfInteger mults(1, pointCount);

			for (Standard_Integer i = 1; i <= pointCount; i++)
			{
				IIfcCartesianPoint^ cp = pline->Points[i - 1];
				gp_Pnt2d pnt(cp->X, cp->Y);
				poles.SetValue(i, pnt);
				knots.SetValue(i, Standard_Real(i - 1));
				mults.SetValue(i, 1);
			}
			mults.SetValue(1, 2);
			mults.SetValue(pointCount, 2);
			pCurve2D = new Handle(Geom2d_Curve);
			*pCurve2D = new Geom2d_BSplineCurve(poles, knots, mults, 1);
		}
		void XbimCurve2D::Init(IIfcCircle^ circle, ILogger^ logger)
		{
			double radius = circle->Radius;
			if (radius <= 0)
			{
				XbimGeometryCreator::LogInfo(logger, this, "Illegal circle : The radius is less than or equal to zero");
				return;
			}


			if (dynamic_cast<IIfcAxis2Placement2D^>(circle->Position))
			{
				IIfcAxis2Placement2D^ ax2 = (IIfcAxis2Placement2D^)circle->Position;
				gp_Ax2d gpax2(gp_Pnt2d(ax2->Location->X, ax2->Location->Y), gp_Dir2d(ax2->P[0].X, ax2->P[0].Y));
				GCE2d_MakeCircle maker(gpax2, radius);
				pCurve2D = new Handle(Geom2d_Curve)(maker.Value());
			}
			else if (dynamic_cast<IIfcAxis2Placement3D^>(circle->Position))
			{
				IIfcAxis2Placement3D^ ax2 = (IIfcAxis2Placement3D^)circle->Position;
				gp_Ax2d gpax2(gp_Pnt2d(ax2->Location->X, ax2->Location->Y), gp_Dir2d(ax2->P[0].X, ax2->P[0].Y));
				GCE2d_MakeCircle maker(gpax2, radius);
				pCurve2D = new Handle(Geom2d_Curve)(maker.Value());
			}
			else
			{

				System::Type^ type = circle->Position->GetType();
				throw gcnew System::Exception(System::String::Format("WC001: Circle #{0} with Placement of type {1} is not implemented", circle->EntityLabel, type->Name));
				return;
			}

		}
		void XbimCurve2D::Init(IIfcEllipse^ ellipse, ILogger^ logger)
		{
			IIfcAxis2Placement2D^ ax2 = (IIfcAxis2Placement2D^)ellipse->Position;
			double semiAx1 = ellipse->SemiAxis1;
			double semiAx2 = ellipse->SemiAxis2;

			if (semiAx1 <= 0)
			{
				XbimGeometryCreator::LogWarning(logger, ellipse, "WC002: Illegal Ellipse Semi Axis 1, must be greater than 0, in entity #{ifcEntityLabel}", ellipse->EntityLabel);
				return;
			}
			if (semiAx2 <= 0)
			{
				XbimGeometryCreator::LogWarning(logger, ellipse, "WE005: Illegal Ellipse Semi Axis 2, must be greater than 0, in entity #{ifcEntityLabel}", ellipse->EntityLabel);
				return;
			}
			bool rotateElipse = false;
			if (semiAx1 < semiAx2)//either same or two is larger than 1			 
			{
				semiAx1 = ellipse->SemiAxis2;
				semiAx2 = ellipse->SemiAxis1;
				rotateElipse = true;
			}
			gp_Pnt2d centre(ax2->Location->X, ax2->Location->Y);
			gp_Ax2d gpax2(centre, gp_Dir2d(ax2->P[rotateElipse ? 1 : 0].X, ax2->P[rotateElipse ? 1 : 0].Y));

			GCE2d_MakeEllipse maker(gpax2, semiAx1, semiAx2);
			pCurve2D = new Handle(Geom2d_Curve)(maker.Value());
		}

		void XbimCurve2D::Init(IIfcLine^ line, ILogger^ /*logger*/)
		{
			IIfcCartesianPoint^ cp = line->Pnt;
			IIfcVector^ ifcVec = line->Dir;
			IIfcDirection^ dir = ifcVec->Orientation;
			gp_Pnt2d pnt(cp->X, cp->Y);
			gp_Dir2d vec(dir->X, dir->Y);
			GCE2d_MakeLine maker(pnt, vec);
			pCurve2D = new Handle(Geom2d_Curve)(maker.Value());
		}

		void XbimCurve2D::Init(IIfcTrimmedCurve^ curve, ILogger^ logger)
		{
			Init(curve->BasisCurve, logger);
			if (IsValid)
			{
				//check if we have an ellipse in case we have to correct axis
				IIfcEllipse^ ellipse = dynamic_cast<IIfcEllipse^>(curve->BasisCurve);
				bool rotateEllipse = false;
				gp_Pnt2d centre;
				if (ellipse != nullptr)
				{
					if (ellipse->SemiAxis1 < ellipse->SemiAxis2)	rotateEllipse = true;
					IIfcAxis2Placement2D^ ax2 = (IIfcAxis2Placement2D^)ellipse->Position;
					centre = gp_Pnt2d(ax2->Location->X, ax2->Location->Y);
				}
				bool isConic = (dynamic_cast<IIfcConic^>(curve->BasisCurve) != nullptr);
				double parameterFactor = isConic ? _modelServices->RadianFactor : 1;
				double precision = _modelServices->MinimumGap;
				bool trim_cartesian = (curve->MasterRepresentation == IfcTrimmingPreference::CARTESIAN);

				double u1 = 0;
				gp_Pnt2d p1;
				bool u1Found = false, u2Found = false, p1Found = false, p2Found = false;
				double u2 = 0;
				gp_Pnt2d p2;
				for each (IIfcTrimmingSelect ^ trim in curve->Trim1)
				{
					if (dynamic_cast<IIfcCartesianPoint^>(trim))
					{
						p1 = XbimConvert::GetPoint2d((IIfcCartesianPoint^)trim);
						if (rotateEllipse) //if we have had to rotate the elipse, then rotate the trims							
							p1.Rotate(centre, 90.0);
						p1Found = true;
					}
					else if (dynamic_cast<Xbim::Ifc4::MeasureResource::IfcParameterValue^>(trim))
					{
						u1 = (Xbim::Ifc4::MeasureResource::IfcParameterValue)trim;
						if (isConic) u1 *= parameterFactor; //correct to radians
						u1Found = true;
					}
				}
				for each (IIfcTrimmingSelect ^ trim in curve->Trim2)
				{
					if (dynamic_cast<IIfcCartesianPoint^>(trim))
					{
						p2 = XbimConvert::GetPoint2d((IIfcCartesianPoint^)trim);
						if (rotateEllipse) //if we have had to rotate the elipse, then rotate the trims							
							p2.Rotate(centre, 90.0);
						p2Found = true;
					}
					else if (dynamic_cast<Xbim::Ifc4::MeasureResource::IfcParameterValue^>(trim))
					{
						u2 = (Xbim::Ifc4::MeasureResource::IfcParameterValue)trim;
						if (isConic) u2 *= parameterFactor; //correct to radians
						u2Found = true;
					}
				}
				if (trim_cartesian) //if we prefer cartesian and we have the points override the parameters
				{
					if (p1Found)	 GeomLib_Tool::Parameter(*pCurve2D, p1, precision, u1);
					if (p2Found)  GeomLib_Tool::Parameter(*pCurve2D, p2, precision, u2);
				}
				else //if we prefer parameters or don't care, use u1 nad u2 unless we don't have them
				{
					if (!u1Found)  GeomLib_Tool::Parameter(*pCurve2D, p1, precision, u1);
					if (!u2Found)  GeomLib_Tool::Parameter(*pCurve2D, p2, precision, u2);
				}
				//now just go with
				*pCurve2D = new Geom2d_TrimmedCurve(*pCurve2D, u1, u2, curve->SenseAgreement);
			}
		}

		void XbimCurve2D::Init(IIfcRationalBSplineCurveWithKnots^ bspline, ILogger^ /*logger*/)
		{
			TColgp_Array1OfPnt2d poles(1, Enumerable::Count(bspline->ControlPointsList));
			int i = 1;
			for each (IIfcCartesianPoint ^ cp in bspline->ControlPointsList)
			{
				poles.SetValue(i, gp_Pnt2d(cp->X, cp->Y));
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
			pCurve2D = new Handle(Geom2d_Curve);
			*pCurve2D = new Geom2d_BSplineCurve(poles, weights, knots, knotMultiplicities, (Standard_Integer)bspline->Degree);

		}

		void XbimCurve2D::Init(IIfcBSplineCurveWithKnots^ bspline, ILogger^ /*logger*/)
		{
			TColgp_Array1OfPnt2d poles(1, Enumerable::Count(bspline->ControlPointsList));
			int i = 1;
			for each (IIfcCartesianPoint ^ cp in bspline->ControlPointsList)
			{
				poles.SetValue(i, gp_Pnt2d(cp->X, cp->Y));
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
			pCurve2D = new Handle(Geom2d_Curve);
			*pCurve2D = new Geom2d_BSplineCurve(poles, knots, knotMultiplicities, (Standard_Integer)bspline->Degree);
		}
	}
}
