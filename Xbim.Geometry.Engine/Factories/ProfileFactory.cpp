#include "ProfileFactory.h"
namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			TopoDS_Shape ProfileFactory::Build(IIfcProfileDef^ profileDef)
			{
				XProfileDefType profileType;
				if (!Enum::TryParse<XProfileDefType>(profileDef->ExpressType->ExpressName, profileType))
					throw gcnew XbimGeometryFactoryException("Unsupported ProfileDef type: " + profileDef->ExpressType->ExpressName);
				switch (profileType)
				{
				case XProfileDefType::IfcArbitraryClosedProfileDef:
					return Build(static_cast<IIfcArbitraryClosedProfileDef^>(profileDef));
				case XProfileDefType::IfcArbitraryProfileDefWithVoids:
					break;
				case XProfileDefType::IfcArbitraryOpenProfileDef:
					break;
				case XProfileDefType::IfcCenterLineProfileDef:
					break;
				case XProfileDefType::IfcCompositeProfileDef:
					break;
				case XProfileDefType::IfcDerivedProfileDef:
					break;
				case XProfileDefType::IfcMirroredProfileDef:
					break;
				case XProfileDefType::IfcAsymmetricIShapeProfileDef:
					break;
				case XProfileDefType::IfcCShapeProfileDef:
					break;
				case XProfileDefType::IfcCircleProfileDef:
					break;
				case XProfileDefType::IfcCircleHollowProfileDef:
					break;
				case XProfileDefType::IfcEllipseProfileDef:
					break;
				case XProfileDefType::IfcIShapeProfileDef:
					break;
				case XProfileDefType::IfcLShapeProfileDef:
					break;
				case XProfileDefType::IfcRectangleProfileDef:
					break;
				case XProfileDefType::IfcRectangleHollowProfileDef:
					break;
				case XProfileDefType::IfcRoundedRectangleProfileDef:
					break;
				case XProfileDefType::IfcTShapeProfileDef:
					break;
				case XProfileDefType::IfcTrapeziumProfileDef:
					break;
				case XProfileDefType::IfcUShapeProfileDef:
					break;
				case XProfileDefType::IfcZShapeProfileDef:
					break;
				default:
					break;
				}
				throw gcnew XbimGeometryFactoryException("Not implemented. Profile Definition Type type: " + profileType.ToString());
			}

			TopoDS_Shape ProfileFactory::Build(IIfcArbitraryClosedProfileDef^ arbitraryClosedProfile)
			{
				//validation
				//WR1 The curve used for the outer curve definition shall have the dimensionality of 2.
				if (2 != (int)arbitraryClosedProfile->OuterCurve->Dim)
					throw gcnew XbimGeometryFactoryException("WR1 The curve used for the outer curve definition shall have the dimensionality of 2");
				//WR2 The outer curve shall not be of type IfcLine as IfcLine is not a closed curve.
				if (dynamic_cast<IIfcLine^>(arbitraryClosedProfile->OuterCurve) != nullptr)
					throw gcnew XbimGeometryFactoryException("WR2 The outer curve shall not be of type IfcLine as IfcLine is not a closed curve");
				//WR3 The outer curve shall not be of type IfcOffsetCurve2D as it should not be defined as an offset of another curve.
				if (dynamic_cast<IIfcOffsetCurve2D^>(arbitraryClosedProfile->OuterCurve) != nullptr)
					throw gcnew XbimGeometryFactoryException("WR3 The outer curve shall not be of type IfcOffsetCurve2D as it should not be defined as an offset of another curve");
				Handle(Geom_Surface) hSurface;
				TopoDS_Wire wire = _wireFactory->Build(arbitraryClosedProfile->OuterCurve, hSurface);
				if (wire.IsNull())
					throw gcnew XbimGeometryFactoryException(String::Format("Failed to create: #{0}={1}", arbitraryClosedProfile->EntityLabel, arbitraryClosedProfile->GetType()->Name));

				if (arbitraryClosedProfile->ProfileType == IfcProfileTypeEnum::AREA) //we will need to make a face
				{
					TopoDS_Face face = Ptr()->MakeFace(wire, hSurface);
					if (face.IsNull())
						throw gcnew XbimGeometryFactoryException(String::Format("Failed to create: #{0}={1}", arbitraryClosedProfile->EntityLabel, arbitraryClosedProfile->GetType()->Name));
					else
						return face;
				}
				else
					return wire;
			}
		}
	}
}