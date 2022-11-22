#include "ProfileFactory.h"
#include "WireFactory.h"
#include <TopoDS.hxx>
#include "../BRep/XWire.h"
#include "../BRep/XFace.h"
#include "../BRep/XEdge.h"

using namespace System;
using namespace Xbim::Geometry::BRep;
namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			/*IXShape^ ProfileFactory::Build(IIfcProfileDef^ profileDef)
			{
				TopoDS_Shape shape = BuildProfile(profileDef);
				
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
					return gcnew XFace(TopoDS::Face(shape));
				case TopAbs_WIRE:
					return gcnew XWire(TopoDS::Wire(shape));
				case TopAbs_EDGE:
					return gcnew XEdge(TopoDS::Edge(shape));
				case TopAbs_VERTEX:
					break;
				case TopAbs_SHAPE:
					break;
				default:
					RaiseGeometryFactoryException("ProfileDef return type not supported", profileDef);
				}
				
			}*/
			IXFace^ ProfileFactory::BuildFace(IIfcProfileDef^ profileDef)
			{
				throw gcnew System::NotImplementedException();
				// TODO: insert return statement here
			}
			IXWire^ ProfileFactory::BuildWire(IIfcProfileDef^ profileDef)
			{
				throw gcnew System::NotImplementedException();
				// TODO: insert return statement here
			}
			IXEdge^ ProfileFactory::BuildEdge(IIfcProfileDef^ profileDef)
			{
				throw gcnew System::NotImplementedException();
				// TODO: insert return statement here
			}
			//NB all profiles are 2d
			TopoDS_Shape ProfileFactory::BuildProfile(IIfcProfileDef^ profileDef)
			{
				XProfileDefType profileType;
				////WR1 The curve used for the outer curve definition shall have the dimensionality of 2.
				//if (2 != (int)profileDef.->OuterCurve->Dim)
				//	throw gcnew XbimGeometryFactoryException("WR1 The curve used for the outer curve definition shall have the dimensionality of 2");
				if (!Enum::TryParse<XProfileDefType>(profileDef->ExpressType->ExpressName, profileType))
					RaiseGeometryFactoryException("Profile Type is not implemented", profileDef);
				switch (profileType)
				{
				case XProfileDefType::IfcArbitraryClosedProfileDef:
					return BuildProfile(static_cast<IIfcArbitraryClosedProfileDef^>(profileDef));
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
					RaiseGeometryFactoryException("Profile Type is not implemented", profileDef);
				}
				
			}

			TopoDS_Shape ProfileFactory::BuildProfile(IIfcArbitraryClosedProfileDef^ arbitraryClosedProfile)
			{
				//validation
				//WR1 The curve used for the outer curve definition shall have the dimensionality of 2. All profiles are 2D checked in BuildProfile
				//WR2 The outer curve shall not be of type IfcLine as IfcLine is not a closed curve.
				if (dynamic_cast<IIfcLine^>(arbitraryClosedProfile->OuterCurve) != nullptr)
					RaiseGeometryFactoryException("WR2 The outer curve shall not be of type IfcLine as IfcLine is not a closed curve", arbitraryClosedProfile);
				//WR3 The outer curve shall not be of type IfcOffsetCurve2D as it should not be defined as an offset of another curve.
				if (dynamic_cast<IIfcOffsetCurve2D^>(arbitraryClosedProfile->OuterCurve) != nullptr)
					RaiseGeometryFactoryException("WR3 The outer curve shall not be of type IfcOffsetCurve2D as it should not be defined as an offset of another curve", arbitraryClosedProfile);
				

				TopoDS_Wire wire = WIRE_FACTORY->Build2d(arbitraryClosedProfile->OuterCurve);
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