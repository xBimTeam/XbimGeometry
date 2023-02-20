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

			IXCurve^ ProfileFactory::BuildCurve(IIfcProfileDef^ profileDef)
			{
				XProfileDefType profileType;
				auto curve = BuildCurve(profileDef, profileType);
				return gcnew XCurve(curve, XCurveType::IfcCurve);
			}

			Handle(Geom_Curve) ProfileFactory::BuildCurve(IIfcProfileDef^ profileDef, XProfileDefType% profileType)
			{

				if (!Enum::TryParse<XProfileDefType>(profileDef->ExpressType->ExpressName, profileType))
					throw RaiseGeometryFactoryException("Profile Type is not implemented", profileDef);
				Handle(Geom_Curve) curve3d;
				switch (profileType)
				{
				case XProfileDefType::IfcArbitraryProfileDefWithVoids:
					throw RaiseGeometryFactoryException("A single curve cannot be built from an IfcArbitraryProfileDefWithVoids", profileDef);
				case XProfileDefType::IfcArbitraryClosedProfileDef:
					return BuildCurve((static_cast<IIfcArbitraryClosedProfileDef^>(profileDef)));
				case XProfileDefType::IfcArbitraryOpenProfileDef:
					return BuildCurve((static_cast<IIfcArbitraryOpenProfileDef^>(profileDef)));
				case XProfileDefType::IfcCenterLineProfileDef:
					return BuildCurve(static_cast<IIfcCenterLineProfileDef^>(profileDef));
					/*case XProfileDefType::IfcCompositeProfileDef:
						return BuildProfileFace(static_cast<IIfcCompositeProfileDef^>(profileDef));
					case XProfileDefType::IfcDerivedProfileDef:
						return BuildProfileFace(static_cast<IIfcDerivedProfileDef^>(profileDef));
					case XProfileDefType::IfcMirroredProfileDef:
						return BuildProfileFace(static_cast<IIfcMirroredProfileDef^>(profileDef));
					case XProfileDefType::IfcAsymmetricIShapeProfileDef:
						return BuildProfileFace(static_cast<IIfcAsymmetricIShapeProfileDef^>(profileDef));
					case XProfileDefType::IfcCShapeProfileDef:
						return BuildProfileFace(static_cast<IIfcCShapeProfileDef^>(profileDef));
					case XProfileDefType::IfcCircleHollowProfileDef:
						return BuildProfileFace(static_cast<IIfcCircleHollowProfileDef^>(profileDef));
					case XProfileDefType::IfcCircleProfileDef:
						return BuildProfileFace(static_cast<IIfcCircleProfileDef^>(profileDef));
					case XProfileDefType::IfcEllipseProfileDef:
						return BuildProfileFace(static_cast<IIfcEllipseProfileDef^>(profileDef));
					case XProfileDefType::IfcIShapeProfileDef:
						return BuildProfileFace(static_cast<IIfcIShapeProfileDef^>(profileDef));
					case XProfileDefType::IfcLShapeProfileDef:
						return BuildProfileFace(static_cast<IIfcLShapeProfileDef^>(profileDef));
					case XProfileDefType::IfcRectangleProfileDef:
						return BuildProfileFace(static_cast<IIfcRectangleProfileDef^>(profileDef));
					case XProfileDefType::IfcRectangleHollowProfileDef:
						return BuildProfileFace(static_cast<IIfcRectangleHollowProfileDef^>(profileDef));
					case XProfileDefType::IfcRoundedRectangleProfileDef:
						return BuildProfileFace(static_cast<IIfcRoundedRectangleProfileDef^>(profileDef));
					case XProfileDefType::IfcTShapeProfileDef:
						return BuildProfileFace(static_cast<IIfcTShapeProfileDef^>(profileDef));
					case XProfileDefType::IfcTrapeziumProfileDef:
						return BuildProfileFace(static_cast<IIfcTrapeziumProfileDef^>(profileDef));
					case XProfileDefType::IfcUShapeProfileDef:
						return BuildProfileFace(static_cast<IIfcUShapeProfileDef^>(profileDef));
					case XProfileDefType::IfcZShapeProfileDef:
						return BuildProfileFace(static_cast<IIfcZShapeProfileDef^>(profileDef));*/
				default:
					throw RaiseGeometryFactoryException("Profile Type is not implemented", profileDef);
				}

				return Handle(Geom_Curve)();
			}

			Handle(Geom_Curve) ProfileFactory::BuildCurve(IIfcArbitraryClosedProfileDef^ ifcArbitraryClosedProfileDef)
			{
				//WR1 The curve used for the outer curve definition shall have the dimensionality of 2. All profiles are 2D checked in BuildProfile
				if (2 == (int)ifcArbitraryClosedProfileDef->OuterCurve->Dim)
					throw RaiseGeometryFactoryException("WR1 The curve used for the outer curve definition shall have the dimensionality of 2", ifcArbitraryClosedProfileDef);
				//WR2 The outer curve shall not be of type IfcLine as IfcLine is not a closed curve.
				if (dynamic_cast<IIfcLine^>(ifcArbitraryClosedProfileDef->OuterCurve) != nullptr)
					throw RaiseGeometryFactoryException("WR2 The outer curve shall not be of type IfcLine as IfcLine is not a closed curve", ifcArbitraryClosedProfileDef);
				//WR3 The outer curve shall not be of type IfcOffsetCurve2D as it should not be defined as an offset of another curve.
				if (dynamic_cast<IIfcOffsetCurve2D^>(ifcArbitraryClosedProfileDef->OuterCurve) != nullptr)
					throw RaiseGeometryFactoryException("WR3 The outer curve shall not be of type IfcOffsetCurve2D as it should not be defined as an offset of another curve", ifcArbitraryClosedProfileDef);
				return CURVE_FACTORY->BuildCurve(ifcArbitraryClosedProfileDef->OuterCurve);

			}
			Handle(Geom_Curve) ProfileFactory::BuildCurve(IIfcArbitraryOpenProfileDef^ ifcArbitraryOpenProfileDef)
			{
				//WR12 The curve used for the outer curve definition shall have the dimensionality of 2. All profiles are 2D checked in BuildProfile
				if (2 == (int)ifcArbitraryOpenProfileDef->Curve->Dim)
					throw RaiseGeometryFactoryException("WR12 The curve used for the outer curve definition shall have the dimensionality of 2", ifcArbitraryOpenProfileDef);

				//WR11 The profile type is a .CURVE., an open profile can only be used to define a swept surface.
				if (ifcArbitraryOpenProfileDef->ProfileType != IfcProfileTypeEnum::CURVE)
					throw RaiseGeometryFactoryException("WR11 The profile type must be a .CURVE., an open profile can only be used to define a swept surface.", ifcArbitraryOpenProfileDef);
				return CURVE_FACTORY->BuildCurve(ifcArbitraryOpenProfileDef->Curve);

			}

			Handle(Geom_Curve) ProfileFactory::BuildCurve(IIfcCenterLineProfileDef^ ifcCenterLineProfileDef)
			{
				if (ifcCenterLineProfileDef->Thickness <= 0)
					throw RaiseGeometryFactoryException("IfcCenterLineProfileDef has invalid thickness", ifcCenterLineProfileDef);
				Handle(Geom2d_Curve) centreLine = CURVE_FACTORY->BuildCurve2d(ifcCenterLineProfileDef->Curve); //throws exceptions
				if (centreLine.IsNull())
					throw RaiseGeometryFactoryException("IfcCenterLineProfileDef has invalid curve", ifcCenterLineProfileDef);
				auto aCurve = CURVE_FACTORY->Ref().BuildOffsetCurve2d(centreLine, ifcCenterLineProfileDef->Thickness / 2);
				auto bCurve = CURVE_FACTORY->Ref().BuildOffsetCurve2d(centreLine, ifcCenterLineProfileDef->Thickness / -2);
				if (aCurve.IsNull() || bCurve.IsNull())
					throw RaiseGeometryFactoryException("IfcCenterLineProfileDef has invalid offset curves", ifcCenterLineProfileDef);

				TColGeom2d_SequenceOfBoundedCurve segments;
				gp_Pnt2d aStart, aEnd, bStart, bEnd;

				aCurve->D0(aCurve->FirstParameter(), aStart);
				aCurve->D0(aCurve->LastParameter(), aEnd);
				bCurve->D0(bCurve->FirstParameter(), bStart);
				bCurve->D0(bCurve->LastParameter(), bEnd);
				segments.Append(new Geom2d_TrimmedCurve(aCurve, aCurve->FirstParameter(), aCurve->LastParameter()));
				segments.Append(CURVE_FACTORY->BuildLinearSegment(aStart, bStart));
				segments.Append(new Geom2d_TrimmedCurve(bCurve, bCurve->FirstParameter(), bCurve->LastParameter()));
				segments.Append(CURVE_FACTORY->BuildLinearSegment(bEnd, aEnd));

				Handle(Geom2d_BSplineCurve) bSpline = CURVE_FACTORY->Ref().BuildCompositeCurve2d(segments, ModelGeometryService->Precision);
				if (bSpline.IsNull())
					throw RaiseGeometryFactoryException("IfcCenterLineProfileDef could not be built as a Curve", ifcCenterLineProfileDef);
				return GeomLib::To3d(gp_Ax2(), bSpline);
			}


		}
	}
}