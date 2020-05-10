#include "NCurveFactory.h"
namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			namespace Unmanaged
			{
				Handle(Geom_Line) NCurveFactory::Build3d(gp_Pnt pnt, gp_Vec dir)
				{
					try
					{
						return new Geom_Line(pnt, dir);						
					}
					catch (const std::exception& e)
					{
						pLoggingService->LogError(e.what());
						return Handle(Geom_Line)(); //return null handle for checking
					}
				}

				Handle(Geom2d_Line) NCurveFactory::Build2d(gp_Pnt2d pnt, gp_Vec2d dir)
				{
					try
					{
						return new Geom2d_Line(pnt, dir);						
					}
					catch (const std::exception& e)
					{
						pLoggingService->LogError(e.what());
						return Handle(Geom2d_Line)(); //return null handle for checking
					}
				}
			}
		}
	}
}