#include "XbimWire.h"
#include "XbimEdge.h"
#include "XbimFace.h"
#include "XbimGeometryCreator.h"
#include "XbimGeomPrim.h"
#include "XbimEdgeSet.h"
#include "XbimVertexSet.h"

#include <BRepBuilderAPI_Transform.hxx>
#include <TopExp_Explorer.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <ShapeFix_ShapeTolerance.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <ShapeFix_Shape.hxx>
#include <TopoDS.hxx>
#include <gp_Pnt.hxx>
#include <BRep_Builder.hxx>
#include <GC_MakeLine.hxx>
#include <gp_Ax2.hxx>
#include <gp_Circ.hxx>
#include <GC_MakeCircle.hxx>
#include <gp_Ax3.hxx>
#include <GC_MakeEllipse.hxx>
#include <gp_Elips.hxx>
#include <TopTools_Array1OfShape.hxx>
#include <Geom_Curve.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <Adaptor3d_HCurve.hxx>
#include <Geom_BezierCurve.hxx>
#include <Geom_Parabola.hxx>
#include <Geom_Hyperbola.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <TopExp.hxx>
#include <Geom_Plane.hxx>
#include <gp_Trsf.hxx>
#include <TopLoc_Location.hxx>
#include <BRepBuilderAPI_FindPlane.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Ellipse.hxx>
#include <BRepAdaptor_CompCurve.hxx>
#include <BRepAdaptor_HCompCurve.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepFilletAPI_MakeFillet2d.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepTools.hxx>
#include <gp_Dir2d.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Lin2d.hxx>
#include <IntAna2d_AnaIntersection.hxx>
#include <BRepOffsetAPI_MakeOffset.hxx>

using namespace Xbim::Common;
using namespace Xbim::Ifc2x3::MeasureResource;

namespace Xbim
{
	namespace Geometry
	{

#pragma region Destructors and cleanup 

		/*Ensures native pointers are deleted and garbage collected*/
		void XbimWire::InstanceCleanup()
		{
			IntPtr temp = System::Threading::Interlocked::Exchange(ptrContainer, IntPtr::Zero);
			if (temp != IntPtr::Zero)
				delete (TopoDS_Wire*)(temp.ToPointer());
			System::GC::SuppressFinalize(this);
		}
#pragma endregion

#pragma region Constructors

		XbimWire::XbimWire(const TopoDS_Wire& wire)
		{
			pWire = new TopoDS_Wire();
			*pWire = wire;
		}
		XbimWire::XbimWire(const std::vector<gp_Pnt>& points, double tolerance)
		{
			BRepBuilderAPI_MakePolygon polyMaker;
			for (size_t i = 0; i < points.size(); i++)
			{
				polyMaker.Add(points[i]);
			}
			polyMaker.Close();
			pWire = new TopoDS_Wire();
			*pWire = polyMaker.Wire();
			ShapeFix_ShapeTolerance fixer;
			fixer.SetTolerance(*pWire, tolerance, TopAbs_VERTEX);
		}
		XbimWire::XbimWire(double precision){ Init(precision); }
		XbimWire::XbimWire(IfcPolyline^ profile){ Init(profile); }
		XbimWire::XbimWire(IfcCompositeCurve^ profile){ Init(profile); }
		XbimWire::XbimWire(IfcCompositeCurveSegment^ profile){ Init(profile); };
		XbimWire::XbimWire(IfcTrimmedCurve^ profile){ Init(profile); }
		XbimWire::XbimWire(IfcBSplineCurve^ bspline){ Init(bspline); }
		XbimWire::XbimWire(IfcBezierCurve^ bez){ Init(bez); }
		XbimWire::XbimWire(IfcRationalBezierCurve^ bez){ Init(bez); }
		XbimWire::XbimWire(IfcCurve^ profile){ Init(profile); }
		XbimWire::XbimWire(IfcBoundedCurve^ profile){ Init(profile); }
		XbimWire::XbimWire(IfcPolyLoop ^ profile){ Init(profile); }
		XbimWire::XbimWire(IfcArbitraryClosedProfileDef^ profile){ Init(profile); }
		XbimWire::XbimWire(IfcArbitraryOpenProfileDef^ profile){ Init(profile); }
		XbimWire::XbimWire(IfcCenterLineProfileDef^ profile){ Init(profile); }
		//parametrised profiles
		XbimWire::XbimWire(IfcProfileDef ^ profile){ Init(profile); }
		XbimWire::XbimWire(IfcDerivedProfileDef ^ profile){ Init(profile); }
		XbimWire::XbimWire(IfcParameterizedProfileDef ^ profile){ Init(profile); }
		XbimWire::XbimWire(IfcCircleProfileDef ^ profile){ Init(profile); }
		XbimWire::XbimWire(IfcRectangleProfileDef^ profile){ Init(profile); }
		XbimWire::XbimWire(IfcLShapeProfileDef ^ profile){ Init(profile); }
		XbimWire::XbimWire(IfcUShapeProfileDef ^ profile){ Init(profile); }
		XbimWire::XbimWire(IfcCraneRailFShapeProfileDef ^ profile){ Init(profile); }
		XbimWire::XbimWire(IfcCraneRailAShapeProfileDef ^ profile){ Init(profile); }
		XbimWire::XbimWire(IfcEllipseProfileDef ^ profile){ Init(profile); }
		XbimWire::XbimWire(IfcIShapeProfileDef ^ profile){ Init(profile); }
		XbimWire::XbimWire(IfcZShapeProfileDef ^ profile){ Init(profile); }
		XbimWire::XbimWire(IfcCShapeProfileDef ^ profile){ Init(profile); }
		XbimWire::XbimWire(IfcTShapeProfileDef ^ profile){ Init(profile); }
		XbimWire::XbimWire(double x, double y, double tolerance){ Init(x,y,tolerance); }

#pragma endregion


#pragma region Equality Overrides

		bool XbimWire::Equals(Object^ obj)
		{
			XbimWire^ w = dynamic_cast<XbimWire^>(obj);
			// Check for null
			if (w == nullptr) return false;
			return this == w;
		}

		bool XbimWire::Equals(IXbimWire^ obj)
		{
			// Check for null
			XbimWire^ w = dynamic_cast<XbimWire^>(obj);
			if (w == nullptr) return false;		
			return this == w;
		}

		int XbimWire::GetHashCode()
		{
			if (!IsValid) return 0;
			return pWire->HashCode(Int32::MaxValue);
		}

		bool XbimWire::operator ==(XbimWire^ left, XbimWire^ right)
		{
			// If both are null, or both are same instance, return true.
			if (System::Object::ReferenceEquals(left, right))
				return true;
			// If one is null, but not both, return false.
			if (((Object^)left == nullptr) || ((Object^)right == nullptr))
				return false;
			return  ((const TopoDS_Wire&)left).IsSame(right) == Standard_True;
		}

		bool XbimWire::operator !=(XbimWire^ left, XbimWire^ right)
		{
			return !(left == right);
		}


#pragma endregion


#pragma region Non-Parameterised profiles

		void XbimWire::Init(IfcArbitraryClosedProfileDef^ profile)
		{
			if (dynamic_cast<IfcArbitraryProfileDefWithVoids^>(profile))
			{
				XbimGeometryCreator::logger->ErrorFormat("WW024: IfcArbitraryProfileDefWithVoids #{0} cannot be created as a wire, call the XbimFace method", profile->EntityLabel);
				return;
			}
			else
			{

				XbimWire^ loop = gcnew XbimWire(profile->OuterCurve);
				if (!loop->IsValid)
				{
					XbimGeometryCreator::logger->WarnFormat("WW025: IfcArbitraryClosedProfileDef #{0} has an invalid outer bound. Discarded", profile->EntityLabel);
					return;
				}
				pWire = new TopoDS_Wire();
				*pWire = loop;
			}
		}

		void XbimWire::Init(IfcArbitraryOpenProfileDef^ profile)
		{
			if (dynamic_cast<IfcCenterLineProfileDef^>(profile))
			{
				return Init((IfcCenterLineProfileDef^)profile);
			}
			else
			{

				XbimWire^ loop = gcnew XbimWire(profile->Curve);
				if (!loop->IsValid)
				{
					XbimGeometryCreator::logger->WarnFormat("WW026: IfcArbitraryOpenProfileDef #{0} has an invalid curve. Discarded", profile->EntityLabel);
					return;
				}
				pWire = new TopoDS_Wire();
				*pWire = loop;
			}
		}

		void XbimWire::Init(IfcCenterLineProfileDef^ profile)
		{
			
			XbimWire^ centreWire = gcnew XbimWire(profile->Curve);
			
			if (!centreWire->IsValid)
			{
				
				XbimGeometryCreator::logger->WarnFormat("WW027: IfcCenterLineProfileDef #{0} has an invalid curve. Discarded", profile->EntityLabel);
				return;
			}

			BRepAdaptor_CompCurve cc(centreWire, Standard_True);
			gp_Pnt wStart = cc.Value(cc.FirstParameter());
			gp_Pnt wEnd = cc.Value(cc.LastParameter());

			BRepOffsetAPI_MakeOffset offseter(centreWire);
			Standard_Real offset = profile->Thickness / 2;
			offseter.Perform(offset);
			
			double precision = profile->ModelOf->ModelFactors->Precision;
			if (offseter.IsDone() && offseter.Shape().ShapeType() == TopAbs_WIRE)
			{
				//need to change radiused ends to straight lines
				BRepBuilderAPI_MakeWire wireMaker;
				for (BRepTools_WireExplorer exp(TopoDS::Wire(offseter.Shape())); exp.More(); exp.Next())
				{
					TopoDS_Edge e = exp.Current();
					TopLoc_Location loc;
					Standard_Real start, end;
					Handle(Geom_Curve) c3d = BRep_Tool::Curve(e, loc, start, end);
					if (!c3d.IsNull())
					{
						Handle(Geom_Curve) c3dptr = Handle(Geom_Curve)::DownCast(c3d->Transformed(loc.Transformation()));
						Handle(Standard_Type) cType = c3dptr->DynamicType();					
						if (cType == STANDARD_TYPE(Geom_Circle))
						{
							Handle(Geom_Circle) circ = Handle(Geom_Circle)::DownCast(c3dptr);
							if (Math::Abs(circ->Radius() - offset) <= precision) //it could be the end
							{
								//make sure we are at a start or end point
								if (Math::Abs(circ->Axis().Location().Distance(wStart) <= precision) ||
									Math::Abs(circ->Axis().Location().Distance(wEnd) <= precision))
								{						
									gp_Pnt s, e;
									c3d->D0(start, s);
									c3d->D0(end, e);
									//make straight edge
									wireMaker.Add(BRepBuilderAPI_MakeEdge(s, e));
									continue;
								}
							}
						}
						
					}
					wireMaker.Add(e);
				}
				
				pWire = new TopoDS_Wire();
				*pWire = wireMaker.Wire();

			}
		}

		void XbimWire::Init(IfcPolyline^ pLine)
		{
			int total = pLine->Points->Count;
			if (total < 2)
			{
				XbimGeometryCreator::logger->WarnFormat("WW001: Line with zero length found in IfcPolyline = #{0}. Ignored", pLine->EntityLabel);
				return;
			}

			double tolerance = pLine->ModelOf->ModelFactors->Precision;
			//Make all the vertices
			Standard_Boolean closed = Standard_False;

			if (pLine->Points[0]->IsEqual(pLine->Points[total - 1], tolerance))
			{
				total--; //skip the last point
				if (total > 2) closed = Standard_True;//closed polyline with two points is not a valid closed shape
			}

			TopTools_Array1OfShape vertexStore(1, pLine->Points->Count + 1);
			BRep_Builder builder;
			TopoDS_Wire wire;
			builder.MakeWire(wire);
			bool is3D = pLine->Dim == 3;

			gp_Pnt first;
			gp_Pnt previous;

			for (int i = 0; i < total; i++) //add all the points into unique collection
			{
				IfcCartesianPoint^ p = pLine->Points[i];
				gp_Pnt current(p->X, p->Y, is3D ? p->Z : 0);
				TopoDS_Vertex v;
				builder.MakeVertex(v, current, tolerance);
				vertexStore.SetValue(i + 1, v);
			}
			int firstIdx = 1;
			bool edgeAdded = false;
			for (int pt = 1; pt <= total; pt++)
			{
				int next = pt + 1;
				if (pt == total) //we are at the last point
				{
					if (closed == Standard_True) //add the last edge in
						next = firstIdx;
					else
						break; //stop
				}
				const TopoDS_Vertex& v1 = TopoDS::Vertex(vertexStore.Value(pt));
				const TopoDS_Vertex& v2 = TopoDS::Vertex(vertexStore.Value(next));
				try
				{
					BRepBuilderAPI_MakeEdge edgeMaker(v1, v2);
					BRepBuilderAPI_EdgeError edgeErr = edgeMaker.Error();
					if (edgeErr != BRepBuilderAPI_EdgeDone)
					{
						gp_Pnt p1 = BRep_Tool::Pnt(v1);
						gp_Pnt p2 = BRep_Tool::Pnt(v2);
						String^ errMsg = XbimEdge::GetBuildEdgeErrorMessage(edgeErr);
						XbimGeometryCreator::logger->InfoFormat("WW002: Invalid edge found in IfcPolyline = #{0}, Start = {1}, {2}, {3} End = {4}, {5}, {6}. Ignored",
							pLine->EntityLabel, p1.X(), p1.Y(), p1.Z(), p2.X(), p2.Y(), p2.Z());

					}
					else
					{
						builder.Add(wire, edgeMaker.Edge());
						if (!edgeAdded) firstIdx = pt; //we need this in case the first edge is invalid and we need to close properly
						edgeAdded = true;
					}
				}
				catch (System::Runtime::InteropServices::SEHException^)
				{
					gp_Pnt p1 = BRep_Tool::Pnt(v1);
					gp_Pnt p2 = BRep_Tool::Pnt(v2);
					XbimGeometryCreator::logger->InfoFormat("WW003: Invalid edge found in IfcPolyline = #{0}, Start = {1}, {2}, {3} End = {4}, {5}, {6}. Ignored",
						pLine->EntityLabel, p1.X(), p1.Y(), p1.Z(), p2.X(), p2.Y(), p2.Z());
				}
			}
			wire.Closed(closed);
			if (BRepCheck_Analyzer(wire, Standard_True).IsValid() == Standard_True)
			{
				pWire = new TopoDS_Wire();
				*pWire = wire;
			}
			else
			{

				double toleranceMax = pLine->ModelOf->ModelFactors->PrecisionMax;
				ShapeFix_Shape sfs(wire);
				sfs.SetPrecision(tolerance);
				sfs.SetMinTolerance(tolerance);
				sfs.SetMaxTolerance(toleranceMax);
				sfs.Perform();

				if (BRepCheck_Analyzer(sfs.Shape(), Standard_True).IsValid() == Standard_True && sfs.Shape().ShapeType() == TopAbs_WIRE) //in release builds except the geometry is not compliant
				{
					pWire = new TopoDS_Wire();
					*pWire = TopoDS::Wire(sfs.Shape());
				}
				else
				{
					XbimGeometryCreator::logger->WarnFormat("WW004: Invalid IfcPolyline #{0} found. Discarded", pLine->EntityLabel);
					return;
				}
			}
		}

		void XbimWire::Init(IfcBSplineCurve^ bspline)
		{
			IfcBezierCurve^ bez = dynamic_cast<IfcBezierCurve^>(bspline);
			if (bez != nullptr) 
				Init(bez);
			else
				XbimGeometryCreator::logger->WarnFormat("WW030: Unsupported IfcBSplineCurve type #{0} found. Ignored", bspline->EntityLabel);
		}

		void XbimWire::Init(IfcBezierCurve^ bez)
		{
			IfcRationalBezierCurve^ ratBez = dynamic_cast<IfcRationalBezierCurve^>(bez);
			if (ratBez != nullptr)
				Init(ratBez);
			else
			{
				XbimEdge^ edge = gcnew XbimEdge(bez);
				if (edge->IsValid)
				{
					BRepBuilderAPI_MakeWire b(edge);
					pWire = new TopoDS_Wire();
					*pWire = b.Wire();
				}
			}
		}

		void XbimWire::Init(IfcRationalBezierCurve^ bez)
		{
			XbimEdge^ edge = gcnew XbimEdge(bez);
			if (edge->IsValid)
			{
				BRepBuilderAPI_MakeWire b(edge);
				pWire = new TopoDS_Wire();
				*pWire = b.Wire();
			}
		}

		void XbimWire::Init(IfcCompositeCurveSegment^ compCurveSeg)
		{
			Init(compCurveSeg->ParentCurve);
			if (IsValid && !compCurveSeg->SameSense) this->Reverse();
		}

		void XbimWire::Init(IfcCompositeCurve^ cCurve)
		{
			bool haveWarned = false;
			BRepBuilderAPI_MakeWire wire;
			XbimModelFactors^ mf = cCurve->ModelOf->ModelFactors;
			ShapeFix_ShapeTolerance FTol;
			double precision = mf->PrecisionBoolean; //use a courser precision for trimmed curves
			double currentPrecision = precision;
			double maxTolerance = mf->PrecisionBooleanMax;
			for each(IfcCompositeCurveSegment^ seg in cCurve->Segments)
			{
				XbimWire^ wireSegManaged = gcnew XbimWire(seg->ParentCurve);
				
				if (wireSegManaged->IsValid)
				{
					
					TopoDS_Wire wireSeg = wireSegManaged;
					if (!seg->SameSense) wireSeg.Reverse();
				retryAddWire:
					
					FTol.SetTolerance(wireSeg, currentPrecision, TopAbs_WIRE);
					wire.Add(wireSeg);

					if (!wire.IsDone())
					{
						currentPrecision *= 10;
						if (currentPrecision <= maxTolerance)
							goto retryAddWire;
						else
						{
							haveWarned = true;
							XbimGeometryCreator::logger->WarnFormat("WW005: IfcCompositeCurveSegment {0} was not contiguous with any edges in IfcCompositeCurve #{1}. It has been ignored", seg->EntityLabel, cCurve->EntityLabel);
						}
					}
				}
				
			}

			if (wire.IsDone())
			{
				TopoDS_Wire w = wire.Wire();
				if (BRepCheck_Analyzer(w, Standard_True).IsValid() == Standard_True)
				{
					pWire = new TopoDS_Wire();
					*pWire = w;
				}
				else
				{
					double toleranceMax = cCurve->ModelOf->ModelFactors->PrecisionMax;
					ShapeFix_Shape sfs(w);
					sfs.SetMinTolerance(mf->Precision);
					sfs.SetMaxTolerance(mf->OneMilliMetre * 50);
					sfs.Perform();
					if (BRepCheck_Analyzer(sfs.Shape(), Standard_True).IsValid() == Standard_True && sfs.Shape().ShapeType() == TopAbs_WIRE) //in release builds except the geometry is not compliant
					{
						pWire = new TopoDS_Wire();
						*pWire = TopoDS::Wire(sfs.Shape());
					}
					else
						XbimGeometryCreator::logger->WarnFormat("WW006: Invalid IfcCompositeCurveSegment #{0} found. It has been ignored", cCurve->EntityLabel);
				}
			}
			else if (!haveWarned) //don't do it twice
			{
				BRepBuilderAPI_WireError err = wire.Error();
				switch (err)
				{
				case BRepBuilderAPI_EmptyWire:
					XbimGeometryCreator::logger->WarnFormat("WW007: Illegal bound found in IfcCompositeCurve = #{0}, it has no edges. Ignored", cCurve->EntityLabel);
					break;
				case BRepBuilderAPI_DisconnectedWire:
					XbimGeometryCreator::logger->WarnFormat("WW008: Illegal bound found in IfcCompositeCurve = #{0}, all edges could not be connected. Ignored", cCurve->EntityLabel);
					break;
				case BRepBuilderAPI_NonManifoldWire:
					XbimGeometryCreator::logger->WarnFormat("WW009: Illegal found in IfcCompositeCurve = #{0}, it is non-manifold. Ignored", cCurve->EntityLabel);
					break;
				default:
					XbimGeometryCreator::logger->WarnFormat("WW010: Illegal bound found in IfcCompositeCurve = #{0}, unknown error. Ignored", cCurve->EntityLabel);
					break;
				}
			}
		}

		void XbimWire::Init(IfcTrimmedCurve^ tCurve)
		{
			ShapeFix_ShapeTolerance FTol;
			bool isConic = (dynamic_cast<IfcConic^>(tCurve->BasisCurve) != nullptr);

			XbimModelFactors^ mf = tCurve->ModelOf->ModelFactors;
			double tolerance = mf->Precision;
			double toleranceMax = mf->PrecisionMax;
			double parameterFactor = isConic ? mf->AngleToRadiansConversionFactor : 1;
			Handle(Geom_Curve) curve;
			bool rotateElipse = false;
			IfcAxis2Placement2D^ ax2;
			//it could be based on a circle, ellipse or line
			if (dynamic_cast<IfcCircle^>(tCurve->BasisCurve))
			{
				IfcCircle^ c = (IfcCircle^)tCurve->BasisCurve;
				if (dynamic_cast<IfcAxis2Placement2D^>(c->Position))
				{
					IfcAxis2Placement2D^ ax2 = (IfcAxis2Placement2D^)c->Position;
					gp_Ax2 gpax2(gp_Pnt(ax2->Location->X, ax2->Location->Y, 0), gp_Dir(0, 0, 1), gp_Dir(ax2->P[0].X, ax2->P[0].Y, 0.));
					gp_Circ gc(gpax2, c->Radius);
					curve = GC_MakeCircle(gc);
				}
				else if (dynamic_cast<IfcAxis2Placement3D^>(c->Position))
				{
					IfcAxis2Placement3D^ ax2 = (IfcAxis2Placement3D^)c->Position;
					gp_Ax3 	gpax3 = XbimGeomPrim::ToAx3(ax2);
					gp_Circ gc(gpax3.Ax2(), c->Radius);
					curve = GC_MakeCircle(gc);
				}
				else
				{
					Type ^ type = c->Position->GetType();
					throw(gcnew NotImplementedException(String::Format("XbimFaceBound. Circle with Placement of type {0} is not implemented", type->Name)));
				}
			}
			else if (dynamic_cast<IfcEllipse^>(tCurve->BasisCurve))
			{
				IfcEllipse^ c = (IfcEllipse^)tCurve->BasisCurve;
				if (dynamic_cast<IfcAxis2Placement2D^>(c->Position))
				{
					ax2 = (IfcAxis2Placement2D^)c->Position;
					double s1;
					double s2;

					if (c->SemiAxis1 > c->SemiAxis2)
					{
						s1 = c->SemiAxis1;
						s2 = c->SemiAxis2;
						rotateElipse = false;
					}
					else //either same or two is larger than 1
					{
						s1 = c->SemiAxis2;
						s2 = c->SemiAxis1;
						rotateElipse = true;
					}

					gp_Ax2 gpax2(gp_Pnt(ax2->Location->X, ax2->Location->Y, 0), gp_Dir(0, 0, 1), gp_Dir(ax2->P[rotateElipse ? 1 : 0].X, ax2->P[rotateElipse ? 1 : 0].Y, 0.));

					gp_Elips gc(gpax2, s1, s2);
					curve = GC_MakeEllipse(gc);


				}
				else if (dynamic_cast<IfcAxis2Placement3D^>(c->Position))
				{
					Type ^ type = c->Position->GetType();
					throw(gcnew NotImplementedException(String::Format("XbimFaceBound. Ellipse with Placement of type {0} is not implemented", type->Name)));
				}
				else
				{
					Type ^ type = c->Position->GetType();
					throw(gcnew NotImplementedException(String::Format("XbimFaceBound. Ellipse with Placement of type {0} is not implemented", type->Name)));
				}
			}
			else if (dynamic_cast<IfcLine^>(tCurve->BasisCurve))
			{
				IfcLine^ line = (IfcLine^)(tCurve->BasisCurve);
				IfcCartesianPoint^ cp = line->Pnt;

				IfcVector^ dir = line->Dir;
				gp_Pnt pnt(cp->X, cp->Y, cp->Dim == 3 ? cp->Z : 0);

				gp_Vec vec(dir->Orientation->X, dir->Orientation->Y, dir->Dim == 3 ? dir->Orientation->Z : 0);
				parameterFactor = dir->Magnitude;
				vec *= dir->Magnitude;
				curve = GC_MakeLine(pnt, vec);
			}
			else
			{
				Type ^ type = tCurve->BasisCurve->GetType();
				throw(gcnew NotImplementedException(String::Format("XbimFaceBound. CompositeCurveSegments with BasisCurve of type {0} is not implemented", type->Name)));
			}


			bool trim_cartesian = (tCurve->MasterRepresentation == IfcTrimmingPreference::CARTESIAN);

			bool trimmed1 = false;
			bool trimmed2 = false;
			bool sense_agreement = tCurve->SenseAgreement;
			double flt1;
			gp_Pnt pnt1;
			double x, y, z;

			for each (IfcTrimmingSelect^ trim in tCurve->Trim1)
			{

				if (dynamic_cast<IfcCartesianPoint^>(trim) && trim_cartesian)
				{
					IfcCartesianPoint^ cp = (IfcCartesianPoint^)trim;

					cp->XYZ(x, y, z);
					pnt1.SetXYZ(gp_XYZ(x, y, z));
					trimmed1 = true;
				}
				else if (dynamic_cast<IfcParameterValue^>(trim) && !trim_cartesian)
				{
					IfcParameterValue^ pv = (IfcParameterValue^)trim;
					const double value = (double)(pv->Value);
					flt1 = value * parameterFactor;
					trimmed1 = true;
				}
			}
			BRep_Builder b;
			TopoDS_Wire w;
			b.MakeWire(w);
			for each (IfcTrimmingSelect^ trim in tCurve->Trim2)
			{
				if (dynamic_cast<IfcCartesianPoint^>(trim) && trim_cartesian && trimmed1)
				{
					IfcCartesianPoint^ cp = (IfcCartesianPoint^)trim;
					cp->XYZ(x, y, z);
					gp_Pnt pnt2(x, y, z);
					if (!pnt1.IsEqual(pnt2, tolerance))
					{

						if (rotateElipse) //if we have had to roate the elipse, then rotate the trims
						{
							gp_Ax1 centre(gp_Pnt(ax2->Location->X, ax2->Location->Y, 0), gp_Dir(0, 0, 1));
							pnt1.Rotate(centre, 90.0);
							pnt2.Rotate(centre, 90.0);
						}
						TopoDS_Vertex v1, v2;
						double currentTolerance = tolerance;
						b.MakeVertex(v1, pnt1, currentTolerance);
						b.MakeVertex(v2, pnt2, currentTolerance);
						if (dynamic_cast<IfcLine^>(tCurve->BasisCurve)) //we have a line and two points, just build it
						{
							b.Add(w, BRepBuilderAPI_MakeEdge(sense_agreement ? v1 : v2, sense_agreement ? v2 : v1));
						}
						else //we need to trim
						{

						TryMakeEdge:
							BRepBuilderAPI_MakeEdge e(curve, sense_agreement ? v1 : v2, sense_agreement ? v2 : v1);
							BRepBuilderAPI_EdgeError err = e.Error();
							if (err != BRepBuilderAPI_EdgeDone)
							{
								currentTolerance *= 10;
								if (currentTolerance <= toleranceMax)
								{
									FTol.SetTolerance(v1, currentTolerance);
									FTol.SetTolerance(v2, currentTolerance);
									goto TryMakeEdge;
								}
								String^ errMsg = XbimEdge::GetBuildEdgeErrorMessage(err);
								XbimGeometryCreator::logger->WarnFormat("WW011: Construction of Trimmed Curve #{0}, failed, {1}. A line segment has been used", tCurve->EntityLabel, errMsg);
								b.Add(w, BRepBuilderAPI_MakeEdge(sense_agreement ? v1 : v2, sense_agreement ? v2 : v1));
							}
							else
								b.Add(w, e.Edge());
						}
						trimmed2 = true;
					}
					break;
				}
				else if (dynamic_cast<IfcParameterValue^>(trim) && !trim_cartesian && trimmed1)
				{
					IfcParameterValue^ pv = (IfcParameterValue^)trim;
					double value = (double)(pv->Value);
					double flt2 = (value * parameterFactor);
					if (isConic && Math::Abs(Math::IEEERemainder(flt2 - flt1, (double)(Math::PI*2.0)) - 0.0) <= Precision::Confusion())
					{
						BRepBuilderAPI_MakeEdge edgeMaker(curve);
						BRepBuilderAPI_EdgeError edgeErr = edgeMaker.Error();
						if (edgeErr != BRepBuilderAPI_EdgeDone)
						{
							String^ errMsg = XbimEdge::GetBuildEdgeErrorMessage(edgeErr);
							XbimGeometryCreator::logger->WarnFormat("WW012: Invalid edge found in IfcTrimmedCurve = #{0}, {1}. It has been ignored", tCurve->EntityLabel, errMsg);
							return;
						}
						else
						{
							b.Add(w, edgeMaker.Edge());
						}
					}
					else
					{
						if (rotateElipse) //if we have had to roate the elipse, then rotate the trims
						{
							flt1 += (90 * parameterFactor);
							flt2 += (90 * parameterFactor);
						}
						BRepBuilderAPI_MakeEdge edgeMaker(curve, sense_agreement ? flt1 : flt2, sense_agreement ? flt2 : flt1);
						BRepBuilderAPI_EdgeError edgeErr = edgeMaker.Error();
						if (edgeErr != BRepBuilderAPI_EdgeDone)
						{
							String^ errMsg = XbimEdge::GetBuildEdgeErrorMessage(edgeErr);
							XbimGeometryCreator::logger->WarnFormat("WW013: Invalid edge found in IfcTrimmedCurve = #{0},{1} .It has been ignored", tCurve->EntityLabel, errMsg);
							return;
						}
						else
						{
							b.Add(w, edgeMaker.Edge());
						}
					}
					trimmed2 = true;
					break;
				}
			}
			pWire = new TopoDS_Wire();
			*pWire = w;
		}

		void XbimWire::Init(IfcCurve^ curve)
		{
			IfcBoundedCurve^ bc;
			
			IfcLine^ l;
			IfcEllipse^ e;
			IfcCircle^ c;
			if ((bc = dynamic_cast<IfcBoundedCurve^>(curve)) != nullptr) return Init(bc);	
			else if ((c =  dynamic_cast<IfcCircle^>(curve)) != nullptr)
			{
				XbimEdge^ edge = gcnew XbimEdge(c);
				if (edge->IsValid)
				{
					BRepBuilderAPI_MakeWire b(edge);
					pWire = new TopoDS_Wire();
					*pWire = b.Wire();
				}
			}
			else if ((l = dynamic_cast<IfcLine^>(curve)) != nullptr)
			{
				XbimEdge^ edge = gcnew XbimEdge(l);
				if (edge->IsValid)
				{
					BRepBuilderAPI_MakeWire b(edge);
					pWire = new TopoDS_Wire();
					*pWire = b.Wire();
				}
			}
			else if ((e = dynamic_cast<IfcEllipse^>(curve)) != nullptr)
			{
				XbimEdge^ edge = gcnew XbimEdge(e);
				if (edge->IsValid)
				{
					BRepBuilderAPI_MakeWire b(edge);
					pWire = new TopoDS_Wire();
					*pWire = b.Wire();
				}
			}
			else
			{
				Type ^ type = curve->GetType();
				XbimGeometryCreator::logger->ErrorFormat("EL009: Curve #{0} of type {1} is not implemented", curve->EntityLabel, type->Name);
				return;
			}

		}
		void XbimWire::Init(IfcPolyLoop ^ loop)
		{
			int lastPt = loop->Polygon->Count;
			if (lastPt < 3)
			{
				XbimGeometryCreator::logger->WarnFormat("WW015: Invalid loop in IfcPolyloop #{0}, it has less than three points. Loop discarded", loop->EntityLabel);
				return;
			}
			double tolerance = loop->ModelOf->ModelFactors->Precision;
			//check we haven't got duplicate start and end points
			IfcCartesianPoint^ first = loop->Polygon[0];
			IfcCartesianPoint^ last = loop->Polygon[lastPt - 1];
			if (first->IsEqual(last, tolerance))
			{
				XbimGeometryCreator::logger->WarnFormat("WW016: Invalid edge found in IfcPolyloop = #{0}, Start = #{7}({1}, {2}, {3}) End = #{8}({4}, {5}, {6}). Edge discarded", loop->EntityLabel, first->X, first->Y, first->Z, last->X, last->Y, last->Z, first->EntityLabel, last->EntityLabel);
				lastPt--;
				if (lastPt<3)
				{
					XbimGeometryCreator::logger->WarnFormat("WW017: Invalid loop in IfcPolyloop #{0}, it has less than three points. Loop discarded", loop->EntityLabel);
					return;
				}
			}

			int totalEdges = 0;
			bool is3D = (loop->Polygon[0]->Dim == 3);
			BRep_Builder builder;
			TopoDS_Wire wire;
			builder.MakeWire(wire);
			for (int p = 1; p <= lastPt; p++)
			{
				IfcCartesianPoint^ p1;
				IfcCartesianPoint^ p2;
				if (p == lastPt)
				{
					p2 = loop->Polygon[0];
					p1 = loop->Polygon[p - 1];
				}
				else
				{
					p1 = loop->Polygon[p - 1];
					p2 = loop->Polygon[p];
				}
				TopoDS_Vertex v1, v2;
				gp_Pnt pt1(p1->X, p1->Y, is3D ? p1->Z : 0);
				gp_Pnt pt2(p2->X, p2->Y, is3D ? p2->Z : 0);

				builder.MakeVertex(v1, pt1, tolerance);
				builder.MakeVertex(v2, pt2, tolerance);
				BRepBuilderAPI_MakeEdge edgeMaker(v1, v2);
				BRepBuilderAPI_EdgeError edgeErr = edgeMaker.Error();
				if (edgeErr != BRepBuilderAPI_EdgeDone)
				{
					String^ errMsg = XbimEdge::GetBuildEdgeErrorMessage(edgeErr);
					XbimGeometryCreator::logger->WarnFormat("Invalid edge, {9},  in IfcPolyloop = #{0}. Start = #{7}({1}, {2}, {3}) End = #{8}({4}, {5}, {6}).\nEdge discarded",
						loop->EntityLabel, pt1.X(), pt1.Y(), pt1.Z(), pt2.X(), pt2.Y(), pt2.Z(), p1, p2, errMsg);
				}
				else
				{
					TopoDS_Edge edge = edgeMaker.Edge();
					builder.Add(wire, edge);
					totalEdges++;
				}
			}
			if (totalEdges<3)
			{
				XbimGeometryCreator::logger->WarnFormat("Invalid loop. IfcPolyloop = #{0} only has {1} edge(s), a minimum of 3 is required. Bound discarded", loop->EntityLabel, totalEdges);
				return;
			}
			wire.Closed(Standard_True);
			ShapeFix_ShapeTolerance FTol;
			FTol.SetTolerance(wire, loop->ModelOf->ModelFactors->Precision, TopAbs_VERTEX);
			pWire = new TopoDS_Wire();
			*pWire = wire;
		}

		void XbimWire::Init(IfcBoundedCurve^ bCurve)
		{
			BRepBuilderAPI_MakeWire wire;
			IfcPolyline^ p;
			IfcTrimmedCurve^ t;
			IfcCompositeCurve^ c;
			IfcBSplineCurve^ b;
			if ((p = dynamic_cast<IfcPolyline^>(bCurve)) != nullptr)
				return Init(p);
			else if ((t = dynamic_cast<IfcTrimmedCurve^>(bCurve)) != nullptr)
				return Init(t);
			else if ((c = dynamic_cast<IfcCompositeCurve^>(bCurve)) != nullptr)
				return Init(c);
			else if ((b = dynamic_cast<IfcBSplineCurve^>(bCurve)) != nullptr)
				return Init(b);
			else
			{
				Type ^ type = bCurve->GetType();
				XbimGeometryCreator::logger->ErrorFormat("EL010: BoundedCurve #{0} of type {1} is not implemented", bCurve->EntityLabel, type->Name);
				return;
			}
		}


#pragma endregion

#pragma region IXbimWire Interface

		IXbimGeometryObject^ XbimWire::Transform(XbimMatrix3D matrix3D)
		{
			BRepBuilderAPI_Copy copier(this);
			BRepBuilderAPI_Transform gTran(copier.Shape(), XbimGeomPrim::ToTransform(matrix3D));
			TopoDS_Wire temp = TopoDS::Wire(gTran.Shape());
			return gcnew XbimWire(temp);
		}

		XbimRect3D XbimWire::BoundingBox::get()
		{
			if (pWire == nullptr)return XbimRect3D::Empty;
			Bnd_Box pBox;
			BRepBndLib::Add(*pWire, pBox);
			Standard_Real srXmin, srYmin, srZmin, srXmax, srYmax, srZmax;
			if (pBox.IsVoid()) return XbimRect3D::Empty;
			pBox.Get(srXmin, srYmin, srZmin, srXmax, srYmax, srZmax);
			return XbimRect3D(srXmin, srYmin, srZmin, (srXmax - srXmin), (srYmax - srYmin), (srZmax - srZmin));
		}
		 
		IXbimEdgeSet^ XbimWire::Edges::get()
		{
			if (!IsValid) return XbimEdgeSet::Empty;
			return gcnew XbimEdgeSet(this);
		}

		IEnumerable<XbimPoint3D>^ XbimWire::Points::get()
		{
			return IntervalPoints;
		}

		IXbimVertexSet^ XbimWire::Vertices::get()
		{
			if (!IsValid) return XbimVertexSet::Empty;
			return gcnew XbimVertexSet(*pWire);
		}

		XbimVector3D XbimWire::Normal::get()
		{
			if (!IsValid) return XbimVector3D();

			double x = 0, y = 0, z = 0;
			gp_Pnt currentStart, previousEnd, first;
			int count = 0;
			TopLoc_Location loc;
			Standard_Real start, end;

			for (BRepTools_WireExplorer wEx(*pWire); wEx.More(); wEx.Next())
			{
				
				const TopoDS_Vertex& v = wEx.CurrentVertex();
				currentStart = BRep_Tool::Pnt(v);
				Handle(Geom_Curve) c3d = BRep_Tool::Curve(wEx.Current(), loc, start, end);
				if (!c3d.IsNull())
				{
					Handle(Geom_Curve) c3dptr = Handle(Geom_Curve)::DownCast(c3d->Transformed(loc.Transformation()));
					Handle(Standard_Type) cType = c3dptr->DynamicType();
					if (cType == STANDARD_TYPE(Geom_Line))
					{
						if (count > 0)
							AddNewellPoint(previousEnd, currentStart, x, y, z);
						else
							first = currentStart;
						previousEnd = currentStart;
					}
					else if (wEx.Current().Closed() && ((cType == STANDARD_TYPE(Geom_Circle)) ||
						(cType == STANDARD_TYPE(Geom_Ellipse)) ||
						(cType == STANDARD_TYPE(Geom_Parabola)) ||
						(cType == STANDARD_TYPE(Geom_Hyperbola)))) //it is a conic
					{
						Handle(Geom_Conic) conic = Handle(Geom_Conic)::DownCast(c3dptr);
						gp_Dir dir = conic->Axis().Direction();
						return XbimVector3D(dir.X(), dir.Y(), dir.Z());
					}
					else if ((cType == STANDARD_TYPE(Geom_Circle)) ||
						(cType == STANDARD_TYPE(Geom_Ellipse)) ||
						(cType == STANDARD_TYPE(Geom_Parabola)) ||
						(cType == STANDARD_TYPE(Geom_Hyperbola)) ||
						(cType == STANDARD_TYPE(Geom_BezierCurve)) ||
						(cType == STANDARD_TYPE(Geom_BSplineCurve)))
					{		
						BRepAdaptor_Curve curve(wEx.Current());
						double us = curve.FirstParameter();
						double ue = curve.LastParameter();
						double umiddle = (us + ue) / 2;
						gp_Pnt mid;
						gp_Vec V;
						curve.D1(umiddle, mid, V);
						if (count > 0)
						{
							AddNewellPoint(previousEnd, currentStart, x, y, z);
							AddNewellPoint(currentStart, mid, x, y, z);
							previousEnd = mid;
						}
						else
						{
							first = currentStart;
							AddNewellPoint(first, mid, x, y, z);
							previousEnd = mid;
						}

					}
					else //throw AN EXCEPTION
					{
						throw gcnew XbimGeometryException("Unsupported Edge type");
					}
				}
				count++;
			}
			//do the last one
			AddNewellPoint(previousEnd, first, x, y, z);
			XbimVector3D vec(x, y, z);
			vec.Normalize();
			return vec;
		}

		bool XbimWire::IsPlanar::get()
		{
			if (!IsValid) return false;
			BRepBuilderAPI_FindPlane finder(*pWire);
			return (finder.Found() == Standard_True);
		}

		XbimPoint3D XbimWire::Start::get()
		{
			if (!IsValid) return XbimPoint3D();
			BRepAdaptor_CompCurve cc(*pWire, Standard_True);
			gp_Pnt p = cc.Value(cc.FirstParameter());
			return XbimPoint3D(p.X(),p.Y(),p.Z());
		}

		XbimPoint3D XbimWire::End::get()
		{
			if (!IsValid) return XbimPoint3D();
			BRepAdaptor_CompCurve cc(*pWire, Standard_True);
			gp_Pnt p = cc.Value(cc.LastParameter());
			return XbimPoint3D(p.X(), p.Y(), p.Z());
		}
		
		double XbimWire::Length::get()
		{
			if (!IsValid) return 0.;
			BRepAdaptor_CompCurve cc(*pWire, Standard_True);
			return cc.LastParameter() - cc.FirstParameter();
		}

		List<double>^ XbimWire::IntervalParameters::get()
		{
			if (!IsValid) return gcnew List<double>(0);
			BRepAdaptor_CompCurve cc(*pWire, Standard_True);
			GeomAbs_Shape continuity = cc.Continuity();
			int numIntervals = cc.NbIntervals(continuity);
			TColStd_Array1OfReal res(1, numIntervals+1);
			cc.Intervals(res, GeomAbs_C0);
			List<double>^ intervals = gcnew List<double>(numIntervals+1);
			for (Standard_Integer i = 1; i <= numIntervals; i++)
				intervals->Add(res.Value(i));
			intervals->Add(cc.LastParameter());
			return intervals;
		}

		array<ContourVertex>^ XbimWire::Contour()
		{
			if (!IsValid) return gcnew array<ContourVertex>(0);
			TopoDS_Wire ccWire = *pWire;
			int i = 0;
			TopTools_IndexedMapOfShape map;
			TopExp::MapShapes(ccWire, TopAbs_VERTEX, map);
			if (map.Extent() == 0) return gcnew array<ContourVertex>(0);
			array<ContourVertex>^ contour = gcnew array<ContourVertex>(map.Extent());
			i = 0;
			for (BRepTools_WireExplorer exp(ccWire); exp.More(); exp.Next())
			{
				gp_Pnt p = BRep_Tool::Pnt(exp.CurrentVertex());
				contour[i].Position.X = (float)p.X();
				contour[i].Position.Y = (float)p.Y();
				contour[i].Position.Z = (float)p.Z();
				i++;
			}	
			return contour;
		}


		List<XbimPoint3D>^ XbimWire::IntervalPoints::get()
		{
			if (!IsValid) return gcnew List<XbimPoint3D>(1);
			BRepAdaptor_CompCurve cc(*pWire, Standard_True);
			GeomAbs_Shape continuity = cc.Continuity();
			int numIntervals = cc.NbIntervals(continuity);
			TColStd_Array1OfReal res(1, numIntervals + 1);
			cc.Intervals(res, GeomAbs_C0);
			List<XbimPoint3D>^ intervals = gcnew List<XbimPoint3D>(numIntervals + 1);
			for (Standard_Integer i = 1; i <= numIntervals; i++)
			{
				gp_Pnt p = cc.Value(res.Value(i));
				intervals->Add(XbimPoint3D(p.X(), p.Y(), p.Z()));
			}
			if (!IsClosed)
			{
				gp_Pnt l = cc.Value(cc.LastParameter());
				intervals->Add(XbimPoint3D(l.X(), l.Y(), l.Z()));
			}
			return intervals;
		}

#pragma endregion

#pragma region Parameterised profiles

		void XbimWire::Init(IfcProfileDef ^ profile)
		{
			if (dynamic_cast<IfcArbitraryClosedProfileDef^>(profile))
				return Init((IfcArbitraryClosedProfileDef^)profile);
			else if (dynamic_cast<IfcParameterizedProfileDef^>(profile))
				return Init((IfcParameterizedProfileDef^)profile);
			else if (dynamic_cast<IfcDerivedProfileDef^>(profile))
				return Init((IfcDerivedProfileDef^)profile);
			else if (dynamic_cast<IfcArbitraryOpenProfileDef^>(profile))
				return Init((IfcArbitraryOpenProfileDef^)profile);
			else
				XbimGeometryCreator::logger->ErrorFormat("WW018: Profile definition {0} is not implemented", profile->GetType()->Name);

		}

		void XbimWire::Init(IfcDerivedProfileDef ^ profile)
		{
			Init(profile->ParentProfile);
			gp_Trsf trsf = XbimGeomPrim::ToTransform(profile->Operator);
			pWire->Move(TopLoc_Location(trsf));
			
		}

		void XbimWire::Init(IfcParameterizedProfileDef ^ profile)
		{
			if (dynamic_cast<IfcRectangleHollowProfileDef^>(profile))
				return Init((IfcRectangleProfileDef^)profile);
			else if (dynamic_cast<IfcRectangleProfileDef^>(profile))
				return Init((IfcRectangleProfileDef^)profile);
			else if (dynamic_cast<IfcCircleHollowProfileDef^>(profile))
				return Init((IfcCircleHollowProfileDef^)profile);
			else if (dynamic_cast<IfcCircleProfileDef^>(profile))
				return Init((IfcCircleProfileDef^)profile);
			else if (dynamic_cast<IfcLShapeProfileDef^>(profile))
				return Init((IfcLShapeProfileDef^)profile);
			else if (dynamic_cast<IfcUShapeProfileDef^>(profile))
				return Init((IfcUShapeProfileDef^)profile);
			else if (dynamic_cast<IfcIShapeProfileDef^>(profile))
				return Init((IfcIShapeProfileDef^)profile);
			else if (dynamic_cast<IfcCShapeProfileDef^>(profile))
				return Init((IfcCShapeProfileDef^)profile);
			else if (dynamic_cast<IfcTShapeProfileDef^>(profile))
				return Init((IfcTShapeProfileDef^)profile);
			else if (dynamic_cast<IfcZShapeProfileDef^>(profile))
				return Init((IfcZShapeProfileDef^)profile);
			else if (dynamic_cast<IfcCraneRailFShapeProfileDef^>(profile))
				return Init((IfcCraneRailFShapeProfileDef^)profile);
			else if (dynamic_cast<IfcCraneRailAShapeProfileDef^>(profile))
				return Init((IfcCraneRailAShapeProfileDef^)profile);
			else if (dynamic_cast<IfcEllipseProfileDef^>(profile))
				return Init((IfcEllipseProfileDef^)profile);
			else 
				XbimGeometryCreator::logger->ErrorFormat("WW019: Profile definition {0} is not implemented", profile->GetType()->Name);
		}

		//Builds a wire from a CircleProfileDef
		void XbimWire::Init(IfcCircleProfileDef ^ circProfile)
		{
			if (dynamic_cast<IfcCircleHollowProfileDef^>(circProfile))
			{
				XbimGeometryCreator::logger->ErrorFormat("WW020: IfcCircleHollowProfileDef #{0} cannot be created as a wire, call the XbimFace method", circProfile->EntityLabel);
				return;
			}
			IfcAxis2Placement2D^ ax2 = (IfcAxis2Placement2D^)circProfile->Position;
			gp_Ax2 gpax2(gp_Pnt(ax2->Location->X, ax2->Location->Y, 0), gp_Dir(0, 0, 1), gp_Dir(ax2->P[0].X, ax2->P[0].Y, 0.));
			gp_Circ gc(gpax2, circProfile->Radius);
			Handle(Geom_Circle) hCirc = GC_MakeCircle(gc);
			TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(hCirc);
			BRep_Builder b;
			TopoDS_Wire wire;
			b.MakeWire(wire);
			b.Add(wire, edge);
			ShapeFix_ShapeTolerance FTol;
			FTol.SetTolerance(wire, circProfile->ModelOf->ModelFactors->Precision, TopAbs_VERTEX);
			pWire = new TopoDS_Wire();
			*pWire = wire;
		}


		void XbimWire::Init(IfcRectangleProfileDef^ rectProfile)
		{
			if (rectProfile->XDim <= 0 || rectProfile->YDim <= 0)
			{
				XbimGeometryCreator::logger->WarnFormat("WW021:Invalid IfcRectangleProfileDef: #{0}, XDim = {1}, YDim = {2}. Face discarded", rectProfile->EntityLabel, rectProfile->XDim, rectProfile->YDim);
			}
			else
			{
				if (dynamic_cast<IfcRectangleHollowProfileDef^>(rectProfile))
				{
					XbimGeometryCreator::logger->ErrorFormat("WW022: IfcRectangleHollowProfileDef #{0} cannot be created as a wire, call the XbimFace method", rectProfile->EntityLabel);
					return;
				}
				else
				{
					double xOff = rectProfile->XDim / 2;
					double yOff = rectProfile->YDim / 2;
					double precision = rectProfile->ModelOf->ModelFactors->Precision;
					gp_Pnt bl(-xOff, -yOff, 0);
					gp_Pnt br(xOff, -yOff, 0);
					gp_Pnt tr(xOff, yOff, 0);
					gp_Pnt tl(-xOff, yOff, 0);
					//make the vertices
					BRep_Builder builder;
					TopoDS_Vertex vbl, vbr, vtr, vtl;
					builder.MakeVertex(vbl, bl, precision);
					builder.MakeVertex(vbr, br, precision);
					builder.MakeVertex(vtr, tr, precision);
					builder.MakeVertex(vtl, tl, precision);
					//make the edges
					TopoDS_Wire wire;
					builder.MakeWire(wire);
					builder.Add(wire, BRepBuilderAPI_MakeEdge(vbl, vbr));
					builder.Add(wire, BRepBuilderAPI_MakeEdge(vbr, vtr));
					builder.Add(wire, BRepBuilderAPI_MakeEdge(vtr, vtl));
					builder.Add(wire, BRepBuilderAPI_MakeEdge(vtl, vbl));

					//apply the position transformation
					wire.Move(XbimGeomPrim::ToLocation(rectProfile->Position));
					pWire = new TopoDS_Wire();
					*pWire = wire;
				}
			}
			
		}
		
		//Creates a rectangle approx to the max bounds / 2
		void XbimWire::Init(double precision)
		{
			
			double xOff = 2e10;
			double yOff = 2e10;
					
			gp_Pnt bl(-xOff, -yOff, 0);
			gp_Pnt br(xOff, -yOff, 0);
			gp_Pnt tr(xOff, yOff, 0);
			gp_Pnt tl(-xOff, yOff, 0);
			//make the vertices
			BRep_Builder builder;
			TopoDS_Vertex vbl, vbr, vtr, vtl;
			builder.MakeVertex(vbl, bl, precision);
			builder.MakeVertex(vbr, br, precision);
			builder.MakeVertex(vtr, tr, precision);
			builder.MakeVertex(vtl, tl, precision);
			//make the edges
			TopoDS_Wire wire;
			builder.MakeWire(wire);
			builder.Add(wire, BRepBuilderAPI_MakeEdge(vbl, vbr));
			builder.Add(wire, BRepBuilderAPI_MakeEdge(vbr, vtr));
			builder.Add(wire, BRepBuilderAPI_MakeEdge(vtr, vtl));
			builder.Add(wire, BRepBuilderAPI_MakeEdge(vtl, vbl));

			//apply the position transformation
			pWire = new TopoDS_Wire();
			*pWire = wire;		
		}

		void XbimWire::Init(IfcLShapeProfileDef ^ profile)
		{
			bool detailed = profile->ModelOf->ModelFactors->ProfileDefLevelOfDetail == 1;
			double dY = profile->Depth / 2;
			double dX;
			if (profile->Width.HasValue)
				dX = profile->Width.Value / 2;
			else
				dX = dY;
			double tF = profile->Thickness;
			gp_Pnt p1(-dX, dY, 0);
			gp_Pnt p2(-dX + tF, dY, 0);
			gp_Pnt p3(-dX + tF, -dY + tF, 0);
			if (detailed && profile->LegSlope.HasValue)
			{
				double radConv = profile->ModelOf->ModelFactors->AngleToRadiansConversionFactor;
				p3.SetX(p3.X() + (((dY * 2) - tF)* Math::Tan(profile->LegSlope.Value*radConv)));
				p3.SetY(p3.Y() + (((dX * 2) - tF)* Math::Tan(profile->LegSlope.Value*radConv)));
			}
			gp_Pnt p4(dX, -dY + tF, 0);	
			gp_Pnt p5(dX, -dY, 0);
			gp_Pnt p6(-dX, -dY, 0);

			BRepBuilderAPI_MakeWire wireMaker;
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p1, p2));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p2, p3));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p3, p4));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p4, p5));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p5, p6));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p6, p1));
			TopoDS_Wire wire = wireMaker.Wire();
			wire.Closed(Standard_True);
			if (detailed && (profile->EdgeRadius.HasValue || profile->FilletRadius.HasValue)) //consider fillets
			{				
				BRepBuilderAPI_MakeFace faceMaker(wire, true);
				BRepFilletAPI_MakeFillet2d filleter(faceMaker.Face());
				double fRad = profile->FilletRadius.Value;
				int i = 1;
				for (BRepTools_WireExplorer exp(wire); exp.More(); exp.Next())
				{
					if ((i == 2 || i== 4) && profile->EdgeRadius.HasValue)
						filleter.AddFillet(exp.CurrentVertex(), profile->EdgeRadius.Value);
					else if (i == 3 && profile->FilletRadius.HasValue)
						filleter.AddFillet(exp.CurrentVertex(), profile->FilletRadius.Value);
					i++;
				}
				filleter.Build();
				if (filleter.IsDone())
				{
					TopoDS_Shape shape = filleter.Shape();
					for (TopExp_Explorer exp(shape, TopAbs_WIRE); exp.More(); exp.Next()) //just take the first wire
					{
						wire = TopoDS::Wire(exp.Current());
						break;
					}
				}
			}

			wire.Move(XbimGeomPrim::ToLocation(profile->Position));
			if (profile->CentreOfGravityInX.HasValue || profile->CentreOfGravityInY.HasValue)
			{
				double transX = 0;
				double transY = 0;
				if (profile->CentreOfGravityInX.HasValue) transX = profile->CentreOfGravityInX.Value;
				if (profile->CentreOfGravityInY.HasValue) transY = profile->CentreOfGravityInY.Value;
				gp_Vec v(transX, transY, 0);
				gp_Trsf t;
				t.SetTranslation(v);
				wire.Move(t);
			}
			ShapeFix_ShapeTolerance FTol;
			FTol.SetTolerance(wire, profile->ModelOf->ModelFactors->Precision, TopAbs_VERTEX);
			pWire = new TopoDS_Wire();
			*pWire = wire;
		}

		
		void XbimWire::Init(IfcUShapeProfileDef ^ profile)
		{
			bool detailed = profile->ModelOf->ModelFactors->ProfileDefLevelOfDetail == 1;
			double dX = profile->FlangeWidth / 2;
			double dY = profile->Depth / 2;
			double tF = profile->FlangeThickness;
			double tW = profile->WebThickness;
			
			gp_Pnt p1(-dX, dY, 0);
			gp_Pnt p2(dX, dY, 0);
			gp_Pnt p3(dX, dY - tF, 0);
			gp_Pnt p4(-dX + tW, dY - tF, 0);
			
			gp_Pnt p5(-dX + tW, -dY + tF, 0);
			gp_Pnt p6(dX, -dY + tF, 0);
			gp_Pnt p7(dX, -dY, 0);
			gp_Pnt p8(-dX, -dY, 0);

			if (detailed && profile->FlangeSlope.HasValue)
			{
				double radConv = profile->ModelOf->ModelFactors->AngleToRadiansConversionFactor;
				p4.SetY(p4.Y() - (((dX * 2) - tW)* Math::Tan(profile->FlangeSlope.Value*radConv)));
				p5.SetY(p5.Y() + (((dX * 2) - tW)* Math::Tan(profile->FlangeSlope.Value*radConv)));

			}
			BRepBuilderAPI_MakeWire wireMaker;
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p1, p2));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p2, p3));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p3, p4));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p4, p5));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p5, p6));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p6, p7));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p7, p8));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p8, p1));
			TopoDS_Wire wire = wireMaker.Wire();
			wire.Closed(Standard_True);

			if (detailed && (profile->EdgeRadius.HasValue || profile->FilletRadius.HasValue)) //consider fillets
			{
				BRepBuilderAPI_MakeFace faceMaker(wire, true);
				BRepFilletAPI_MakeFillet2d filleter(faceMaker.Face());
				double fRad = profile->FilletRadius.Value;
				int i = 1;
				for (BRepTools_WireExplorer exp(wire); exp.More(); exp.Next())
				{
					if ((i == 3 || i == 6) && profile->EdgeRadius.HasValue)
						filleter.AddFillet(exp.CurrentVertex(), profile->EdgeRadius.Value);
					else if ((i == 4 || i== 5) && profile->FilletRadius.HasValue)
						filleter.AddFillet(exp.CurrentVertex(), profile->FilletRadius.Value);
					i++;
				}
				filleter.Build();
				if (filleter.IsDone())
				{
					TopoDS_Shape shape = filleter.Shape();
					for (TopExp_Explorer exp(shape, TopAbs_WIRE); exp.More(); exp.Next()) //just take the first wire
					{
						wire = TopoDS::Wire(exp.Current());
						break;
					}
				}
			}

			wire.Move(XbimGeomPrim::ToLocation(profile->Position));
			if (profile->CentreOfGravityInX.HasValue)
			{
				gp_Vec v(profile->CentreOfGravityInX.Value, 0, 0);
				gp_Trsf t;
				t.SetTranslation(v);
				wire.Move(t);
			}
			ShapeFix_ShapeTolerance FTol;
			FTol.SetTolerance(wire, profile->ModelOf->ModelFactors->Precision, TopAbs_VERTEX);
			pWire = new TopoDS_Wire();
			*pWire = wire;
		}

		// SRL: Builds a wire from a composite IfcCraneRailFShapeProfileDef
		//TODO: SRL: Support for fillet radii needs to be added, nb set the hascurves=true when added
		// and note too that this will decrease performance due to use of OCC for triangulation
		//NB. This is untested as we haven't enountered one yet
		void XbimWire::Init(IfcCraneRailFShapeProfileDef ^ profile)
		{
			double dX = profile->HeadWidth / 2;
			double dY = profile->OverallHeight / 2;
			double hd2 = profile->HeadDepth2;
			double hd3 = profile->HeadDepth3;
			double tW = profile->WebThickness;
			double bd1 = profile->BaseDepth1;
			double bd2 = profile->BaseDepth2;

			gp_Pnt p1(-dX, dY, 0);
			gp_Pnt p2(dX, dY, 0);
			gp_Pnt p3(dX, dY - hd3, 0);
			gp_Pnt p4(tW / 2, dY - hd2, 0);
			gp_Pnt p5(tW / 2, -dY + bd2, 0);
			gp_Pnt p6(dX, -dY + bd1, 0);
			gp_Pnt p7(dX, -dY, 0);
			gp_Pnt p8(-dX, -dY, 0);
			gp_Pnt p9(-dX, -dY + bd1, 0);
			gp_Pnt p10(-tW / 2, -dY + bd2, 0);
			gp_Pnt p11(tW / 2, dY - hd2, 0);
			gp_Pnt p12(-dX, dY - hd3, 0);

			BRepBuilderAPI_MakeWire wireMaker;

			wireMaker.Add(BRepBuilderAPI_MakeEdge(p1, p2));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p2, p3));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p3, p4));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p4, p5));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p5, p6));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p6, p7));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p7, p8));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p8, p9));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p9, p10));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p10, p11));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p11, p12));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p12, p1));
			TopoDS_Wire wire = wireMaker.Wire();
			wire.Move(XbimGeomPrim::ToLocation(profile->Position));
			ShapeFix_ShapeTolerance FTol;
			FTol.SetTolerance(wire, profile->ModelOf->ModelFactors->Precision, TopAbs_VERTEX);
			pWire = new TopoDS_Wire();
			*pWire = wire;
		}
		
		// SRL: Builds a wire from a composite IfcCraneRailAShapeProfileDef
		//TODO: SRL: Support for fillet radii needs to be added, nb set the hascurves=true when added
		// and note too that this will decrease performance due to use of OCC for triangulation
		//NB. This is untested as we haven't enountered one yet
		void XbimWire::Init(IfcCraneRailAShapeProfileDef ^ profile)
		{
			double bW = profile->HeadWidth / 2;
			double dY = profile->OverallHeight / 2;
			double hd2 = profile->HeadDepth2;
			double hd3 = profile->HeadDepth3;
			double tW = profile->WebThickness;
			double bd1 = profile->BaseDepth1;
			double bd2 = profile->BaseDepth2;
			double bd3 = profile->BaseDepth3;
			double bw2 = profile->BaseWidth2 / 2;
			double bw4 = profile->BaseWidth4 / 2;

			gp_Pnt p1(-bw4, dY, 0);
			gp_Pnt p2(bw4, dY, 0);
			gp_Pnt p3(bw4, dY - hd3, 0);
			gp_Pnt p4(tW / 2, dY - hd2, 0);
			gp_Pnt p5(tW / 2, -dY + bd2, 0);
			gp_Pnt p6(bw4, -dY + bd3, 0);
			gp_Pnt p7(bw2, -dY + bd1, 0);
			gp_Pnt p8(bw2, -dY, 0);
			gp_Pnt p9(-bw2, -dY, 0);
			gp_Pnt p10(-bw2, -dY + bd1, 0);
			gp_Pnt p11(-bw4, -dY + bd3, 0);
			gp_Pnt p12(tW / 2, -dY + bd2, 0);
			gp_Pnt p13(tW / 2, dY - hd2, 0);
			gp_Pnt p14(-bw4, dY - hd3, 0);

			BRepBuilderAPI_MakeWire wireMaker;

			wireMaker.Add(BRepBuilderAPI_MakeEdge(p1, p2));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p2, p3));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p3, p4));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p4, p5));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p5, p6));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p6, p7));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p7, p8));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p8, p9));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p9, p10));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p10, p11));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p11, p12));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p12, p13));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p13, p14));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p14, p1));
			TopoDS_Wire wire = wireMaker.Wire();
			wire.Move(XbimGeomPrim::ToLocation(profile->Position));
			ShapeFix_ShapeTolerance FTol;
			FTol.SetTolerance(wire, profile->ModelOf->ModelFactors->Precision, TopAbs_VERTEX);
			pWire = new TopoDS_Wire();
			*pWire = wire;
		}
		
		// SRL: Builds a wire from a composite IfcEllipseProfileDef
		//TODO: SRL: Support for fillet radii needs to be added, nb set the hascurves=true when added
		// and note too that this will decrease performance due to use of OCC for triangulation
		//NB. This is untested as we haven't enountered one yet
		void XbimWire::Init(IfcEllipseProfileDef ^ profile)
		{

			IfcAxis2Placement2D^ ax2 = (IfcAxis2Placement2D^)profile->Position;
			gp_Ax2 gpax2(gp_Pnt(ax2->Location->X, ax2->Location->Y, 0), gp_Dir(0, 0, 1), gp_Dir(ax2->P[0].X, ax2->P[0].Y, 0.));
			double semiAx1 = profile->SemiAxis1;
			double semiAx2 = profile->SemiAxis2;
			if (semiAx1 <= 0)
			{
				XbimModelFactors^ mf = profile->ModelOf->ModelFactors;
				semiAx1 = mf->OneMilliMetre;
				//	throw gcnew XbimGeometryException("Illegal Ellipse Semi Axix, for IfcEllipseProfileDef, must be greater than 0, in entity #" + profile->EntityLabel);
			}
			if (semiAx2 <= 0)
			{
				XbimModelFactors^ mf = profile->ModelOf->ModelFactors;
				semiAx2 = mf->OneMilliMetre;
				//	throw gcnew XbimGeometryException("Illegal Ellipse Semi Axix, for IfcEllipseProfileDef, must be greater than 0, in entity #" + profile->EntityLabel);
			}
			gp_Elips gc(gpax2, semiAx1, semiAx2);
			Handle(Geom_Ellipse) hellipse = GC_MakeEllipse(gc);
			TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(hellipse);
			BRep_Builder b;
			TopoDS_Wire wire;
			b.MakeWire(wire);
			b.Add(wire, edge);
			ShapeFix_ShapeTolerance FTol;
			FTol.SetTolerance(wire, profile->ModelOf->ModelFactors->Precision, TopAbs_VERTEX);
			pWire = new TopoDS_Wire();
			*pWire = wire;
		}

		
		//TODO: SRL: Support for fillet radii needs to be added, nb set the hascurves=true when added
		// and note too that this will decrease performance due to use of OCC for triangulation
		void XbimWire::Init(IfcIShapeProfileDef ^ profile)
		{
			bool detailed = profile->ModelOf->ModelFactors->ProfileDefLevelOfDetail == 1;
			double dX = profile->OverallWidth / 2;
			double dY = profile->OverallDepth / 2;
			double tF = profile->FlangeThickness;
			double tW = profile->WebThickness;


			gp_Pnt p1(-dX, dY, 0);
			gp_Pnt p2(dX, dY, 0);
			gp_Pnt p3(dX, dY - tF, 0);
			gp_Pnt p4(tW / 2, dY - tF, 0);
			gp_Pnt p5(tW / 2, -dY + tF, 0);
			gp_Pnt p6(dX, -dY + tF, 0);
			gp_Pnt p7(dX, -dY, 0);
			gp_Pnt p8(-dX, -dY, 0);
			gp_Pnt p9(-dX, -dY + tF, 0);
			gp_Pnt p10(-tW / 2, -dY + tF, 0);
			gp_Pnt p11(-tW / 2, dY - tF, 0);
			gp_Pnt p12(-dX, dY - tF, 0);

			TopoDS_Vertex v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12;
			BRep_Builder b;
			double t = profile->ModelOf->ModelFactors->Precision;
			b.MakeVertex(v1, p1, t);
			b.MakeVertex(v2, p2, t);
			b.MakeVertex(v3, p3, t);
			b.MakeVertex(v4, p4, t);
			b.MakeVertex(v5, p5, t);
			b.MakeVertex(v6, p6, t);
			b.MakeVertex(v7, p7, t);
			b.MakeVertex(v8, p8, t);
			b.MakeVertex(v9, p9, t);
			b.MakeVertex(v10, p10, t);
			b.MakeVertex(v11, p11, t);
			b.MakeVertex(v12, p12, t);

			BRepBuilderAPI_MakePolygon polyMaker;
			polyMaker.Add(v1);
			polyMaker.Add(v2);
			polyMaker.Add(v3);
			polyMaker.Add(v4);
			polyMaker.Add(v5);
			polyMaker.Add(v6);
			polyMaker.Add(v7);
			polyMaker.Add(v8);
			polyMaker.Add(v9);
			polyMaker.Add(v10);
			polyMaker.Add(v11);
			polyMaker.Add(v12);
			polyMaker.Close();
			TopoDS_Wire wire = polyMaker.Wire();
			wire.Move(XbimGeomPrim::ToLocation(profile->Position));
			if (detailed && profile->FilletRadius.HasValue)
			{
				BRepBuilderAPI_MakeFace faceMaker(wire, true);
				BRepFilletAPI_MakeFillet2d filleter(faceMaker.Face());
				double fRad = profile->FilletRadius.Value;
				int i = 1;
				for (BRepTools_WireExplorer exp(wire); exp.More(); exp.Next())
				{
					if (i == 4 || i == 5 || i == 10 || i == 11)
						filleter.AddFillet(exp.CurrentVertex(), fRad);
					i++;

				}
				filleter.Build();
				if (filleter.IsDone())
				{
					TopoDS_Shape shape = filleter.Shape();
					for (TopExp_Explorer exp(shape, TopAbs_WIRE); exp.More(); exp.Next()) //just take the first wire
					{
						wire = TopoDS::Wire(exp.Current());
						break;
					}
				}
			}
			
			pWire = new TopoDS_Wire();
			*pWire = wire;
			
		}

	
		void XbimWire::Init(IfcZShapeProfileDef ^ profile)
		{
			bool detailed = profile->ModelOf->ModelFactors->ProfileDefLevelOfDetail == 1;
			double dX = profile->FlangeWidth;
			double dY = profile->Depth / 2;
			double tF = profile->FlangeThickness;
			double tW = profile->WebThickness;


			gp_Pnt p1(-dX + (tW / 2), dY, 0);
			gp_Pnt p2(tW / 2, dY, 0);
			gp_Pnt p3(tW / 2, -dY + tF, 0);
			gp_Pnt p4(dX - tW / 2, -dY + tF, 0);
			gp_Pnt p5(dX - tW / 2, -dY, 0);
			gp_Pnt p6(-tW / 2, -dY, 0);
			gp_Pnt p7(-tW / 2, dY - tF, 0);
			gp_Pnt p8(-dX + (tW / 2), dY - tF, 0);


			BRepBuilderAPI_MakeWire wireMaker;

			wireMaker.Add(BRepBuilderAPI_MakeEdge(p1, p2));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p2, p3));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p3, p4));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p4, p5));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p5, p6));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p6, p7));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p7, p8));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p8, p1));

			TopoDS_Wire wire = wireMaker.Wire();

			if (detailed && (profile->FilletRadius.HasValue||profile->EdgeRadius.HasValue))
			{
				BRepBuilderAPI_MakeFace faceMaker(wire, true);
				BRepFilletAPI_MakeFillet2d filleter(faceMaker.Face());
				int i = 1;
				for (BRepTools_WireExplorer exp(wire); exp.More(); exp.Next())
				{
					if ((i == 3 || i == 7) && profile->FilletRadius.HasValue)
						filleter.AddFillet(exp.CurrentVertex(), profile->FilletRadius.Value);
					else if ((i == 4 || i == 8) && profile->EdgeRadius.HasValue)
						filleter.AddFillet(exp.CurrentVertex(), profile->EdgeRadius.Value);
					i++;
				}
				filleter.Build();
				if (filleter.IsDone())
				{
					TopoDS_Shape shape = filleter.Shape();
					for (TopExp_Explorer exp(shape, TopAbs_WIRE); exp.More(); exp.Next()) //just take the first wire
					{
						wire = TopoDS::Wire(exp.Current());
						break;
					}
				}
			}
			wire.Move(XbimGeomPrim::ToLocation(profile->Position));
			ShapeFix_ShapeTolerance FTol;
			FTol.SetTolerance(wire, profile->ModelOf->ModelFactors->Precision, TopAbs_VERTEX);
			pWire = new TopoDS_Wire();
			*pWire = wire;
		}

		// SRL: Builds a wire from a composite IfcCShapeProfileDef
		//TODO: SRL: Support for fillet radii needs to be added, nb set the hascurves=true when added
		// and note too that this will decrease performance due to use of OCC for triangulation
		void XbimWire::Init(IfcCShapeProfileDef ^ profile)
		{
			bool detailed = profile->ModelOf->ModelFactors->ProfileDefLevelOfDetail == 1;
			double dX = profile->Width / 2;
			double dY = profile->Depth / 2;
			double dG = profile->Girth;
			double tW = profile->WallThickness;

			if (tW <= 0)
			{
				XbimModelFactors^ mf = profile->ModelOf->ModelFactors;
				tW = mf->OneMilliMetre * 3;
				XbimGeometryCreator::logger->WarnFormat("WW023: Illegal wall thickness for IfcCShapeProfileDef, it must be greater than 0, in entity #{0}. Adjusted to be 3mm thick", profile->EntityLabel);
			}
			BRepBuilderAPI_MakeWire wireMaker;
			if (dG>0)
			{
				gp_Pnt p1(-dX, dY, 0);
				gp_Pnt p2(dX, dY, 0);
				gp_Pnt p3(dX, dY - dG, 0);
				gp_Pnt p4(dX - tW, dY - dG, 0);
				gp_Pnt p5(dX - tW, dY - tW, 0);
				gp_Pnt p6(-dX + tW, dY - tW, 0);
				gp_Pnt p7(-dX + tW, -dY + tW, 0);
				gp_Pnt p8(dX - tW, -dY + tW, 0);
				gp_Pnt p9(dX - tW, -dY + dG, 0);
				gp_Pnt p10(dX, -dY + dG, 0);
				gp_Pnt p11(dX, -dY, 0);
				gp_Pnt p12(-dX, -dY, 0);


				wireMaker.Add(BRepBuilderAPI_MakeEdge(p1, p2));
				wireMaker.Add(BRepBuilderAPI_MakeEdge(p2, p3));
				wireMaker.Add(BRepBuilderAPI_MakeEdge(p3, p4));
				wireMaker.Add(BRepBuilderAPI_MakeEdge(p4, p5));
				wireMaker.Add(BRepBuilderAPI_MakeEdge(p5, p6));
				wireMaker.Add(BRepBuilderAPI_MakeEdge(p6, p7));
				wireMaker.Add(BRepBuilderAPI_MakeEdge(p7, p8));
				wireMaker.Add(BRepBuilderAPI_MakeEdge(p8, p9));
				wireMaker.Add(BRepBuilderAPI_MakeEdge(p9, p10));
				wireMaker.Add(BRepBuilderAPI_MakeEdge(p10, p11));
				wireMaker.Add(BRepBuilderAPI_MakeEdge(p11, p12));
				wireMaker.Add(BRepBuilderAPI_MakeEdge(p12, p1));
			}
			else
			{
				gp_Pnt p1(-dX, dY, 0);
				gp_Pnt p2(dX, dY, 0);
				gp_Pnt p5(dX, dY - tW, 0);
				gp_Pnt p6(-dX + tW, dY - tW, 0);
				gp_Pnt p7(-dX + tW, -dY + tW, 0);
				gp_Pnt p8(dX, -dY + tW, 0);
				gp_Pnt p11(dX, -dY, 0);
				gp_Pnt p12(-dX, -dY, 0);

				wireMaker.Add(BRepBuilderAPI_MakeEdge(p1, p2));
				wireMaker.Add(BRepBuilderAPI_MakeEdge(p2, p5));
				wireMaker.Add(BRepBuilderAPI_MakeEdge(p5, p6));
				wireMaker.Add(BRepBuilderAPI_MakeEdge(p6, p7));
				wireMaker.Add(BRepBuilderAPI_MakeEdge(p7, p8));
				wireMaker.Add(BRepBuilderAPI_MakeEdge(p8, p11));
				wireMaker.Add(BRepBuilderAPI_MakeEdge(p11, p12));
				wireMaker.Add(BRepBuilderAPI_MakeEdge(p12, p1));
			}

			TopoDS_Wire wire = wireMaker.Wire();

			if (detailed && profile->InternalFilletRadius.HasValue ) //consider fillets
			{
				BRepBuilderAPI_MakeFace faceMaker(wire, true);
				BRepFilletAPI_MakeFillet2d filleter(faceMaker.Face());
				double iRad = profile->InternalFilletRadius.Value;
				double oRad = iRad + tW;
				int i = 1;
				for (BRepTools_WireExplorer exp(wire); exp.More(); exp.Next())
				{
					if (i == 1 || i == 2 || i == 11 || i == 12)
						filleter.AddFillet(exp.CurrentVertex(),oRad);
					else if (i == 5 || i == 6|| i == 7|| i == 8)
						filleter.AddFillet(exp.CurrentVertex(), iRad);
					i++;
				}
				filleter.Build();
				if (filleter.IsDone())
				{
					TopoDS_Shape shape = filleter.Shape();
					for (TopExp_Explorer exp(shape, TopAbs_WIRE); exp.More(); exp.Next()) //just take the first wire
					{
						wire = TopoDS::Wire(exp.Current());
						break;
					}
				}
			}
			wire.Move(XbimGeomPrim::ToLocation(profile->Position));
			if (profile->CentreOfGravityInX.HasValue)
			{
				gp_Vec v(profile->CentreOfGravityInX.Value, 0, 0);
				gp_Trsf t;
				t.SetTranslation(v);
				wire.Move(t);
			}
			ShapeFix_ShapeTolerance FTol;
			FTol.SetTolerance(wire, profile->ModelOf->ModelFactors->Precision, TopAbs_VERTEX);
			pWire = new TopoDS_Wire();
			*pWire = wire;
		}
		
		// SRL: Builds a wire from a composite IfcTShapeProfileDef
		//TODO: SRL: Support for fillet radii needs to be added, nb set the hascurves=true when added
		// and note too that this will decrease performance due to use of OCC for triangulation
		void XbimWire::Init(IfcTShapeProfileDef ^ profile)
		{
			bool detailed = profile->ModelOf->ModelFactors->ProfileDefLevelOfDetail == 1;
			double dX = profile->FlangeWidth / 2;
			double dY = profile->Depth / 2;
			double tF = profile->FlangeThickness;
			double tW = profile->WebThickness;

			gp_Pnt p1(-dX, dY, 0);
			gp_Pnt p2(dX, dY, 0);
			gp_Pnt p3(dX, dY - tF, 0);
			gp_Pnt p4(tW / 2, dY - tF, 0);
			gp_Pnt p5(tW / 2, -dY, 0);
			gp_Pnt p6(-tW / 2, -dY, 0);
			gp_Pnt p7(-tW / 2, dY - tF, 0);
			gp_Pnt p8(-dX, dY - tF, 0);
			double radConv = profile->ModelOf->ModelFactors->AngleToRadiansConversionFactor;
			if (detailed && (profile->FlangeSlope.HasValue || profile->WebSlope.HasValue))
			{
				double fSlope = 0;
				if (profile->FlangeSlope.HasValue) fSlope = profile->FlangeSlope.Value;
				double wSlope = 0;
				if (profile->WebSlope.HasValue) wSlope = profile->WebSlope.Value;
				double bDiv4 = profile->FlangeWidth / 4;
				
				p3.SetY(p3.Y() + (bDiv4 * Math::Tan(fSlope*radConv)));
				p8.SetY(p8.Y() + (bDiv4 * Math::Tan(fSlope*radConv)));
				
				
				gp_Lin2d flangeLine(gp_Pnt2d(bDiv4, dY - tF), gp_Dir2d(1, Math::Tan(fSlope*radConv)));
				gp_Lin2d webLine(gp_Pnt2d(tW / 2.0, 0), gp_Dir2d(Math::Tan(wSlope*radConv), 1));
				IntAna2d_AnaIntersection intersector(flangeLine, webLine);
				const IntAna2d_IntPoint& intersectPoint = intersector.Point(1);
				gp_Pnt2d p2d = intersectPoint.Value();

				p4.SetX(p2d.X());
				p4.SetY(p2d.Y());
				p7.SetX(-p2d.X());
				p7.SetY(p2d.Y());

				p5.SetX(p5.X() - (dY * Math::Tan(wSlope*radConv)));
				p6.SetX(p6.X() + (dY * Math::Tan(wSlope*radConv)));
			}

			BRepBuilderAPI_MakeWire wireMaker;

			wireMaker.Add(BRepBuilderAPI_MakeEdge(p1, p2));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p2, p3));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p3, p4));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p4, p5));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p5, p6));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p6, p7));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p7, p8));
			wireMaker.Add(BRepBuilderAPI_MakeEdge(p8, p1));
			TopoDS_Wire wire = wireMaker.Wire();
			if (detailed && (profile->FlangeEdgeRadius.HasValue || profile->FilletRadius.HasValue ||profile->WebEdgeRadius.HasValue)) //consider fillets
			{
				BRepBuilderAPI_MakeFace faceMaker(wire, true);
				BRepFilletAPI_MakeFillet2d filleter(faceMaker.Face());
				int i = 1;
				for (BRepTools_WireExplorer exp(wire); exp.More(); exp.Next())
				{
					if ((i == 3 || i == 8) && profile->FlangeEdgeRadius.HasValue)
						filleter.AddFillet(exp.CurrentVertex(), profile->FlangeEdgeRadius.Value);
					else if ((i == 4 || i == 7) && profile->FilletRadius.HasValue)
						filleter.AddFillet(exp.CurrentVertex(), profile->FilletRadius.Value);
					else if ((i == 5 || i == 6) && profile->WebEdgeRadius.HasValue)
						filleter.AddFillet(exp.CurrentVertex(), profile->WebEdgeRadius.Value);
					i++;
				}
				filleter.Build();
				if (filleter.IsDone())
				{
					TopoDS_Shape shape = filleter.Shape();
					for (TopExp_Explorer exp(shape, TopAbs_WIRE); exp.More(); exp.Next()) //just take the first wire
					{
						wire = TopoDS::Wire(exp.Current());
						break;
					}
				}
			}

			wire.Move(XbimGeomPrim::ToLocation(profile->Position));
			if (profile->CentreOfGravityInY.HasValue)
			{
				gp_Vec v( 0, profile->CentreOfGravityInY.Value, 0);
				gp_Trsf t;
				t.SetTranslation(v);
				wire.Move(t);
			}
			ShapeFix_ShapeTolerance FTol;
			FTol.SetTolerance(wire, profile->ModelOf->ModelFactors->Precision, TopAbs_VERTEX);
			pWire = new TopoDS_Wire();
			*pWire = wire;
		}

		void XbimWire::Init(double x, double y, double tolerance)
		{	
			gp_Pnt bl(0, 0, 0);
			gp_Pnt br(x, 0, 0);
			gp_Pnt tr(x, y, 0);
			gp_Pnt tl(0, y, 0);
			//make the vertices
			BRep_Builder builder;
			TopoDS_Vertex vbl, vbr, vtr, vtl;
			builder.MakeVertex(vbl, bl, tolerance);
			builder.MakeVertex(vbr, br, tolerance);
			builder.MakeVertex(vtr, tr, tolerance);
			builder.MakeVertex(vtl, tl, tolerance);
			//make the edges
			TopoDS_Wire wire;
			builder.MakeWire(wire);
			builder.Add(wire, BRepBuilderAPI_MakeEdge(vbl, vbr));
			builder.Add(wire, BRepBuilderAPI_MakeEdge(vbr, vtr));
			builder.Add(wire, BRepBuilderAPI_MakeEdge(vtr, vtl));
			builder.Add(wire, BRepBuilderAPI_MakeEdge(vtl, vbl));	
			wire.Closed(Standard_True);
			pWire = new TopoDS_Wire();
			*pWire = wire;
		}


#pragma endregion
#pragma region Methods
	
		XbimPoint3D XbimWire::PointAtParameter(double param)
		{
			if (!IsValid) return XbimPoint3D();
			BRepAdaptor_CompCurve cc(*pWire, Standard_True);
			gp_Pnt p;
			cc.D0(param, p);
			return XbimPoint3D(p.X(), p.Y(), p.Z());
		}


		IXbimWire^ XbimWire::Trim(double first, double last, double tolerance)
		{
			if (!IsValid) return this;
			BRepAdaptor_CompCurve cc(*pWire, Standard_True);
			GeomAbs_Shape continuity = cc.Continuity();
			int numIntervals = cc.NbIntervals(continuity);
			if (numIntervals == 1)
			{
				TopoDS_Edge edge;
				Standard_Real uoe;
				cc.Edge(last, edge, uoe);
				Standard_Real l, f;
				Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, f, l);
				Handle(Geom_TrimmedCurve) trimmed = new Geom_TrimmedCurve(curve, first, last);
				BRepBuilderAPI_MakeWire wm;
				wm.Add(BRepBuilderAPI_MakeEdge(trimmed));
				return gcnew XbimWire(wm.Wire());
			}
			else
			{
				BRepBuilderAPI_MakeWire wm;
				TColStd_Array1OfReal res(1, numIntervals+1);
				cc.Intervals(res, GeomAbs_C0);
				for (Standard_Integer i = 2; i <= numIntervals+1; i++)
				{
					Standard_Real f = res.Value(i - 1);
					Standard_Real l = res.Value(i );
					if (l>first  && f<last)
					{
						TopoDS_Edge edge;
						Standard_Real uoe;
						cc.Edge(last, edge, uoe);
						Standard_Real l, f;
						Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, f, l);
						Standard_Real a = Math::Max(f, first);
						Standard_Real b = Math::Min(l, last);
						wm.Add(BRepBuilderAPI_MakeEdge(new Geom_TrimmedCurve(curve, a, b)));
					}
				}
				return gcnew XbimWire(wm.Wire());
			}
		}


		void XbimWire::Move(IfcAxis2Placement3D^ position)
		{
			if (!IsValid) return;
			gp_Trsf toPos = XbimGeomPrim::ToTransform(position);
			pWire->Move(toPos);
		}

		void XbimWire::Translate(XbimVector3D translation)
		{
			if (!IsValid) return;
			gp_Vec v(translation.X, translation.Y, translation.Z);
			gp_Trsf t;
			t.SetTranslation(v);
			pWire->Move(t);
		}

		void XbimWire::Reverse()
		{
			if (!IsValid) return;
			pWire->Reverse();
		}

#pragma endregion



#pragma region Helper functions

		void XbimWire::AddNewellPoint(const gp_Pnt& previous, const gp_Pnt& current, double & x, double & y, double & z)
		{
			const double& xn = previous.X();
			const double& yn = previous.Y();
			const double& zn = previous.Z();
			const double& xn1 = current.X();
			const double& yn1 = current.Y();
			const double& zn1 = current.Z();
			x += (yn - yn1)*(zn + zn1);
			y += (xn + xn1)*(zn - zn1);
			z += (xn - xn1)*(yn + yn1);
		}
#pragma endregion

	}
}