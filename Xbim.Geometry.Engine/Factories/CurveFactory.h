#pragma once
#include "../BRep/XbimLine.h"
#include "../BRep/Xbim2dLine.h"
#include "../Services/LoggingService.h"
#include "./Unmanaged/NCurveFactory.h"
#include "GeomProcFactory.h"
using namespace Xbim::Geometry::BRep;
using namespace Xbim::Common::Geometry;
using namespace Xbim::Ifc4::Interfaces;
using namespace Xbim::Geometry::Abstractions;
using namespace Xbim::Geometry::Services;
using namespace Xbim::Geometry::Factories::Unmanaged;
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
			public:
				CurveFactory(LoggingService^ loggingService) : XbimHandle(new NCurveFactory(loggingService))
				{
					GpFactory = gcnew GeomProcFactory();
					Logger = loggingService;
				}
				//Top level abstraction for building any curve
				IXCurve^ Build(IIfcCurve^ curve);
				IXLine^ Build2d(IIfcLine^ ifcLine);
				IXLine^ Build3d(IIfcLine^ ifcLine);
				/*IXTrimmedCurve^ Build2d(IIfcTrimmedCurve^ ifcTrimmedCurve);
				IXTrimmedCurve^ Build3d(IIfcTrimmedCurve^ ifcTrimmedCurve);*/
			};

		}
	}
}