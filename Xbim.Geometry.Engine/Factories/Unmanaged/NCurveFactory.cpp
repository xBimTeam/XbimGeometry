#include "NCurveFactory.h"

namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			namespace Unmanaged
			{
				Handle(Geom_LineWithMagnitude) NCurveFactory::BuildLine3d(gp_Pnt pnt, gp_Dir dir, double magnitude)
				{
					try
					{
						return new Geom_LineWithMagnitude(pnt, dir, magnitude);
					}
					catch (const std::exception& e)
					{
						pLoggingService->LogError(e.what());
						return Handle(Geom_LineWithMagnitude)(); //return null handle for checking
					}
				}

				Handle(Geom_Circle) NCurveFactory::BuildCircle3d(gp_Ax2 axis, double radius)
				{
					try
					{
						return new Geom_Circle(axis, radius);
					}
					catch (const std::exception& e)
					{
						pLoggingService->LogError(e.what());
						return Handle(Geom_Circle)(); //return null handle for checking
					}
				}

				Handle(Geom2d_Circle) NCurveFactory::BuildCircle2d(gp_Ax2d axis, double radius)
				{
					try
					{
						return new Geom2d_Circle(axis, radius);
					}
					catch (const std::exception& e)
					{
						pLoggingService->LogError(e.what());
						return Handle(Geom2d_Circle)(); //return null handle for checking
					}
				}

				Handle(Geom_Ellipse) NCurveFactory::BuildEllipse3d(gp_Ax2 axis, double major, double minor)
				{
					try
					{
						return new Geom_Ellipse(axis, major, minor);
					}
					catch (const std::exception& e)
					{
						pLoggingService->LogError(e.what());
						return Handle(Geom_Ellipse)(); //return null handle for checking
					}
				}

				Handle(Geom2d_Ellipse) NCurveFactory::BuildEllipse2d(gp_Ax2d axis, double major, double minor)
				{
					try
					{
						return new Geom2d_Ellipse(axis, major, minor);
					}
					catch (const std::exception& e)
					{
						pLoggingService->LogError(e.what());
						return Handle(Geom2d_Ellipse)(); //return null handle for checking
					}
				}

				Handle(Geom2d_LineWithMagnitude) NCurveFactory::BuildLine2d(gp_Pnt2d pnt, gp_Dir2d dir, double magnitude)
				{
					try
					{
						return new Geom2d_LineWithMagnitude(pnt, dir, magnitude);
					}
					catch (const std::exception& e)
					{
						pLoggingService->LogError(e.what());
						return Handle(Geom2d_LineWithMagnitude)(); //return null handle for checking
					}
				}
				Handle(Geom_TrimmedCurve) NCurveFactory::BuildTrimmedCurve3d(Handle(Geom_Curve) basisCurve, double u1, double u2, bool sense)
				{
					try
					{
						return new Geom_TrimmedCurve(basisCurve, u1, u2, sense);
					}
					catch (const std::exception& e)
					{
						pLoggingService->LogError(e.what());
						return Handle(Geom_TrimmedCurve)(); //return null handle for checking
					}
				}
				Handle(Geom2d_TrimmedCurve) NCurveFactory::BuildTrimmedCurve2d(Handle(Geom2d_Curve) basisCurve, double u1, double u2, bool sense)
				{
					try
					{
						return new Geom2d_TrimmedCurve(basisCurve, u1, u2, sense);
					}
					catch (const std::exception& e)
					{
						pLoggingService->LogError(e.what());
						return Handle(Geom2d_TrimmedCurve)(); //return null handle for checking
					}
				}
			}
		}
	}
}