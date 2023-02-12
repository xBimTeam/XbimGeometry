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

#pragma region Build wires from profiles

			IXWire^ ProfileFactory::BuildWire(IIfcProfileDef^ profileDef)
			{
				XProfileDefType profileType;

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
					/*case XProfileDefType::IfcCompositeProfileDef:
						return BuildProfileFace(static_cast<IIfcCompositeProfileDef^>(profileDef));
					case XProfileDefType::IfcDerivedProfileDef:
						return BuildProfileFace(static_cast<IIfcDerivedProfileDef^>(profileDef));
					case XProfileDefType::IfcMirroredProfileDef:
						return BuildProfileFace(static_cast<IIfcMirroredProfileDef^>(profileDef));
					case XProfileDefType::IfcAsymmetricIShapeProfileDef:
						return BuildProfileFace(static_cast<IIfcAsymmetricIShapeProfileDef^>(profileDef));
					case XProfileDefType::IfcCShapeProfileDef:
						return BuildProfileFace(static_cast<IIfcCShapeProfileDef^>(profileDef));*/
					case XProfileDefType::IfcCircleHollowProfileDef:
						throw RaiseGeometryFactoryException("A single wire cannot be built from an IfcCircleHollowProfileDef, use BuildFace", profileDef);
				/*	case XProfileDefType::IfcCircleProfileDef:
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
				if(wire.IsNull())
					throw RaiseGeometryFactoryException("ProfileDef could not be built as a wire", profileDef);
				return gcnew XWire(wire);
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
				TopLoc_Location location;
				if (rectangleProfile != nullptr)
					location = GEOMETRY_FACTORY->BuildAxis2PlacementLocation(rectangleProfile->Position);
				auto wire = EXEC_NATIVE->BuildRectangle(rectangleProfile->XDim, rectangleProfile->YDim, location);
				if (wire.IsNull())
					throw RaiseGeometryFactoryException("Profile wire cound not be built", rectangleProfile);
				else
					return wire;
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
				if(centrLineStart.Distance(centreLineEnd)< ModelGeometryService->Precision)
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
				if(wire.IsNull())
					throw RaiseGeometryFactoryException("IfcCenterLineProfileDef could not be built as a wire", ifcCenterLineProfileDef);
				return wire;
				
			}

#pragma endregion
			
			
			IXEdge^ ProfileFactory::BuildEdge(IIfcProfileDef^ profileDef)
			{
				XProfileDefType profileDefType;
				auto curve = BuildCurve(profileDef, profileDefType);
				if(curve.IsNull())
					throw RaiseGeometryFactoryException("Profile could not be built as a Curve");
				auto edge = EXEC_NATIVE->MakeEdge(curve);
				if (edge.IsNull())
					throw RaiseGeometryFactoryException("Profile could not be built as an Edge");
				return gcnew XEdge(edge);
			}

			IXCurve^ ProfileFactory::BuildCurve(IIfcProfileDef^ profileDef)
			{
				throw gcnew System::NotImplementedException();
				// TODO: insert return statement here
			}

			TopoDS_Face ProfileFactory::BuildProfileFace(const TopoDS_Wire& wire)
			{
				TopoDS_Face face = EXEC_NATIVE->MakeFace(wire);
				if (face.IsNull())
					throw RaiseGeometryFactoryException("Profile could not be built as a Face");
				return face;
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

				for each (IIfcCurve^ innerWire in arbitraryClosedProfileWithVoids->InnerCurves)
				{
					if (innerLoopIds->Add(innerWire->EntityLabel)) //only add a loop once
					{
						TopoDS_Wire wire = WIRE_FACTORY->BuildWire(innerWire, false); //throws exception
						innerLoops.Append(wire);
					}
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
					LogDebug(ifcCircleHollowProfileDef, "Circle hollow profile has a wall thickness of less than zero");
					face = EXEC_NATIVE->MakeFace(outerWire);
				}
				else
				{
					auto innerEdge = EDGE_FACTORY->BuildCircle(ifcCircleHollowProfileDef->Radius - ifcCircleHollowProfileDef->WallThickness, gpax2); //throws an exception
					auto innerWire = EXEC_NATIVE->MakeWire(innerEdge);
					innerWire.Reverse();
					face = EXEC_NATIVE->MakeFace(outerWire, innerWire);
				}
				if (face.IsNull())
					throw RaiseGeometryFactoryException("Failed to create profile face", ifcCircleHollowProfileDef);
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
					throw RaiseGeometryFactoryException("IfcArbitraryProfileDefWithVoids cannot be built as an edge, use BuildProfileFace", profileDef);
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
			
#pragma region Build Curves from Profile Definition



			IXCurve^ ProfileFactory::BuildCurve(IIfcProfileDef^ profileDef)
			{
				XProfileDefType profileType;
				auto curve =  BuildCurve(profileDef, profileType);
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
				if(2 == (int)ifcArbitraryClosedProfileDef->OuterCurve->Dim)
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
				if(ifcCenterLineProfileDef->Thickness<=0)
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
				if(bSpline.IsNull())
					throw RaiseGeometryFactoryException("IfcCenterLineProfileDef could not be built as a Curve", ifcCenterLineProfileDef);
				return GeomLib::To3d(gp_Ax2(), bSpline);
			}

#pragma endregion

		}
	}
}