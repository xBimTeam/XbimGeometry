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

using namespace System;
using namespace Xbim::Geometry::BRep;
using namespace Xbim::Ifc4::Interfaces;
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
					throw RaiseGeometryFactoryException("ProfileDef return type not supported", profileDef);
				}

			}*/
			IXFace^ ProfileFactory::BuildFace(IIfcProfileDef^ profileDef)
			{
				TopoDS_Face face = BuildProfileFace(profileDef);
				return gcnew XFace(face);
			}
			IXWire^ ProfileFactory::BuildWire(IIfcProfileDef^ profileDef)
			{
				throw gcnew System::NotImplementedException();
				// TODO: insert return statement here
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
			TopoDS_Wire ProfileFactory::BuildProfileWire(IIfcCircleProfileDef^ arbitraryClosedProfile)
			{
				auto edge = BuildProfileEdge(arbitraryClosedProfile); //throws an exception
				auto wire = EXEC_NATIVE->MakeWire(edge);
				if (wire.IsNull())
					throw RaiseGeometryFactoryException("Profile wire cound not be built", arbitraryClosedProfile);
				else
					return wire;
			}

			TopoDS_Wire ProfileFactory::BuildProfileWire(IIfcRectangleProfileDef^ rectangleProfile)
			{
				TopLoc_Location location;
				if (rectangleProfile != nullptr)
					location = GEOMETRY_FACTORY->BuildAxis2PlacementLocation(rectangleProfile->Position);
				auto wire = EXEC_NATIVE->BuildRectangle(rectangleProfile->XDim, rectangleProfile->YDim, location);
				if (wire.IsNull())
					throw RaiseGeometryFactoryException("Profile wire cound not be built", rectangleProfile);
				else
					return wire;
			}

			IXEdge^ ProfileFactory::BuildEdge(IIfcProfileDef^ profileDef)
			{
				throw gcnew System::NotImplementedException();
				// TODO: insert return statement here
			}


			TopoDS_Face ProfileFactory::BuildProfileFace(const TopoDS_Wire& wire)
			{
				TopoDS_Face face = OccHandle().MakeFace(wire);
				if (face.IsNull())
					throw RaiseGeometryFactoryException("Profile cound not be built from wire");
				return face;
			}
			//NB all profiles are 2d
			TopoDS_Face ProfileFactory::BuildProfileFace(IIfcProfileDef^ profileDef)
			{
				XProfileDefType profileType;
				////WR1 The curve used for the outer curve definition shall have the dimensionality of 2.
				//if (2 != (int)profileDef.->OuterCurve->Dim)
				//	throw gcnew XbimGeometryFactoryException("WR1 The curve used for the outer curve definition shall have the dimensionality of 2");
				if (!Enum::TryParse<XProfileDefType>(profileDef->ExpressType->ExpressName, profileType))
					throw RaiseGeometryFactoryException("Profile Type is not implemented", profileDef);
				switch (profileType)
				{
				case XProfileDefType::IfcArbitraryProfileDefWithVoids:
					return BuildProfileFace(static_cast<IIfcArbitraryProfileDefWithVoids^>(profileDef));
				case XProfileDefType::IfcArbitraryClosedProfileDef:
					return BuildProfileFace(static_cast<IIfcArbitraryClosedProfileDef^>(profileDef));
				case XProfileDefType::IfcArbitraryOpenProfileDef:
					return BuildProfileFace(static_cast<IIfcArbitraryOpenProfileDef^>(profileDef));
				case XProfileDefType::IfcCenterLineProfileDef:
					return BuildProfileFace(static_cast<IIfcCenterLineProfileDef^>(profileDef));
				case XProfileDefType::IfcCompositeProfileDef:
					return BuildProfileFace(static_cast<IIfcCompositeProfileDef^>(profileDef));
				case XProfileDefType::IfcDerivedProfileDef:
					return BuildProfileFace(static_cast<IIfcDerivedProfileDef^>(profileDef));
				case XProfileDefType::IfcMirroredProfileDef:
					return BuildProfileFace(static_cast<IIfcMirroredProfileDef^>(profileDef));
				case XProfileDefType::IfcAsymmetricIShapeProfileDef:
					return BuildProfileFace(static_cast<IIfcAsymmetricIShapeProfileDef^>(profileDef));
				case XProfileDefType::IfcCShapeProfileDef:
					return BuildProfileFace(static_cast<IIfcCShapeProfileDef^>(profileDef));
				case XProfileDefType::IfcCircleProfileDef:
					return BuildProfileFace(static_cast<IIfcCircleProfileDef^>(profileDef));
				case XProfileDefType::IfcCircleHollowProfileDef:
					return BuildProfileFace(static_cast<IIfcCircleHollowProfileDef^>(profileDef));
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
					return BuildProfileFace(static_cast<IIfcZShapeProfileDef^>(profileDef));
				default:
					throw RaiseGeometryFactoryException("Profile Type is not implemented", profileDef);
				}

			}
			TopoDS_Face ProfileFactory::BuildProfileFace(IIfcArbitraryProfileDefWithVoids^ arbitraryClosedProfileWithVoids)
			{

				TopoDS_Wire outerProfile = BuildProfileWire(static_cast<IIfcArbitraryClosedProfileDef^>(arbitraryClosedProfileWithVoids));
				TopTools_SequenceOfShape innerLoops;
				for each (auto innerWire in arbitraryClosedProfileWithVoids->InnerCurves)
				{
					TopoDS_Wire wire = WIRE_FACTORY->BuildWire(innerWire, false); //throws exception
					innerLoops.Append(wire);
				}
				TopoDS_Face face = EXEC_NATIVE->MakeFace(outerProfile, innerLoops);
				if (face.IsNull())
					throw RaiseGeometryFactoryException("Failed to create IfcArbitraryProfileDefWithVoids", arbitraryClosedProfileWithVoids);
				else
					return face;
			}

			TopoDS_Face ProfileFactory::BuildProfileFace(IIfcArbitraryClosedProfileDef^ arbitraryClosedProfile)
			{

				TopoDS_Wire wire = BuildProfileWire(arbitraryClosedProfile); //throws exception
				TopoDS_Face face = EXEC_NATIVE->MakeFace(wire);

				if (face.IsNull())
					throw RaiseGeometryFactoryException("Failed to create IfcArbitraryClosedProfileDef", arbitraryClosedProfile);
				else
					return face;

			}
			TopoDS_Face ProfileFactory::BuildProfileFace(IIfcDerivedProfileDef^ ifcDerivedProfileDef)
			{
				throw RaiseGeometryFactoryException("Failed to create Profile", ifcDerivedProfileDef);
			}
			TopoDS_Face ProfileFactory::BuildProfileFace(IIfcParameterizedProfileDef^ ifcParameterizedProfileDef)
			{
				throw RaiseGeometryFactoryException("Failed to create Profile", ifcParameterizedProfileDef);
			}
			TopoDS_Face ProfileFactory::BuildProfileFace(IIfcCircleProfileDef^ ifcCircleProfileDef)
			{
				auto wire = BuildProfileWire(ifcCircleProfileDef); //throws an exception
				if (wire.IsNull())
					throw RaiseGeometryFactoryException("Failed to create IfcCircleProfileDef, invalid wire", ifcCircleProfileDef);
				TopoDS_Face face = EXEC_NATIVE->MakeFace(wire);
				if (face.IsNull())
					throw RaiseGeometryFactoryException("Failed to create profile face", ifcCircleProfileDef);
				else
					return face;
			}
			TopoDS_Face ProfileFactory::BuildProfileFace(IIfcRectangleProfileDef^ ifcRectangleProfileDef)
			{
				auto wire = BuildProfileWire(ifcRectangleProfileDef);
				if (wire.IsNull())
					throw RaiseGeometryFactoryException("Failed to create profile wire", ifcRectangleProfileDef);
				TopoDS_Face face = EXEC_NATIVE->MakeFace(wire);
				if (face.IsNull())
					throw RaiseGeometryFactoryException("Failed to create profile face", ifcRectangleProfileDef);
				else
					return face;
			}
			TopoDS_Face ProfileFactory::BuildProfileFace(IIfcRoundedRectangleProfileDef^ ifcRoundedRectangleProfileDef)
			{
				throw RaiseGeometryFactoryException("Failed to create Profile", ifcRoundedRectangleProfileDef);
			}
			TopoDS_Face ProfileFactory::BuildProfileFace(IIfcLShapeProfileDef^ ifcLShapeProfileDef)
			{
				throw RaiseGeometryFactoryException("Failed to create Profile", ifcLShapeProfileDef);
			}
			TopoDS_Face ProfileFactory::BuildProfileFace(IIfcUShapeProfileDef^ ifcUShapeProfileDef)
			{
				throw RaiseGeometryFactoryException("Failed to create Profile", ifcUShapeProfileDef);
			}
			TopoDS_Face ProfileFactory::BuildProfileFace(IIfcEllipseProfileDef^ ifcEllipseProfileDef)
			{
				throw RaiseGeometryFactoryException("Failed to create Profile", ifcEllipseProfileDef);
			}
			TopoDS_Face ProfileFactory::BuildProfileFace(IIfcIShapeProfileDef^ ifcIShapeProfileDef)
			{
				throw RaiseGeometryFactoryException("Failed to create Profile", ifcIShapeProfileDef);
			}
			TopoDS_Face ProfileFactory::BuildProfileFace(IIfcZShapeProfileDef^ ifcZShapeProfileDef)
			{
				throw RaiseGeometryFactoryException("Failed to create Profile", ifcZShapeProfileDef);
			}
			TopoDS_Face ProfileFactory::BuildProfileFace(IIfcCShapeProfileDef^ ifcCShapeProfileDef)
			{
				throw RaiseGeometryFactoryException("Failed to create Profile", ifcCShapeProfileDef);
			}
			TopoDS_Face ProfileFactory::BuildProfileFace(IIfcTShapeProfileDef^ ifcTShapeProfileDef)
			{
				throw RaiseGeometryFactoryException("Failed to create Profile", ifcTShapeProfileDef);
			}
			TopoDS_Face ProfileFactory::BuildProfileFace(IIfcArbitraryOpenProfileDef^ ifcArbitraryOpenProfileDef)
			{

				throw RaiseGeometryFactoryException("Failed to create Profile", ifcArbitraryOpenProfileDef);
			}
			TopoDS_Face ProfileFactory::BuildProfileFace(IIfcCenterLineProfileDef^ ifcCenterLineProfileDef)
			{
				throw RaiseGeometryFactoryException("Failed to create Profile", ifcCenterLineProfileDef);
				XCurveType curveType;
				Handle(Geom2d_Curve) centreLine = CURVE_FACTORY->BuildCurve2d(ifcCenterLineProfileDef->Curve, curveType);
				if (centreLine.IsNull())
					throw RaiseGeometryFactoryException("Error building centre line Curve", ifcCenterLineProfileDef);

				return TopoDS_Face();
			}
			TopoDS_Face ProfileFactory::BuildProfileFace(double x, double y, double tolerance, bool centre)
			{
				throw RaiseGeometryFactoryException("Failed to create Profile");
				return TopoDS_Face();
			}

			TopoDS_Edge ProfileFactory::BuildProfileEdge(IIfcProfileDef^ profileDef)
			{
				XProfileDefType profileType;
				////WR1 The curve used for the outer curve definition shall have the dimensionality of 2.
				//if (2 != (int)profileDef.->OuterCurve->Dim)
				//	throw gcnew XbimGeometryFactoryException("WR1 The curve used for the outer curve definition shall have the dimensionality of 2");
				if (!Enum::TryParse<XProfileDefType>(profileDef->ExpressType->ExpressName, profileType))
					throw RaiseGeometryFactoryException("Profile Type is not implemented", profileDef);
				switch (profileType)
				{
				case XProfileDefType::IfcArbitraryClosedProfileDef:
					return BuildProfileEdge(static_cast<IIfcArbitraryClosedProfileDef^>(profileDef));
				case XProfileDefType::IfcArbitraryProfileDefWithVoids:
					return BuildProfileEdge(static_cast<IIfcArbitraryProfileDefWithVoids^>(profileDef));
				case XProfileDefType::IfcArbitraryOpenProfileDef:
					return BuildProfileEdge(static_cast<IIfcArbitraryOpenProfileDef^>(profileDef));
				case XProfileDefType::IfcCenterLineProfileDef:
					throw RaiseGeometryFactoryException("An Edge built from an IfcCenterLineProfileDef is not supported, use BuildProfileWire", profileDef);;
				case XProfileDefType::IfcCompositeProfileDef:
					return BuildProfileEdge(static_cast<IIfcCompositeProfileDef^>(profileDef));
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
				case XProfileDefType::IfcCircleHollowProfileDef:
					return BuildProfileEdge(static_cast<IIfcCircleHollowProfileDef^>(profileDef));
				case XProfileDefType::IfcEllipseProfileDef:
					return BuildProfileEdge(static_cast<IIfcEllipseProfileDef^>(profileDef));
				case XProfileDefType::IfcIShapeProfileDef:
					return BuildProfileEdge(static_cast<IIfcIShapeProfileDef^>(profileDef));
				case XProfileDefType::IfcLShapeProfileDef:
					return BuildProfileEdge(static_cast<IIfcLShapeProfileDef^>(profileDef));
				case XProfileDefType::IfcRectangleProfileDef:
					return BuildProfileEdge(static_cast<IIfcRectangleProfileDef^>(profileDef));
				case XProfileDefType::IfcRectangleHollowProfileDef:
					return BuildProfileEdge(static_cast<IIfcRectangleHollowProfileDef^>(profileDef));
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
					throw RaiseGeometryFactoryException("Profile Type is not implemented", profileDef);
				}

			}
			TopoDS_Edge ProfileFactory::BuildProfileEdge(IIfcDerivedProfileDef^ ifcDerivedProfileDef)
			{
				return TopoDS_Edge();
			}
			TopoDS_Edge ProfileFactory::BuildProfileEdge(IIfcParameterizedProfileDef^ ifcParameterizedProfileDef)
			{
				return TopoDS_Edge();
			}
			TopoDS_Edge ProfileFactory::BuildProfileEdge(IIfcCircleProfileDef^ ifcCircleProfileDef)
			{
				if (ifcCircleProfileDef->Radius <= 0)
					throw RaiseGeometryFactoryException("IfcCircleProfileDef cannot be built with radius that is not a positive value", ifcCircleProfileDef);

				gp_Ax22d gpax2;
				IIfcAxis2Placement2D^ ax2 = dynamic_cast<IIfcAxis2Placement2D^>(ifcCircleProfileDef->Position);
				if (ax2 != nullptr)
				{
					gpax2.SetLocation(gp_Pnt2d(ax2->Location->X, ax2->Location->Y));
					if (ax2->RefDirection == nullptr)
						gpax2.SetXDirection(gp_Dir2d(1, 0));
					else
						gpax2.SetXDirection(gp_Dir2d(ax2->RefDirection->X, ax2->RefDirection->Y));
				}
				//make the outer wire
				gp_Circ2d outer(gpax2, ifcCircleProfileDef->Radius);
				Handle(Geom2d_Circle) hOuter = GCE2d_MakeCircle(outer);
				TopoDS_Edge  edge = EXEC_NATIVE->MakeEdge2d(hOuter);
				if (edge.IsNull())
					throw RaiseGeometryFactoryException("Failed to create IfcCircleProfileDef, invalid edge", ifcCircleProfileDef);
				else
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
					Handle(Geom_Curve) curve = CURVE_FACTORY->BuildCurve3d(ifcArbitraryOpenProfileDef->Curve);
					return EDGE_FACTORY->BuildEdge(curve);
				}
			}

			TopoDS_Edge ProfileFactory::BuildProfileEdge(IIfcArbitraryClosedProfileDef^ ifcArbitraryClosedProfileDef)
			{
				return TopoDS_Edge();
			}
		}
	}
}