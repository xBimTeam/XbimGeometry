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
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepBuilderAPI_GTransform.hxx>
using namespace System;
using namespace Xbim::Geometry::BRep;
using namespace Xbim::Ifc4::Interfaces;
namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{

			IXWire^ ProfileFactory::BuildWire(IIfcProfileDef^ profileDef)
			{
				XProfileDefType profileType;
				auto wire = BuildProfileWire(profileDef, profileType);
				return gcnew XWire(wire);
			}
			TopoDS_Wire ProfileFactory::BuildProfileWire(IIfcProfileDef^ profileDef, XProfileDefType& profileType)
			{

				if (!Enum::TryParse<XProfileDefType>(profileDef->ExpressType->ExpressName, profileType))
					throw RaiseGeometryFactoryException("Profile Type is not implemented", profileDef);
				TopoDS_Wire wire;
				switch (profileType)
				{
				case XProfileDefType::IfcArbitraryProfileDefWithVoids:
					throw RaiseGeometryFactoryException("A single wire cannot be built from an IfcArbitraryProfileDefWithVoids, use BuildFace", profileDef);
				case XProfileDefType::IfcArbitraryClosedProfileDef:
					wire = BuildProfileWire((static_cast<IIfcArbitraryClosedProfileDef^>(profileDef)));
					break;
				case XProfileDefType::IfcArbitraryOpenProfileDef:
					wire = BuildProfileWire((static_cast<IIfcArbitraryOpenProfileDef^>(profileDef)));
					break;
				case XProfileDefType::IfcCenterLineProfileDef:
					wire = BuildProfileWire(static_cast<IIfcCenterLineProfileDef^>(profileDef));
					break;
				case XProfileDefType::IfcCompositeProfileDef:
					throw RaiseGeometryFactoryException("A single wire cannot be built from an IfcCompositeProfileDef", profileDef);
				case XProfileDefType::IfcDerivedProfileDef:
					throw RaiseGeometryFactoryException("A single wire cannot be built from an IfcDerivedProfileDef", profileDef);
					/*case XProfileDefType::IfcMirroredProfileDef:
						return BuildProfileFace(static_cast<IIfcMirroredProfileDef^>(profileDef));
					case XProfileDefType::IfcAsymmetricIShapeProfileDef:
						return BuildProfileFace(static_cast<IIfcAsymmetricIShapeProfileDef^>(profileDef));
					case XProfileDefType::IfcCShapeProfileDef:
						return BuildProfileFace(static_cast<IIfcCShapeProfileDef^>(profileDef));*/
				case XProfileDefType::IfcCircleHollowProfileDef:
					throw RaiseGeometryFactoryException("A single wire cannot be built from an IfcCircleHollowProfileDef, use BuildFace", profileDef);
				case XProfileDefType::IfcRectangleProfileDef:
					return BuildProfileWire(static_cast<IIfcRectangleProfileDef^>(profileDef));
				case XProfileDefType::IfcRectangleHollowProfileDef:
					throw RaiseGeometryFactoryException("A single wire cannot be built from an IfcRectangleProfileDef, use BuildFace", profileDef);
					/*	case XProfileDefType::IfcCircleProfileDef:
							return BuildProfileFace(static_cast<IIfcCircleProfileDef^>(profileDef));
						case XProfileDefType::IfcEllipseProfileDef:
							return BuildProfileFace(static_cast<IIfcEllipseProfileDef^>(profileDef));
						case XProfileDefType::IfcIShapeProfileDef:
							return BuildProfileFace(static_cast<IIfcIShapeProfileDef^>(profileDef));
						case XProfileDefType::IfcLShapeProfileDef:
							return BuildProfileFace(static_cast<IIfcLShapeProfileDef^>(profileDef));*/
						
						
						case XProfileDefType::IfcRoundedRectangleProfileDef:
							return BuildProfileWire(static_cast<IIfcRoundedRectangleProfileDef^>(profileDef));
					/*	case XProfileDefType::IfcTShapeProfileDef:
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
				if (wire.IsNull())
					throw RaiseGeometryFactoryException("ProfileDef could not be built as a wire", profileDef);
				return wire;
			}
			TopoDS_Wire ProfileFactory::BuildProfileWire(IIfcArbitraryClosedProfileDef^ arbitraryClosedProfile)
			{
				//validation
				//WR1 The curve used for the outer curve definition shall have the dimensionality of 2. All profiles are 2D checked in BuildProfile
				//WR2 The outer curve shall not be of type IfcLine as IfcLine is not a closed curve.
				if (dynamic_cast<IIfcLine^>(arbitraryClosedProfile->OuterCurve) != nullptr)
					throw RaiseGeometryFactoryException("WR2 The outer curve shall not be of type IfcLine as IfcLine is not a closed curve", arbitraryClosedProfile);
				//WR3 The outer curve shall not be of type IfcOffsetCurve2D as it should not be defined as an offset of another curve.
				if (dynamic_cast<IIfcOffsetCurve2D^>(arbitraryClosedProfile->OuterCurve) != nullptr)
					throw RaiseGeometryFactoryException("WR3 The outer curve shall not be of type IfcOffsetCurve2D as it should not be defined as an offset of another curve", arbitraryClosedProfile);

				TopoDS_Wire wire = WIRE_FACTORY->BuildWire(arbitraryClosedProfile->OuterCurve, false); //throws exception
				return wire;
			}

			TopoDS_Wire ProfileFactory::BuildProfileWire(IIfcArbitraryOpenProfileDef^ arbitraryOpenProfile)
			{
				//WR12 The curve used for the outer curve definition shall have the dimensionality of 2. All profiles are 2D checked in BuildProfile
				if (2 == (int)arbitraryOpenProfile->Curve->Dim)
					throw RaiseGeometryFactoryException("WR12 The curve used for the outer curve definition shall have the dimensionality of 2", arbitraryOpenProfile);

				//WR11 The profile type is a .CURVE., an open profile can only be used to define a swept surface.
				if (arbitraryOpenProfile->ProfileType != IfcProfileTypeEnum::CURVE)
					throw RaiseGeometryFactoryException("WR11 The profile type must be a .CURVE., an open profile can only be used to define a swept surface.", arbitraryOpenProfile);

				TopoDS_Wire wire = WIRE_FACTORY->BuildWire(arbitraryOpenProfile->Curve, false); //throws exception
				return wire;
			}



			TopoDS_Wire ProfileFactory::BuildProfileWire(IIfcCircleProfileDef^ circleProfileDef)
			{
				auto edge = BuildProfileEdge(circleProfileDef); //throws an exception
				auto wire = EXEC_NATIVE->MakeWire(edge);
				if (wire.IsNull())
					throw RaiseGeometryFactoryException("Profile wire cound not be built", circleProfileDef);
				else
					return wire;
			}


			TopoDS_Wire ProfileFactory::BuildProfileWire(IIfcRectangleProfileDef^ rectangleProfile)
			{
				if (rectangleProfile->XDim <= 0 || rectangleProfile->YDim <= 0)
					throw RaiseGeometryFactoryException("Invalid rectangle profile with at least one zero or less dimension", rectangleProfile);
				TopLoc_Location location = BuildParameterizedProfilePosition(rectangleProfile->Position);
				auto wire = EXEC_NATIVE->BuildRectangle(rectangleProfile->XDim, rectangleProfile->YDim, location);
				if (wire.IsNull())
					throw RaiseGeometryFactoryException("Profile wire cound not be built", rectangleProfile);
				else
					return wire;
			}
			
			TopoDS_Wire ProfileFactory::BuildProfileWire(IIfcRoundedRectangleProfileDef^ roundedRectangleProfile)
			{
				if (roundedRectangleProfile->XDim <= 0 || roundedRectangleProfile->YDim <= 0)
					throw RaiseGeometryFactoryException("Invalid rectangle profile with at least one zero or less dimension", roundedRectangleProfile);
				TopLoc_Location location = BuildParameterizedProfilePosition(roundedRectangleProfile->Position);
				auto wire = EXEC_NATIVE->BuildRoundedRectangle(roundedRectangleProfile->XDim, roundedRectangleProfile->YDim, roundedRectangleProfile->RoundingRadius, location, ModelGeometryService->Precision);
				if (wire.IsNull())
					throw RaiseGeometryFactoryException("Profile wire cound not be built", roundedRectangleProfile);
				else
					return wire;
			}

			TopoDS_Wire ProfileFactory::BuildProfileWire(IIfcIShapeProfileDef^ iShapedProfile)
			{
				if (iShapedProfile->OverallWidth <= 0 || iShapedProfile->OverallDepth <= 0)
					throw RaiseGeometryFactoryException("Invalid I Shaped profile with at least one zero or less dimension", iShapedProfile);
				TopLoc_Location location = BuildParameterizedProfilePosition(iShapedProfile->Position);
				auto wire = EXEC_NATIVE->BuildIShape(iShapedProfile->OverallWidth, iShapedProfile->OverallDepth, iShapedProfile->FlangeThickness, iShapedProfile->WebThickness,
					iShapedProfile->FilletRadius.HasValue? (double)iShapedProfile->FilletRadius.Value:0., location, ModelGeometryService->Precision, true);
				if (wire.IsNull())
					throw RaiseGeometryFactoryException("Profile wire cound not be built", iShapedProfile);
				else
					return wire;
			}

			void ProfileFactory::BuildProfileWire(IIfcCompositeProfileDef^ compositeProfile, TopTools_ListOfShape& profileWires)
			{
				if (compositeProfile->Profiles->Count < 1)
					throw RaiseGeometryFactoryException("At least 2 profiles are required for a valid composite profile definition", compositeProfile);
				if ((compositeProfile->Profiles->Count < 2))
					LogInformation(compositeProfile, "A composite profile should have at least 2 profiles, only one profile found");
				auto uniformType = Enumerable::First(compositeProfile->Profiles)->ProfileType;
				for each (auto profile in compositeProfile->Profiles)
				{
					if (dynamic_cast<IIfcCompositeProfileDef^>(profile) != nullptr)
						throw RaiseGeometryFactoryException("Composite Profiles cannot contain nested Composite Profiles", profile);
					if (profile->ProfileType != uniformType)
						throw RaiseGeometryFactoryException("Composite Profiles must comprise Profiles of all the same Profile Type (Area or Curve)", profile);
					XProfileDefType profileType;
					auto wire = BuildProfileWire(profile, profileType); //throws exception
					profileWires.Append(wire);
				}
			}

			
			TopLoc_Location ProfileFactory::BuildParameterizedProfilePosition(IIfcAxis2Placement^ position)
			{
				if (position == nullptr) return TopLoc_Location();
				auto position2d = dynamic_cast<IIfcAxis2Placement2D^>(position);
				if (position2d == nullptr) throw RaiseGeometryFactoryException("Parameterized Profile positions must be 2D", position);
				return GEOMETRY_FACTORY->BuildAxis2PlacementLocation(position);
			}

			TopoDS_Wire ProfileFactory::BuildProfileWire(IIfcCenterLineProfileDef^ ifcCenterLineProfileDef)
			{
				if (ifcCenterLineProfileDef->Thickness <= 0)
					throw RaiseGeometryFactoryException("IfcCenterLineProfileDef has invalid thickness", ifcCenterLineProfileDef);
				Handle(Geom2d_Curve) centreLine = CURVE_FACTORY->BuildCurve2d(ifcCenterLineProfileDef->Curve); //throws exceptions
				if (centreLine.IsNull())
					throw RaiseGeometryFactoryException("IfcCenterLineProfileDef has invalid curve", ifcCenterLineProfileDef);
				gp_Pnt2d centrLineStart, centreLineEnd;
				centreLine->D0(centreLine->FirstParameter(), centrLineStart);
				centreLine->D0(centreLine->LastParameter(), centreLineEnd);
				if (centrLineStart.Distance(centreLineEnd) < ModelGeometryService->Precision)
					throw RaiseGeometryFactoryException("IfcCenterLineProfileDef must have an open curve for the centre line", ifcCenterLineProfileDef);
				auto aCurve = CURVE_FACTORY->Ref().BuildOffsetCurve2d(centreLine, ifcCenterLineProfileDef->Thickness / 2);
				auto bCurve = CURVE_FACTORY->Ref().BuildOffsetCurve2d(centreLine, ifcCenterLineProfileDef->Thickness / -2);
				if (aCurve.IsNull() || bCurve.IsNull())
					throw RaiseGeometryFactoryException("IfcCenterLineProfileDef has invalid offset curves", ifcCenterLineProfileDef);

				TopTools_ListOfShape edges;
				gp_Pnt2d aStart, aEnd, bStart, bEnd;

				aCurve->D0(aCurve->FirstParameter(), aStart);
				aCurve->D0(aCurve->LastParameter(), aEnd);
				bCurve->D0(bCurve->FirstParameter(), bStart);
				bCurve->D0(bCurve->LastParameter(), bEnd);

				edges.Append(EXEC_NATIVE->MakeEdge(aCurve));
				edges.Append(EXEC_NATIVE->MakeEdge(aStart, bStart));
				edges.Append(EXEC_NATIVE->MakeEdge(bCurve));
				edges.Append(EXEC_NATIVE->MakeEdge(bEnd, aEnd));

				TopoDS_Wire wire = EXEC_NATIVE->MakeWire(edges);
				if (wire.IsNull())
					throw RaiseGeometryFactoryException("IfcCenterLineProfileDef could not be built as a wire", ifcCenterLineProfileDef);
				return wire;

			}



		}
	}
}