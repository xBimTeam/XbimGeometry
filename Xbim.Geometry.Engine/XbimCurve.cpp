#include "XbimCurve.h"
#include "XbimCurve2D.h"
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
				delete (Handle_Geom_Curve*)(temp.ToPointer());
			System::GC::SuppressFinalize(this);
		}

		XbimCurve::XbimCurve(const Handle_Geom_Curve& curve)
		{
			this->pCurve = new Handle_Geom_Curve();
			*pCurve = curve;
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
			Standard_Real fp = (*pCurve)->FirstParameter();
			Standard_Real lp = (*pCurve)->LastParameter();
			return  lp - fp;
		}
		bool XbimCurve::IsClosed::get()
		{
			if(!IsValid) return false;
			return (*pCurve)->IsClosed() == Standard_True;
		}

		
		double XbimCurve::GetParameter(XbimPoint3D point, double tolerance)
		{
			if(!IsValid) return 0;
			double u1;
			gp_Pnt p1(point.X, point.Y, point.Z);
			GeomLib_Tool::Parameter(*pCurve, p1, tolerance, u1);
			return u1;
		}

		XbimPoint3D XbimCurve::GetPoint(double parameter)
		{
			if (!IsValid) return XbimPoint3D();
			gp_Pnt pt = (*pCurve)->Value(parameter);
			return XbimPoint3D(pt.X(),pt.Y(), pt.Z());
		}
		


		IXbimGeometryObject^ XbimCurve::Transform(XbimMatrix3D matrix3D)
		{
			throw gcnew Exception("Tranformation of curves is not currently supported");
		}
		
		IXbimGeometryObject^ XbimCurve::TransformShallow(XbimMatrix3D matrix3D)
		{
			throw gcnew Exception("TransformShallow of curves is not currently supported");
		}

		IEnumerable<XbimPoint3D>^ XbimCurve::Intersections(IXbimCurve^ intersector, double tolerance)
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

		void XbimCurve::Init(IIfcCurve^ curve)
		{
			if (dynamic_cast<IIfcPolyline^>(curve)) Init((IIfcPolyline^)curve);
			else if (dynamic_cast<IIfcCircle^>(curve)) Init((IIfcCircle^)curve);
			else if (dynamic_cast<IIfcEllipse^>(curve)) Init((IIfcEllipse^)curve);
			else if (dynamic_cast<IIfcTrimmedCurve^>(curve)) Init((IIfcTrimmedCurve^)curve);
			else if (dynamic_cast<IIfcLine^>(curve)) Init((IIfcLine^)curve);
			else if (dynamic_cast<IIfcRationalBSplineCurveWithKnots^>(curve)) Init((IIfcRationalBSplineCurveWithKnots^)curve);
			else if (dynamic_cast<IIfcBSplineCurveWithKnots^>(curve)) Init((IIfcBSplineCurveWithKnots^)curve);
			else if (dynamic_cast<IIfcOffsetCurve3D^>(curve)) Init((IIfcOffsetCurve3D^)curve);
			else throw gcnew Exception(String::Format("Unsupported Curve Type {0}", curve->GetType()->Name));
		}
				
		void XbimCurve::Init(IIfcOffsetCurve3D^ offset)
		{
			Init(offset->BasisCurve);
			if (IsValid)
			{
				gp_Dir dir = XbimConvert::GetDir3d(offset->RefDirection);
				*pCurve = new Geom_OffsetCurve(*pCurve, offset->Distance, dir);
			}
		}

		void XbimCurve::Init(IIfcPolyline^ curve)
		{
			//only deal with the first two points of a polyline, should really use wire for more than one segment
			List<IIfcCartesianPoint^>^ pts = Enumerable::ToList(curve->Points);
			if (pts->Count != 2) throw gcnew Exception("XbimCurves can only be created with polylines that have a single segment");

			gp_Pnt start = XbimConvert::GetPoint3d(pts[0]);
			gp_Pnt end = XbimConvert::GetPoint3d(pts[1]);
			
			GC_MakeLine lineMaker(start, end);			
			if(lineMaker.IsDone())
			{	
				double u1; double u2;
				GeomLib_Tool::Parameter(lineMaker.Value(), start, Precision::Confusion(), u1);
				GeomLib_Tool::Parameter(lineMaker.Value(), end, Precision::Confusion(), u2);
				pCurve = new Handle_Geom_Curve();
				*pCurve = new Geom_TrimmedCurve(lineMaker.Value(), u1, u2);
			}
		}
		void XbimCurve::Init(IIfcCircle^ circle)
		{
			double radius = circle->Radius;
			pCurve = new Handle_Geom_Curve();
			if (dynamic_cast<IIfcAxis2Placement2D^>(circle->Position))
			{
				IIfcAxis2Placement2D^ ax2 = (IIfcAxis2Placement2D^)circle->Position;
				gp_Ax2 gpax2(gp_Pnt(ax2->Location->X, ax2->Location->Y, 0), gp_Dir(0, 0, 1), gp_Dir(ax2->P[0].X, ax2->P[0].Y, 0.));				
				*pCurve = GC_MakeCircle(gpax2,radius);
			}
			else if (dynamic_cast<IIfcAxis2Placement3D^>(circle->Position))
			{
				IIfcAxis2Placement3D^ ax2 = (IIfcAxis2Placement3D^)circle->Position;
				gp_Ax3 	gpax3 = XbimConvert::ToAx3(ax2);
				*pCurve = GC_MakeCircle(gpax3.Ax2(), radius);
			}
			else
			{
				delete pCurve; //tidy up
				Type ^ type = circle->Position->GetType();
				throw gcnew Exception(String::Format("WC001: Circle #{0} with Placement of type {1} is not implemented", circle->EntityLabel, type->Name));				
				return;
			}
			
		}
		void XbimCurve::Init(IIfcEllipse^ ellipse)
		{
			IIfcAxis2Placement2D^ ax2 = (IIfcAxis2Placement2D^)ellipse->Position;			
			double semiAx1 = ellipse->SemiAxis1;
			double semiAx2 = ellipse->SemiAxis2;
			if (semiAx1 <= 0)
			{
				throw gcnew Exception(String::Format("WC002: Illegal Ellipse Semi Axis 1, must be greater than 0, in entity #{0}", ellipse->EntityLabel));
			}
			if (semiAx2 <= 0)
			{
				throw gcnew Exception(String::Format("WE005: Illegal Ellipse Semi Axis 2, must be greater than 0, in entity #{0}", ellipse->EntityLabel));
			}				
			bool rotateElipse;
			if (semiAx1 <= semiAx2)//either same or two is larger than 1			 
			{
				semiAx1 = ellipse->SemiAxis2;
				semiAx2 = ellipse->SemiAxis1;
				rotateElipse = true;
			}
			gp_Ax2 gpax2(gp_Pnt(ax2->Location->X, ax2->Location->Y, 0), gp_Dir(0, 0, 1), gp_Dir(ax2->P[rotateElipse ? 1 : 0].X, ax2->P[rotateElipse ? 1 : 0].Y, 0.));
			pCurve = new Handle_Geom_Curve();
			*pCurve = GC_MakeEllipse(gpax2,semiAx1, semiAx2);
		}
		void XbimCurve::Init(IIfcLine^ line)
		{
			IIfcCartesianPoint^ cp = line->Pnt;
			IIfcVector^ ifcVec = line->Dir;
			IIfcDirection^ dir = ifcVec->Orientation;
			gp_Pnt pnt(cp->X, cp->Y, cp->Z);			
			gp_Dir vec(dir->X, dir->Y, dir->Z);
			pCurve = new Handle_Geom_Curve();
			*pCurve = GC_MakeLine(pnt, vec);		
		}

		void XbimCurve::Init(IIfcTrimmedCurve^ curve)
		{
			Init(curve->BasisCurve);
			if(IsValid)
			{
				//check if we have an ellipse in case we have to correct axis
				IIfcEllipse^ ellipse = dynamic_cast<IIfcEllipse^>(curve->BasisCurve);
				bool rotateEllipse = false;
				gp_Ax1 centre;
				if (ellipse != nullptr)
				{
					if(ellipse->SemiAxis1 < ellipse->SemiAxis2)	rotateEllipse = true;
					IIfcAxis2Placement2D^ ax2 = (IIfcAxis2Placement2D^)ellipse->Position;
					centre = gp_Ax1(gp_Pnt(ax2->Location->X, ax2->Location->Y, 0), gp_Dir(0, 0, 1));
				}
				bool isConic = (dynamic_cast<IIfcConic^>(curve->BasisCurve) != nullptr);					
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
						p1=XbimConvert::GetPoint3d((IIfcCartesianPoint^)trim);
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
				for each (IIfcTrimmingSelect^ trim in curve->Trim2)
				{
					if (dynamic_cast<IIfcCartesianPoint^>(trim))
					{
						p2 = XbimConvert::GetPoint3d((IIfcCartesianPoint^)trim);
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
				if(trim_cartesian) //if we prefer cartesian and we have the points override the parameters
				{
					if(p1Found)	 GeomLib_Tool::Parameter(*pCurve, p1, precision,u1);
					if(p2Found)  GeomLib_Tool::Parameter(*pCurve, p2, precision, u2);
				}
				else //if we prefer parameters or don't care, use u1 nad u2 unless we don't have them
				{
					if (!u1Found)  GeomLib_Tool::Parameter(*pCurve, p1, precision, u1);
					if (!u2Found)  GeomLib_Tool::Parameter(*pCurve, p2, precision, u2);
				}
				//now just go with
				*pCurve = new Geom_TrimmedCurve(*pCurve, u1, u2, curve->SenseAgreement);
			}
		}

		void XbimCurve::Init(IIfcRationalBSplineCurveWithKnots^ bspline)
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
			pCurve = new Handle_Geom_Curve();
			*pCurve = new Geom_BSplineCurve(poles, weights, knots, knotMultiplicities, (Standard_Integer)bspline->Degree);

		}

		void XbimCurve::Init(IIfcBSplineCurveWithKnots^ bspline)
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
			pCurve = new Handle_Geom_Curve();
			*pCurve = new Geom_BSplineCurve(poles, knots, knotMultiplicities, (Standard_Integer)bspline->Degree);		
		}
	}
}
