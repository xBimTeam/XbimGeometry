#pragma once
#include "../BRep/XbimLine.h"
#include "../BRep/XbimLine2d.h"
#include "../Services/LoggingService.h"
#include "./Unmanaged/NCurveFactory.h"
#include "GeomProcFactory.h"

#include <Geom_Curve.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom_Line.hxx>
#include <Geom2d_Line.hxx>
#include <Geom_Circle.hxx>
#include <Geom2d_Circle.hxx>
#include <Geom_Ellipse.hxx>
#include <Geom2d_Ellipse.hxx>

#include <Geom_TrimmedCurve.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <Precision.hxx>
using namespace Xbim::Geometry::BRep;
using namespace Xbim::Common::Geometry;
using namespace Xbim::Ifc4::Interfaces;
using namespace Xbim::Geometry::Abstractions;
using namespace Xbim::Geometry::Services;
using namespace Xbim::Geometry::Factories::Unmanaged;
using namespace Xbim::Common;
namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			public ref class CurveFactory : XbimHandle<NCurveFactory>
			{
			private:
				GeomProcFactory^ GpFactory;
				LoggingService^ Logger;
				IModel^ ifcModel;
				//The distance between two points at which they are determined to be equal points
				double _modelTolerance;
				//the multiplier to convert model angular units to radians, for a radian uni this = 1 and for a degree unit = PI/180;
				double _radiansFactor;
				//The distance between a point and a curve at which it is determined to be a point on the curve
				double _pointOnCurveTolerance;
				
			public:
				CurveFactory(LoggingService^ loggingService, IModel^ ifcModel) : XbimHandle(new NCurveFactory(loggingService))
				{
					GpFactory = gcnew GeomProcFactory();
					Logger = loggingService;
					
					_modelTolerance = ifcModel->ModelFactors->Precision;
					_radiansFactor = ifcModel->ModelFactors->AngleToRadiansConversionFactor;
					_pointOnCurveTolerance = _modelTolerance * 10;
				}
				
				//Top level abstraction for building any curve

				IXCurve^ BuildXCurve(Handle(Geom_Curve) curve, XCurveType curveType);
				IXCurve^ BuildXCurve(Handle(Geom2d_Curve) curve, XCurveType curveType);
				//Geometry builders
				IXCurve^ Build(IIfcCurve^ curve);
				Handle(Geom_Curve) BuildGeom3d(IIfcCurve^ curve, XCurveType %curveType);
				Handle(Geom2d_Curve) BuildGeom2d(IIfcCurve^ curve, XCurveType %curveType);

				Handle(Geom_LineWithMagnitude) BuildGeom3d(IIfcLine^ ifcLine);
				Handle(Geom2d_LineWithMagnitude) BuildGeom2d(IIfcLine^ ifcLine);

				Handle(Geom_Circle) BuildGeom3d(IIfcCircle^ ifcCircle);
				Handle(Geom2d_Circle) BuildGeom2d(IIfcCircle^ ifcCircle);
				
				Handle(Geom_Ellipse) BuildGeom3d(IIfcEllipse^ ifcEllipse);
				Handle(Geom2d_Ellipse) BuildGeom2d(IIfcEllipse^ ifcEllipse);

				Handle(Geom_TrimmedCurve) BuildGeom3d(IIfcTrimmedCurve^ ifcTrimmedCurve);
				Handle(Geom2d_TrimmedCurve) BuildGeom2d(IIfcTrimmedCurve^ ifcTrimmedCurve);
			};

		}
	}
}