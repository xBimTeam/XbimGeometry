#include "LineFactory.h"
#include "../Exceptions/XbimGeometryFactoryException.h"
#include "../BRep//XbimLine.h"
#pragma region Unmanaged
#pragma managed(push, off)

using namespace Xbim::Geometry::Exceptions;



bool LineFactoryNative::Build(Geom_Line** pLine, gp_Pnt pnt, gp_Vec dir)
{
	try
	{
		*pLine = new Geom_Line(pnt, dir);
		return true;
	}
	catch (const std::exception& e)
	{
		pLoggingService->LogError(e.what());
		return false;
	}

}

bool LineFactoryNative::Build2d(Geom2d_Line** pLine2d, gp_Pnt2d pnt, gp_Vec2d dir)
{
	try
	{
		*pLine2d = new Geom2d_Line(pnt, dir);
		return true;
	}
	catch (const std::exception& e)
	{
		pLoggingService->LogError(e.what());
		return false;
	}

	return new Geom2d_Line(gp_Pnt2d(Origin.X(), Origin.Y()), gp_Dir2d(Direction.X(), Direction.Y()));
}

#pragma endregion

#pragma region Managed
#pragma managed(pop)
namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			IXLine^ LineFactory::Build(IIfcLine^ ifcLine)
			{
				if (2 == (int)ifcLine->Dim) //make a 2d line
				{
					gp_Pnt2d origin = GpFactory->BuildPoint2d(ifcLine->Pnt);
					gp_Vec2d direction = GpFactory->BuildVector2d(ifcLine->Dir);
					Geom2d_Line* pLine;
					if (Ptr()->Build2d(&pLine, origin, direction))
						return gcnew Xbim2dLine(pLine, ifcLine->Dir->Magnitude);
					else
						throw gcnew XbimGeometryFactoryException("Failed to build Line");
				}
				else //make a 3d line
				{
					gp_Pnt origin = GpFactory->BuildPoint(ifcLine->Pnt);
					gp_Vec direction = GpFactory->BuildVector(ifcLine->Dir);
					Geom_Line* pLine;
					if (Ptr()->Build(&pLine, origin, direction))
						return gcnew Xbim::Geometry::BRep::XbimLine(pLine, ifcLine->Dir->Magnitude);
					else
						throw gcnew XbimGeometryFactoryException("Failed to build Line");
				}
			}

		}
	}
}
#pragma endregion
