#pragma once
#include "../BRep/XbimLine.h"
#include "../BRep/Xbim2dLine.h"
#include "../Services/LoggingService.h"
#include "GeomProcFactory.h"
using namespace Xbim::Geometry::BRep;
using namespace Xbim::Common::Geometry;
using namespace Xbim::Ifc4::Interfaces;
using namespace Xbim::Geometry::Abstractions;
using namespace Xbim::Geometry::Services;
class LineFactoryNative
{
public:
	gp_Pnt Origin;
	gp_Dir Direction;
    LoggingServiceNative* pLoggingService;
	LineFactoryNative(LoggingServiceNative* loggingService)
	{
		pLoggingService = loggingService;
	};
	bool Build(Geom_Line** pLine, gp_Pnt pnt, gp_Vec dir);
	bool  Build2d(Geom2d_Line** pLine2d, gp_Pnt2d pnt, gp_Vec2d dir);
};
namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			public ref class LineFactory : XbimHandle<LineFactoryNative>
			{
			private:
				GeomProcFactory^ GpFactory;
				LoggingService^ Logger;
			public:
				LineFactory(LoggingService^ loggingService) : XbimHandle(new LineFactoryNative(loggingService))
				{
					GpFactory = gcnew GeomProcFactory();
					Logger = loggingService;
				}
				IXLine^ Build(IIfcLine^ ifcLine);				
			};

		}
	}
}