#include "CurveFactory.h"
#include "GeometryFactory.h"
#include <GeomLib_Tool.hxx>

#include "../BRep/XLine.h"
#include "../BRep/XLine2d.h"
#include "../BRep/XCircle.h"
#include "../BRep/XCircle2d.h"
#include "../BRep/XEllipse.h"
#include "../BRep/XEllipse2d.h"
#include "../BRep/XTrimmedCurve.h"
#include "../BRep/XTrimmedCurve2d.h"
#include "../BRep/XBSplineCurve.h"

/*
The approach of the curve factory is to build all curves as IXCurve using the build method.
This will ensure correct dimensionality of the curves is maintained
Most curve types can have a 2D or a 3D variant, Ifc hides this in the Dim method.
Any 3D shape can be built from a 2D definition with the Z coordinate set to Zero
It is nor permitted to create 2D shapes from 3D definitions and an exception is thrown.
Managed code is used to navigate the definitions and provide the framework for the unmanaged code to build the native curves
All validation is done is managed code
All operations where an OCC exception will be thrown are implemented in unmanaged code. (NCurveFactory)
These exceptions are caught and logged in the managed code
Unmanaged build methods reurn a null handle to the specified geometry type when a critical or error exception has been thrown
*/
using namespace System;
using namespace Xbim::Geometry::Exceptions;
using namespace Xbim::Ifc4::MeasureResource;
using namespace Xbim::Geometry::BRep;
using namespace Xbim::Common::Metadata;
using namespace System::Linq;


namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{

			IXCurve^ CurveFactory::Build(IIfcCurve^ curve)
			{
				//IDisposable^ scope = Logger->BeginScope(String::Format("Building #{0}={1}", curve->EntityLabel, curve->GetType()->Name));
				int dim = (int)curve->Dim;

				XCurveType curveType;

				if (dim == 2)
				{
					Handle(Geom2d_Curve) hCurve2d = BuildCurve2d(curve, curveType); //this will throw an exception if it fails				
					return BuildXCurve(hCurve2d, curveType);
				}
				else
				{
					Handle(Geom_Curve) hCurve = BuildCurve3d(curve, curveType);//this will throw an exception if it fails				
					return BuildXCurve(hCurve, curveType);
				}
			}

#pragma region 2d Curve builders



			Handle(Geom2d_Curve) CurveFactory::BuildCurve2d(IIfcCurve^ curve, XCurveType% curveType)
			{
				if (!Enum::TryParse<XCurveType>(curve->ExpressType->ExpressName, curveType))
					RaiseGeometryFactoryException("Unsupported curve type: " + curve->ExpressType->ExpressName, curve);

				switch (curveType)
				{
					/*case XCurveType::BoundaryCurve:
						return  Build2d(static_cast<IIfcBoundedCurve^>(curve));
					case XCurveType::BSplineCurveWithKnots:
						return Build2d(static_cast<IIfcBSplineCurveWithKnots^>(curve));*/
				case XCurveType::IfcCircle:
					return BuildCurve2d(static_cast<IIfcCircle^>(curve));
					/*	case XCurveType::CompositeCurve:
							return Build2d(static_cast<IIfcCompositeCurve^>(curve)));
						case XCurveType::CompositeCurveOnSurface:
							return Build2d(static_cast<IIfcCompositeCurveOnSurface^>(curve)));*/
				case XCurveType::IfcEllipse:
					return BuildCurve2d(static_cast<IIfcEllipse^>(curve));
					/*case XCurveType::IndexedPolyCurve:
						return Build2d(static_cast<IIfcIndexedPolyCurve^>(curve)));*/
				case XCurveType::IfcLine:
					return BuildCurve2d(static_cast<IIfcLine^>(curve));
					/*case XCurveType::OffsetCurve2D:
						return Build2d(static_cast<IIfcOffsetCurve2D^>(curve));
					case XCurveType::Pcurve:
						return Build2d(static_cast<IIfcPcurve^>(curve)) ;
					case XCurveType::Polyline:
						return Build2d(static_cast<IIfcPolyline^>(curve));
					case XCurveType::RationalBSplineCurveWithKnots:
						return Build2d(static_cast<IIfcRationalBSplineCurveWithKnots^>(curve));
					case XCurveType::SurfaceCurve:
						return Build2d(static_cast<IIfcSurfaceCurve^>(curve));*/
				case XCurveType::IfcTrimmedCurve:
					return BuildCurve2d(static_cast<IIfcTrimmedCurve^>(curve));
				default:
					RaiseGeometryFactoryException("Unsupported 2d curve type");
				}

			}

			Handle(Geom2d_Circle) CurveFactory::BuildCurve2d(IIfcCircle^ ifcCircle)
			{
				if (ifcCircle->Radius <= 0) 
					RaiseGeometryFactoryException("Circle radius cannot be <= 0.", ifcCircle);
				if (2 != (int)ifcCircle->Dim) 
					RaiseGeometryFactoryException("Cannot build a 2D circle from a 3D circle", ifcCircle);
				IIfcAxis2Placement2D^ axis2d = dynamic_cast<IIfcAxis2Placement2D^>(ifcCircle->Position);
				if (axis2d == nullptr) 
					RaiseGeometryFactoryException("Cannot build a 2D curve with 3D placement", ifcCircle->Position);
				gp_Ax22d pos = GEOMETRY_FACTORY->BuildAxis2Placement2d(axis2d); //this will raise an exception if it fails
				Handle(Geom2d_Circle) circle2d = EXEC_NATIVE->BuildCircle2d(pos, ifcCircle->Radius);
				if (circle2d.IsNull())
					RaiseGeometryFactoryException("Cannot build 2D circle, see logs", ifcCircle);
				return circle2d;
			}

			Handle(Geom2d_LineWithMagnitude) CurveFactory::BuildCurve2d(IIfcLine^ ifcLine)
			{
				if (2 != (int)ifcLine->Dim)
					RaiseGeometryFactoryException("Cannot build a 2D curve from a 3D curve", ifcLine);
				gp_Pnt2d origin = GEOMETRY_FACTORY->BuildPoint2d(ifcLine->Pnt);
				gp_Dir2d direction = GEOMETRY_FACTORY->BuildDirection2d(ifcLine->Dir->Orientation);
				Handle(Geom2d_LineWithMagnitude) line = EXEC_NATIVE->BuildLine2d(origin, direction, ifcLine->Dir->Magnitude);
				if (line.IsNull())
					RaiseGeometryFactoryException("Cannot build 2D line, see logs", ifcLine);
				return line;
			}

#pragma endregion


			Handle(Geom_Curve) CurveFactory::BuildCurve3d(IIfcCurve^ curve, XCurveType% curveType)
			{
				if (!Enum::TryParse<XCurveType>(curve->ExpressType->ExpressName, curveType))
					throw gcnew XbimGEOMETRY_FACTORYException("Unsupported curve type: " + curve->ExpressType->ExpressName);

				switch (curveType)
				{
					/*case XCurveType::IfcBoundaryCurve:
						return Build3d(static_cast<IIfcBoundaryCurve^>(curve));
					case XCurveType::BSplineCurveWithKnots:
						return Build3d(static_cast<IIfcBSplineCurveWithKnots^>(curve));*/
				case XCurveType::IfcCircle:
					return BuildGeom3d(static_cast<IIfcCircle^>(curve));
				case XCurveType::IfcCompositeCurve:
					return BuildGeom3d(static_cast<IIfcCompositeCurve^>(curve));
					/*	case XCurveType::CompositeCurveOnSurface:
							return Build3d(static_cast<IIfcCompositeCurveOnSurface^>(curve));*/
				case XCurveType::IfcEllipse:
					return BuildGeom3d(static_cast<IIfcEllipse^>(curve));
					/*	case XCurveType::IndexedPolyCurve:
							return Build3d(static_cast<IIfcIndexedPolyCurve^>(curve));*/
				case XCurveType::IfcLine:
					return BuildGeom3d(static_cast<IIfcLine^>(curve));
					/*
					case XCurveType::OffsetCurve3D:
						return Build2d(static_cast<IIfcOffsetCurve3D^>(curve));
					case XCurveType::Pcurve:
						return Build3d(static_cast<IIfcPcurve^>(curve));*/
				case XCurveType::IfcPolyline:
					return BuildGeom3d(static_cast<IIfcPolyline^>(curve));
					/*case XCurveType::RationalBSplineCurveWithKnots:
						return Build3d(static_cast<IIfcRationalBSplineCurveWithKnots^>(curve));
					case XCurveType::SurfaceCurve:
						return Build3d(static_cast<IIfcSurfaceCurve^>(curve));*/
				case XCurveType::IfcTrimmedCurve:
					return BuildGeom3d(static_cast<IIfcTrimmedCurve^>(curve));

				default:
					break;
				}
				throw gcnew XbimGEOMETRY_FACTORYException("Not implemented. Curve type: " + curveType.ToString());
			}


			Handle(Geom_Curve) CurveFactory::BuildDirectrix(IIfcCurve^ curve, double startParam, double endParam, XCurveType% curveType)
			{
				double sameParams = Math::Abs(endParam - startParam) < ModelService->Precision;
				if (sameParams || ((startParam == -1 || endParam == -1) && !IsBoundedCurve(curve)))
					throw gcnew XbimGEOMETRY_FACTORYException("DirectrixBounded: If the values for StartParam or EndParam are omited, then the Directrix has to be a bounded or closed curve.");
				if (3 != (int)curve->Dim)
					throw gcnew XbimGEOMETRY_FACTORYException("DirectrixDim: The Directrix shall be a curve in three dimensional space.");

				Handle(Geom_Curve) geomCurve = BuildGeom3d(curve, curveType);

				if (geomCurve.IsNull())
					throw gcnew XbimGEOMETRY_FACTORYException("Directrix is invalid");
				//trimming
				if (startParam != -1 || endParam != -1)
				{
					if (startParam == -1) startParam = geomCurve->FirstParameter();
					if (endParam == -1) endParam = geomCurve->LastParameter();
					Handle(Geom_Curve)  geomCurveTrimmed = Ptr()->TrimDirectrix(geomCurve, startParam, endParam, ModelService->Precision);
					if (geomCurve.IsNull())
						throw gcnew XbimGEOMETRY_FACTORYException("Directrix could not be trimmed");
					return geomCurveTrimmed;
				}
				return geomCurve;

			}


			IXCurve^ CurveFactory::BuildDirectrix(IIfcCurve^ curve, Nullable<double> startParam, Nullable<double> endParam)
			{
				if (!startParam.HasValue) startParam = -1;
				if (!endParam.HasValue) endParam = -1;
				return BuildXDirectrix(curve, startParam.Value, endParam.Value);
			}

			IXCurve^ CurveFactory::BuildXCurve(Handle(Geom_Curve) curve, XCurveType curveType)
			{
				switch (curveType)
				{
					/*case XCurveType::BoundaryCurve:
						return Build3d(dynamic_cast<IIfcBoundedCurve^>(curve));
					case XCurveType::BSplineCurveWithKnots:
						return Build3d(dynamic_cast<IIfcBSplineCurveWithKnots^>(curve));*/
				case XCurveType::IfcCircle:
					return gcnew XCircle(Handle(Geom_Circle)::DownCast(curve));
				case XCurveType::IfcCompositeCurve:
					return gcnew XBSplineCurve(Handle(Geom_BSplineCurve)::DownCast(curve));
					/*case XCurveType::CompositeCurveOnSurface:
						return Build3d(dynamic_cast<IIfcCompositeCurveOnSurface^>(curve));*/
				case XCurveType::IfcEllipse:
					return gcnew XEllipse(Handle(Geom_Ellipse)::DownCast(curve));
					/*	case XCurveType::IndexedPolyCurve:
							return Build3d(dynamic_cast<IIfcIndexedPolyCurve^>(curve));*/
				case XCurveType::IfcLine:
					return gcnew Xbim::Geometry::BRep::XLine(Handle(Geom_LineWithMagnitude)::DownCast(curve));
					/*
					case XCurveType::OffsetCurve3D:
						return Build2d(dynamic_cast<IIfcOffsetCurve3D^>(curve));
					case XCurveType::Pcurve:
						return Build3d(dynamic_cast<IIfcPcurve^>(curve));
					case XCurveType::Polyline:
						return Build3d(dynamic_cast<IIfcPolyline^>(curve));
					case XCurveType::RationalBSplineCurveWithKnots:
						return Build3d(dynamic_cast<IIfcRationalBSplineCurveWithKnots^>(curve));
					case XCurveType::SurfaceCurve:
						return Build3d(dynamic_cast<IIfcSurfaceCurve^>(curve));*/
				case XCurveType::IfcTrimmedCurve:
					return gcnew XTrimmedCurve(Handle(Geom_TrimmedCurve)::DownCast(curve));
					break;
				default:
					throw gcnew XbimGEOMETRY_FACTORYException("Unsupported curve type");
				}
				throw gcnew XbimGEOMETRY_FACTORYException("Unsupported curve type");
			}

			IXCurve^ CurveFactory::BuildXCurve(Handle(Geom2d_Curve) curve, XCurveType curveType)
			{
				switch (curveType)
				{
					/*case XCurveType::BoundaryCurve:
						return  Build2d(dynamic_cast<IIfcBoundedCurve^>(curve));
					case XCurveType::BSplineCurveWithKnots:
						return Build2d(dynamic_cast<IIfcBSplineCurveWithKnots^>(curve));*/
				case XCurveType::IfcCircle:
					return gcnew XCircle2d(Handle(Geom2d_Circle)::DownCast(curve));
					/*	case XCurveType::CompositeCurve:
							return Build2d(dynamic_cast<IIfcCompositeCurve^>(curve)));
						case XCurveType::CompositeCurveOnSurface:
							return Build2d(dynamic_cast<IIfcCompositeCurveOnSurface^>(curve)));*/
				case XCurveType::IfcEllipse:
					return gcnew XEllipse2d(Handle(Geom2d_Ellipse)::DownCast(curve));
					/*	case XCurveType::IndexedPolyCurve:
							return Build2d(dynamic_cast<IIfcIndexedPolyCurve^>(curve)));*/
				case XCurveType::IfcLine:
					return gcnew XLine2d(Handle(Geom2d_LineWithMagnitude)::DownCast(curve));
					/*case XCurveType::OffsetCurve2D:
						return Build2d(dynamic_cast<IIfcOffsetCurve2D^>(curve));
					case XCurveType::Pcurve:
						return Build2d(dynamic_cast<IIfcPcurve^>(curve)) ;
					case XCurveType::Polyline:
						return Build2d(dynamic_cast<IIfcPolyline^>(curve));
					case XCurveType::RationalBSplineCurveWithKnots:
						return Build2d(dynamic_cast<IIfcRationalBSplineCurveWithKnots^>(curve));
					case XCurveType::SurfaceCurve:
						return Build2d(dynamic_cast<IIfcSurfaceCurve^>(curve));*/
				case XCurveType::IfcTrimmedCurve:
					return gcnew XTrimmedCurve2d(Handle(Geom2d_TrimmedCurve)::DownCast(curve));
					break;
				default:
					throw gcnew XbimGEOMETRY_FACTORYException("Unsupported 2d curve type");
				}
				throw gcnew XbimGEOMETRY_FACTORYException("Unsupported 2d curve type");
			}

			IXCurve^ CurveFactory::BuildXDirectrix(IIfcCurve^ curve, double startParam, double endParam)
			{
				XCurveType curveType;
				Handle(Geom_Curve) directix = BuildDirectrix(curve, startParam, endParam, curveType);
				switch (curveType)
				{
					/*case XCurveType::BoundaryCurve:
						return Build3d(dynamic_cast<IIfcBoundedCurve^>(curve));
					case XCurveType::BSplineCurveWithKnots:
						return Build3d(dynamic_cast<IIfcBSplineCurveWithKnots^>(curve));*/
				case XCurveType::IfcCircle:
					return gcnew XCircle(Handle(Geom_Circle)::DownCast(directix));
				case XCurveType::IfcCompositeCurve:
					return gcnew XBSplineCurve(Handle(Geom_BSplineCurve)::DownCast(directix));
					/*case XCurveType::CompositeCurveOnSurface:
						return Build3d(dynamic_cast<IIfcCompositeCurveOnSurface^>(curve));*/
				case XCurveType::IfcEllipse:
					return gcnew XEllipse(Handle(Geom_Ellipse)::DownCast(directix));
					/*	case XCurveType::IndexedPolyCurve:
							return Build3d(dynamic_cast<IIfcIndexedPolyCurve^>(directix));*/
				case XCurveType::IfcLine:
					return gcnew Xbim::Geometry::BRep::XLine(Handle(Geom_LineWithMagnitude)::DownCast(directix));
					/*
					case XCurveType::OffsetCurve3D:
						return Build2d(dynamic_cast<IIfcOffsetCurve3D^>(directix));
					case XCurveType::Pcurve:
						return Build3d(dynamic_cast<IIfcPcurve^>(directix));
					case XCurveType::Polyline:
						return Build3d(dynamic_cast<IIfcPolyline^>(directix));
					case XCurveType::RationalBSplineCurveWithKnots:
						return Build3d(dynamic_cast<IIfcRationalBSplineCurveWithKnots^>(directix));
					case XCurveType::SurfaceCurve:
						return Build3d(dynamic_cast<IIfcSurfaceCurve^>(directix));*/
				case XCurveType::IfcTrimmedCurve:
					return gcnew XTrimmedCurve(Handle(Geom_TrimmedCurve)::DownCast(directix));
					break;
				default:
					throw gcnew XbimGEOMETRY_FACTORYException("Unsupported curve type");
				}
				throw gcnew XbimGEOMETRY_FACTORYException("Unsupported curve type");
			}

			Handle(Geom_LineWithMagnitude) CurveFactory::BuildGeom3d(IIfcLine^ ifcLine)
			{
				gp_Pnt origin = GEOMETRY_FACTORY->BuildPoint(ifcLine->Pnt);
				gp_Dir direction = GEOMETRY_FACTORY->BuildDirection(ifcLine->Dir->Orientation);
				return Ptr()->BuildLine3d(origin, direction, ifcLine->Dir->Magnitude);

			}



			Handle(Geom_Circle) CurveFactory::BuildGeom3d(IIfcCircle^ ifcCircle)
			{
				if (ifcCircle->Radius <= 0) throw gcnew XbimGEOMETRY_FACTORYException("Circle radius cannot be <= 0.");
				IIfcAxis2Placement3D^ axis3d = dynamic_cast<IIfcAxis2Placement3D^>(ifcCircle->Position);
				if (axis3d == nullptr) throw gcnew XbimGEOMETRY_FACTORYException("Cannot build a 3D curve with 2D placement");
				gp_Ax2 pos = GEOMETRY_FACTORY->BuildAxis2Placement(axis3d);
				return Ptr()->BuildCircle3d(pos, ifcCircle->Radius);

			}

			

			Handle(Geom_Ellipse) CurveFactory::BuildGeom3d(IIfcEllipse^ ifcEllipse)
			{
				IIfcAxis2Placement3D^ axis3d = dynamic_cast<IIfcAxis2Placement3D^>(ifcEllipse->Position);
				if (axis3d == nullptr) throw gcnew XbimGEOMETRY_FACTORYException("Cannot build a 3D curve with 2D placement");

				//SELF\IfcConic.Position.Position.P[1] is the direction of the SemiAxis1. 
				gp_Ax2 pos = GEOMETRY_FACTORY->BuildAxis2Placement(axis3d);
				return Ptr()->BuildEllipse3d(pos, ifcEllipse->SemiAxis1, ifcEllipse->SemiAxis2);
			}

			Handle(Geom2d_Ellipse) CurveFactory::BuildCurve2d(IIfcEllipse^ ifcEllipse)
			{
				if (2 != (int)ifcEllipse->Dim) throw gcnew XbimGEOMETRY_FACTORYException("Cannot build a 2D curve from a 3D curve");
				IIfcAxis2Placement2D^ axis2d = dynamic_cast<IIfcAxis2Placement2D^>(ifcEllipse->Position);
				if (axis2d == nullptr) throw gcnew XbimGEOMETRY_FACTORYException("Cannot build a 2D curve with 3D placement");
				gp_Ax22d pos = GEOMETRY_FACTORY->BuildAxis2Placement2d(axis2d);
				return Ptr()->BuildEllipse2d(pos, ifcEllipse->SemiAxis1, ifcEllipse->SemiAxis2);
			}

			Handle(Geom_TrimmedCurve) CurveFactory::BuildGeom3d(IIfcTrimmedCurve^ ifcTrimmedCurve)
			{
				try
				{
					//Validation
					if (dynamic_cast<IIfcBoundedCurve^>(ifcTrimmedCurve->BasisCurve))
						throw gcnew XbimGEOMETRY_FACTORYException("Ifc Formal Proposition: NoTrimOfBoundedCurves. Already bounded curves shall not be trimmed.");
					XCurveType curveType;
					Handle(Geom_Curve) basisCurve = BuildGeom3d(ifcTrimmedCurve->BasisCurve, curveType);
					if (!basisCurve.IsNull())
					{
						bool isConic = (dynamic_cast<IIfcConic^>(ifcTrimmedCurve->BasisCurve) != nullptr);
						bool isLine = (dynamic_cast<IIfcLine^>(ifcTrimmedCurve->BasisCurve) != nullptr);
						bool isEllipse = (dynamic_cast<IIfcEllipse^>(ifcTrimmedCurve->BasisCurve) != nullptr);
						bool sense = ifcTrimmedCurve->SenseAgreement;
						//get the parametric values
						IfcTrimmingPreference trimPref = ifcTrimmedCurve->MasterRepresentation;

						bool trim_cartesian = (ifcTrimmedCurve->MasterRepresentation == IfcTrimmingPreference::CARTESIAN);

						double u1 = double::NegativeInfinity, u2 = double::PositiveInfinity;
						IIfcCartesianPoint^ cp1 = nullptr;
						IIfcCartesianPoint^ cp2 = nullptr;

						for each (IIfcTrimmingSelect ^ trim in ifcTrimmedCurve->Trim1)
						{
							if (dynamic_cast<IIfcCartesianPoint^>(trim))cp1 = (IIfcCartesianPoint^)trim;
							else u1 = (double)(IfcParameterValue)trim; //its parametric	
						}
						for each (IIfcTrimmingSelect ^ trim in ifcTrimmedCurve->Trim2)
						{
							if (dynamic_cast<IIfcCartesianPoint^>(trim))cp2 = (IIfcCartesianPoint^)trim;
							else u2 = (double)(IfcParameterValue)trim; //its parametric	
						}

						if ((trim_cartesian && cp1 != nullptr && cp2 != nullptr) ||
							(cp1 != nullptr && cp2 != nullptr &&
								(double::IsNegativeInfinity(u1) || double::IsPositiveInfinity(u2)))) //we want cartesian and we have both or we don't have both parameters but have cartesians
						{
							gp_Pnt p1 = XbimConvert::GetPoint3d(cp1);
							gp_Pnt p2 = XbimConvert::GetPoint3d(cp2);
							if (!GeomLib_Tool::Parameter(basisCurve, p1, ModelService->MinimumGap, u1))
								throw gcnew XbimGEOMETRY_FACTORYException("Trim Point1 is not on the basis curve");
							if (!GeomLib_Tool::Parameter(basisCurve, p2, ModelService->MinimumGap, u2))
								throw gcnew XbimGEOMETRY_FACTORYException("Trim Point2 is not on the basis curve");
						}
						else if (double::IsNegativeInfinity(u1) || double::IsPositiveInfinity(u2)) //non-compliant
							throw gcnew XbimGEOMETRY_FACTORYException("Ifc Formal Proposition: TrimValuesConsistent. Either a single value is specified for Trim, or the two trimming values are of different type (point and parameter)");
						else //we prefer to use parameters but need to adjust
						{
							if (isConic)
							{
								u1 *= ModelService->RadianFactor; //correct to radians
								u2 *= ModelService->RadianFactor; //correct to radians

							}
						}

						if (double::IsNegativeInfinity(u1) || double::IsPositiveInfinity(u2)) //sanity check in case the logic has missed a situtation
							throw gcnew XbimGEOMETRY_FACTORYException("Error converting Ifc Trim Points");

						if (Math::Abs(u1 - u2) < ModelService->Precision) //if the parameters are the same trimming will fail if not a conic curve
						{
							if (isConic) return Ptr()->BuildTrimmedCurve3d(basisCurve, 0, Math::PI * 2, true); //return a full circle
							throw gcnew XbimGEOMETRY_FACTORYException("Parametric Trim Points are equal and will result in an empty curve");
						}
						else
							return Ptr()->BuildTrimmedCurve3d(basisCurve, u1, u2, sense);
					}
					else
						throw gcnew XbimGEOMETRY_FACTORYException("Failed to build Trimmed Basis Curve");
				}
				catch (Exception^ ex)
				{
					LoggerService->LogInformation(String::Format("Trimmed Curve #{0} failed: {1}", ifcTrimmedCurve->EntityLabel, ex->Message));
					return nullptr;
				}
			}

			Handle(Geom2d_TrimmedCurve) CurveFactory::BuildCurve2d(IIfcTrimmedCurve^ ifcTrimmedCurve)
			{
				try
				{

					//Validation
					if (dynamic_cast<IIfcBoundedCurve^>(ifcTrimmedCurve->BasisCurve))
						throw gcnew XbimGEOMETRY_FACTORYException("Ifc Formal Proposition: NoTrimOfBoundedCurves. Already bounded curves shall not be trimmed.");
					XCurveType curveType;
					Handle(Geom2d_Curve) basisCurve = BuildCurve2d(ifcTrimmedCurve->BasisCurve, curveType);
					if (!basisCurve.IsNull())
					{
						bool isConic = (dynamic_cast<IIfcConic^>(ifcTrimmedCurve->BasisCurve) != nullptr);
						bool isLine = (dynamic_cast<IIfcLine^>(ifcTrimmedCurve->BasisCurve) != nullptr);
						bool isEllipse = (dynamic_cast<IIfcEllipse^>(ifcTrimmedCurve->BasisCurve) != nullptr);
						bool sense = ifcTrimmedCurve->SenseAgreement;
						//get the parametric values
						IfcTrimmingPreference trimPref = ifcTrimmedCurve->MasterRepresentation;

						bool trim_cartesian = (ifcTrimmedCurve->MasterRepresentation == IfcTrimmingPreference::CARTESIAN);

						double u1 = double::NegativeInfinity, u2 = double::PositiveInfinity;
						IIfcCartesianPoint^ cp1 = nullptr;
						IIfcCartesianPoint^ cp2 = nullptr;

						for each (IIfcTrimmingSelect ^ trim in ifcTrimmedCurve->Trim1)
						{
							if (dynamic_cast<IIfcCartesianPoint^>(trim))cp1 = (IIfcCartesianPoint^)trim;
							else u1 = (double)(IfcParameterValue)trim; //its parametric	
						}
						for each (IIfcTrimmingSelect ^ trim in ifcTrimmedCurve->Trim2)
						{
							if (dynamic_cast<IIfcCartesianPoint^>(trim))cp2 = (IIfcCartesianPoint^)trim;
							else u2 = (double)(IfcParameterValue)trim; //its parametric	
						}

						if ((trim_cartesian && cp1 != nullptr && cp2 != nullptr) ||
							(cp1 != nullptr && cp2 != nullptr &&
								(double::IsNegativeInfinity(u1) || double::IsPositiveInfinity(u2)))) //we want cartesian and we have both or we don't have both parameters but have cartesians
						{
							gp_Pnt2d p1 = XbimConvert::GetPoint2d(cp1);
							gp_Pnt2d p2 = XbimConvert::GetPoint2d(cp2);
							if (!GeomLib_Tool::Parameter(basisCurve, p1, ModelService->MinimumGap, u1))
								throw gcnew XbimGEOMETRY_FACTORYException("Trim Point1 is not on the basis curve");
							if (!GeomLib_Tool::Parameter(basisCurve, p2, ModelService->MinimumGap, u2))
								throw gcnew XbimGEOMETRY_FACTORYException("Trim Point2 is not on the basis curve");
						}
						else if (double::IsNegativeInfinity(u1) || double::IsPositiveInfinity(u2)) //non-compliant
							throw gcnew XbimGEOMETRY_FACTORYException("Ifc Formal Proposition: TrimValuesConsistent. Either a single value is specified for Trim, or the two trimming values are of different type (point and parameter)");
						else //we prefer to use parameters but need to adjust
						{
							if (isConic)
							{
								u1 *= ModelService->RadianFactor; //correct to radians
								u2 *= ModelService->RadianFactor; //correct to radians
							}
						}
						if (double::IsNegativeInfinity(u1) || double::IsPositiveInfinity(u2)) //sanity check in case the logic has missed a situtation
							throw gcnew XbimGEOMETRY_FACTORYException("Error converting Ifc Trim Points");
						if (Math::Abs(u1 - u2) < ModelService->Precision) //if the parameters are the same trimming will fail if not a conic curve
						{
							if (isConic) return Ptr()->BuildTrimmedCurve2d(basisCurve, 0, Math::PI * 2, true); //return a full circle
							throw gcnew XbimGEOMETRY_FACTORYException("Parametric Trim Points are equal and will result in an empty curve");
						}
						else
							return Ptr()->BuildTrimmedCurve2d(basisCurve, u1, u2, sense);
					}
					else
						throw gcnew XbimGEOMETRY_FACTORYException("Failed to build Trimmed Basis Curve");
				}
				catch (Exception^ ex)
				{
					LoggerService->LogInformation(String::Format("Trimmed Curve #{0} failed: {1}", ifcTrimmedCurve->EntityLabel, ex->Message));
					return nullptr;
				}

			}

			Handle(Geom_Curve) CurveFactory::BuildGeom3d(IIfcPolyline^ ifcPolyline)
			{
				//validate
				int pointCount = ifcPolyline->Points->Count;
				if (pointCount < 2)
					throw gcnew XbimGEOMETRY_FACTORYException("IfcPolyline has less than 2 points. It cannot be built");
				if (pointCount == 2) //just build a line
				{
					gp_Pnt start = GEOMETRY_FACTORY->BuildPoint(ifcPolyline->Points[0]);
					gp_Pnt end = GEOMETRY_FACTORY->BuildPoint(ifcPolyline->Points[1]);
					if (start.IsEqual(end, ModelService->Precision))
						throw gcnew XbimGEOMETRY_FACTORYException(String::Format("IfcPolyline has only 2 identical points( #{0} and #{1}. It cannot be built", ifcPolyline->Points[0]->EntityLabel, ifcPolyline->Points[1]->EntityLabel));
					Handle(Geom_TrimmedCurve) lineSeg = Ptr()->BuildBoundedLine3d(start, end);
					if (lineSeg.IsNull())
						throw gcnew XbimGEOMETRY_FACTORYException("Invalid IfcPolyline definition");
					return lineSeg;
				}
				else
				{
					TColgp_Array1OfPnt points(1, pointCount);
					GEOMETRY_FACTORY->GetPolylinePoints(ifcPolyline, points);
					Handle(Geom_BSplineCurve) polyline = Ptr()->BuildPolyline(points, ModelService->Precision);
					if (polyline.IsNull())
						throw gcnew XbimGEOMETRY_FACTORYException("Failed to build IfcPolyline");
					return polyline;
				}

			}





			Handle(Geom_BSplineCurve) CurveFactory::BuildGeom3d(IIfcCompositeCurve^ ifcCompositeCurve)
			{
				XCurveType curveType;
				TColGeom_SequenceOfCurve segments;

				for each (IIfcCompositeCurveSegment ^ segment in ifcCompositeCurve->Segments)
				{
					if (!IsBoundedCurve(segment->ParentCurve))
						throw gcnew XbimGEOMETRY_FACTORYException("Composite curve is invalid, only curve segments that are bounded curves are permitted");
					Handle(Geom_Curve) hSegment = BuildGeom3d(segment->ParentCurve, curveType);
					Handle(Geom_BoundedCurve) boundedCurve = Handle(Geom_BoundedCurve)::DownCast(hSegment);
					if (boundedCurve.IsNull())
						gcnew XbimGEOMETRY_FACTORYException(String::Format("Compound curve segments must be bounded curves #{0}", segment->EntityLabel));
					if (hSegment.IsNull())
						throw gcnew XbimGEOMETRY_FACTORYException(String::Format("Composite curve is invalid, curve segment #{0} is null", segment->EntityLabel));
					segments.Append(boundedCurve);
				}
				Handle(Geom_BSplineCurve) bSpline = Ptr()->BuildCompositeCurve(segments, ModelService->MinimumGap); //use minimum gap for tolerance to avoid issues with cirves and line tolerance errors
				if (bSpline.IsNull())
					throw gcnew XbimGEOMETRY_FACTORYException(String::Format("Composite curve #{0} could not be built", ifcCompositeCurve->EntityLabel));
				else
					return bSpline;
			}


			bool CurveFactory::IsBoundedCurve(IIfcCurve^ curve)
			{
				if (dynamic_cast<IIfcLine^>(curve)) return false;
				if (dynamic_cast<IIfcOffsetCurve3D^>(curve)) return IsBoundedCurve((static_cast<IIfcOffsetCurve3D^>(curve))->BasisCurve);
				if (dynamic_cast<IIfcOffsetCurve2D^>(curve)) return IsBoundedCurve((static_cast<IIfcOffsetCurve2D^>(curve))->BasisCurve);
				if (dynamic_cast<IIfcPcurve^>(curve)) return false;
				if (dynamic_cast<IIfcSurfaceCurve^>(curve)) return false;
				return true;
			}
		}
	}
}
