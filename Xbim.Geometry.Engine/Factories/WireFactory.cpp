#include "WireFactory.h"
#include "CurveFactory.h"
#include "GeometryFactory.h"
#include "EdgeFactory.h"
#include "ProfileFactory.h"
#include <BRepBuilderAPI_MakeEdge2d.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <TopoDS.hxx>
#include <Geom_Plane.hxx>
#include <TColgp_SequenceOfPnt2d.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <map>
#include <TopTools_DataMapOfIntegerShape.hxx>
#include <Geom_Ellipse.hxx>
#include "../BRep/OccExtensions/KeyedPnt.h"
#include <TColGeom_SequenceOfCurve.hxx>
#include <TopExp_Explorer.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <TopExp.hxx>
#include <TColgp_SequenceOfPnt.hxx>
#include <ShapeAnalysis.hxx>
#include <ShapeAnalysis_Wire.hxx>
#include <TColGeom2d_SequenceOfCurve.hxx>

#include "../BRep/XWire.h"
#include <GeomLib_Tool.hxx>

using namespace System;
using namespace System::Linq;
using namespace Xbim::Geometry::BRep;
using namespace Xbim::Ifc4::MeasureResource;
namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{




			TopoDS_Wire WireFactory::BuildWire(const TopTools_SequenceOfShape& edgeList)
			{
				TopoDS_Wire wire = EXEC_NATIVE->BuildWire(edgeList);
				if (wire.IsNull())
					throw RaiseGeometryFactoryException("Edge list could not be built as a wire");
				return wire;
			}

			TopoDS_Wire WireFactory::BuildWire(IIfcCurve^ ifcCurve, bool asSingleEdge)
			{
				XCurveType curveType;
				if (!Enum::TryParse<XCurveType>(ifcCurve->ExpressType->ExpressName, curveType))
					throw RaiseGeometryFactoryException("Unsupported curve type", ifcCurve);

				switch (curveType)
				{
				case XCurveType::IfcBSplineCurveWithKnots:
					return BuildWire(static_cast<IIfcBSplineCurveWithKnots^>(ifcCurve), true);
				case XCurveType::IfcCircle:
					return BuildWire(static_cast<IIfcCircle^>(ifcCurve), asSingleEdge);
				case XCurveType::IfcCompositeCurve:
					return BuildWire(static_cast<IIfcCompositeCurve^>(ifcCurve), asSingleEdge);
					/*case XCurveType::IfcCompositeCurveOnSurface:
						return BuildWire2d(static_cast<IIfcCompositeCurveOnSurface^>(ifcCurve), asSingleEdge);*/
				case XCurveType::IfcEllipse:
					return BuildWire(static_cast<IIfcEllipse^>(ifcCurve), true);
				case XCurveType::IfcIndexedPolyCurve:
					return BuildWire(static_cast<IIfcIndexedPolyCurve^>(ifcCurve), asSingleEdge);
				case XCurveType::IfcLine:
					return BuildWire(static_cast<IIfcLine^>(ifcCurve), true);
				case XCurveType::IfcOffsetCurve2D:
					return BuildWire(static_cast<IIfcOffsetCurve2D^>(ifcCurve), asSingleEdge);

					//return BuildWire(static_cast<IIfcOffsetCurve3D^>(ifcCurve), asSingleEdge);
					/*case XCurveType::IfcPcurve:
						return BuildCurve2d(static_cast<IIfcPcurve^>(curve));*/
				case XCurveType::IfcPolyline:
					return BuildWire(static_cast<IIfcPolyline^>(ifcCurve), asSingleEdge);
				case XCurveType::IfcRationalBSplineCurveWithKnots:
					return BuildWire(static_cast<IIfcRationalBSplineCurveWithKnots^>(ifcCurve), true);
					/*case XCurveType::IfcSurfaceCurve:
						return BuildCurve2d(static_cast<IIfcSurfaceCurve^>(curve));*/
				case XCurveType::IfcTrimmedCurve:
					return BuildWire(static_cast<IIfcTrimmedCurve^>(ifcCurve), asSingleEdge);
				case XCurveType::IfcOffsetCurve3D:
				default:
					throw RaiseGeometryFactoryException("Unsupported curve type", ifcCurve);
				}

			}

			TopoDS_Wire WireFactory::BuildWire(IIfcBSplineCurveWithKnots^ ifcBSplineCurveWithKnots)
			{
				TopoDS_Edge edge = EDGE_FACTORY->BuildEdge(ifcBSplineCurveWithKnots); //throws exception
				TopoDS_Wire wire = EXEC_NATIVE->BuildWire(edge);
				if (wire.IsNull())
					throw RaiseGeometryFactoryException("Circle could not be built as a wire", ifcBSplineCurveWithKnots);
				return wire;
			}

			TopoDS_Wire WireFactory::BuildWire(IIfcCircle^ ifcCircle)
			{
				TopoDS_Edge edge = EDGE_FACTORY->BuildEdge(ifcCircle); //throws exception
				TopoDS_Wire wire = EXEC_NATIVE->BuildWire(edge);
				if (wire.IsNull())
					throw RaiseGeometryFactoryException("Circle could not be built as a wire", ifcCircle);
				return wire;
			}



			TopoDS_Wire WireFactory::BuildWire(IIfcEllipse^ ifcEllipse)
			{
				TopoDS_Edge edge = EDGE_FACTORY->BuildEdge(ifcEllipse); //throws exception
				TopoDS_Wire wire = EXEC_NATIVE->BuildWire(edge);
				if (wire.IsNull())
					throw RaiseGeometryFactoryException("IfcEllipse could not be built as a wire", ifcEllipse);
				return wire;
			}



			TopoDS_Wire WireFactory::BuildWire(IIfcLine^ ifcLine)
			{
				TopoDS_Edge edge = EDGE_FACTORY->BuildEdge(ifcLine); //throws exception
				TopoDS_Wire wire = EXEC_NATIVE->BuildWire(edge);
				if (wire.IsNull())
					throw RaiseGeometryFactoryException("IfcLine could not be built as a wire", ifcLine);
				return wire;
			}


			TopoDS_Wire WireFactory::BuildWire(IIfcOffsetCurve2D^ ifcOffsetCurve2D, bool asSingleEdge)
			{
				TopoDS_Wire basisWire = BuildWire(ifcOffsetCurve2D->BasisCurve, asSingleEdge);//throws exception
				TopoDS_Wire wire = EXEC_NATIVE->BuildOffset(basisWire, ifcOffsetCurve2D->Distance);
				if (wire.IsNull())
					throw RaiseGeometryFactoryException("IfcOffsetCurve2D could not be built as a wire", ifcOffsetCurve2D);
				return wire;
			}
			TopoDS_Wire WireFactory::BuildWire(IIfcOffsetCurve3D^ ifcOffsetCurve3D, bool asSingleEdge)
			{
				TopoDS_Wire basisWire = BuildWire(ifcOffsetCurve3D->BasisCurve, asSingleEdge);//throws exception
				TopoDS_Wire wire = EXEC_NATIVE->BuildOffset(basisWire, ifcOffsetCurve3D->Distance);
				if (wire.IsNull())
					throw RaiseGeometryFactoryException("IfcOffsetCurve3D could not be built as a wire", ifcOffsetCurve3D);
				return wire;
			}
			TopoDS_Wire WireFactory::BuildWire(IIfcPolyline^ ifcPolyline, bool asSingleEdge)
			{
				if (asSingleEdge)
				{
					TopoDS_Edge edge = EDGE_FACTORY->BuildEdge(ifcPolyline); //throws exception
					TopoDS_Wire wire = EXEC_NATIVE->BuildWire(edge);
					if (wire.IsNull())
						throw RaiseGeometryFactoryException("IIfcPolyline could not be built as a wire", ifcPolyline);
					return wire;
				}
				else
				{
					if (2 == (int)ifcPolyline->Dim)
					{
						TColgp_Array1OfPnt2d points(1, ifcPolyline->Points->Count);
						int id = 0;

						for each (IIfcCartesianPoint ^ cp in ifcPolyline->Points)
						{
							points.SetValue(++id, GEOMETRY_FACTORY->BuildPoint2d(cp));
						}
						TopoDS_Wire wire = EXEC_NATIVE->BuildPolyline2d(points, ModelGeometryService->Precision);
						if (wire.IsNull())
							throw RaiseGeometryFactoryException("IIfcPolyline could not be built as a wire", ifcPolyline);
						return wire;
					}
					else
					{
						TColgp_Array1OfPnt points(1, ifcPolyline->Points->Count);
						int id = 0;

						for each (IIfcCartesianPoint ^ cp in ifcPolyline->Points)
						{
							points.SetValue(++id, GEOMETRY_FACTORY->BuildPoint3d(cp));
						}
						TopoDS_Wire wire = EXEC_NATIVE->BuildPolyline3d(points, ModelGeometryService->Precision);
						if (wire.IsNull())
							throw RaiseGeometryFactoryException("IIfcPolyline could not be built as a wire", ifcPolyline);
						return wire;
					}
				}
			}

			TopoDS_Wire WireFactory::BuildWire(IIfcRationalBSplineCurveWithKnots^ ifcRationalBSplineCurveWithKnots)
			{
				TopoDS_Edge edge = EDGE_FACTORY->BuildEdge(ifcRationalBSplineCurveWithKnots); //throws exception
				TopoDS_Wire wire = EXEC_NATIVE->BuildWire(edge);
				if (wire.IsNull())
					throw RaiseGeometryFactoryException("IfcLine could not be built as a wire", ifcRationalBSplineCurveWithKnots);
				return wire;
			}

			TopoDS_Wire WireFactory::BuildWire(IIfcTrimmedCurve^ ifcTrimmedCurve, bool asSingleEdge)
			{
				TopoDS_Wire basisWire = BuildWire(ifcTrimmedCurve->BasisCurve, asSingleEdge);//throws exception
				bool isConic = (dynamic_cast<IIfcConic^>(ifcTrimmedCurve->BasisCurve) != nullptr);
				bool isLine = (dynamic_cast<IIfcLine^>(ifcTrimmedCurve->BasisCurve) != nullptr);
				bool isEllipse = (dynamic_cast<IIfcEllipse^>(ifcTrimmedCurve->BasisCurve) != nullptr);
				bool sense = ifcTrimmedCurve->SenseAgreement;
				//get the parametric values
				IfcTrimmingPreference trimPref = ifcTrimmedCurve->MasterRepresentation;

				bool trim_cartesian = (ifcTrimmedCurve->MasterRepresentation == IfcTrimmingPreference::CARTESIAN);

				double u1 = double::NegativeInfinity;
				double u2 = double::PositiveInfinity;
				IIfcCartesianPoint^ cp1 = nullptr;
				IIfcCartesianPoint^ cp2 = nullptr;
				gp_Pnt p1;
				gp_Pnt p2;
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
					p1 = GEOMETRY_FACTORY->BuildPoint3d(cp1);
					p2 = GEOMETRY_FACTORY->BuildPoint3d(cp2);
				}
				else if (double::IsNegativeInfinity(u1) || double::IsPositiveInfinity(u2)) //non-compliant
					throw RaiseGeometryFactoryException("Ifc Formal Proposition: TrimValuesConsistent. Either a single value is specified for Trim, or the two trimming values are of different type (point and parameter)", ifcTrimmedCurve);
				else //we prefer to use parameters but need to adjust
				{
					trim_cartesian = false;
					if (isConic)
					{
						u1 *= ModelGeometryService->RadianFactor; //correct to radians
						u2 *= ModelGeometryService->RadianFactor; //correct to radians
					}
				}

				if (Math::Abs(u1 - u2) < ModelGeometryService->Precision) //if the parameters are the same trimming will fail we cannot support periodic wires conic curve
				{
					throw RaiseGeometryFactoryException("Parametric Trim Points are equal and will result in an empty wire", ifcTrimmedCurve->BasisCurve);
				}
				else
				{
					TopoDS_Wire trimmedWire = EXEC_NATIVE->BuildTrimmedWire(basisWire, p1, p2, u1, u2, trim_cartesian, ifcTrimmedCurve->SenseAgreement, ModelGeometryService->MinimumGap);
					if (trimmedWire.IsNull())
						throw RaiseGeometryFactoryException("IfcTrimmedCurve could not be built as a wire", ifcTrimmedCurve);
					return trimmedWire;
				}
			}


#pragma region Interface implementation



			IXWire^ WireFactory::BuildWire(array<IXPoint^>^ xPoints)
			{
				//validate
				if (xPoints->Length == 0)
					throw RaiseGeometryFactoryException("Points has zero length");
				TColgp_Array1OfPnt points(1, xPoints->Length);
				int id = 0;

				for each (IXPoint ^ cp in xPoints)
				{
					points.SetValue(++id, GEOMETRY_FACTORY->BuildPoint3d(cp));
				}
				TopoDS_Wire wire = EXEC_NATIVE->BuildPolyline3d(points,/* -1, -1,*/ ModelGeometryService->Precision);
				if (wire.IsNull() || wire.NbChildren() == 0)
					throw RaiseGeometryFactoryException("Resulting wire is empty");

				if (OccHandle().IsClosed(wire, ModelGeometryService->Precision))
					wire.Closed(true);
				return gcnew XWire(wire);
			}


			IXWire^ WireFactory::Build(IIfcCurve^ ifcCurve)
			{
				TopoDS_Wire wire = BuildWire(ifcCurve, false); //asume all compund edges are implemented as segments, not bsplines
				if (wire.IsNull() || wire.NbChildren() == 0)
					throw RaiseGeometryFactoryException("Resulting wire is empty", ifcCurve);
				return gcnew XWire(wire);
			}

			IXWire^ WireFactory::Build(IIfcProfileDef^ ifcProfileDef)
			{
				return PROFILE_FACTORY->BuildWire(ifcProfileDef);
			}




#pragma endregion


			TopoDS_Wire WireFactory::BuildWire(IIfcCompositeCurve^ ifcCompositeCurve, bool asSingleEdge)
			{
				if (asSingleEdge)
				{
					TopoDS_Edge edge = EDGE_FACTORY->BuildEdge(ifcCompositeCurve); //throws exception
					TopoDS_Wire wire = EXEC_NATIVE->BuildWire(edge);
					if (wire.IsNull())
						throw RaiseGeometryFactoryException("IfcCompositeCurve could not be built as a wire", ifcCompositeCurve);
					return wire;
				}
				else
				{
					if ((int)ifcCompositeCurve->Dim == 2)
					{
						TColGeom2d_SequenceOfBoundedCurve segments;
						CURVE_FACTORY->BuildCompositeCurveSegments2d(ifcCompositeCurve, segments);
						TopoDS_Wire wire = EXEC_NATIVE->BuildWire(segments, ModelGeometryService->Precision, ModelGeometryService->MinimumGap);
						if (wire.IsNull())
							throw RaiseGeometryFactoryException("IfcCompositeCurve could not be built as a wire", ifcCompositeCurve);
						return wire;
					}
					else
					{
						TColGeom_SequenceOfBoundedCurve segments;
						CURVE_FACTORY->BuildCompositeCurveSegments3d(ifcCompositeCurve, segments);
						TopoDS_Wire wire = EXEC_NATIVE->BuildWire(segments, ModelGeometryService->Precision, ModelGeometryService->MinimumGap);
						if (wire.IsNull())
							throw RaiseGeometryFactoryException("IfcCompositeCurve could not be built as a wire", ifcCompositeCurve);
						return wire;
					}
				}
			}

			TopoDS_Wire WireFactory::BuildWire(IIfcIndexedPolyCurve^ ifcIndexedPolyCurve, bool asSingleEdge)
			{
				if (asSingleEdge)
				{
					TopoDS_Edge edge = EDGE_FACTORY->BuildEdge(ifcIndexedPolyCurve); //throws exception
					TopoDS_Wire wire = EXEC_NATIVE->BuildWire(edge);
					if (wire.IsNull())
						throw RaiseGeometryFactoryException("IfcIndexedPolyCurve could not be built as a wire", ifcIndexedPolyCurve);
					return wire;
				}
				else
				{
					if (2 == (int)ifcIndexedPolyCurve->Dim)
					{
						TColGeom2d_SequenceOfBoundedCurve segments;
						CURVE_FACTORY->BuildIndexPolyCurveSegments2d(ifcIndexedPolyCurve, segments);
						TopoDS_Wire wire = EXEC_NATIVE->BuildWire(segments, ModelGeometryService->Precision, ModelGeometryService->MinimumGap);
						if (wire.IsNull())
							throw RaiseGeometryFactoryException("IfcIndexedPolyCurve could not be built as a wire", ifcIndexedPolyCurve);
						return wire;
					}
					else
					{
						TColGeom_SequenceOfBoundedCurve segments;
						CURVE_FACTORY->BuildIndexPolyCurveSegments3d(ifcIndexedPolyCurve, segments);
						TopoDS_Wire wire = EXEC_NATIVE->BuildWire(segments, ModelGeometryService->Precision, ModelGeometryService->MinimumGap);
						if (wire.IsNull())
							throw RaiseGeometryFactoryException("IfcIndexedPolyCurve could not be built as a wire", ifcIndexedPolyCurve);
						return wire;
					}
				}
			}

			TopoDS_Wire WireFactory::BuildDirectrixWire(IIfcCurve^ ifcCurve, double startParam, double endParam)
			{
				
				TopoDS_Wire wire = BuildWire(ifcCurve, false); //throws exception
			
				if (double::IsNaN(startParam) && double::IsNaN(endParam)) return wire; //no trimming required
				
				TopoDS_Wire directrix = EXEC_NATIVE->BuildTrimmedWire(wire, startParam, endParam, true, ModelGeometryService->Precision);
				if (directrix.IsNull())
					throw RaiseGeometryFactoryException("Directrix could not be built", ifcCurve);
				return directrix;
			}

			//void WireFactory::AdjustDirectrixTrimParameters(IIfcCurve^ basisCurve, Nullable<IfcParameterValue> startParam, Nullable<IfcParameterValue> endParam, double& start, double& end)
			//{
			//	if (!startParam.HasValue && !endParam.HasValue) //no trim required
			//	{
			//		start = -1;
			//		end = -1;
			//		return;
			//	}
			//	//BRepAdaptor_CompCurve cc(wire, Standard_True);

			//	//if we have a trimmed curve we need to get the basis curve for correct parameterisation

			//	//IIfcCurve^ basisCurve = directrix;
			//	while (dynamic_cast<IIfcTrimmedCurve^>(basisCurve))
			//		basisCurve = ((IIfcTrimmedCurve^)basisCurve)->BasisCurve;
			//	double start = 0;
			//	double end = double::PositiveInfinity;

			//	if (dynamic_cast<IIfcLine^>(basisCurve)) //params are different need to consider magnitude
			//	{
			//		IIfcLine^ line = (IIfcLine^)(basisCurve);
			//		double mag = line->Dir->Magnitude;
			//		if (startParam.HasValue && endParam.HasValue)
			//		{
			//			start = startParam.Value * mag;
			//			end = endParam.Value * mag;
			//		}
			//		else if (startParam.HasValue && !endParam.HasValue)
			//		{
			//			start = startParam.Value * mag;
			//			endPar = cc.LastParameter();
			//		}
			//		else if (!startParam.HasValue && endParam.HasValue)
			//		{
			//			startPar = cc.FirstParameter();
			//			endPar = endParam.Value * mag;
			//		}
			//	}
			//	else if (dynamic_cast<IIfcCompositeCurve^>(basisCurve)) //params are different
			//	{

			//		if (startParam.HasValue)
			//			startPar = startParam.Value;
			//		if (endParam.HasValue)
			//			endPar = endParam.Value;
			//		double occStart = 0;
			//		double occEnd = 0;
			//		double totCurveLen = 0;

			//		// for each segment we encounter, we will see if the threshold falls within its length
			//		//
			//		IIfcCompositeCurve^ curve = (IIfcCompositeCurve^)(basisCurve);
			//		for each (IIfcCompositeCurveSegment ^ segment in curve->Segments)
			//		{
			//			XbimWire^ segWire = gcnew XbimWire(segment, logger);
			//			double wireLen = segWire->Length;       // this is the length to add to the OCC command if we use all of the segment
			//			double segValue = SegLength(segment, logger);   // this is the IFC size of the segment
			//			totCurveLen += wireLen;


			//			if (startPar > 0)
			//			{
			//				double ratio = System::Math::Min(startPar / segValue, 1.0);
			//				startPar -= ratio * segValue; // reduce the outstanding amount (since it's been accounted for in the segment just processed)
			//				occStart += ratio * wireLen; // progress the occ amount by the ratio of the lenght
			//			}

			//			if (endPar > 0)
			//			{
			//				double ratio = System::Math::Min(endPar / segValue, 1.0);
			//				endPar -= ratio * segValue; // reduce the outstanding amount (since it's been accounted for in the segment just processed)
			//				occEnd += ratio * wireLen; // progress the occ amount by the ratio of the lenght
			//			}
			//		}
			//		double precision = XbimConvert::ModelGeometryService(directrix)->MinimumGap;
			//		// only trim if needed either from start or end
			//		if ((occStart > 0 && System::Math::Abs(occStart - 0.0) > precision) || (occEnd < totCurveLen && System::Math::Abs(occEnd - totCurveLen) > precision))
			//		{
			//			return (XbimWire^)wire->Trim(occStart, occEnd, precision, logger);
			//		}
			//		else
			//			return wire;
			//	}
			//	else if (dynamic_cast<IIfcPolyline^>(basisCurve) &&
			//		(double)startParam.Value == 0. &&
			//		(double)endParam.Value == 1. &&
			//		directrix->Model->ModelFactors->ApplyWorkAround(XbimGeometryCreator::PolylineTrimLengthOneForEntireLine)) //consider work around for incorrectly set trims
			//	{
			//		startPar = startParam.Value;
			//		endPar = cc.LastParameter();
			//		XbimGeometryCreator::LogInfo(logger, directrix, "Polyline trim (0:1) does not comply with schema. {0}", directrix->Model->Header->FileName->OriginatingSystem);
			//	}
			//	else
			//	{
			//		bool isConic = (dynamic_cast<IIfcConic^>(basisCurve) != nullptr);
			//		double parameterFactor = isConic ? directrix->Model->ModelFactors->AngleToRadiansConversionFactor : 1;
			//		if (isConic && directrix->Model->ModelFactors->ApplyWorkAround(XbimGeometryCreator::SurfaceOfLinearExtrusion)) //part of a family of revit issues with Ifc4
			//		{
			//			XbimGeometryCreator::LogInfo(logger, directrix, "Workaround for Revit conic trim export applied, trims ignored");
			//			//do nothing
			//			return wire;
			//		}
			//		else
			//		{
			//			if (startParam.HasValue && endParam.HasValue)
			//			{
			//				startPar = startParam.Value * parameterFactor;
			//				endPar = endParam.Value * parameterFactor;
			//			}
			//			else if (startParam.HasValue && !endParam.HasValue)
			//			{

			//				startPar = startParam.Value * parameterFactor;
			//				endPar = cc.LastParameter();
			//			}
			//			else if (!startParam.HasValue && endParam.HasValue)
			//			{
			//				startPar = cc.FirstParameter();
			//				endPar = endParam.Value * parameterFactor;
			//			}
			//		}

			//	}
			//}




		}
	}
}