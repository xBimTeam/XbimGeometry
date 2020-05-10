#include "CurveFactory.h"
#include "../Exceptions/XbimGeometryFactoryException.h"
#include "../BRep//XbimLine.h"
/*
The approach of the curve factory is to build all curves as IXCurve using the build method.
This will ensure correct dimensionality of the curves is maintained
Most curve types can have a 2D or a 3D variant, Ifc hides this in the Dim method.
Any 3D shape can be built from a 2D definition with the Z coordinate set to Zero
It is nor permitted to create 2D shapes from 3D definitions and an exception is thrown.
Managed code is used to navigate the definitions and provide the framework for the unmanaged code to build the native curves
All operations where an OCC excpetion will be thrown are implemented in unmanaged code. (NCurveFactory)
These exceptions are caunght and logged in the managed code
Unmanaged build methods reurn a null handle to the specified geometry type when a critical or error exception has been thrown
*/
using namespace Xbim::Geometry::Exceptions;

namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			IXCurve^ CurveFactory::Build(IIfcCurve^ curve)
			{
				String^ body = curve->GetType()->Name->Substring(4); //remove the "IIfc"
				XCurveType ct = Enum::Parse<XCurveType>(body);
				int dim = (int)curve->Dim;
				bool is2d = dim == 2;
				switch (ct)
				{
					/*case Xbim::Geometry::Abstractions::XCurveType::BoundaryCurve:
						return is2d ? Build2d(dynamic_cast<IIfcBoundedCurve^>(curve)):Build3d(dynamic_cast<IIfcBoundedCurve^>(curve));
					case Xbim::Geometry::Abstractions::XCurveType::BSplineCurveWithKnots:
						return is2d ? Build2d(dynamic_cast<IIfcBSplineCurveWithKnots^>(curve)) : Build3d(dynamic_cast<IIfcBSplineCurveWithKnots^>(curve));
					case Xbim::Geometry::Abstractions::XCurveType::Circle:
						return is2d ? Build2d(dynamic_cast<IIfcCircle^>(curve)) : Build3d(dynamic_cast<IIfcCircle^>(curve));
					case Xbim::Geometry::Abstractions::XCurveType::CompositeCurve:
						return is2d ? Build2d(dynamic_cast<IIfcCompositeCurve^>(curve)) : Build3d(dynamic_cast<IIfcCompositeCurve^>(curve));
					case Xbim::Geometry::Abstractions::XCurveType::CompositeCurveOnSurface:
						return is2d ? Build2d(dynamic_cast<IIfcCompositeCurveOnSurface^>(curve)) : Build3d(dynamic_cast<IIfcCompositeCurveOnSurface^>(curve));
					case Xbim::Geometry::Abstractions::XCurveType::Ellipse:
						return is2d ? Build2d(dynamic_cast<IIfcEllipse^>(curve)) : Build3d(dynamic_cast<IIfcEllipse^>(curve));
					case Xbim::Geometry::Abstractions::XCurveType::IndexedPolyCurve:
						return is2d ? Build2d(dynamic_cast<IIfcIndexedPolyCurve^>(curve)) : Build3d(dynamic_cast<IIfcIndexedPolyCurve^>(curve));*/
				case Xbim::Geometry::Abstractions::XCurveType::Line:
					return is2d ? Build2d(dynamic_cast<IIfcLine^>(curve)) : Build3d(dynamic_cast<IIfcLine^>(curve));
					/*case Xbim::Geometry::Abstractions::XCurveType::OffsetCurve2D:
						return Build2d(dynamic_cast<IIfcOffsetCurve2D^>(curve));
					case Xbim::Geometry::Abstractions::XCurveType::OffsetCurve3D:
						return Build2d(dynamic_cast<IIfcOffsetCurve3D^>(curve));
					case Xbim::Geometry::Abstractions::XCurveType::Pcurve:
						return is2d ? Build2d(dynamic_cast<IIfcPcurve^>(curve)) : Build3d(dynamic_cast<IIfcPcurve^>(curve));
					case Xbim::Geometry::Abstractions::XCurveType::Polyline:
						return is2d ? Build2d(dynamic_cast<IIfcPolyline^>(curve)) : Build3d(dynamic_cast<IIfcPolyline^>(curve));
					case Xbim::Geometry::Abstractions::XCurveType::RationalBSplineCurveWithKnots:
						return is2d ? Build2d(dynamic_cast<IIfcRationalBSplineCurveWithKnots^>(curve)) : Build3d(dynamic_cast<IIfcRationalBSplineCurveWithKnots^>(curve));
					case Xbim::Geometry::Abstractions::XCurveType::SurfaceCurve:
						return is2d ? Build2d(dynamic_cast<IIfcSurfaceCurve^>(curve)) : Build3d(dynamic_cast<IIfcSurfaceCurve^>(curve));*/
				case Xbim::Geometry::Abstractions::XCurveType::TrimmedCurve:
					//return is2d ? Build2d(dynamic_cast<IIfcTrimmedCurve^>(curve)) : Build3d(dynamic_cast<IIfcTrimmedCurve^>(curve));
					break;
				default:
					throw gcnew XbimGeometryFactoryException("Unsupported curve type");
				}
				throw gcnew XbimGeometryFactoryException("Unsupported curve type");
			}

			IXLine^ CurveFactory::Build3d(IIfcLine^ ifcLine)
			{
				gp_Pnt origin = GpFactory->BuildPoint(ifcLine->Pnt);
				gp_Vec direction = GpFactory->BuildVector(ifcLine->Dir);
				Handle(Geom_Line) hl = Ptr()->Build3d(origin, direction);
				if (!hl.IsNull())
					return gcnew Xbim::Geometry::BRep::XbimLine(hl, ifcLine->Dir->Magnitude);
				else
					throw gcnew XbimGeometryFactoryException("Failed to build Line");
			}

			IXLine^ CurveFactory::Build2d(IIfcLine^ ifcLine)
			{
				if (2 != (int)ifcLine->Dim) throw gcnew XbimGeometryFactoryException("Cannot build a 2D curve from a 3D curve");
				gp_Pnt2d origin = GpFactory->BuildPoint2d(ifcLine->Pnt);
				gp_Vec2d direction = GpFactory->BuildVector2d(ifcLine->Dir);
				Handle(Geom2d_Line) hl = Ptr()->Build2d(origin, direction);
				if (!hl.IsNull())
					return gcnew Xbim2dLine(hl, ifcLine->Dir->Magnitude);
				else
					throw gcnew XbimGeometryFactoryException("Failed to build Line");

			}

			/*IXTrimmedCurve^ CurveFactory::Build2d(IIfcTrimmedCurve^ ifcTrimmedCurve)
			{
				if (2 != (int)ifcTrimmedCurve->Dim) throw gcnew XbimGeometryFactoryException("Cannot build a 2D curve from a 3D curve");
			}

			IXTrimmedCurve^ CurveFactory::Build3d(IIfcTrimmedCurve^ ifcTrimmedCurve)
			{

			}*/
		}
	}
}
