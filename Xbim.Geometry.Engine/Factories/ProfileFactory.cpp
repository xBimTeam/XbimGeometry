#include "ProfileFactory.h"
#include <TopoDS.hxx>
#include "../BRep/XbimWire.h"
#include "../BRep/XbimFace.h"
#include "../BRep/XbimEdge.h"

using namespace Xbim::Geometry::BRep;
namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			IXShape^ ProfileFactory::Build(IIfcProfileDef^ profileDef)
			{
				TopoDS_Shape shape = BuildShape(profileDef);
				
				switch (shape.ShapeType())
				{
				case TopAbs_COMPOUND:
					break;
				case TopAbs_COMPSOLID:
					break;
				case TopAbs_SOLID:
					break;
				case TopAbs_SHELL:
					break;
				case TopAbs_FACE:
					return gcnew XbimFace(TopoDS::Face(shape));
				case TopAbs_WIRE:
					return gcnew XbimWire(TopoDS::Wire(shape));
				case TopAbs_EDGE:
					return gcnew XbimEdge(TopoDS::Edge(shape));
				case TopAbs_VERTEX:
					break;
				case TopAbs_SHAPE:
					break;
				default:
					break;
				}
				throw gcnew XbimGeometryFactoryException("ProfileDef return type not supported");
			}
			TopoDS_Shape ProfileFactory::BuildShape(IIfcProfileDef^ profileDef)
			{
				XProfileDefType profileType;
				if (!Enum::TryParse<XProfileDefType>(profileDef->ExpressType->ExpressName, profileType))
					throw gcnew XbimGeometryFactoryException("Unsupported ProfileDef type: " + profileDef->ExpressType->ExpressName);
				switch (profileType)
				{
				case XProfileDefType::IfcArbitraryClosedProfileDef:
					return BuildShape(static_cast<IIfcArbitraryClosedProfileDef^>(profileDef));
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

			TopoDS_Shape ProfileFactory::BuildShape(IIfcArbitraryClosedProfileDef^ arbitraryClosedProfile)
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
				TopoDS_Wire wire = _wireFactory->BuildWire(arbitraryClosedProfile->OuterCurve, hSurface);
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