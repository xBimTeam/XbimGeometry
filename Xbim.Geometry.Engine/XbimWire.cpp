#include "XbimWire.h"
#include "XbimEdge.h"
#include "XbimFace.h"
#include "XbimGeometryCreator.h"
#include "XbimConvert.h"
#include "XbimEdgeSet.h"
#include "XbimVertexSet.h"
#include "XbimCompound.h"
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepBuilderAPI_GTransform.hxx>
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
#include <Geom_BSplineCurve.hxx>
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
#include <TColStd_IndexedDataMapOfTransientTransient.hxx>
#include <TopTools_ListIteratorOfListOfShape.hxx>
#include <ShapeAnalysis.hxx>
#include <BRepFilletAPI_MakeFillet.hxx>
#include <GeomLib_Tool.hxx>
#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>
#include <GC_MakeSegment.hxx>
#include <ShapeAnalysis_Wire.hxx>
#include <msclr\lock.h>
#include <ShapeAnalysis_WireOrder.hxx>

using namespace Xbim::Common;
using namespace System::Linq;
// using namespace System::Diagnostics;

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

		
		XbimWire::XbimWire(XbimEdge^ edge)
		{
			pWire = new TopoDS_Wire();
			BRepBuilderAPI_MakeWire wireMaker(edge);
			*pWire = wireMaker.Wire();
		}
		XbimWire::XbimWire(const TopoDS_Wire& wire)
		{
			pWire = new TopoDS_Wire();
			*pWire = wire;
		}

		XbimWire::XbimWire(const TopoDS_Wire& wire, Object^ tag) :XbimWire(wire)
		{
			Tag = tag;
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
		XbimWire::XbimWire(IIfcPolyline^ profile, ILogger^ logger) { Init(profile, false, logger); }
		XbimWire::XbimWire(IIfcPolyline^ profile, bool attemptClosing, ILogger^ logger) { Init(profile, attemptClosing, logger); }
		XbimWire::XbimWire(IIfcCompositeCurve^ profile, ILogger^ logger){ Init(profile, logger); }
		XbimWire::XbimWire(IIfcCompositeCurveSegment^ profile, ILogger^ logger){ Init(profile, logger); };
		XbimWire::XbimWire(IIfcTrimmedCurve^ profile, ILogger^ logger){ Init(profile, logger); }
		XbimWire::XbimWire(IIfcBSplineCurve^ bspline, ILogger^ logger){ Init(bspline, logger); }
		XbimWire::XbimWire(IIfcBSplineCurveWithKnots^ bSpline, ILogger^ logger){ Init(bSpline, logger); }
		XbimWire::XbimWire(IIfcRationalBSplineCurveWithKnots^ bSpline, ILogger^ logger){ Init(bSpline, logger); }
		XbimWire::XbimWire(IIfcCurve^ profile, ILogger^ logger){ Init(profile, logger); }
		XbimWire::XbimWire(IIfcIndexedPolyCurve^ pcurve, ILogger^ logger){ Init(pcurve, logger); };
		XbimWire::XbimWire(IIfcBoundedCurve^ profile, ILogger^ logger){ Init(profile, logger); }
		XbimWire::XbimWire(IIfcPolyLoop ^ profile, ILogger^ logger){ Init(profile, logger); }
		XbimWire::XbimWire(IIfcArbitraryClosedProfileDef^ profile, ILogger^ logger){ Init(profile, logger); }
		XbimWire::XbimWire(IIfcArbitraryOpenProfileDef^ profile, ILogger^ logger){ Init(profile, logger); }
		XbimWire::XbimWire(IIfcCenterLineProfileDef^ profile, ILogger^ logger){ Init(profile, logger); }
		//parametrised profiles
		XbimWire::XbimWire(IIfcProfileDef ^ profile, ILogger^ logger){ Init(profile, logger); }
		XbimWire::XbimWire(IIfcDerivedProfileDef ^ profile, ILogger^ logger){ Init(profile, logger); }
		XbimWire::XbimWire(IIfcParameterizedProfileDef ^ profile, ILogger^ logger){ Init(profile, logger); }
		XbimWire::XbimWire(IIfcCircleProfileDef ^ profile, ILogger^ logger){ Init(profile, logger); }
		XbimWire::XbimWire(IIfcRectangleProfileDef^ profile, ILogger^ logger){ Init(profile, logger); }
		XbimWire::XbimWire(IIfcRoundedRectangleProfileDef^ profile, ILogger^ logger){ Init(profile, logger); }
		XbimWire::XbimWire(IIfcLShapeProfileDef ^ profile, ILogger^ logger){ Init(profile, logger); }
		XbimWire::XbimWire(IIfcUShapeProfileDef ^ profile, ILogger^ logger){ Init(profile, logger); }
		XbimWire::XbimWire(IIfcEllipseProfileDef ^ profile, ILogger^ logger){ Init(profile, logger); }
		XbimWire::XbimWire(IIfcIShapeProfileDef ^ profile, ILogger^ logger){ Init(profile, logger); }
		XbimWire::XbimWire(IIfcZShapeProfileDef ^ profile, ILogger^ logger){ Init(profile, logger); }
		XbimWire::XbimWire(IIfcCShapeProfileDef ^ profile, ILogger^ logger){ Init(profile, logger); }
		XbimWire::XbimWire(IIfcTShapeProfileDef ^ profile, ILogger^ logger){ Init(profile, logger); }
		XbimWire::XbimWire(double x, double y, double tolerance, bool centre){ Init(x,y,tolerance, centre); }
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

		void XbimWire::Init(IIfcArbitraryClosedProfileDef^ profile, ILogger^ logger)
		{
			if (dynamic_cast<IIfcArbitraryProfileDefWithVoids^>(profile))
			{
				throw gcnew Exception("IfcArbitraryProfileDefWithVoids cannot be created as a wire, call the XbimFace method");
			}
			else
			{
				XbimWire^ loop;
				if (dynamic_cast<IIfcPolyline^>(profile->OuterCurve))
				{
					loop = gcnew XbimWire((IIfcPolyline^)(profile->OuterCurve), true, logger);
				}
				else
				{
					loop = gcnew XbimWire(profile->OuterCurve, logger);
				}
				
				
				if (!loop->IsValid)
				{
					XbimGeometryCreator::LogWarning(logger,profile, "Invalid outer bound. Wire discarded");
					return;
				}
				pWire = new TopoDS_Wire();
				if (!loop->IsClosed && loop->Edges->Count>1) //we need to close it if we have more thn one edge
				{
					// todo: this code is not quite robust, it did not manage to close fairly simple polylines.
					//
					double oneMilli = profile->Model->ModelFactors->OneMilliMeter;
					XbimFace^ face = gcnew XbimFace(loop, logger);
					ShapeFix_Wire wireFixer(loop,face, profile->Model->ModelFactors->Precision);
					wireFixer.ClosedWireMode() = Standard_True;
					wireFixer.FixGaps2dMode() = Standard_True;
					wireFixer.FixGaps3dMode() = Standard_True;
					wireFixer.ModifyGeometryMode() = Standard_True;
					wireFixer.SetMinTolerance(profile->Model->ModelFactors->Precision);
					wireFixer.SetPrecision(oneMilli);
					wireFixer.SetMaxTolerance(oneMilli*10);
					Standard_Boolean closed = wireFixer.Perform();
					if(closed)
						*pWire = wireFixer.Wire();
					else
						*pWire = loop;
				}
				else
					*pWire = loop;
			}
		}

		void XbimWire::Init(IIfcArbitraryOpenProfileDef^ profile, ILogger^ logger)
		{
			if (dynamic_cast<IIfcCenterLineProfileDef^>(profile))
			{
				return Init((IIfcCenterLineProfileDef^)profile, logger);
			}
			else
			{

				XbimWire^ loop = gcnew XbimWire(profile->Curve, logger);
				if (!loop->IsValid)
				{
					XbimGeometryCreator::LogWarning(logger,profile, "Invalid curve. Wire discarded");
					return;
				}
				pWire = new TopoDS_Wire();
				*pWire = loop;
			}
		}

		void XbimWire::Init(IIfcCenterLineProfileDef^ profile, ILogger^ logger)
		{
			/*if ((int)(profile->Curve->Dim) != 2)
			{
				XbimGeometryCreator::LogError(logger, profile, "IfcCenterLineProfileDef must have a dimensionality of 2");
				return;
			}*/
			double precision = profile->Model->ModelFactors->Precision;
			XbimWire^ centreWire = gcnew XbimWire(profile->Curve, logger);
			TopoDS_Wire spine = centreWire;
			//nb the curve must be 2d so place it on the Z plane so that the offseter can get the correct normal
			XbimFace^ xFace = gcnew XbimFace(XbimVector3D(0, 0, 1), logger);
			xFace->Add(centreWire);
			TopoDS_Face spineFace = xFace;
			if (!centreWire->IsValid)
			{
				XbimGeometryCreator::LogWarning(logger,profile, "Invalid curve. Wire discarded");
				return;
			}

			BRepAdaptor_CompCurve cc(spine, Standard_True);
			gp_Pnt wStart = cc.Value(cc.FirstParameter());
			gp_Pnt wEnd = cc.Value(cc.LastParameter());

			BRepOffsetAPI_MakeOffset offseter(spineFace);
			Standard_Real offset = profile->Thickness / 2;
			// Pyatkov 15.06.2017. (Artoymyp on Github)
			// Somewhere in the BRepOffsetAPI_MakeOffset.Perform() a static variable is used:
			// static BRepMAT2d_Explorer Exp;
			// That is why calls to this function in a multi-threaded mode 
			// lead to an unpredictable behavior.
			// SRL it was probably the xbimwire going out of scope and causing a memory access violation, I am leaving the lock code in at present just in case
			{
				msclr::lock l(_makeOffsetLock);
				offseter.Perform(offset);
			} // local scope ends, destructor of lock is called (lock is released).
			
			
			bool done = offseter.IsDone();
		
			
			if (done && offseter.Shape().ShapeType() == TopAbs_WIRE)
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
									gp_Pnt s, pe;
									c3d->D0(start, s);
									c3d->D0(end, pe);
									//make straight edge
									wireMaker.Add(BRepBuilderAPI_MakeEdge(s, pe));
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

		void XbimWire::Init(IIfcPolyline^ pLine, ILogger^ logger)
		{
			Init(pLine, false, logger);
		}

		void XbimWire::Init(IIfcPolyline^ pLine, bool attemptClosing, ILogger^ logger)
		{
			List<IIfcCartesianPoint^>^ pointList = Enumerable::ToList(pLine->Points);
			int total = pointList->Count;
			if (total < 2)
			{
				XbimGeometryCreator::LogWarning(logger,pLine, "Polyline with less than 2 points found. Wire discarded");
				return;
			}

			// prepare tolerance for geometry fixes 
			double tolerance = pLine->Model->ModelFactors->Precision;

			// check for contiguous point tolerance
			for (int i = 1; i < total;) 
			{
				if (XbimConvert::IsEqual(pointList[i - 1], pointList[i], tolerance))
				{
					// remove the redundant point
					XbimGeometryCreator::LogDebug(logger,pLine, "Polyline with redundant points simplified. Point {0} discarded.", pointList[i]->EntityLabel);
					pointList->RemoveAt(i);
					total--;
				}
				else
				{
					// check next
					i++;
				}
			}

			// check for closing point
			Standard_Boolean closed = Standard_False;
			if (attemptClosing)
				closed = Standard_True;
			do
			{
				if (XbimConvert::IsEqual(pointList[0], pointList[total - 1], tolerance))
				{
					total--; //skip the last point
					closed = Standard_True;
				}
				else
					break;
			} while (total > 1);

			if (total < 2)
			{
				XbimGeometryCreator::LogWarning(logger,pLine, "Polyline with less than 2 points found. Wire discarded");
				return;
			}

			//Make all the vertices
			TopTools_Array1OfShape vertexStore(1, total + 1);
			BRep_Builder builder;
			TopoDS_Wire wire;
			builder.MakeWire(wire);
			bool is3D = XbimConvert::Is3D(pLine);
			gp_Pnt first;
			gp_Pnt previous;

			for (int i = 0; i < total; i++) //add all the points into unique collection
			{
				IIfcCartesianPoint^ p = pointList[i];
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
						XbimGeometryCreator::LogWarning(logger,pLine, "Invalid edge found in polyline, {6}.Start = {0}, {1}, {2} End = {3}, {4}, {5}. Edge discarded.",
							 p1.X(), p1.Y(), p1.Z(), p2.X(), p2.Y(), p2.Z(), errMsg);
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
					XbimGeometryCreator::LogInfo(logger,pLine, "Invalid edge, Start = {0}, {1}, {2} End = {3}, {4}, {5}. Edge discarded",
						 p1.X(), p1.Y(), p1.Z(), p2.X(), p2.Y(), p2.Z());
				}
			}
			wire.Closed(closed);
			if (total > 2) //if we have more than a line segment check it is ok and fix if self interecting
			{
				XbimFace^ xFace = nullptr;
				if (is3D)
				{
					XbimWire^ xWire = gcnew XbimWire(wire);
					XbimVector3D norm = xWire->Normal;
					if(!norm.IsInvalid()) //this is not a polyline on a face so we cannot fix any problems just go with it.
					    xFace = gcnew XbimFace(norm, logger);
				}
				else
					xFace = gcnew XbimFace(XbimVector3D(0, 0, 1), logger);
				if (xFace != nullptr)
				{
					ShapeAnalysis_Wire wireChecker(wire, xFace, tolerance);
					Standard_Boolean needsFixing = wireChecker.CheckSelfIntersection();
					if (needsFixing == Standard_True)
					{
						ShapeFix_Wire wireFixer(wire, xFace, tolerance);
						/*wireFixer.FixAddCurve3dMode();
						wireFixer.FixConnectedMode();
						wireFixer.FixDegeneratedMode();
						wireFixer.FixGaps3dMode();
						wireFixer.FixGaps2dMode();
						wireFixer.FixIntersectingEdgesMode();
						wireFixer.FixLackingMode();
						wireFixer.FixNotchedEdgesMode();
						wireFixer.FixReorderMode();
						wireFixer.FixSeamMode();
						wireFixer.FixTailMode();
						wireFixer.FixVertexToleranceMode();*/
						wireFixer.FixTailMode() = Standard_True;
						wireFixer.SetMaxTailWidth(tolerance * 10000);
						wireFixer.ModifyGeometryMode() = Standard_True;
						wireFixer.ModifyTopologyMode() = Standard_True;
						wireFixer.SetMaxTailAngle(0.0174533); //1 degree
						wireFixer.ClosedWireMode() = closed;
						wireFixer.FixSelfIntersectionMode() = Standard_True;
						wireFixer.FixSelfIntersectingEdgeMode() = Standard_True;
						wireFixer.FixIntersectingEdgesMode() = Standard_True;
						wireFixer.FixVertexToleranceMode() = Standard_True;
						wireFixer.Perform();
						wire = wireFixer.Wire();
					}
				}
			}
			pWire = new TopoDS_Wire();
			*pWire = wire;
			ShapeFix_ShapeTolerance fixTol;
			fixTol.LimitTolerance(*pWire, tolerance);
		}

		void XbimWire::Init(IIfcBSplineCurve^ bspline, ILogger^ logger)
		{
			IIfcBSplineCurveWithKnots^ bez = dynamic_cast<IIfcBSplineCurveWithKnots^>(bspline);
			if (bez != nullptr) Init(bez, logger);
			else throw gcnew NotImplementedException("Unsupported IfcBSplineCurve type found.");
		}
		
		void XbimWire::Init(IIfcBSplineCurveWithKnots^ bSpline, ILogger^ logger)
		{
			IIfcRationalBSplineCurveWithKnots^ ratBez = dynamic_cast<IIfcRationalBSplineCurveWithKnots^>(bSpline);
			if (ratBez != nullptr)
				Init(ratBez, logger);
			else
			{
				XbimEdge^ edge = gcnew XbimEdge(bSpline, logger);
				if (edge->IsValid)
				{
					BRepBuilderAPI_MakeWire b(edge);
					pWire = new TopoDS_Wire();
					*pWire = b.Wire();
				}
			}
		}

		void XbimWire::Init(IIfcRationalBSplineCurveWithKnots^ bSpline, ILogger^ logger)
		{
			XbimEdge^ edge = gcnew XbimEdge(bSpline, logger);
			if (edge->IsValid)
			{
				BRepBuilderAPI_MakeWire b(edge);
				pWire = new TopoDS_Wire();
				*pWire = b.Wire();
			}
		}


		void XbimWire::Init(IIfcCompositeCurveSegment^ compCurveSeg, ILogger^ logger)
		{
			Init(compCurveSeg->ParentCurve,logger);
			if (IsValid && !compCurveSeg->SameSense) this->Reverse();
		}

		void XbimWire::Init(IIfcCompositeCurve^ cCurve, ILogger^ logger)
		{
			
			ShapeFix_ShapeTolerance FTol;
			double precision = cCurve->Model->ModelFactors->Precision; //use a courser precision for trimmed curves	
			
			
						
			bool isContinuous = true;
			
			TopoDS_Builder b;		
			TopoDS_Wire degenWire;
			b.MakeWire(degenWire);
			TopTools_ListOfShape edgesList;
			for each(IIfcCompositeCurveSegment^ seg in cCurve->Segments)
			{				
				XbimWire^ wireSegManaged = gcnew XbimWire(seg->ParentCurve, logger);
				if (seg->Transition == IfcTransitionCode::DISCONTINUOUS) isContinuous = false;
				if (wireSegManaged->IsValid)
				{					
					if (!seg->SameSense) 
						wireSegManaged->Reverse();
					for each (XbimEdge^ edge in wireSegManaged->Edges)
					{
						TopoDS_Edge ed = edge;
						FTol.LimitTolerance(ed, precision);															
						edgesList.Append(ed);
						b.Add(degenWire,ed);
					}			
				}				
			}		
			//int edgeCount = 0;
			//for (TopExp_Explorer exp(degenWire, TopAbs_EDGE); exp.More(); exp.Next()) //just take the first wire
			//{
			//	edgeCount++;
			//}
			BRepBuilderAPI_MakeWire wireMaker1;
			wireMaker1.Add(edgesList);
			if (wireMaker1.IsDone())
			{
				pWire = new TopoDS_Wire();
				*pWire = wireMaker1.Wire();
				FTol.LimitTolerance(*pWire, precision);
				return;
			}
			else //coursen the precision to 1 mm 
			{
				//we are going to use one millimeter for the precision when edges don't join
				double oneMilli = cCurve->Model->ModelFactors->OneMilliMeter;

				TopoDS_Face	face = BRepBuilderAPI_MakeFace(degenWire, Standard_True);
				ShapeFix_Wire wireFixer(degenWire, face, precision);
				if (cCurve->ClosedCurve.Value != nullptr && cCurve->ClosedCurve.Value)
					wireFixer.ClosedWireMode() = Standard_True;
				wireFixer.SetPrecision(oneMilli);
				wireFixer.FixConnected();
				wireFixer.FixLacking();
				
				
				BRepBuilderAPI_MakeWire wireMaker2(wireFixer.Wire());			
				if (wireMaker2.IsDone())
				{
					pWire = new TopoDS_Wire();
					*pWire = wireMaker2.Wire();
					FTol.LimitTolerance(*pWire, precision);
					return;
				}
				else //coursen the precision to 5 mm
				{
					ShapeFix_Wire wireFixer2(degenWire, face, precision);
					if (cCurve->ClosedCurve.Value != nullptr && cCurve->ClosedCurve.Value)
						wireFixer.ClosedWireMode() = Standard_True;;
					wireFixer2.SetPrecision(oneMilli * 5);
					wireFixer2.FixConnected();
					wireFixer2.FixLacking();
					BRepBuilderAPI_MakeWire wireMaker3(wireFixer2.Wire());					
					if (wireMaker3.IsDone())
					{
						pWire = new TopoDS_Wire();
						*pWire = wireMaker3.Wire();
						FTol.LimitTolerance(*pWire, precision);
						return;
					}
					
				}			
			}
			//give up bad shape
			XbimGeometryCreator::LogWarning(logger,cCurve, "Invalid part of a composite curve found. It has been discarded");
		}

		void XbimWire::Init(IIfcTrimmedCurve^ tCurve, ILogger^ logger)
		{
			ShapeFix_ShapeTolerance FTol;
			bool isConic = (dynamic_cast<IIfcConic^>(tCurve->BasisCurve) != nullptr);

			IModelFactors^ mf = tCurve->Model->ModelFactors;
			double tolerance = mf->Precision;
			double toleranceMax = mf->PrecisionMax;
			double parameterFactor = isConic ? mf->AngleToRadiansConversionFactor : 1;
			Handle(Geom_Curve) curve;
			bool rotateElipse = false;
			
			//it could be based on a circle, ellipse or line
			if (dynamic_cast<IIfcCircle^>(tCurve->BasisCurve))
			{
				curve = gcnew XbimCurve((IIfcCircle^)tCurve->BasisCurve, logger);
			}
			else if (dynamic_cast<IIfcEllipse^>(tCurve->BasisCurve))
			{	
				IIfcEllipse^ ellipse = (IIfcEllipse^)tCurve->BasisCurve;
				curve = gcnew XbimCurve(ellipse, logger);
				rotateElipse = ellipse->SemiAxis1 < ellipse->SemiAxis2;				
			}
			else if (dynamic_cast<IIfcLine^>(tCurve->BasisCurve))
			{	
				IIfcLine^ line = (IIfcLine^)tCurve->BasisCurve;
				curve = gcnew XbimCurve(line, logger);
				parameterFactor *= line->Dir->Magnitude; //adjust the trim value for lines magnitude
			}
			else if (dynamic_cast<IIfcBSplineCurveWithKnots^>(tCurve->BasisCurve))
			{			
				curve = gcnew XbimCurve((IIfcBSplineCurveWithKnots^)tCurve->BasisCurve, logger);
			}
			else if (dynamic_cast<IIfcOffsetCurve3D^>(tCurve->BasisCurve))
			{
				curve = gcnew XbimCurve((IIfcOffsetCurve3D^)tCurve->BasisCurve, logger);
			}
			else if (dynamic_cast<IIfcOffsetCurve2D^>(tCurve->BasisCurve))
			{
				curve = gcnew XbimCurve((IIfcOffsetCurve2D^)tCurve->BasisCurve, logger);
			}
			else if (dynamic_cast<IIfcPcurve^>(tCurve->BasisCurve))
			{
				curve = gcnew XbimCurve((IIfcPcurve^)tCurve->BasisCurve, logger);
			}
			else if (dynamic_cast<IIfcPolyline^>(tCurve->BasisCurve))
			{
				curve = gcnew XbimCurve((IIfcPolyline^)tCurve->BasisCurve, logger);
			}
			else if (dynamic_cast<IIfcTrimmedCurve^>(tCurve->BasisCurve))
			{
				curve = gcnew XbimCurve((IIfcTrimmedCurve^)tCurve->BasisCurve, logger);
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
			
			for each (IIfcTrimmingSelect^ trim in tCurve->Trim1)
			{

				if (dynamic_cast<IIfcCartesianPoint^>(trim) && trim_cartesian)
				{
					IIfcCartesianPoint^ cp = (IIfcCartesianPoint^)trim;
					pnt1.SetXYZ(gp_XYZ(cp->X, cp->Y, XbimConvert::GetZValueOrZero(cp)));
					trimmed1 = true;
				}
				else if (dynamic_cast<Xbim::Ifc4::MeasureResource::IfcParameterValue^>(trim) && !trim_cartesian)
				{
					Xbim::Ifc4::MeasureResource::IfcParameterValue^ pv = (Xbim::Ifc4::MeasureResource::IfcParameterValue^)trim;
				    double value = (double)(pv->Value);
					flt1 = value * parameterFactor;
					trimmed1 = true;
				}
			}
			BRepLib_MakeWire wireMaker;
			BRep_Builder b;
			for each (IIfcTrimmingSelect^ trim in tCurve->Trim2)
			{
				if (dynamic_cast<IIfcCartesianPoint^>(trim) && trim_cartesian && trimmed1)
				{
					IIfcCartesianPoint^ cp = (IIfcCartesianPoint^)trim;
					
					gp_Pnt pnt2(cp->X, cp->Y, XbimConvert::GetZValueOrZero(cp));
					if (!pnt1.IsEqual(pnt2, tolerance))
					{
						
						TopoDS_Vertex v1, v2;
						double currentTolerance = tolerance;
						b.MakeVertex(v1, pnt1, currentTolerance);
						b.MakeVertex(v2, pnt2, currentTolerance);
						if (dynamic_cast<IIfcLine^>(tCurve->BasisCurve)) //we have a line and two points, just build it
						{
							wireMaker.Add(BRepBuilderAPI_MakeEdge(sense_agreement ? v1 : v2, sense_agreement ? v2 : v1));
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
								XbimGeometryCreator::LogWarning(logger,tCurve, "Construction of trimmed curve failed, {0}. A line segment has been used",  errMsg);
								wireMaker.Add(BRepBuilderAPI_MakeEdge(sense_agreement ? v1 : v2, sense_agreement ? v2 : v1));
							}
							else
								wireMaker.Add( e.Edge());
						}
						trimmed2 = true;
					}
					break;
				}
				else if (dynamic_cast<Xbim::Ifc4::MeasureResource::IfcParameterValue^>(trim) && !trim_cartesian && trimmed1)
				{
					Xbim::Ifc4::MeasureResource::IfcParameterValue^ pv = (Xbim::Ifc4::MeasureResource::IfcParameterValue^)trim;
					double value = (double)(pv->Value);
					double flt2 = (value * parameterFactor);
					if (isConic && Math::Abs(Math::IEEERemainder(flt2 - flt1, (double)(Math::PI*2.0)) - 0.0) <= Precision::Confusion())
					{
						BRepBuilderAPI_MakeEdge edgeMaker(curve);
						BRepBuilderAPI_EdgeError edgeErr = edgeMaker.Error();
						if (edgeErr != BRepBuilderAPI_EdgeDone)
						{
							String^ errMsg = XbimEdge::GetBuildEdgeErrorMessage(edgeErr);
							XbimGeometryCreator::LogWarning(logger,tCurve, "Invalid edge found in trimmed curve, {0}. It has been ignored", errMsg);
							return;
						}
						else
						{
							if (sense_agreement)
								wireMaker.Add(edgeMaker.Edge());
							else
								wireMaker.Add(TopoDS::Edge(edgeMaker.Edge().Reversed()));
						}

					}
					else
					{
						if (rotateElipse) //if we have had to rotate the elipse, then rotate the trims
						{
							flt1 -= Math::PI / 2;
							flt2 -= Math::PI / 2;
						}

						Handle(Geom_TrimmedCurve) tc = new Geom_TrimmedCurve(curve, sense_agreement ? flt1 : flt2, sense_agreement ? flt2 : flt1);
						
						BRepBuilderAPI_MakeEdge edgeMaker(tc);
						
						BRepBuilderAPI_EdgeError edgeErr = edgeMaker.Error();
						if (edgeErr != BRepBuilderAPI_EdgeDone)
						{
							String^ errMsg = XbimEdge::GetBuildEdgeErrorMessage(edgeErr);
							XbimGeometryCreator::LogWarning(logger,tCurve, "Invalid edge found in trimmed curve, {0} .It has been ignored", errMsg);
							return;
						}
						else
						{						
							if(sense_agreement)
								wireMaker.Add(edgeMaker.Edge());
							else
								wireMaker.Add(TopoDS::Edge(edgeMaker.Edge().Reversed()));
						}
					}
					trimmed2 = true;
					break;
				}
			}
			if (wireMaker.IsDone())
			{
				pWire = new TopoDS_Wire();
				*pWire = wireMaker.Wire();
			}
		}
		
		void XbimWire::Init(IIfcCurve^ curve, ILogger^ logger)
		{
			IIfcBoundedCurve^ bc;
			
			IIfcLine^ l;
			IIfcEllipse^ e;
			IIfcCircle^ c;
			if ((bc = dynamic_cast<IIfcBoundedCurve^>(curve)) != nullptr) 
				return Init(bc, logger);
			else if ((c =  dynamic_cast<IIfcCircle^>(curve)) != nullptr)
			{
				XbimEdge^ edge = gcnew XbimEdge(c, logger);
				if (edge->IsValid)
				{
					BRepBuilderAPI_MakeWire b(edge);
					pWire = new TopoDS_Wire();
					*pWire = b.Wire();
				}
			}
			else if ((l = dynamic_cast<IIfcLine^>(curve)) != nullptr)
			{
				XbimEdge^ edge = gcnew XbimEdge(l, logger);
				if (edge->IsValid)
				{
					BRepBuilderAPI_MakeWire b(edge);
					pWire = new TopoDS_Wire();
					*pWire = b.Wire();
				}
			}
			else if ((e = dynamic_cast<IIfcEllipse^>(curve)) != nullptr)
			{
				XbimEdge^ edge = gcnew XbimEdge(e, logger);
				if (edge->IsValid)
				{
					BRepBuilderAPI_MakeWire b(edge);
					pWire = new TopoDS_Wire();
					*pWire = b.Wire();
				}
			}
			else if (3 ==(int)curve->Dim )
			{
				XbimCurve^ xcurve = gcnew XbimCurve(curve, logger);
				XbimEdge^ edge = gcnew XbimEdge(xcurve);
				if (edge->IsValid)
				{
					BRepBuilderAPI_MakeWire b(edge);
					pWire = new TopoDS_Wire();
					*pWire = b.Wire();
				}
			}
			else if (2 == (int) curve->Dim )
			{
				XbimCurve2D^ xcurve = gcnew XbimCurve2D(curve, logger);
				XbimEdge^ edge = gcnew XbimEdge(xcurve, logger);
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
				throw gcnew NotImplementedException ("Curve type is not implemented. " + type); 
				return;
			}

		}

		void XbimWire::Init(IIfcIndexedPolyCurve^ polyCurve, ILogger^ /*logger*/)
		{
			double precision = polyCurve->Model->ModelFactors->Precision;
			IIfcCartesianPointList3D^ points3D = dynamic_cast<IIfcCartesianPointList3D^>(polyCurve->Points);
			IIfcCartesianPointList2D^ points2D = dynamic_cast<IIfcCartesianPointList2D^>(polyCurve->Points);
			List<XbimVertex^>^ vertices;
			Dictionary<XbimPoint3DWithTolerance^, XbimVertex^>^ uniqueVertices = gcnew	Dictionary<XbimPoint3DWithTolerance^, XbimVertex^>();
			if (points3D != nullptr)
			{
				vertices = gcnew List<XbimVertex^>();
				for each (IEnumerable<Ifc4::MeasureResource::IfcLengthMeasure>^ coll in points3D->CoordList)
				{
					IEnumerator<Ifc4::MeasureResource::IfcLengthMeasure>^ enumer = coll->GetEnumerator();
					enumer->MoveNext();
					double x = (double)enumer->Current; 
					enumer->MoveNext();
					double y =  (double) enumer->Current;
					enumer->MoveNext();
					double z = (double)enumer->Current;
					XbimPoint3DWithTolerance^ p3d = gcnew XbimPoint3DWithTolerance(x, y, z, precision);
					XbimVertex^ vertex; 
					if (!uniqueVertices->TryGetValue(p3d,vertex))
					{
						vertex = gcnew XbimVertex(p3d);
						uniqueVertices->Add(p3d, vertex);
					}
					vertices->Add(vertex);
				}
			}
			else if (points2D != nullptr) //it is 2D
			{
				vertices = gcnew List<XbimVertex^>();
				for each (IEnumerable<Ifc4::MeasureResource::IfcLengthMeasure>^ coll in points2D->CoordList)
				{
					IEnumerator<Ifc4::MeasureResource::IfcLengthMeasure>^ enumer = coll->GetEnumerator();
					enumer->MoveNext();
					double x = (double)enumer->Current;
					enumer->MoveNext();
					double y = (double)enumer->Current;
					XbimPoint3DWithTolerance^ p3d = gcnew XbimPoint3DWithTolerance(x, y, 0, precision);
					XbimVertex^ vertex;
					if (!uniqueVertices->TryGetValue(p3d, vertex))
					{
						vertex = gcnew XbimVertex(p3d);
						uniqueVertices->Add(p3d, vertex);
					}
					vertices->Add(vertex);
				}
			}

			BRepBuilderAPI_MakeWire wireMaker;
			if (polyCurve->Segments->Count > 0)
			{
				for each (IIfcSegmentIndexSelect^ segment in  polyCurve->Segments)
				{
					Ifc4::GeometryResource::IfcArcIndex^ arcIndex = dynamic_cast<Ifc4::GeometryResource::IfcArcIndex^>(segment);
					Ifc4::GeometryResource::IfcLineIndex^ lineIndex = dynamic_cast<Ifc4::GeometryResource::IfcLineIndex^>(segment);
					if (arcIndex != nullptr)
					{
						List<Ifc4::MeasureResource::IfcPositiveInteger>^ indices = (List<Ifc4::MeasureResource::IfcPositiveInteger>^)arcIndex->Value;
						XbimEdge^ e = gcnew XbimEdge(vertices[(int)indices[0] - 1], vertices[(int)indices[1] - 1], vertices[(int)indices[2] - 1]);
						wireMaker.Add(e);
					}
					else if (lineIndex != nullptr)
					{
						List<Ifc4::MeasureResource::IfcPositiveInteger>^ indices = (List<Ifc4::MeasureResource::IfcPositiveInteger>^)lineIndex->Value;
						for (int i = 0; i < indices->Count - 1; i++)
						{
							XbimVertex^ start = vertices[(int)indices[i] - 1];
							XbimVertex^ end = vertices[(int)indices[i + 1] - 1];
							if (start != end)
							{
								XbimEdge^ e = gcnew XbimEdge(start,end);
								wireMaker.Add(e);
							}
						}
					}
				}
			}
			else
			{
				// To be compliant with:
				// "In the case that the list of Segments is not provided, all points in the IfcCartesianPointList are connected by straight line segments in the order they appear in the IfcCartesianPointList."
				// http://www.buildingsmart-tech.org/ifc/IFC4/Add1/html/schema/ifcgeometryresource/lexical/ifcindexedpolycurve.htm
				int count = 0;
				if (points3D != nullptr)
				{
					count = points3D->CoordList->Count;
				}
				else if (points2D != nullptr)
				{
					count = points2D->CoordList->Count;
				}

				for (int i = 0; i < count - 1; i++)
				{
					XbimVertex^ start = vertices[i];
					XbimVertex^ end = vertices[i + 1];
					if (start != end)
					{
						XbimEdge^ e = gcnew XbimEdge(start, end);
						wireMaker.Add(e);
					}
				}
			}

			pWire = new TopoDS_Wire();
			*pWire = wireMaker.Wire();
			
		}
		void XbimWire::Init(IIfcPolyLoop ^ loop, ILogger^ logger)
		{
			List<IIfcCartesianPoint^>^ polygon = Enumerable::ToList(loop->Polygon);			
			int lastPt = polygon->Count;
			if (lastPt < 3)
			{
				XbimGeometryCreator::LogWarning(logger,loop, "Invalid loop, it has less than three points. Wire discarded");
				return;
			}
			double tolerance = loop->Model->ModelFactors->Precision;
			//check we haven't got duplicate start and end points
			IIfcCartesianPoint^ first = polygon[0];
			IIfcCartesianPoint^ last = polygon[lastPt - 1];
			if (XbimConvert::IsEqual(first, last, tolerance))
			{
				XbimGeometryCreator::LogWarning(logger,loop, "Invalid edge found, Start and End are the same point. Start = #{0} End = #{1}. Edge discarded", first->EntityLabel, last->EntityLabel);
				lastPt--;
				if (lastPt<3)
				{
					XbimGeometryCreator::LogWarning(logger,loop, "Invalid loop, it has less than three points. Wire discarded");
					return;
				}
			}

			int totalEdges = 0;
			bool is3D = XbimConvert::Is3D(loop);
			BRep_Builder builder;
			TopoDS_Wire wire;
			builder.MakeWire(wire);
			for (int p = 1; p <= lastPt; p++)
			{
				IIfcCartesianPoint^ p1;
				IIfcCartesianPoint^ p2;
				if (p == lastPt)
				{
					p2 = polygon[0];
					p1 = polygon[p - 1];
				}
				else
				{
					p1 = polygon[p - 1];
					p2 = polygon[p];
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
					XbimGeometryCreator::LogWarning(logger,loop, "Invalid edge, {0}. Start = #{1} End = #{2}. Edge discarded",errMsg, p1, p2);
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
				XbimGeometryCreator::LogWarning(logger,loop, "Invalid loop it only has {0} edge(s), a minimum of 3 is required. Wire discarded", totalEdges);
				return;
			}
			wire.Closed(Standard_True);
			ShapeFix_ShapeTolerance FTol;
			FTol.SetTolerance(wire, loop->Model->ModelFactors->Precision, TopAbs_VERTEX);
			pWire = new TopoDS_Wire();
			*pWire = wire;
		}

		void XbimWire::Init(IIfcBoundedCurve^ bCurve, ILogger^ logger)
		{
			BRepBuilderAPI_MakeWire wire;
			IIfcPolyline^ p;
			IIfcTrimmedCurve^ t;
			IIfcCompositeCurve^ c;
			IIfcBSplineCurve^ b;
			IIfcIndexedPolyCurve^ ipc; 
			if ((p = dynamic_cast<IIfcPolyline^>(bCurve)) != nullptr)
				return Init(p, logger);
			else if ((t = dynamic_cast<IIfcTrimmedCurve^>(bCurve)) != nullptr)
				return Init(t, logger);
			else if ((c = dynamic_cast<IIfcCompositeCurve^>(bCurve)) != nullptr)
				return Init(c, logger);
			else if ((b = dynamic_cast<IIfcBSplineCurve^>(bCurve)) != nullptr)
				return Init(b, logger);
			else if ((ipc = dynamic_cast<IIfcIndexedPolyCurve^>(bCurve)) != nullptr) 
				return Init(ipc, logger);
			else
			{				
				throw gcnew NotImplementedException("Bounded Curve type is not implemented");
			
			}
		}


#pragma endregion

#pragma region IXbimWire Interface

		IXbimGeometryObject^ XbimWire::Transform(XbimMatrix3D matrix3D)
		{
			BRepBuilderAPI_Copy copier(this);
			BRepBuilderAPI_Transform gTran(copier.Shape(), XbimConvert::ToTransform(matrix3D));
			TopoDS_Wire temp = TopoDS::Wire(gTran.Shape());
			return gcnew XbimWire(temp);
		}

		IXbimGeometryObject^ XbimWire::TransformShallow(XbimMatrix3D matrix3D)
		{
			TopoDS_Wire wire = TopoDS::Wire(pWire->Moved(XbimConvert::ToTransform(matrix3D)));
			GC::KeepAlive(this);
			return gcnew XbimWire(wire);
		}

		XbimRect3D XbimWire::BoundingBox::get()
		{
			if (pWire == nullptr)return XbimRect3D::Empty;
			Bnd_Box pBox;
			BRepBndLib::Add(*pWire, pBox);
			Standard_Real srXmin, srYmin, srZmin, srXmax, srYmax, srZmax;
			if (pBox.IsVoid()) return XbimRect3D::Empty;
			pBox.Get(srXmin, srYmin, srZmin, srXmax, srYmax, srZmax);
			GC::KeepAlive(this);
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
			if (!IsValid) 
				return XbimVector3D();
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
						// we identify the Us of quadrant points along the curve to compute the normal
						//
						BRepAdaptor_Curve curve(wEx.Current());
						TopAbs_Orientation or = wEx.Current().Orientation();
						double uStart = curve.FirstParameter();
						double uEnd = curve.LastParameter();
						double u0, u1, u2, u3;
						double delta;
						if (or != TopAbs_REVERSED)
						{
							u0 = uStart; // start from the start
							delta = (uEnd - uStart) / 4; // and go forward a bit for each step
						}
						else
						{
							u0 = uEnd; // start from the end
							delta = (uEnd - uStart) / -4; // and go back a bit for each step
						}
						u1 = u0 + delta;
						u2 = u1 + delta;
						u3 = u2 + delta;
						
						// then we get the points
						gp_Pnt p0; gp_Pnt p1; gp_Pnt p2; gp_Pnt p3;
						curve.D0(u0, p0);
						curve.D0(u1, p1);
						curve.D0(u2, p2);
						curve.D0(u3, p3);

						// then add the points to the newell evaluation
						if (count > 0)
						{
							AddNewellPoint(previousEnd, p0, x, y, z);
							AddNewellPoint(p0, p1, x, y, z);
							AddNewellPoint(p1, p2, x, y, z);
							AddNewellPoint(p2, p3, x, y, z);
							previousEnd = p3;
						}
						else
						{
							first = p0;
							AddNewellPoint(first, p1, x, y, z);
							AddNewellPoint(p1, p2, x, y, z);
							AddNewellPoint(p2, p3, x, y, z);
							previousEnd = p3;
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
			/*
			// this can be used to look at the wire points in autocad for debugging
			Debug::WriteLine("_.CIRCLE");
			Debug::WriteLine("{0},{1},{2}", previousEnd.X(), previousEnd.Y(), previousEnd.Z());
			Debug::WriteLine("7");
			Debug::WriteLine("_.CIRCLE");
			Debug::WriteLine("{0},{1},{2}", first.X(), first.Y(), first.Z());
			Debug::WriteLine("15");
			*/
			
			GC::KeepAlive(this);
			// XbimVector3D
			return vec.Normalized();
		}

		bool XbimWire::IsPlanar::get()
		{
			if (!IsValid) return false;
			BRepBuilderAPI_FindPlane finder(*pWire);
			GC::KeepAlive(this);
			return (finder.Found() == Standard_True);
		}

		XbimPoint3D XbimWire::Start::get()
		{
			if (!IsValid) return XbimPoint3D();
			BRepAdaptor_CompCurve cc(*pWire, Standard_True);
			gp_Pnt p = cc.Value(cc.FirstParameter());
			GC::KeepAlive(this);
			return XbimPoint3D(p.X(),p.Y(),p.Z());
		}

		XbimPoint3D XbimWire::End::get()
		{
			if (!IsValid) return XbimPoint3D();
			BRepAdaptor_CompCurve cc(*pWire, Standard_True);
			gp_Pnt p = cc.Value(cc.LastParameter());
			return XbimPoint3D(p.X(), p.Y(), p.Z());
		}
		double XbimWire::Area::get()
		{
			return ShapeAnalysis::ContourArea(this);
		}

		double XbimWire::Length::get()
		{
			if (!IsValid) return 0.;
			BRepAdaptor_CompCurve cc(*pWire, Standard_True);
			GC::KeepAlive(this);
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
			if(!IsClosed) intervals->Add(cc.LastParameter());
			return intervals;
		}

		array<ContourVertex>^ XbimWire::Contour()
		{
			if (!IsValid) return gcnew array<ContourVertex>(0);
			TopoDS_Wire ccWire = *pWire;
			int t = 0;
			for (BRepTools_WireExplorer exp(ccWire); exp.More(); exp.Next()) t++;			
			if (t <= 2) return gcnew array<ContourVertex>(0);
			array<ContourVertex>^ contour = gcnew array<ContourVertex>(t);
			int i = 0;
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
			List<XbimPoint3D>^ intervals = gcnew List<XbimPoint3D>(numIntervals);
			for (Standard_Integer i = 1; i <= numIntervals; i++)
			{
				gp_Pnt p = cc.Value(res.Value(i));
				intervals->Add(XbimPoint3D(p.X(), p.Y(), p.Z()));
			}
			/*if (!IsClosed)
			{
				gp_Pnt l = cc.Value(cc.LastParameter());
				intervals->Add(XbimPoint3D(l.X(), l.Y(), l.Z()));
			}*/
			return intervals;
		}

#pragma endregion

#pragma region Parameterised profiles

		void XbimWire::Init(IIfcProfileDef ^ profile, ILogger^ logger)
		{
			if (dynamic_cast<IIfcArbitraryClosedProfileDef^>(profile))
				return Init((IIfcArbitraryClosedProfileDef^)profile, logger);
			else if (dynamic_cast<IIfcParameterizedProfileDef^>(profile))
				return Init((IIfcParameterizedProfileDef^)profile, logger);
			else if (dynamic_cast<IIfcDerivedProfileDef^>(profile))
				return Init((IIfcDerivedProfileDef^)profile, logger);
			else if (dynamic_cast<IIfcArbitraryOpenProfileDef^>(profile))
				return Init((IIfcArbitraryOpenProfileDef^)profile, logger);
			else
				XbimGeometryCreator::LogError(logger,profile, "Profile definition {0} is not implemented", profile->GetType()->Name);

		}

		void XbimWire::Init(IIfcDerivedProfileDef ^ profile, ILogger^ logger)
		{
			Init(profile->ParentProfile, logger);
			if (IsValid && !dynamic_cast<IIfcMirroredProfileDef^>(profile))
			{
				gp_Trsf trsf = XbimConvert::ToTransform(profile->Operator);
				pWire->Move(TopLoc_Location(trsf));
			}
			if (IsValid && dynamic_cast<IIfcMirroredProfileDef^>(profile))
			{
				//we need to mirror about the Y axis
				gp_Pnt origin(0, 0, 0);
				gp_Dir xDir(0, 1, 0);
				gp_Ax1 mirrorAxis(origin, xDir);
				gp_Trsf aTrsf;
				aTrsf.SetMirror(mirrorAxis);
				BRepBuilderAPI_Transform aBrepTrsf(*pWire, aTrsf);
				*pWire = TopoDS::Wire(aBrepTrsf.Shape());
				Reverse();//correct the normal to be correct
			}
		}

		void XbimWire::Init(IIfcParameterizedProfileDef ^ profile, ILogger^ logger)
		{
			if (dynamic_cast<IIfcRectangleHollowProfileDef^>(profile))
				return Init((IIfcRectangleProfileDef^)profile, logger);
			else if (dynamic_cast<IIfcRectangleProfileDef^>(profile))
				return Init((IIfcRectangleProfileDef^)profile, logger);
			else if (dynamic_cast<IIfcCircleHollowProfileDef^>(profile))
				return Init((IIfcCircleHollowProfileDef^)profile, logger);
			else if (dynamic_cast<IIfcCircleProfileDef^>(profile))
				return Init((IIfcCircleProfileDef^)profile, logger);
			else if (dynamic_cast<IIfcLShapeProfileDef^>(profile))
				return Init((IIfcLShapeProfileDef^)profile, logger);
			else if (dynamic_cast<IIfcUShapeProfileDef^>(profile))
				return Init((IIfcUShapeProfileDef^)profile, logger);
			else if (dynamic_cast<IIfcIShapeProfileDef^>(profile))
				return Init((IIfcIShapeProfileDef^)profile, logger);
			else if (dynamic_cast<IIfcCShapeProfileDef^>(profile))
				return Init((IIfcCShapeProfileDef^)profile, logger);
			else if (dynamic_cast<IIfcTShapeProfileDef^>(profile))
				return Init((IIfcTShapeProfileDef^)profile, logger);
			else if (dynamic_cast<IIfcZShapeProfileDef^>(profile))
				return Init((IIfcZShapeProfileDef^)profile, logger);
			/*else if (dynamic_cast<IIfcCraneRailFShapeProfileDef^>(profile))
				return Init((IIfcCraneRailFShapeProfileDef^)profile);
			else if (dynamic_cast<IIfcCraneRailAShapeProfileDef^>(profile))
				return Init((IIfcCraneRailAShapeProfileDef^)profile);*/
			else if (dynamic_cast<IIfcEllipseProfileDef^>(profile))
				return Init((IIfcEllipseProfileDef^)profile, logger);
			else 
				XbimGeometryCreator::LogError(logger,profile, "Profile type {0} is not implemented", profile->GetType()->Name);
		}

		//Builds a wire from a CircleProfileDef
		void XbimWire::Init(IIfcCircleProfileDef ^ circProfile, ILogger^ logger)
		{
			if (dynamic_cast<IIfcCircleHollowProfileDef^>(circProfile))
			{
				XbimGeometryCreator::LogError(logger,circProfile, "Circle hollow profile defintions cannot be created as a wire, call the XbimFace method");
				return;
			}
			gp_Ax2 gpax2;
			if (circProfile->Position != nullptr)
			{
				IIfcAxis2Placement2D^ ax2 = (IIfcAxis2Placement2D^)circProfile->Position;
				gpax2.SetLocation(gp_Pnt(ax2->Location->X, ax2->Location->Y, 0));
				gpax2.SetDirection(gp_Dir(0, 0, 1));
				gpax2.SetXDirection(gp_Dir(ax2->P[0].X, ax2->P[0].Y, 0.));
			}
			
			gp_Circ gc(gpax2, circProfile->Radius);
			Handle(Geom_Circle) hCirc = GC_MakeCircle(gc);
			TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(hCirc);
			BRep_Builder b;
			TopoDS_Wire wire;
			b.MakeWire(wire);
			b.Add(wire, edge);
			ShapeFix_ShapeTolerance FTol;
			FTol.SetTolerance(wire, circProfile->Model->ModelFactors->Precision, TopAbs_VERTEX);
			pWire = new TopoDS_Wire();
			*pWire = wire;
		}

		void XbimWire::Init(IIfcRoundedRectangleProfileDef^ rectProfile, ILogger^ /*logger*/)
		{
			//make the basic shapes
			double xOff = rectProfile->XDim / 2;
			double yOff = rectProfile->YDim / 2;
			double precision = rectProfile->Model->ModelFactors->Precision;
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
			double fRad = rectProfile->RoundingRadius;
			if (fRad > 0) //consider fillets
			{
				BRepBuilderAPI_MakeFace faceMaker(wire, true);
				BRepFilletAPI_MakeFillet2d filleter(faceMaker.Face());
				for (BRepTools_WireExplorer exp(wire); exp.More(); exp.Next())
				{
					filleter.AddFillet(exp.CurrentVertex(), fRad);
				}
				filleter.Build();
				if (filleter.IsDone())
				{
					TopoDS_Shape shape = filleter.Shape();
					for (TopExp_Explorer exp(shape, TopAbs_WIRE); exp.More(); ) //just take the first wire
					{
						wire = TopoDS::Wire(exp.Current());
						break;
					}
				}
			}

			//apply the position transformation
			if (rectProfile->Position != nullptr)
				wire.Move(XbimConvert::ToLocation(rectProfile->Position));
			pWire = new TopoDS_Wire();
			*pWire = wire;
		}


		void XbimWire::Init(IIfcRectangleProfileDef^ rectProfile, ILogger^ logger)
		{
			if (rectProfile->XDim <= 0 || rectProfile->YDim <= 0)
			{
				XbimGeometryCreator::LogInfo(logger,rectProfile, "Invalid rectangle profile with a zero or less dimension, XDim = {0}, YDim = {1}. Face discarded", rectProfile->XDim, rectProfile->YDim);
			}
			else
			{
				if (dynamic_cast<IIfcRectangleHollowProfileDef^>(rectProfile))
				{
					XbimGeometryCreator::LogError(logger,rectProfile, "Rectangle hollow profile cannot be created as a wire, call the XbimFace method");
					return;
				}
				else if (dynamic_cast<IIfcRoundedRectangleProfileDef^>(rectProfile))
				{
					Init((IIfcRoundedRectangleProfileDef^)rectProfile, logger);
				}
				else
				{
					double xOff = rectProfile->XDim / 2;
					double yOff = rectProfile->YDim / 2;
					double precision = rectProfile->Model->ModelFactors->Precision;
					gp_Pnt bl(-xOff, -yOff, 0);
					gp_Pnt br(xOff, -yOff, 0);
					gp_Pnt tr(xOff, yOff, 0);
					gp_Pnt tl(-xOff, yOff, 0);					
					Handle(Geom_TrimmedCurve) aSeg1 = GC_MakeSegment(bl, br);
					Handle(Geom_TrimmedCurve) aSeg2 = GC_MakeSegment(br, tr);
					Handle(Geom_TrimmedCurve) aSeg3 = GC_MakeSegment(tr, tl);
					Handle(Geom_TrimmedCurve) aSeg4 = GC_MakeSegment(tl, bl);
					TopoDS_Edge e1 = BRepBuilderAPI_MakeEdge(aSeg1);
					TopoDS_Edge e2 = BRepBuilderAPI_MakeEdge(aSeg2);
					TopoDS_Edge e3 = BRepBuilderAPI_MakeEdge(aSeg3);
					TopoDS_Edge e4 = BRepBuilderAPI_MakeEdge(aSeg4);
					TopoDS_Wire wire = BRepBuilderAPI_MakeWire(e1, e2, e3,e4);
					ShapeFix_ShapeTolerance tol;
					//set the correct precision
					tol.SetTolerance(wire, precision, TopAbs_VERTEX);					
					//apply the position transformation
					if (rectProfile->Position!=nullptr)
						wire.Move(XbimConvert::ToLocation(rectProfile->Position));
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

		void XbimWire::Init(IIfcLShapeProfileDef ^ profile, ILogger^ /*logger*/)
		{
			bool detailed = profile->Model->ModelFactors->ProfileDefLevelOfDetail == 1;
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
				double radConv = profile->Model->ModelFactors->AngleToRadiansConversionFactor;
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
					for (TopExp_Explorer exp(shape, TopAbs_WIRE); exp.More(); ) //just take the first wire
					{
						wire = TopoDS::Wire(exp.Current());
						break;
					}
				}
			}
			if (profile->Position!=nullptr)
				wire.Move(XbimConvert::ToLocation(profile->Position));
			//removed not in Ifc4
			/*if (profile->CentreOfGravityInX.HasValue || profile->CentreOfGravityInY.HasValue)
			{
				double transX = 0;
				double transY = 0;
				if (profile->CentreOfGravityInX.HasValue) transX = profile->CentreOfGravityInX.Value;
				if (profile->CentreOfGravityInY.HasValue) transY = profile->CentreOfGravityInY.Value;
				gp_Vec v(transX, transY, 0);
				gp_Trsf t;
				t.SetTranslation(v);
				wire.Move(t);
			}*/
			ShapeFix_ShapeTolerance FTol;
			FTol.SetTolerance(wire, profile->Model->ModelFactors->Precision, TopAbs_VERTEX);
			pWire = new TopoDS_Wire();
			*pWire = wire;
		}

		
		void XbimWire::Init(IIfcUShapeProfileDef ^ profile, ILogger^ /*logger*/)
		{
			bool detailed = profile->Model->ModelFactors->ProfileDefLevelOfDetail == 1;
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
				double radConv = profile->Model->ModelFactors->AngleToRadiansConversionFactor;
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
					for (TopExp_Explorer exp(shape, TopAbs_WIRE); exp.More(); ) //just take the first wire
					{
						wire = TopoDS::Wire(exp.Current());
						break;
					}
				}
			}
			if (profile->Position!=nullptr)
				wire.Move(XbimConvert::ToLocation(profile->Position));
			//removed in Ifc4
			/*if (profile->CentreOfGravityInX.HasValue)
			{
				gp_Vec v(profile->CentreOfGravityInX.Value, 0, 0);
				gp_Trsf t;
				t.SetTranslation(v);
				wire.Move(t);
			}*/
			ShapeFix_ShapeTolerance FTol;
			FTol.SetTolerance(wire, profile->Model->ModelFactors->Precision, TopAbs_VERTEX);
			pWire = new TopoDS_Wire();
			*pWire = wire;
		}

		// SRL: Builds a wire from a composite IfcCraneRailFShapeProfileDef
		//TODO: SRL: Support for fillet radii needs to be added, nb set the hascurves=true when added
		// and note too that this will decrease performance due to use of OCC for triangulation
		//NB. This is untested as we haven't enountered one yet
		//void XbimWire::Init(IIfcCraneRailFShapeProfileDef ^ profile, ILogger^ logger)
		//{
		//	double dX = profile->HeadWidth / 2;
		//	double dY = profile->OverallHeight / 2;
		//	double hd2 = profile->HeadDepth2;
		//	double hd3 = profile->HeadDepth3;
		//	double tW = profile->WebThickness;
		//	double bd1 = profile->BaseDepth1;
		//	double bd2 = profile->BaseDepth2;

		//	gp_Pnt p1(-dX, dY, 0);
		//	gp_Pnt p2(dX, dY, 0);
		//	gp_Pnt p3(dX, dY - hd3, 0);
		//	gp_Pnt p4(tW / 2, dY - hd2, 0);
		//	gp_Pnt p5(tW / 2, -dY + bd2, 0);
		//	gp_Pnt p6(dX, -dY + bd1, 0);
		//	gp_Pnt p7(dX, -dY, 0);
		//	gp_Pnt p8(-dX, -dY, 0);
		//	gp_Pnt p9(-dX, -dY + bd1, 0);
		//	gp_Pnt p10(-tW / 2, -dY + bd2, 0);
		//	gp_Pnt p11(tW / 2, dY - hd2, 0);
		//	gp_Pnt p12(-dX, dY - hd3, 0);

		//	BRepBuilderAPI_MakeWire wireMaker;

		//	wireMaker.Add(BRepBuilderAPI_MakeEdge(p1, p2));
		//	wireMaker.Add(BRepBuilderAPI_MakeEdge(p2, p3));
		//	wireMaker.Add(BRepBuilderAPI_MakeEdge(p3, p4));
		//	wireMaker.Add(BRepBuilderAPI_MakeEdge(p4, p5));
		//	wireMaker.Add(BRepBuilderAPI_MakeEdge(p5, p6));
		//	wireMaker.Add(BRepBuilderAPI_MakeEdge(p6, p7));
		//	wireMaker.Add(BRepBuilderAPI_MakeEdge(p7, p8));
		//	wireMaker.Add(BRepBuilderAPI_MakeEdge(p8, p9));
		//	wireMaker.Add(BRepBuilderAPI_MakeEdge(p9, p10));
		//	wireMaker.Add(BRepBuilderAPI_MakeEdge(p10, p11));
		//	wireMaker.Add(BRepBuilderAPI_MakeEdge(p11, p12));
		//	wireMaker.Add(BRepBuilderAPI_MakeEdge(p12, p1));
		//	TopoDS_Wire wire = wireMaker.Wire();
		//	wire.Move(XbimConvert::ToLocation(profile->Position));
		//	ShapeFix_ShapeTolerance FTol;
		//	FTol.SetTolerance(wire, profile->Model->ModelFactors->Precision, TopAbs_VERTEX);
		//	pWire = new TopoDS_Wire();
		//	*pWire = wire;
		//}
		//
		//// SRL: Builds a wire from a composite IfcCraneRailAShapeProfileDef
		////TODO: SRL: Support for fillet radii needs to be added, nb set the hascurves=true when added
		//// and note too that this will decrease performance due to use of OCC for triangulation
		////NB. This is untested as we haven't enountered one yet
		//void XbimWire::Init(IIfcCraneRailAShapeProfileDef ^ profile, ILogger^ logger)
		//{
		//	double bW = profile->HeadWidth / 2;
		//	double dY = profile->OverallHeight / 2;
		//	double hd2 = profile->HeadDepth2;
		//	double hd3 = profile->HeadDepth3;
		//	double tW = profile->WebThickness;
		//	double bd1 = profile->BaseDepth1;
		//	double bd2 = profile->BaseDepth2;
		//	double bd3 = profile->BaseDepth3;
		//	double bw2 = profile->BaseWidth2 / 2;
		//	double bw4 = profile->BaseWidth4 / 2;

		//	gp_Pnt p1(-bw4, dY, 0);
		//	gp_Pnt p2(bw4, dY, 0);
		//	gp_Pnt p3(bw4, dY - hd3, 0);
		//	gp_Pnt p4(tW / 2, dY - hd2, 0);
		//	gp_Pnt p5(tW / 2, -dY + bd2, 0);
		//	gp_Pnt p6(bw4, -dY + bd3, 0);
		//	gp_Pnt p7(bw2, -dY + bd1, 0);
		//	gp_Pnt p8(bw2, -dY, 0);
		//	gp_Pnt p9(-bw2, -dY, 0);
		//	gp_Pnt p10(-bw2, -dY + bd1, 0);
		//	gp_Pnt p11(-bw4, -dY + bd3, 0);
		//	gp_Pnt p12(tW / 2, -dY + bd2, 0);
		//	gp_Pnt p13(tW / 2, dY - hd2, 0);
		//	gp_Pnt p14(-bw4, dY - hd3, 0);

		//	BRepBuilderAPI_MakeWire wireMaker;

		//	wireMaker.Add(BRepBuilderAPI_MakeEdge(p1, p2));
		//	wireMaker.Add(BRepBuilderAPI_MakeEdge(p2, p3));
		//	wireMaker.Add(BRepBuilderAPI_MakeEdge(p3, p4));
		//	wireMaker.Add(BRepBuilderAPI_MakeEdge(p4, p5));
		//	wireMaker.Add(BRepBuilderAPI_MakeEdge(p5, p6));
		//	wireMaker.Add(BRepBuilderAPI_MakeEdge(p6, p7));
		//	wireMaker.Add(BRepBuilderAPI_MakeEdge(p7, p8));
		//	wireMaker.Add(BRepBuilderAPI_MakeEdge(p8, p9));
		//	wireMaker.Add(BRepBuilderAPI_MakeEdge(p9, p10));
		//	wireMaker.Add(BRepBuilderAPI_MakeEdge(p10, p11));
		//	wireMaker.Add(BRepBuilderAPI_MakeEdge(p11, p12));
		//	wireMaker.Add(BRepBuilderAPI_MakeEdge(p12, p13));
		//	wireMaker.Add(BRepBuilderAPI_MakeEdge(p13, p14));
		//	wireMaker.Add(BRepBuilderAPI_MakeEdge(p14, p1));
		//	TopoDS_Wire wire = wireMaker.Wire();
		//	wire.Move(XbimConvert::ToLocation(profile->Position));
		//	ShapeFix_ShapeTolerance FTol;
		//	FTol.SetTolerance(wire, profile->Model->ModelFactors->Precision, TopAbs_VERTEX);
		//	pWire = new TopoDS_Wire();
		//	*pWire = wire;
		//}
		
		// SRL: Builds a wire from a composite IfcEllipseProfileDef
		//TODO: SRL: Support for fillet radii needs to be added, nb set the hascurves=true when added
		// and note too that this will decrease performance due to use of OCC for triangulation
		//NB. This is untested as we haven't enountered one yet
		void XbimWire::Init(IIfcEllipseProfileDef ^ profile, ILogger^ /*logger*/)
		{
			gp_Ax2 gpax2;
			if (profile->Position != nullptr)
			{
				IIfcAxis2Placement2D^ ax2 = (IIfcAxis2Placement2D^)profile->Position;
				gpax2.SetLocation(gp_Pnt(ax2->Location->X, ax2->Location->Y, 0));
				gpax2.SetDirection(gp_Dir(0, 0, 1));
				gpax2.SetXDirection(gp_Dir(ax2->P[0].X, ax2->P[0].Y, 0.));
			}
			double semiAx1 = profile->SemiAxis1;
			double semiAx2 = profile->SemiAxis2;
			if (semiAx1 <= 0)
			{
				IModelFactors^ mf = profile->Model->ModelFactors;
				semiAx1 = mf->OneMilliMetre;
				//	throw gcnew XbimGeometryException("Illegal Ellipse Semi Axix, for IfcEllipseProfileDef, must be greater than 0, in entity #" + profile->EntityLabel);
			}
			if (semiAx2 <= 0)
			{
				IModelFactors^ mf = profile->Model->ModelFactors;
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
			FTol.SetTolerance(wire, profile->Model->ModelFactors->Precision, TopAbs_VERTEX);
			pWire = new TopoDS_Wire();
			*pWire = wire;
		}

		
		//TODO: SRL: Support for fillet radii needs to be added, nb set the hascurves=true when added
		// and note too that this will decrease performance due to use of OCC for triangulation
		void XbimWire::Init(IIfcIShapeProfileDef ^ profile, ILogger^ /*logger*/)
		{
			bool detailed = profile->Model->ModelFactors->ProfileDefLevelOfDetail == 1;
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
			double t = profile->Model->ModelFactors->Precision;
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
			if (profile->Position!=nullptr)
				wire.Move(XbimConvert::ToLocation(profile->Position));
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
					for (TopExp_Explorer exp(shape, TopAbs_WIRE); exp.More(); ) //just take the first wire
					{
						wire = TopoDS::Wire(exp.Current());
						break;
					}
				}
			}
			
			pWire = new TopoDS_Wire();
			*pWire = wire;
			
		}

	
		void XbimWire::Init(IIfcZShapeProfileDef ^ profile, ILogger^ /*logger*/)
		{
			bool detailed = profile->Model->ModelFactors->ProfileDefLevelOfDetail == 1;
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
					for (TopExp_Explorer exp(shape, TopAbs_WIRE); exp.More(); ) //just take the first wire
					{
						wire = TopoDS::Wire(exp.Current());
						break;
					}
				}
			}
			if (profile->Position!=nullptr)
				wire.Move(XbimConvert::ToLocation(profile->Position));
			ShapeFix_ShapeTolerance FTol;
			FTol.SetTolerance(wire, profile->Model->ModelFactors->Precision, TopAbs_VERTEX);
			pWire = new TopoDS_Wire();
			*pWire = wire;
		}

		// SRL: Builds a wire from a composite IfcCShapeProfileDef
		//TODO: SRL: Support for fillet radii needs to be added, nb set the hascurves=true when added
		// and note too that this will decrease performance due to use of OCC for triangulation
		void XbimWire::Init(IIfcCShapeProfileDef ^ profile, ILogger^ logger)
		{
			bool detailed = profile->Model->ModelFactors->ProfileDefLevelOfDetail == 1;
			double dX = profile->Width / 2;
			double dY = profile->Depth / 2;
			double dG = profile->Girth;
			double tW = profile->WallThickness;

			if (tW <= 0)
			{
				IModelFactors^ mf = profile->Model->ModelFactors;
				tW = mf->OneMilliMetre * 3;
				XbimGeometryCreator::LogWarning(logger,profile, "Illegal wall thickness for profile, it must be greater than 0. Adjusted to be 3mm thick");
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
					for (TopExp_Explorer exp(shape, TopAbs_WIRE); exp.More(); ) //just take the first wire
					{
						wire = TopoDS::Wire(exp.Current());
						break;
					}
				}
			}
			if (profile->Position!=nullptr)
				wire.Move(XbimConvert::ToLocation(profile->Position));
			//removed in Ifc4
			/*if (profile->CentreOfGravityInX.HasValue)
			{
				gp_Vec v(profile->CentreOfGravityInX.Value, 0, 0);
				gp_Trsf t;
				t.SetTranslation(v);
				wire.Move(t);
			}*/
			ShapeFix_ShapeTolerance FTol;
			FTol.SetTolerance(wire, profile->Model->ModelFactors->Precision, TopAbs_VERTEX);
			pWire = new TopoDS_Wire();
			*pWire = wire;
		}
		
		// SRL: Builds a wire from a composite IfcTShapeProfileDef
		//TODO: SRL: Support for fillet radii needs to be added, nb set the hascurves=true when added
		// and note too that this will decrease performance due to use of OCC for triangulation
		void XbimWire::Init(IIfcTShapeProfileDef ^ profile, ILogger^ /*logger*/)
		{
			bool detailed = profile->Model->ModelFactors->ProfileDefLevelOfDetail == 1;
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
			double radConv = profile->Model->ModelFactors->AngleToRadiansConversionFactor;
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
					for (TopExp_Explorer exp(shape, TopAbs_WIRE); exp.More(); ) //just take the first wire
					{
						wire = TopoDS::Wire(exp.Current());
						break;
					}
				}
			}
			if (profile->Position!=nullptr)
				wire.Move(XbimConvert::ToLocation(profile->Position));
			//removed in Ifc4
			/*if (profile->CentreOfGravityInY.HasValue)
			{
				gp_Vec v( 0, profile->CentreOfGravityInY.Value, 0);
				gp_Trsf t;
				t.SetTranslation(v);
				wire.Move(t);
			}*/
			ShapeFix_ShapeTolerance FTol;
			FTol.SetTolerance(wire, profile->Model->ModelFactors->Precision, TopAbs_VERTEX);
			pWire = new TopoDS_Wire();
			*pWire = wire;
		}

		void XbimWire::Init(double x, double y, double tolerance, bool centre)
		{	
			TopoDS_Vertex vbl, vbr, vtr, vtl;
			//make the vertices
			BRep_Builder builder;
			if (centre)
			{
				gp_Pnt bl(-x / 2, -y / 2, 0);
				gp_Pnt br(x / 2, -y / 2, 0);
				gp_Pnt tr(x / 2, y / 2, 0);
				gp_Pnt tl(-x / 2, y / 2, 0);
				builder.MakeVertex(vbl, bl, tolerance);
				builder.MakeVertex(vbr, br, tolerance);
				builder.MakeVertex(vtr, tr, tolerance);
				builder.MakeVertex(vtl, tl, tolerance);				
			}
			else
			{
				gp_Pnt bl(0, 0, 0);
				gp_Pnt br(x, 0, 0);
				gp_Pnt tr(x, y, 0);
				gp_Pnt tl(0, y, 0);
				builder.MakeVertex(vbl, bl, tolerance);
				builder.MakeVertex(vbr, br, tolerance);
				builder.MakeVertex(vtr, tr, tolerance);
				builder.MakeVertex(vtl, tl, tolerance);
			}
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

		double XbimWire::ParameterAtPoint(XbimPoint3D point, double tolerance)
		{
			if (!IsValid) return 0;
			//find the edge that contains the point
			TopoDS_Wire thisWire = *pWire;
			gp_Pnt p(point.X,point.Y,point.Z);
			double paramOffset = 0;
			for (BRepTools_WireExplorer exp(thisWire); exp.More(); exp.Next())
			{				
				Standard_Real u;
				Standard_Real fpar, lpar;
				TopLoc_Location aLoc;				
				const TopoDS_Edge& edge = TopoDS::Edge(exp.Current());
				Handle(Geom_Curve) aCurve = BRep_Tool::Curve(edge, aLoc, fpar, lpar);
				if (GeomLib_Tool::Parameter(aCurve, p, tolerance, u))
				{
					return paramOffset + u;
				}
				GProp_GProps gProps;
				BRepGProp::LinearProperties(edge, gProps);
				paramOffset += gProps.Mass();
			}
			return 0;
		}

		XbimWire^ XbimWire::Trim(XbimVertex^ first, XbimVertex^ last, double tolerance, ILogger^ logger)
		{
			double startParam = ParameterAtPoint(first->VertexGeometry,tolerance);
			double endParam = ParameterAtPoint(last->VertexGeometry,tolerance);
			return (XbimWire^)Trim(startParam, endParam, tolerance, logger);
		}

		IXbimWire^ XbimWire::Trim(double first, double last, double /*tolerance*/, ILogger^ logger)
		{
			if (!IsValid) 
				return this;
			BRepAdaptor_CompCurve cc(*pWire, Standard_True);
			GeomAbs_Shape continuity = cc.Continuity();
			int numIntervals = cc.NbIntervals(continuity);
			if (numIntervals == 1)
			{
				TopoDS_Edge edge; // the edge we are interested in
				Standard_Real uoe; // parameter U on the edge (not used)
				cc.Edge(last, edge, uoe);
				Standard_Real l, f; // the parameter range is returned in f and l
				Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, f, l);
				Standard_Real a = Math::Max(f, first);
				Standard_Real b = Math::Min(l, last);
				Handle(Geom_TrimmedCurve) trimmed = new Geom_TrimmedCurve(curve, a, b);
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
					Standard_Real fp = res.Value(i - 1);
					Standard_Real lp = res.Value(i );
					if (lp>first  && fp<last)
					{
						TopoDS_Edge edge; // the edge we are interested in
						Standard_Real uoe; // parameter U on the edge (not used)
						cc.Edge(fp, edge, uoe);
						Standard_Real l, f; // the parameter range is returned in f and l
						Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, f, l);
						Standard_Real a = Math::Max(f, first);
						Standard_Real b = Math::Min(l, last);
						wm.Add(BRepBuilderAPI_MakeEdge(new Geom_TrimmedCurve(curve, a, b)));
					}
				}
				if (wm.Error() != BRepBuilderAPI_WireDone)
				{
					// todo: for SRL, what is the correct way to handle this?
					XbimGeometryCreator::LogError(logger,this, "Error trimming. Trim discarded");
					return this;
				}
				return gcnew XbimWire(wm.Wire());
			}
		}

		void XbimWire::Move(TopLoc_Location loc)
		{
			if (IsValid) pWire->Move(loc);
		}

		void XbimWire::Mesh(IXbimMeshReceiver ^ /*mesh*/, double /*precision*/, double /*deflection*/, double /*angle*/)
		{
			throw gcnew NotImplementedException("XbimWire::Mesh");
		}

		void XbimWire::Move(IIfcAxis2Placement3D^ position)
		{
			if (!IsValid) return;
			gp_Trsf toPos = XbimConvert::ToTransform(position);
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

		XbimWire^ XbimWire::Reversed()
		{
			XbimWire^ copy = gcnew XbimWire(this);
			copy->Reverse();
			return copy;
			//if (!IsValid) return this;
			//BRep_Builder B;
			//TopoDS_Shape copy = pWire->EmptyCopied();
			//TopoDS_Iterator it(*pWire);
			//while (it.More()) {
			//	B.Add(copy, it.Value().Reversed());
			//	it.Next();
			//}
			////copy.Reverse();
			//return gcnew XbimWire(TopoDS::Wire(copy));
		}

		void XbimWire::Reverse()
		{
			if (!IsValid) return;
			pWire->Reverse();
		}

		void  XbimWire::FuseColinearSegments(double tolerance, double angleTolerance, ILogger^ logger)
		{
			if (!IsValid) return;
			// Tolerances			
			
			/*Standard_Real tol;
			for (TopExp_Explorer ExV(*pWire, TopAbs_VERTEX); ExV.More(); ExV.Next()) {
				TopoDS_Vertex Vertex = TopoDS::Vertex(ExV.Current());
				tol = BRep_Tool::Tolerance(Vertex);
				if (tol > LinTol)
					LinTol = tol;
			}*/

			//// 1. Make a copy to prevent the original shape changes.
			//TopoDS_Shape aWire;
			//TColStd_IndexedDataMapOfTransientTransient aMapTShapes;
			//TNaming_CopyShape::CopyTool(*pWire, aMapTShapes, aWire);
			TopoDS_Wire theWire = TopoDS::Wire(*pWire);
			
			TopoDS_Edge prevEdge;
			TopTools_ListOfShape finalList, currChain;

			BRepTools_WireExplorer wexp(theWire);
			if (wexp.More()) {
				prevEdge = wexp.Current();
				currChain.Append(prevEdge);
				wexp.Next();
			}
			
			for (; wexp.More(); wexp.Next()) {
				TopoDS_Edge anEdge = wexp.Current();
				TopoDS_Vertex CurVertex = wexp.CurrentVertex();
	
				if (!AreEdgesC1(prevEdge, anEdge, tolerance, angleTolerance)) 
				{
					if (currChain.Extent() == 1) 
					{
						// add one edge to the final list
						finalList.Append(currChain.First());
					}
					else 
					{
						// make wire from the list of edges
						BRep_Builder B;
						TopoDS_Wire aCurrWire;
						B.MakeWire(aCurrWire);
						TopTools_ListIteratorOfListOfShape itEdges(currChain);
						for (; itEdges.More(); itEdges.Next()) {
							TopoDS_Shape aValue = itEdges.Value();
							B.Add(aCurrWire, TopoDS::Edge(aValue));
						}
						// make edge from the wire
						XbimEdge^ xanEdge = gcnew XbimEdge(aCurrWire, tolerance, angleTolerance, logger);
						if (!xanEdge->IsValid) //probably could not get C1 continuity just add all edges
						{
							for (itEdges.Initialize(currChain); itEdges.More(); itEdges.Next()) {
								TopoDS_Shape aValue = itEdges.Value();
								finalList.Append(gcnew XbimEdge(TopoDS::Edge(aValue)));
							}
						}
						else
							// add this new edge to the final list
							finalList.Append(anEdge);
					}
					currChain.Clear();
				}
				// add one edge to the chain
				currChain.Append(anEdge);
				prevEdge = anEdge;
			}

			if (currChain.Extent() == 1) {
				// add one edge to the final list
				finalList.Append(currChain.First());
			}
			else 
			{
				// make wire from the list of edges
				BRep_Builder B;
				TopoDS_Wire aCurrWire;
				B.MakeWire(aCurrWire);
				TopTools_ListIteratorOfListOfShape itEdges(currChain);
				for (itEdges.Initialize(currChain); itEdges.More(); itEdges.Next()) {
					TopoDS_Shape aValue = itEdges.Value();
					B.Add(aCurrWire, TopoDS::Edge(aValue));
				}

				// make edge from the wire
				XbimEdge^ anEdge = gcnew XbimEdge(aCurrWire, tolerance, angleTolerance, logger);
				if (!anEdge->IsValid) //probably could not get C1 continuity just add all edges
				{
					itEdges.Initialize(currChain);
					for (; itEdges.More(); itEdges.Next()) {
						TopoDS_Shape aValue = itEdges.Value();
						anEdge = gcnew XbimEdge(TopoDS::Edge(aValue));
						finalList.Append(anEdge);
					}
				}
				else
					// add this new edge to the final list
					finalList.Append(anEdge);
			}

			BRep_Builder B;
			TopoDS_Wire aFinalWire;
			B.MakeWire(aFinalWire);
			TopTools_ListIteratorOfListOfShape itEdges(finalList);
			for (; itEdges.More(); itEdges.Next()) {
				TopoDS_Shape aValue = itEdges.Value();
				B.Add(aFinalWire, TopoDS::Edge(aValue));
			}
			*pWire = aFinalWire;
		}

		bool XbimWire::AreEdgesC1(const TopoDS_Edge& E1, const TopoDS_Edge& E2, double precision, double angularTolerance)
		{
			BRepAdaptor_Curve aCurve1(E1);
			BRepAdaptor_Curve aCurve2(E2);

			if (aCurve1.Continuity() == GeomAbs_C0 || aCurve2.Continuity() == GeomAbs_C0)
				return Standard_False;

			Standard_Real tol, tolMax = precision;
			for (TopExp_Explorer ExV1(E1, TopAbs_VERTEX); ExV1.More(); ExV1.Next()) {
				TopoDS_Vertex Vertex = TopoDS::Vertex(ExV1.Current());
				tol = BRep_Tool::Tolerance(Vertex);
				if (tol > tolMax)
					tolMax = tol;
			}
			for (TopExp_Explorer ExV2(E2, TopAbs_VERTEX); ExV2.More(); ExV2.Next()) {
				TopoDS_Vertex Vertex = TopoDS::Vertex(ExV2.Current());
				tol = BRep_Tool::Tolerance(Vertex);
				if (tol > tolMax)
					tolMax = tol;
			}

			Standard_Real f1, l1, f2, l2;
			f1 = aCurve1.FirstParameter();
			l1 = aCurve1.LastParameter();
			f2 = aCurve2.FirstParameter();
			l2 = aCurve2.LastParameter();

			if (f1 > l1) {
				Standard_Real tmp = f1;
				f1 = l1;
				l1 = tmp;
			}

			if (f2 > l2) {
				Standard_Real tmp = f2;
				f2 = l2;
				l2 = tmp;
			}

			gp_Pnt pf1, pl1, pf2, pl2;
			gp_Vec vf1, vl1, vf2, vl2;
			aCurve1.D1(f1, pf1, vf1);
			aCurve1.D1(l1, pl1, vl1);
			aCurve2.D1(f2, pf2, vf2);
			aCurve2.D1(l2, pl2, vl2);

			// pf1--->---pl1.pf2--->---pl2
			if (pl1.SquareDistance(pf2) < tolMax*tolMax) {
				if (vl1.Angle(vf2) < angularTolerance)
					return Standard_True;
			}
			// pl1---<---pf1.pf2--->---pl2
			else if (pf1.SquareDistance(pf2) < tolMax*tolMax) {
				if (vf1.Angle(-vf2) < angularTolerance)
					return Standard_True;
			}
			// pf1--->---pl1.pl2---<---pf2
			else if (pl1.SquareDistance(pl2) < tolMax*tolMax) {
				if (vl1.Angle(-vl2) < angularTolerance)
					return Standard_True;
			}
			// pl1---<---pf1.pl2---<---pf2
			else {
				if (vf1.Angle(vl2) < angularTolerance)
					return Standard_True;
			}

			return Standard_False;
		}
		//Fillets all points on a spine, not intended for closed shapes mostly for spines
		bool XbimWire::FilletAll(double radius)
		{
			TopoDS_Wire thisWire = *pWire;
			
			//collect the edges
			BRepTools_WireExplorer edgeExp;
			Standard_Integer nbEdges = 0;
			for (edgeExp.Init(thisWire); edgeExp.More(); edgeExp.Next()) nbEdges++;	

			//get an array of all the edges
			TopTools_Array1OfShape edges(1, nbEdges);
			TopTools_Array1OfShape vertices(1, nbEdges);
			TopTools_Array1OfShape filleted(1, nbEdges*2);
			Standard_Integer nb = 0;
			for (edgeExp.Init(thisWire); edgeExp.More(); edgeExp.Next()) 
			{
				nb++;
				edges(nb) = TopoDS::Edge(edgeExp.Current());
				vertices(nb) = TopoDS::Vertex(edgeExp.CurrentVertex());
			}
			//need to do each pair to ensure they are on face
			int totalEdges = 1;
			for (int i = 1; i < nbEdges; i++)
			{
				BRepBuilderAPI_MakeWire filletWireMaker;
				filletWireMaker.Add(TopoDS::Edge(edges(i)));
				filletWireMaker.Add(TopoDS::Edge(edges(i + 1)));
				BRepBuilderAPI_MakeFace faceMaker(filletWireMaker.Wire());
				BRepFilletAPI_MakeFillet2d filleter(faceMaker.Face());
				filleter.AddFillet(TopoDS::Vertex(vertices(i + 1)), radius);
				filleter.Build();
				const TopTools_SequenceOfShape& fillets = filleter.FilletEdges();
				if (filleter.IsDone() && fillets.Length()>0)
				{					
					filleted(2*i-1) = filleter.DescendantEdge(TopoDS::Edge(edges(i)));
					edges(i) = filleted(2 * i - 1);
					filleted(2*i) = fillets(1);
					filleted(2 * i + 1) = filleter.DescendantEdge(TopoDS::Edge(edges(i + 1)));
					edges(i + 1) = filleted(2 * i + 1);
					totalEdges += 2;
					
				}
				else //no fillet happened just store existing
				{
					filleted(2 * i - 1) = edges(i);
					filleted(2 * i ) = edges(i + 1);
					totalEdges ++;
				}
			}
			if (IsClosed && nbEdges>1)
			{
				BRepBuilderAPI_MakeWire filletWireMaker;
				filletWireMaker.Add(TopoDS::Edge(edges(1)));
				filletWireMaker.Add(TopoDS::Edge(edges(nbEdges)));
				BRepBuilderAPI_MakeFace faceMaker(filletWireMaker.Wire());
				BRepFilletAPI_MakeFillet2d filleter(faceMaker.Face());
				filleter.AddFillet(TopoDS::Vertex(vertices(1)), radius);
				filleter.Build();
				const TopTools_SequenceOfShape& fillets = filleter.FilletEdges();
				if (filleter.IsDone() && fillets.Length()>0)
				{
					filleted(2 * nbEdges - 1) = filleter.DescendantEdge(TopoDS::Edge(edges(nbEdges)));					
					filleted(2 * nbEdges) = fillets(1);
					filleted(1) = filleter.DescendantEdge(TopoDS::Edge(edges(1)));		
					totalEdges++;
				}				
			}
			BRepBuilderAPI_MakeWire wireMaker;
			for (int i = 1; i <= totalEdges; i++)
			{
				wireMaker.Add(TopoDS::Edge(filleted(i)));
			}

			if (wireMaker.IsDone())
			{
				*pWire = TopoDS::Wire(wireMaker.Shape());
				return true;
			}
			return false;
		}

		XbimGeometryObject ^ XbimWire::Transformed(IIfcCartesianTransformationOperator ^ transformation)
		{
			IIfcCartesianTransformationOperator3DnonUniform^ nonUniform = dynamic_cast<IIfcCartesianTransformationOperator3DnonUniform^>(transformation);
			if (nonUniform != nullptr)
			{
				gp_GTrsf trans = XbimConvert::ToTransform(nonUniform);
				BRepBuilderAPI_GTransform tr(this, trans, Standard_True); //make a copy of underlying shape
				return gcnew XbimWire(TopoDS::Wire(tr.Shape()),Tag);
			}
			else
			{
				gp_Trsf trans = XbimConvert::ToTransform(transformation);
				BRepBuilderAPI_Transform tr(this, trans, Standard_False); //do not make a copy of underlying shape
				return gcnew XbimWire(TopoDS::Wire(tr.Shape()), Tag);
			}
		}

		XbimGeometryObject ^ XbimWire::Moved(IIfcPlacement ^ placement)
		{
			if (!IsValid) return this;
			XbimWire^ copy = gcnew XbimWire(this, Tag); //take a copy of the shape
			TopLoc_Location loc = XbimConvert::ToLocation(placement);
			copy->Move(loc);
			return copy;
		}

		XbimGeometryObject ^ XbimWire::Moved(IIfcObjectPlacement ^ objectPlacement,ILogger^ logger)
		{
			if (!IsValid) return this;
			XbimWire^ copy = gcnew XbimWire(this, Tag); //take a copy of the shape
			TopLoc_Location loc = XbimConvert::ToLocation(objectPlacement, logger);
			copy->Move(loc);
			return copy;
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
			/*
			Debug::WriteLine("_.LINE");
			Debug::WriteLine("{0},{1},{2}", xn, yn, zn);
			Debug::WriteLine("{0},{1},{2}", xn1, yn1, zn1);
			Debug::WriteLine("");
			*/

			x += (yn - yn1)*(zn + zn1);
			y += (xn + xn1)*(zn - zn1);
			z += (xn - xn1)*(yn + yn1);
			/*
			Debug::WriteLine("-HYPERLINK I O l  {0},{1},{2}", x, y, z);
			Debug::WriteLine("");
			Debug::WriteLine("");
			*/
		}

		bool IfcPolylineComparer::Equals(IIfcPolyline ^x, Xbim::Ifc4::Interfaces::IIfcPolyline ^y)
		{
			//it must be a reverse
			List<IIfcCartesianPoint^>^ xPoints = Enumerable::ToList(x->Points);
			//reverse becase they cannot be identical
			List<IIfcCartesianPoint^>^ yPoints = Enumerable::ToList(y->Points);
			if (xPoints->Count < 2 || yPoints->Count < 2 || xPoints->Count!=yPoints->Count) return false;			
			double precision = x->Model->ModelFactors->Precision;
			for (int i = 0; i < xPoints->Count; i++)
			{
				XbimPoint3DWithTolerance^ xPnt = gcnew XbimPoint3DWithTolerance(xPoints[i]->X, xPoints[i]->Y, xPoints[i]->Z, precision);
				XbimPoint3DWithTolerance^ yPnt = gcnew XbimPoint3DWithTolerance(yPoints[i]->X, yPoints[i]->Y, yPoints[i]->Z, precision);
				if (xPnt != yPnt)
				{
					if (i == 0) //check reverse order
					{
						int lastIndex = yPoints->Count - 1;
						yPnt = gcnew XbimPoint3DWithTolerance(yPoints[lastIndex]->X, yPoints[lastIndex]->Y, yPoints[lastIndex]->Z, precision);
						if (xPnt != yPnt) return false; //they are diferent
						else yPoints = Enumerable::ToList(Enumerable::Reverse(yPoints));
					}
					else return false;
				}
			}
			return true;	
		}

		

		int IfcPolylineComparer::GetHashCode(IIfcPolyline ^pline)
		{
			//simple hash that does not distinguish between the direction of the line
			//only considers the start,  end points and number of points for uniqueness
			
			List<IIfcCartesianPoint^>^ points = Enumerable::ToList(pline->Points);
			int hash = points->Count;
			if (hash == 0) return hash;			
			IIfcCartesianPoint^ start = points[0];
			hash += (int)start->X; //do them all as integers to avoid precision errors
			hash += (int)start->Y;
			hash += (int)start->Z;
			if (hash == 1) return hash;
			IIfcCartesianPoint^ end = points[points->Count-1];					
			hash += (int)end->X; //do them all as integers to avoid precision errors
			hash += (int)end->Y;
			hash += (int)end->Z;
			return hash;
		}

		bool IfcPolylineComparer::IsSameDirection(IIfcPolyline^ pline, IXbimWire^ polyWire)
		{
			IIfcCartesianPoint^ startLookup = Enumerable::FirstOrDefault(pline->Points);			
			IIfcPolyline^ original = dynamic_cast<IIfcPolyline^>(polyWire->Tag);
			if (startLookup == nullptr || original ==nullptr) throw gcnew XbimException("Illegal use of IsSameDirection");
			IIfcCartesianPoint^ startOriginal = Enumerable::FirstOrDefault(original->Points);
			XbimPoint3DWithTolerance^ startLookupPnt = gcnew XbimPoint3DWithTolerance(startLookup->X, startLookup->Y, startLookup->Z, pline->Model->ModelFactors->Precision);
			XbimPoint3DWithTolerance^ startOriginalPnt = gcnew XbimPoint3DWithTolerance(startOriginal->X, startOriginal->Y, startOriginal->Z, pline->Model->ModelFactors->Precision);
			return startLookupPnt == startOriginalPnt;
		}
#pragma warning( push )
#pragma warning( disable : 4701)
		bool XbimWire::SortEdgesForWire(const NCollection_Vector<TopoDS_Edge>& oldedges, NCollection_Vector<TopoDS_Edge>& newedges, NCollection_Vector<TopoDS_Edge>& notTaken, double tol, bool *pClosed, double* pMaxGap)
		{
			int i, n, minID, id, id2;
			NCollection_Vector<int> bTaken;
			NCollection_Vector<gp_Pnt> pnts;
			TopExp_Explorer ex;
			std::vector<TopoDS_Edge> edges;
			gp_Pnt pnt, endPnts[2];
			bool bSucc;
			double minDis, otherDis, minminDis;
			
			TopoDS_Vertex v1, v2;
			TopoDS_Edge degEdge;
			notTaken.Clear();
			newedges.Clear();
			if (pMaxGap)
				*pMaxGap = 0.0;
			n = oldedges.Length();
			if (n == 0)
				return false;
			
			for (i = 0; i<n; ++i)
			{
				TopExp::Vertices(oldedges(i), v1, v2);
				pnts.Append(BRep_Tool::Pnt(v1));
				pnts.Append(BRep_Tool::Pnt(v2));
				bTaken.Append(0);
			}

			endPnts[0] = pnts(0);
			endPnts[1] = pnts(1);
			bTaken(0) = 1;
			edges.push_back(oldedges(0));

			while (1)
			{
				bSucc = false;
				minID = -1;
				minminDis = DBL_MAX;
				for (i = 0; i<n; ++i)
				{
					if (bTaken(i) != 0)
						continue;
					id = GetMatchTwoPntsPair(pnts(2 * i), pnts(2 * i + 1), endPnts[0], endPnts[1], minDis, otherDis);
					if (minDis<minminDis)
					{
						id2 = id;
						minID = i;
						minminDis = minDis;
					}
				}
				if (minID != -1 && minminDis<tol)
				{
					bTaken(minID) = 1;
					bSucc = true;
					if (id2 == 0)
					{
						edges.insert(edges.begin(), oldedges(minID));
						endPnts[0] = pnts(2 * minID + 1);
					}
					else if (id2 == 1)
					{
						edges.push_back(oldedges(minID));
						endPnts[1] = pnts(2 * minID + 1);
					}
					else if (id2 == 2)
					{
						edges.insert(edges.begin(), oldedges(minID));
						endPnts[0] = pnts(2 * minID);
					}
					else if (id2 == 3)
					{
						edges.push_back(oldedges(minID));
						endPnts[1] = pnts(2 * minID);
					}
					if (pMaxGap)
					{
						if (*pMaxGap<minminDis)
							*pMaxGap = minminDis;
					}
				}
				if (!bSucc)
					break;
			}

			if (edges.size() != (size_t)oldedges.Length())
			{
				for (int x = 0; x < n; x++)
				{
					if (bTaken(x) == 0)
						notTaken.Append(oldedges(x));
					else
						newedges.Append(oldedges(x));
				}
				return false;
			}

			if (endPnts[1].Distance(endPnts[0])<tol) //close
			{
				if (pClosed)
					*pClosed = true;
				int startID = -1;
				for (i = 0; i<(int)(edges.size()); ++i)
				{
					if (oldedges(0).IsSame(edges[i]))
					{
						startID = i;
						break;
					}
				}
				if (startID == -1)
				{
					for (i = 0; i<(int)(edges.size()); ++i)
						newedges.Append(edges[i]);
				}
				else
				{
					for (i = startID; i<(int)(edges.size()); ++i)
						newedges.Append(edges[i]);
					for (i = 0; i<startID; ++i)
						newedges.Append(edges[i]);
				}
			}
			else
			{
				if (pClosed)
					*pClosed = false;

				for (i = 0; i<(int)(edges.size()); ++i)
					newedges.Append(edges[i]);
			}

			return true;
		}

#pragma warning( pop )


		int XbimWire::GetMatchTwoPntsPair(const gp_Pnt& b1, const gp_Pnt& e1, const gp_Pnt& b2, const gp_Pnt& e2,
			double& minDis, double& otherDis)
		{
			double distance[4];
			int index = -1, i;

			distance[0] = b1.SquareDistance(b2);
			distance[1] = b1.SquareDistance(e2);
			distance[2] = e1.SquareDistance(b2);
			distance[3] = e1.SquareDistance(e2);
			minDis = DBL_MAX;
			for (i = 0; i<4; ++i)
			{
				if (distance[i]<minDis)
				{
					minDis = distance[i];
					index = i;
				}
			}
			otherDis = distance[3 - index];

			minDis = sqrt(minDis);
			otherDis = sqrt(otherDis);
			return index;
		}
}
}