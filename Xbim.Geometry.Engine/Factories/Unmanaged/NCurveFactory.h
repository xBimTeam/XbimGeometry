#pragma once
#include <gp_Pnt.hxx>
#include <Geom_Line.hxx>
#include <Geom2d_Line.hxx>
#include "../../Services/LoggingService.h"
using namespace Xbim::Geometry::Services;
namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			namespace Unmanaged
			{
				class NCurveFactory
				{
				public:
					gp_Pnt Origin;
					gp_Dir Direction;
					LoggingServiceNative* pLoggingService;
					NCurveFactory(LoggingServiceNative* loggingService)
					{
						pLoggingService = loggingService;
					};
					Handle(Geom2d_Line) Build2d(gp_Pnt2d pnt, gp_Vec2d dir);
					Handle(Geom_Line) Build3d(gp_Pnt pnt, gp_Vec dir);
					
				};
			}
		}
	}
}


