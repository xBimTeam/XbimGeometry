#pragma once
#include "../BRep/XLine.h"
#include "../BRep/XLine2d.h"
#include "../Services/LoggingService.h"
#include "./Unmanaged/NCurveFactory.h"
#include "GeometryProcedures.h"

#include <Geom_Curve.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom_Line.hxx>
#include <Geom2d_Line.hxx>
#include <Geom_Circle.hxx>
#include <Geom2d_Circle.hxx>
#include <Geom_Ellipse.hxx>
#include <Geom2d_Ellipse.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom2d_BSplineCurve.hxx>

#include <Geom_TrimmedCurve.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <Precision.hxx>


using namespace Xbim::Geometry::BRep;
using namespace Xbim::Common::Geometry;
using namespace Xbim::Ifc4::Interfaces;
using namespace Xbim::Geometry::Abstractions;
using namespace Xbim::Geometry::Services;
using namespace Xbim::Common;
namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			public ref class CurveFactory : XbimHandle<NCurveFactory>, IXCurveFactory
			{
			private:
				GeometryProcedures^ GpFactory;
				IXLoggingService^ _loggerService;
				IXModelService^ _modelService;
				
				
			public:
				CurveFactory(IXLoggingService^ loggingService, IXModelService^ modelService) : XbimHandle(new NCurveFactory())
				{
					GpFactory = gcnew GeometryProcedures(loggingService, modelService);
					_loggerService = loggingService;
					_modelService = modelService;
					NLoggingService* logService = new NLoggingService();
					logService->SetLogger(static_cast<WriteLog>(loggingService->LogDelegatePtr.ToPointer()));
					Ptr()->SetLogger(logService);
				}
				
				//Top level abstraction for building any curve

				IXCurve^ BuildXCurve(Handle(Geom_Curve) curve, XCurveType curveType);
				IXCurve^ BuildXCurve(Handle(Geom2d_Curve) curve, XCurveType curveType);
				IXCurve^ BuildXDirectrix(IIfcCurve^ curve, double startParam, double endParam);

				//Geometry builders
				virtual IXCurve^ Build(IIfcCurve^ curve);
				virtual IXCurve^ BuildDirectrix(IIfcCurve^ curve, System::Nullable<double> startParam, System::Nullable<double> endParam);
				virtual property IXModelService^ ModelService {IXModelService^ get() { return _modelService; }};
				virtual property IXLoggingService^ LoggingService {IXLoggingService^ get() { return _loggerService; }};


				Handle(Geom_Curve) BuildGeom3d(IIfcCurve^ curve, XCurveType %curveType);
				Handle(Geom2d_Curve) BuildGeom2d(IIfcCurve^ curve, XCurveType %curveType);
				/*Handle(Geom_TrimmedCurve) BuildGeom3d(IIfcBoundaryCurve^ ifcBoundaryCurve);
				Handle(Geom2d_TrimmedCurve) BuildGeom2d(IIfcBoundaryCurve^ ifcBoundaryCurve);*/

				Handle(Geom_Curve) BuildDirectrix(IIfcCurve^ curve, double startParam, double endParam, XCurveType% curveType);
				/*Handle(Geom_TrimmedCurve) BuildGeom3d(IIfcBoundaryCurve^ ifcBoundaryCurve);
				Handle(Geom2d_TrimmedCurve) BuildGeom2d(IIfcBoundaryCurve^ ifcBoundaryCurve);*/

				Handle(Geom_LineWithMagnitude) BuildGeom3d(IIfcLine^ ifcLine);
				Handle(Geom2d_LineWithMagnitude) BuildGeom2d(IIfcLine^ ifcLine);

				Handle(Geom_Circle) BuildGeom3d(IIfcCircle^ ifcCircle);
				Handle(Geom2d_Circle) BuildGeom2d(IIfcCircle^ ifcCircle);
				
				Handle(Geom_Ellipse) BuildGeom3d(IIfcEllipse^ ifcEllipse);
				Handle(Geom2d_Ellipse) BuildGeom2d(IIfcEllipse^ ifcEllipse);

				Handle(Geom_TrimmedCurve) BuildGeom3d(IIfcTrimmedCurve^ ifcTrimmedCurve);
				Handle(Geom2d_TrimmedCurve) BuildGeom2d(IIfcTrimmedCurve^ ifcTrimmedCurve);

				Handle(Geom_Curve) BuildGeom3d(IIfcPolyline^ ifcPolyline);
				
				//Handle(Geom2d_BSplineCurve) BuildGeom2d(IIfcPolyline^ ifcPolyline);

				Handle(Geom_BSplineCurve) BuildGeom3d(IIfcCompositeCurve^ ifcCompositeCurve);


				bool IsBoundedCurve(IIfcCurve^ curve);
			};

		}
	}
}