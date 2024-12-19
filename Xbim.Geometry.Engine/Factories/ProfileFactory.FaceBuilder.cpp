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
#include <BRepBuilderAPI_GTransform.hxx>
#include <BRepBuilderAPI_Transform.hxx>
using namespace System;
using namespace Xbim::Geometry::BRep;
using namespace Xbim::Ifc4::Interfaces;
namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{

			IXFace^ ProfileFactory::BuildFace(IIfcProfileDef^ profileDef)
			{
				TopoDS_Face face = BuildProfileFace(profileDef);
				return gcnew XFace(face);
			}


			//NB all profiles are 2d
			TopoDS_Face ProfileFactory::BuildProfileFace(IIfcProfileDef^ profileDef)
			{
				XProfileDefType profileType;

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
					return BuildProfileFace(static_cast<IIfcZShapeProfileDef^>(profileDef));
				default:
					throw RaiseGeometryFactoryException("Profile Type is not implemented", profileDef);
				}

			}
			TopoDS_Face ProfileFactory::BuildProfileFace(IIfcArbitraryProfileDefWithVoids^ arbitraryClosedProfileWithVoids)
			{

				TopoDS_Wire outerProfile = BuildProfileWire(static_cast<IIfcArbitraryClosedProfileDef^>(arbitraryClosedProfileWithVoids));
				TopTools_SequenceOfShape innerLoops;
				HashSet<int>^ innerLoopIds = gcnew HashSet<int>();

				for each (IIfcCurve ^ innerWire in arbitraryClosedProfileWithVoids->InnerCurves)
				{
					if (innerLoopIds->Add(innerWire->EntityLabel)) //only add a loop once
					{
						TopoDS_Wire wire = WIRE_FACTORY->BuildWire(innerWire, false); //throws exception
						bool isCounterClockWise;
						wire = EXEC_NATIVE->MakeValidAreaProfileWire(wire, isCounterClockWise);
						if (!wire.IsNull())
						{
							//ensure voids are clockwise winding as outer loop is always counter clockwise
							if (isCounterClockWise) wire.Reverse();
							innerLoops.Append(wire);
						}
						else
							LogDebug(innerWire,"ArbritraryClosedProfileDef with an zero area void. Void ignored");
					}
				}
				TopoDS_Face face = EXEC_NATIVE->MakeFace(outerProfile, innerLoops, ModelGeometryService->Precision);
				if (face.IsNull())
					throw RaiseGeometryFactoryException("Failed to create IfcArbitraryProfileDefWithVoids", arbitraryClosedProfileWithVoids);
				else
					return face;
			}

			TopoDS_Face ProfileFactory::BuildProfileFace(IIfcAsymmetricIShapeProfileDef^ asymmetricIShapeProfile)
			{
				auto wire = BuildProfileWire(asymmetricIShapeProfile);
				if (wire.IsNull())
					throw RaiseGeometryFactoryException("Failed to create profile wire", asymmetricIShapeProfile);
				TopoDS_Face face = EXEC_NATIVE->MakeFace(wire);
				if (face.IsNull())
					throw RaiseGeometryFactoryException("Failed to create profile face", asymmetricIShapeProfile);
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


			void ProfileFactory::BuildProfileFace(IIfcCompositeProfileDef^ compositeProfile, TopTools_ListOfShape& profileFaces)
			{
				TopTools_ListOfShape wires;
				BuildProfileWire(compositeProfile, wires); //throws exception
				for (auto&& wire : wires)
				{
					auto face = EXEC_NATIVE->MakeFace(TopoDS::Wire(wire));
					profileFaces.Append(face);
				}
			}

			TopoDS_Face ProfileFactory::BuildProfileFace(IIfcCircleHollowProfileDef^ ifcCircleHollowProfileDef)
			{
				if (ifcCircleHollowProfileDef->Radius <= 0)
					throw RaiseGeometryFactoryException("Outer radius  is not a positive value", ifcCircleHollowProfileDef);

				gp_Ax22d gpax2;
				if (ifcCircleHollowProfileDef->Position != nullptr)
					GEOMETRY_FACTORY->BuildAxis2Placement2d(ifcCircleHollowProfileDef->Position, gpax2); //returns false if it fails, gpax2 statys as default for a null value of position
				//make the outer edge
				auto outerEdge = EDGE_FACTORY->BuildCircle(ifcCircleHollowProfileDef->Radius, gpax2); //throws an exception
				auto outerWire = EXEC_NATIVE->MakeWire(outerEdge);
				if (outerWire.IsNull())
					throw RaiseGeometryFactoryException("Profile wire could not be built", ifcCircleHollowProfileDef);
				TopoDS_Face face;
				if (ifcCircleHollowProfileDef->WallThickness <= 0)
				{
					LogInformation(ifcCircleHollowProfileDef, "Circle hollow profile has a wall thickness of less than zero");
					face = EXEC_NATIVE->MakeFace(outerWire);
				}
				else
				{
					auto innerEdge = EDGE_FACTORY->BuildCircle(ifcCircleHollowProfileDef->Radius - ifcCircleHollowProfileDef->WallThickness, gpax2); //throws an exception
					auto innerWire = EXEC_NATIVE->MakeWire(innerEdge);
					innerWire.Reverse();
					face = EXEC_NATIVE->MakeFace(outerWire, innerWire, ModelGeometryService->Precision);
				}
				if (face.IsNull())
					throw RaiseGeometryFactoryException("Failed to create profile face", ifcCircleHollowProfileDef);
				else
					return face;
			}

			TopoDS_Face ProfileFactory::BuildProfileFace(IIfcDerivedProfileDef^ derivedProfileDef)
			{

				TopoDS_Face face;
				std::optional<TopoDS_Shape> parentCached = GetCache()->GetShape(derivedProfileDef->ParentProfile->EntityLabel);
				if (parentCached.has_value()) {
					face = TopoDS::Face(TopoDS_Shape(parentCached.value()));
				}
				else {
					face = BuildProfileFace(derivedProfileDef->ParentProfile); //throws an exception	
					GetCache()->InsertShape(derivedProfileDef->ParentProfile->EntityLabel, face);
				}

				if (face.IsNull())
					throw RaiseGeometryFactoryException("Profile face could not be built", derivedProfileDef);
				auto nonUniform2d = dynamic_cast<IIfcCartesianTransformationOperator2DnonUniform^>(derivedProfileDef->Operator);
				if (nonUniform2d != nullptr)
				{
					gp_GTrsf transform = GEOMETRY_FACTORY->ToGTrsf(nonUniform2d);
					BRepBuilderAPI_GTransform aBrepTrsf(face, transform);
					if (!aBrepTrsf.IsDone())
						throw RaiseGeometryFactoryException("Profile wire could not be transformed", derivedProfileDef);
					face = TopoDS::Face(aBrepTrsf.Shape());
				}
				else
				{
					gp_Trsf transform = gp_Trsf(GEOMETRY_FACTORY->ToTrsf2d(derivedProfileDef->Operator));
					BRepBuilderAPI_Transform aBrepTrsf(face, transform);
					if (!aBrepTrsf.IsDone())
						throw RaiseGeometryFactoryException("Profile wire could not be transformed", derivedProfileDef);
					face = TopoDS::Face(aBrepTrsf.Shape());
				}
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

			TopoDS_Face ProfileFactory::BuildProfileFace(IIfcRectangleHollowProfileDef^ ifcRectangleHollowProfileDef)
			{
				if (ifcRectangleHollowProfileDef->WallThickness <= 0)
					throw RaiseGeometryFactoryException("Wall thickness of a rectangle hollow profile must be greater than 0");
				TopLoc_Location location;
				if (ifcRectangleHollowProfileDef->Position != nullptr)
					location = GEOMETRY_FACTORY->BuildAxis2PlacementLocation(ifcRectangleHollowProfileDef->Position);

				TopoDS_Face face = EXEC_NATIVE->BuildRectangleHollowProfileDef(location,
					ifcRectangleHollowProfileDef->XDim,
					ifcRectangleHollowProfileDef->YDim,
					ifcRectangleHollowProfileDef->WallThickness,
					ifcRectangleHollowProfileDef->OuterFilletRadius.HasValue ? (double)(ifcRectangleHollowProfileDef->OuterFilletRadius.Value) : -1.0,
					ifcRectangleHollowProfileDef->InnerFilletRadius.HasValue ? (double)(ifcRectangleHollowProfileDef->InnerFilletRadius.Value) : -1.0,
					ModelGeometryService->Precision);
				if (face.IsNull())
					throw RaiseGeometryFactoryException("Failed to create profile face", ifcRectangleHollowProfileDef);
				else
					return face;
			}

			TopoDS_Face ProfileFactory::BuildProfileFace(IIfcRoundedRectangleProfileDef^ ifcRoundedRectangleProfileDef)
			{
				auto wire = BuildProfileWire(ifcRoundedRectangleProfileDef);
				if (wire.IsNull())
					throw RaiseGeometryFactoryException("Failed to create profile wire", ifcRoundedRectangleProfileDef);
				TopoDS_Face face = EXEC_NATIVE->MakeFace(wire);
				if (face.IsNull())
					throw RaiseGeometryFactoryException("Failed to create profile face", ifcRoundedRectangleProfileDef);
				else
					return face;
			}
			TopoDS_Face ProfileFactory::BuildProfileFace(IIfcTrapeziumProfileDef^ trapeziumProfile)
			{
				auto wire = BuildProfileWire(trapeziumProfile);
				if (wire.IsNull())
					throw RaiseGeometryFactoryException("Failed to create profile wire", trapeziumProfile);
				TopoDS_Face face = EXEC_NATIVE->MakeFace(wire);
				if (face.IsNull())
					throw RaiseGeometryFactoryException("Failed to create profile face", trapeziumProfile);
				else
					return face;
			}
			TopoDS_Face ProfileFactory::BuildProfileFace(IIfcLShapeProfileDef^ ifcLShapeProfileDef)
			{
				auto wire = BuildProfileWire(ifcLShapeProfileDef);
				if (wire.IsNull())
					throw RaiseGeometryFactoryException("Failed to create profile wire", ifcLShapeProfileDef);
				TopoDS_Face face = EXEC_NATIVE->MakeFace(wire);
				if (face.IsNull())
					throw RaiseGeometryFactoryException("Failed to create profile face", ifcLShapeProfileDef);
				else
					return face;
			}
			TopoDS_Face ProfileFactory::BuildProfileFace(IIfcMirroredProfileDef^ mirroredProfile)
			{
				auto face = BuildProfileFace(mirroredProfile->ParentProfile);
				if (face.IsNull())
					throw RaiseGeometryFactoryException("Failed to create parent profile face", mirroredProfile);
				face = EXEC_NATIVE->BuildMirrored(face);
				if (face.IsNull())
					throw RaiseGeometryFactoryException("Failed to create mirrored profile face", mirroredProfile);
				return face;
			}
			TopoDS_Face ProfileFactory::BuildProfileFace(IIfcUShapeProfileDef^ ifcUShapeProfileDef)
			{
				auto wire = BuildProfileWire(ifcUShapeProfileDef);
				if (wire.IsNull())
					throw RaiseGeometryFactoryException("Failed to create profile wire", ifcUShapeProfileDef);
				TopoDS_Face face = EXEC_NATIVE->MakeFace(wire);
				if (face.IsNull())
					throw RaiseGeometryFactoryException("Failed to create profile face", ifcUShapeProfileDef);
				else
					return face;
			}
			TopoDS_Face ProfileFactory::BuildProfileFace(IIfcEllipseProfileDef^ ifcEllipseProfileDef)
			{
				auto wire = BuildProfileWire(ifcEllipseProfileDef);
				if (wire.IsNull())
					throw RaiseGeometryFactoryException("Failed to create profile wire", ifcEllipseProfileDef);
				TopoDS_Face face = EXEC_NATIVE->MakeFace(wire);
				if (face.IsNull())
					throw RaiseGeometryFactoryException("Failed to create profile face", ifcEllipseProfileDef);
				else
					return face;
			}

			TopoDS_Face ProfileFactory::BuildProfileFace(IIfcIShapeProfileDef^ ifcIShapeProfileDef)
			{
				auto wire = BuildProfileWire(ifcIShapeProfileDef);
				if (wire.IsNull())
					throw RaiseGeometryFactoryException("Failed to create profile wire", ifcIShapeProfileDef);
				TopoDS_Face face = EXEC_NATIVE->MakeFace(wire);
				if (face.IsNull())
					throw RaiseGeometryFactoryException("Failed to create profile face", ifcIShapeProfileDef);
				else
					return face;
			}

			TopoDS_Face ProfileFactory::BuildProfileFace(IIfcZShapeProfileDef^ ifcZShapeProfileDef)
			{
				auto wire = BuildProfileWire(ifcZShapeProfileDef);
				if (wire.IsNull())
					throw RaiseGeometryFactoryException("Failed to create profile wire", ifcZShapeProfileDef);
				TopoDS_Face face = EXEC_NATIVE->MakeFace(wire);
				if (face.IsNull())
					throw RaiseGeometryFactoryException("Failed to create profile face", ifcZShapeProfileDef);
				else
					return face;
			}
			TopoDS_Face ProfileFactory::BuildProfileFace(IIfcCShapeProfileDef^ ifcCShapeProfileDef)
			{
				auto wire = BuildProfileWire(ifcCShapeProfileDef);
				if (wire.IsNull())
					throw RaiseGeometryFactoryException("Failed to create profile wire", ifcCShapeProfileDef);
				TopoDS_Face face = EXEC_NATIVE->MakeFace(wire);
				if (face.IsNull())
					throw RaiseGeometryFactoryException("Failed to create profile face", ifcCShapeProfileDef);
				else
					return face;
			}
			TopoDS_Face ProfileFactory::BuildProfileFace(IIfcTShapeProfileDef^ ifcTShapeProfileDef)
			{
				auto wire = BuildProfileWire(ifcTShapeProfileDef);
				if (wire.IsNull())
					throw RaiseGeometryFactoryException("Failed to create profile wire", ifcTShapeProfileDef);
				TopoDS_Face face = EXEC_NATIVE->MakeFace(wire);
				if (face.IsNull())
					throw RaiseGeometryFactoryException("Failed to create profile face", ifcTShapeProfileDef);
				else
					return face;
			}
			TopoDS_Face ProfileFactory::BuildProfileFace(IIfcArbitraryOpenProfileDef^ ifcArbitraryOpenProfileDef)
			{
				throw RaiseGeometryFactoryException("Cannot build a face from an IfcArbitraryOpenProfileDef", ifcArbitraryOpenProfileDef);
			}
			TopoDS_Face ProfileFactory::BuildProfileFace(IIfcCenterLineProfileDef^ ifcCenterLineProfileDef)
			{
				auto wire = BuildProfileWire(ifcCenterLineProfileDef);
				if (wire.IsNull())
					throw RaiseGeometryFactoryException("Error building centre line Curve as a Wire", ifcCenterLineProfileDef);
				auto face = EXEC_NATIVE->MakeFace(wire);
				if (face.IsNull())
					throw RaiseGeometryFactoryException("Error building centre line Curve as a Face", ifcCenterLineProfileDef);
				return face;
			}
			TopoDS_Face ProfileFactory::BuildProfileFace(double x, double y, double tolerance, bool centre)
			{
				throw gcnew NotImplementedException("Failed to create Profile");

			}
		}
	}
}