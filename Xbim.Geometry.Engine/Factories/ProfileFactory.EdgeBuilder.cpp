#include "ProfileFactory.h"
#include "CurveFactory.h"
#include "EdgeFactory.h"
#include "WireFactory.h"
#include <TopoDS.hxx>
#include <Geom_Plane.hxx>
#include "../BRep/XWire.h"
#include "../BRep/XFace.h"
#include "../BRep/XEdge.h"
#include <GC_MakeCircle.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <GCE2d_MakeCircle.hxx>
#include <BRepBuilderAPI_MakeEdge2d.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <GeomLib.hxx>
#include "../BRep/XCurve.h"
using namespace System;
using namespace Xbim::Geometry::BRep;
using namespace Xbim::Ifc4::Interfaces;
namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			IXEdge^ ProfileFactory::BuildEdge(IIfcProfileDef^ profileDef)
			{
				XProfileDefType profileDefType;
				auto curve = BuildCurve(profileDef, profileDefType);
				if (curve.IsNull())
					throw RaiseGeometryFactoryException("Profile could not be built as a Curve");
				auto edge = EXEC_NATIVE->MakeEdge(curve);
				if (edge.IsNull())
					throw RaiseGeometryFactoryException("Profile could not be built as an Edge");
				return gcnew XEdge(edge);
			}

			TopoDS_Edge ProfileFactory::BuildProfileEdge(IIfcProfileDef^ profileDef)
			{
				XProfileDefType profileType;
				if (!Enum::TryParse<XProfileDefType>(profileDef->ExpressType->ExpressName, profileType))
					throw RaiseGeometryFactoryException("Profile Type is not implemented", profileDef);
				switch (profileType)
				{
				case XProfileDefType::IfcArbitraryClosedProfileDef:
					return BuildProfileEdge(static_cast<IIfcArbitraryClosedProfileDef^>(profileDef));
				case XProfileDefType::IfcArbitraryOpenProfileDef:
					return BuildProfileEdge(static_cast<IIfcArbitraryOpenProfileDef^>(profileDef));
				case XProfileDefType::IfcDerivedProfileDef:
					return BuildProfileEdge(static_cast<IIfcDerivedProfileDef^>(profileDef));
				case XProfileDefType::IfcMirroredProfileDef:
					return BuildProfileEdge(static_cast<IIfcMirroredProfileDef^>(profileDef));
				case XProfileDefType::IfcAsymmetricIShapeProfileDef:
					return BuildProfileEdge(static_cast<IIfcAsymmetricIShapeProfileDef^>(profileDef));
				case XProfileDefType::IfcCShapeProfileDef:
					return BuildProfileEdge(static_cast<IIfcCShapeProfileDef^>(profileDef));
				case XProfileDefType::IfcCircleProfileDef:
					return BuildProfileEdge(static_cast<IIfcCircleProfileDef^>(profileDef));
				case XProfileDefType::IfcEllipseProfileDef:
					return BuildProfileEdge(static_cast<IIfcEllipseProfileDef^>(profileDef));
				case XProfileDefType::IfcIShapeProfileDef:
					return BuildProfileEdge(static_cast<IIfcIShapeProfileDef^>(profileDef));
				case XProfileDefType::IfcLShapeProfileDef:
					return BuildProfileEdge(static_cast<IIfcLShapeProfileDef^>(profileDef));
				case XProfileDefType::IfcRectangleProfileDef:
					return BuildProfileEdge(static_cast<IIfcRectangleProfileDef^>(profileDef));
				case XProfileDefType::IfcRoundedRectangleProfileDef:
					return BuildProfileEdge(static_cast<IIfcRoundedRectangleProfileDef^>(profileDef));
				case XProfileDefType::IfcTShapeProfileDef:
					return BuildProfileEdge(static_cast<IIfcTShapeProfileDef^>(profileDef));
				case XProfileDefType::IfcTrapeziumProfileDef:
					return BuildProfileEdge(static_cast<IIfcTrapeziumProfileDef^>(profileDef));
				case XProfileDefType::IfcUShapeProfileDef:
					return BuildProfileEdge(static_cast<IIfcUShapeProfileDef^>(profileDef));
				case XProfileDefType::IfcZShapeProfileDef:
					return BuildProfileEdge(static_cast<IIfcZShapeProfileDef^>(profileDef));
				default:
					throw RaiseGeometryFactoryException("Profile Type cannot be built as an Edge", profileDef);
				}

			}
			TopoDS_Edge ProfileFactory::BuildProfileEdge(IIfcDerivedProfileDef^ ifcDerivedProfileDef)
			{
				return TopoDS_Edge();
			}
			
			TopoDS_Edge ProfileFactory::BuildProfileEdge(IIfcCircleProfileDef^ ifcCircleProfileDef)
			{
				if (ifcCircleProfileDef->Radius <= 0)
					throw RaiseGeometryFactoryException("IfcCircleProfileDef cannot be built with radius that is not a positive value", ifcCircleProfileDef);

				gp_Ax22d gpax2;
				if (ifcCircleProfileDef->Position != nullptr)
					GEOMETRY_FACTORY->BuildAxis2Placement2d(ifcCircleProfileDef->Position, gpax2); //returns false if it fails
				//make the outer edge
				auto edge = EDGE_FACTORY->BuildCircle(ifcCircleProfileDef->Radius, gpax2); //throws an exception
				return edge;
			}

			TopoDS_Edge ProfileFactory::BuildProfileEdge(IIfcRectangleProfileDef^ ifcRectangleProfileDef)
			{
				return TopoDS_Edge();
			}
			TopoDS_Edge ProfileFactory::BuildProfileEdge(IIfcRoundedRectangleProfileDef^ ifcRoundedRectangleProfileDef)
			{
				return TopoDS_Edge();
			}
			TopoDS_Edge ProfileFactory::BuildProfileEdge(IIfcLShapeProfileDef^ ifcLShapeProfileDef)
			{
				return TopoDS_Edge();
			}
			TopoDS_Edge ProfileFactory::BuildProfileEdge(IIfcUShapeProfileDef^ ifcUShapeProfileDef)
			{
				return TopoDS_Edge();
			}
			TopoDS_Edge ProfileFactory::BuildProfileEdge(IIfcEllipseProfileDef^ ifcEllipseProfileDef)
			{
				return TopoDS_Edge();
			}
			TopoDS_Edge ProfileFactory::BuildProfileEdge(IIfcIShapeProfileDef^ ifcIShapeProfileDef)
			{
				return TopoDS_Edge();
			}
			TopoDS_Edge ProfileFactory::BuildProfileEdge(IIfcZShapeProfileDef^ ifcZShapeProfileDef)
			{
				return TopoDS_Edge();
			}
			TopoDS_Edge ProfileFactory::BuildProfileEdge(IIfcCShapeProfileDef^ ifcCShapeProfileDef)
			{
				return TopoDS_Edge();
			}
			TopoDS_Edge ProfileFactory::BuildProfileEdge(IIfcTShapeProfileDef^ ifcTShapeProfileDef)
			{
				return TopoDS_Edge();
			}
			TopoDS_Edge ProfileFactory::BuildProfileEdge(IIfcArbitraryOpenProfileDef^ ifcArbitraryOpenProfileDef)
			{
				if (dynamic_cast<IIfcCenterLineProfileDef^>(ifcArbitraryOpenProfileDef))
					throw RaiseGeometryFactoryException("An Edge built from an IfcCenterLineProfileDef is not supported, use BuildProfileWire", ifcArbitraryOpenProfileDef);
				else
				{
					Handle(Geom_Curve) curve = CURVE_FACTORY->BuildCurve(ifcArbitraryOpenProfileDef->Curve);
					return EDGE_FACTORY->BuildEdge(curve);
				}
			}

			TopoDS_Edge ProfileFactory::BuildProfileEdge(IIfcArbitraryClosedProfileDef^ ifcArbitraryClosedProfileDef)
			{
				if (dynamic_cast<IIfcLine^>(ifcArbitraryClosedProfileDef->OuterCurve) != nullptr)
					throw RaiseGeometryFactoryException("WR2 The outer curve shall not be of type IfcLine as IfcLine is not a closed curve", ifcArbitraryClosedProfileDef);
				//WR3 The outer curve shall not be of type IfcOffsetCurve2D as it should not be defined as an offset of another curve.
				if (dynamic_cast<IIfcOffsetCurve2D^>(ifcArbitraryClosedProfileDef->OuterCurve) != nullptr)
					throw RaiseGeometryFactoryException("WR3 The outer curve shall not be of type IfcOffsetCurve2D as it should not be defined as an offset of another curve", ifcArbitraryClosedProfileDef);

				return EDGE_FACTORY->BuildEdge(ifcArbitraryClosedProfileDef->OuterCurve); //throws exception			
			}
		}
	}
}