#include "EdgeFactory.h"
#include <Geom_Curve.hxx>
#include <Geom2d_Curve.hxx>

namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			XbimEdge^ EdgeFactory::BuildEdge()
			{
				throw gcnew System::NotImplementedException();
				// TODO: insert return statement here
			}

			const TopoDS_Edge& EdgeFactory::BuildTopoEdge(IIfcCurve^ curve)
			{
				XCurveType curveType;
				int dim = (int)curve->Dim;
				if (dim == 2)
				{
					Handle(Geom2d_Curve) hCurve2d = _curveFactory->BuildGeom2d(curve, curveType);
					if (hCurve2d.IsNull()) throw gcnew XbimGeometryFactoryException("Failed to build curve 2d");

				}
				else
				{
					Handle(Geom_Curve) hCurve = _curveFactory->BuildGeom3d(curve, curveType);
					if (hCurve.IsNull()) throw gcnew XbimGeometryFactoryException("Failed to build curve 3d");
				}
			}
		}
	}
}
