#include "XbimFace.h"
#include "XbimWire.h"
#include "XbimWireSet.h"

#include <BRepBuilderAPI_Transform.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <ShapeFix_ShapeTolerance.hxx>
#include <BRepCheck_Analyzer.hxx>
#include "XbimGeometryCreator.h"
#include "XbimGeomPrim.h"
#include <TopExp_Explorer.hxx>
#include <BRep_Builder.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepTools.hxx>
#include <TopoDS.hxx>
#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>
#include <ShapeFix_ShapeTolerance.hxx>
#include <BRepGProp_Face.hxx>
#include <gp_Circ.hxx>
#include <GC_MakeCircle.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <GeomLib_IsPlanarSurface.hxx>
#include <BRep_Tool.hxx>
#include <Geom_Surface.hxx>
#include <Geom_SurfaceOfLinearExtrusion.hxx>
#include <gp_Pln.hxx>
#include <BRepAdaptor_CompCurve.hxx>
#include <Geom_SurfaceOfRevolution.hxx>
#include <Geom_RectangularTrimmedSurface.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopExp.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Line.hxx>
#include <Geom_TrimmedCurve.hxx>
namespace Xbim
{
	namespace Geometry
	{
		/*Ensures native pointers are deleted and garbage collected*/
		void XbimFace::InstanceCleanup()
		{
			IntPtr temp = System::Threading::Interlocked::Exchange(ptrContainer, IntPtr::Zero);
			if (temp != IntPtr::Zero)
				delete (TopoDS_Face*)(temp.ToPointer());
			System::GC::SuppressFinalize(this);
		}

		String^ XbimFace::GetBuildFaceErrorMessage(BRepBuilderAPI_FaceError err)
		{
			switch (err)
			{
			case BRepBuilderAPI_NoFace:
				return "No Face";
			case BRepBuilderAPI_NotPlanar:
				return "Not Planar";
			case BRepBuilderAPI_CurveProjectionFailed:
				return "Curve Projection Failed";
			case BRepBuilderAPI_ParametersOutOfRange:
				return "Parameters Out Of Range";
			case BRepBuilderAPI_FaceDone:
				return "";
			default:
				return "Unknown Error";
			}
		}

		XbimFace::XbimFace(XbimPoint3D l, XbimVector3D n)
		{
			gp_Pln plane(gp_Pnt(l.X, l.Y, l.Z), gp_Dir(n.X, n.Y, n.Z));
			BRepBuilderAPI_MakeFace faceMaker(plane);
			pFace = new TopoDS_Face();
			*pFace = faceMaker.Face();
		}

		XbimFace::XbimFace(XbimVector3D n)
		{
			gp_Pln plane(gp_Pnt(0,0,0),  gp_Dir(n.X,n.Y,n.Z));
			BRepBuilderAPI_MakeFace faceMaker(plane);
			pFace = new TopoDS_Face();
			*pFace = faceMaker.Face();
		}

		XbimFace::XbimFace(const TopoDS_Face& face)
		{
			pFace = new TopoDS_Face();
			*pFace = face;
		}



		XbimFace::XbimFace(IfcProfileDef^ profile)
		{
			Init(profile);
		}

		XbimFace::XbimFace(IfcSurface^ surface)
		{
			Init(surface);
		}

		XbimFace::XbimFace(IfcCurveBoundedPlane^ def)
		{
			Init(def);
		}
		
		XbimFace::XbimFace(IfcRectangularTrimmedSurface^ def)
		{
			Init(def);
		}

		XbimFace::XbimFace(IfcPlane^ plane)
		{
			Init(plane);
		}

		XbimFace::XbimFace(IfcSurfaceOfLinearExtrusion^ sLin)
		{
			Init(sLin);
		}
		
		XbimFace::XbimFace(IfcSurfaceOfRevolution^ sRev)
		{
			Init(sRev);
		}
		XbimFace::XbimFace(IfcCompositeCurve ^ cCurve)
		{
			Init(cCurve);
		}

		XbimFace::XbimFace(IfcPolyline ^ pline)
		{
			Init(pline);
		}

		XbimFace::XbimFace(IfcPolyLoop ^ loop)
		{
			Init(loop);
		}

		XbimFace::XbimFace(IXbimWire^ wire)
		{
			Init(wire);
		}

		XbimFace::XbimFace(IXbimWire^ wire, XbimPoint3D pointOnFace, XbimVector3D faceNormal)
		{
			Init(wire, pointOnFace, faceNormal);
		}

		XbimFace::XbimFace(IXbimFace^ face)
		{
			Init(face);
		}

		XbimFace::XbimFace(double x, double y, double tolerance)
		{
			Init(x,y,tolerance);
		}

		void XbimFace::Init(IfcCompositeCurve ^ cCurve)
		{
			XbimWire^ wire = gcnew XbimWire(cCurve);
			if (wire->IsValid)
			{
				BRepBuilderAPI_MakeFace faceMaker(wire);
				pFace = new TopoDS_Face();
				*pFace = faceMaker.Face();;
			}
		}

		void XbimFace::Init(IfcPolyline ^ pline)
		{
			XbimWire^ wire = gcnew XbimWire(pline);
			if (wire->IsValid)
			{
				BRepBuilderAPI_MakeFace faceMaker(wire);
				pFace = new TopoDS_Face();
				*pFace = faceMaker.Face();;
			}
		}

		void XbimFace::Init(IfcPolyLoop ^ loop)
		{
			XbimWire^ wire = gcnew XbimWire(loop);
			if (wire->IsValid)
			{
				BRepBuilderAPI_MakeFace faceMaker(wire);
				pFace = new TopoDS_Face();
				*pFace = faceMaker.Face();;
			}
		}

		void XbimFace::Init(IXbimWire^ xbimWire, XbimPoint3D pointOnFace,  XbimVector3D faceNormal)
		{
			if (!dynamic_cast<XbimWire^>(xbimWire))
				throw gcnew ArgumentException("Only IXbimWires created by Xbim.OCC modules are supported", "xbimWire");
			XbimWire^ wire = (XbimWire^)xbimWire;
			if (wire->IsValid && !faceNormal.IsInvalid())
			{			
				gp_Pln plane(gp_Pnt(pointOnFace.X, pointOnFace.Y, pointOnFace.Z), gp_Dir(faceNormal.X, faceNormal.Y, faceNormal.Z));
				BRepBuilderAPI_MakeFace faceMaker(plane, wire, Standard_False);
				pFace = new TopoDS_Face();
				*pFace = faceMaker.Face();
			}
			GC::KeepAlive(xbimWire);
		}

		void XbimFace::Init(IXbimWire^ xbimWire)
		{
			if (!dynamic_cast<XbimWire^>(xbimWire))
				throw gcnew ArgumentException("Only IXbimWires created by Xbim.OCC modules are supported", "xbimWire");
			XbimWire^ wire = (XbimWire^)xbimWire;
			if (wire->IsValid)
			{
				XbimPoint3D pw = wire->Vertices->First->VertexGeometry;
				XbimVector3D n = wire->Normal;
				if (n.IsInvalid()) return;
				gp_Pln plane(gp_Pnt(pw.X,pw.Y,pw.Z),gp_Dir(n.X,n.Y,n.Z));
				BRepBuilderAPI_MakeFace faceMaker(plane,wire, Standard_False);
				pFace = new TopoDS_Face();
				*pFace = faceMaker.Face();
			}
			GC::KeepAlive(xbimWire);
		}

		

		void XbimFace::Init(IXbimFace^ face)
		{
			IXbimWire^ outerBound = face->OuterBound;
			XbimWire^ outerWire = dynamic_cast<XbimWire^>(outerBound);
			if (outerWire == nullptr) throw gcnew ArgumentException("Only IXbimWires created by Xbim.OCC modules are supported", "xbimWire");
			XbimVector3D n = face->Normal;		
			XbimPoint3D pw = outerWire->Vertices->First->VertexGeometry;
			gp_Pln plane(gp_Pnt(pw.X, pw.Y, pw.Z), gp_Dir(n.X, n.Y, n.Z));
			BRepBuilderAPI_MakeFace faceMaker(plane, outerWire, Standard_False);
			for each (IXbimWire^ innerBound in face->InnerBounds)
			{
				XbimWire^ innerWire = dynamic_cast<XbimWire^>(innerBound);
				if (innerWire != nullptr)
					faceMaker.Add(innerWire);
			}
			pFace = new TopoDS_Face();
			*pFace = faceMaker.Face();
		}

		void XbimFace::Init(IfcProfileDef^ profile)
		{
			IfcArbitraryProfileDefWithVoids^ arbProfDefVoids = dynamic_cast<IfcArbitraryProfileDefWithVoids^>(profile);
			if (arbProfDefVoids != nullptr) //this is a compound wire so we need to build it at face level
				return Init(arbProfDefVoids);
			IfcCircleHollowProfileDef^ circHollow = dynamic_cast<IfcCircleHollowProfileDef^>(profile);
			if (circHollow != nullptr) //this is a compound wire so we need to build it at face level
				return Init(circHollow);
			IfcRectangleHollowProfileDef^ rectHollow = dynamic_cast<IfcRectangleHollowProfileDef^>(profile);
			if (rectHollow != nullptr) //this is a compound wire so we need to build it at face level
				return Init(rectHollow);
			else if (dynamic_cast<IfcArbitraryOpenProfileDef^>(profile) && !dynamic_cast<IfcCenterLineProfileDef^>(profile))
				throw gcnew Exception("Faces cannot be built with IfcArbitraryOpenProfileDef, a face requires a closed loop");
			else //it is a standard profile that can be built as a single wire
			{
				XbimWire^ wire = gcnew XbimWire(profile);
				if (wire->IsValid)
				{
					double tolerance = profile->ModelOf->ModelFactors->Precision;
					double toleranceMax = profile->ModelOf->ModelFactors->PrecisionMax;
					ShapeFix_ShapeTolerance FTol;
					double currentFaceTolerance = tolerance;
				TryBuildFace:
					BRepBuilderAPI_MakeFace faceMaker(wire, true);
					BRepBuilderAPI_FaceError err = faceMaker.Error();
					if (err == BRepBuilderAPI_NotPlanar)
					{
						currentFaceTolerance *= 10;
						if (currentFaceTolerance <= toleranceMax)
						{
							FTol.SetTolerance(wire, currentFaceTolerance, TopAbs_WIRE);
							goto TryBuildFace;
						}
						String^ errMsg = XbimFace::GetBuildFaceErrorMessage(err);
						XbimGeometryCreator::logger->WarnFormat("WF004: Invalid bound, {0}. Found in ProfileDef = #{1}, face discarded", errMsg, profile->EntityLabel);
						return;
					}
					else
					{
						pFace = new TopoDS_Face();
						*pFace = faceMaker.Face();
					}
				}
			}
		}

		void XbimFace::Init(IfcArbitraryProfileDefWithVoids^ profile)
		{
			double tolerance = profile->ModelOf->ModelFactors->Precision;
			double toleranceMax = profile->ModelOf->ModelFactors->PrecisionMax;
			ShapeFix_ShapeTolerance FTol;
			TopoDS_Face face;
			XbimWire^ loop = gcnew XbimWire(profile->OuterCurve);
			if (loop->IsValid)
			{
				double currentFaceTolerance = tolerance;
			TryBuildFace:
				BRepBuilderAPI_MakeFace faceMaker(loop, false);
				BRepBuilderAPI_FaceError err = faceMaker.Error();
				if (err == BRepBuilderAPI_NotPlanar)
				{
					currentFaceTolerance *= 10;
					if (currentFaceTolerance <= toleranceMax)
					{
						FTol.SetTolerance(loop, currentFaceTolerance, TopAbs_WIRE);
						goto TryBuildFace;
					}
					String^ errMsg = XbimFace::GetBuildFaceErrorMessage(err);
					XbimGeometryCreator::logger->WarnFormat("WF005: Invalid bound, {0}. Found in IfcArbitraryClosedProfileDefWithVoids = #{1}, face discarded", errMsg, profile->EntityLabel);
					return;
				}
				pFace = new TopoDS_Face();
				*pFace = faceMaker.Face();
				XbimVector3D tn = Normal;

				for each(IfcCurve^ curve in profile->InnerCurves)
				{

					XbimWire^ innerWire = gcnew XbimWire(curve);
					if (innerWire->IsClosed) //if the loop is not closed it is not a bound
					{
						XbimVector3D n = innerWire->Normal;
						if (n.DotProduct(tn) > 0) //inner wire should be reverse of outer wire
							innerWire->Reverse();
						double currentloopTolerance = tolerance;
					TryBuildLoop:
						faceMaker.Add(innerWire);
						BRepBuilderAPI_FaceError loopErr = faceMaker.Error();
						if (loopErr != BRepBuilderAPI_FaceDone)
						{
							currentloopTolerance *= 10; //try courser tolerance
							if (currentloopTolerance <= toleranceMax)
							{
								FTol.SetTolerance(innerWire, currentloopTolerance, TopAbs_WIRE);
								goto TryBuildLoop;
							}

							String^ errMsg = XbimFace::GetBuildFaceErrorMessage(loopErr);
							XbimGeometryCreator::logger->WarnFormat("WF006: Invalid void, {0}. IfcCurve #(1) could not be added to IfcArbitraryClosedProfileDefWithVoids = #{2}. Inner Bound ignored", errMsg, curve->EntityLabel, profile->EntityLabel);
						}
						*pFace = faceMaker.Face();
					}
					else
					{
						XbimGeometryCreator::logger->InfoFormat("WF007: Invalid void in IfcArbitraryClosedProfileDefWithVoids #{0}. It is not a hole. Void discarded", curve->EntityLabel);
					}
				}
			}
		}

		//Builds a face from a CircleProfileDef
		void XbimFace::Init(IfcCircleHollowProfileDef ^ circProfile)
		{
			if (circProfile->Radius <= 0)
			{
				XbimGeometryCreator::logger->WarnFormat("WW014:Invalid IfcCircleHollowProfileDef #{0}: Has a radius <= 0. Face discarded", circProfile->EntityLabel);
				return;
			}
			IfcAxis2Placement2D^ ax2 = (IfcAxis2Placement2D^)circProfile->Position;
			gp_Ax2 gpax2(gp_Pnt(ax2->Location->X, ax2->Location->Y, 0), gp_Dir(0, 0, 1), gp_Dir(ax2->P[0].X, ax2->P[0].Y, 0.));

			//make the outer wire
			gp_Circ outer(gpax2, circProfile->Radius);
			Handle(Geom_Circle) hOuter = GC_MakeCircle(outer);
			TopoDS_Edge outerEdge = BRepBuilderAPI_MakeEdge(hOuter);
			BRepBuilderAPI_MakeWire outerWire;
			outerWire.Add(outerEdge);
			double innerRadius = circProfile->Radius - circProfile->WallThickness;
			BRepBuilderAPI_MakeFace faceBlder(outerWire);
			//now add inner wire
			if (innerRadius>0)
			{
				gp_Circ inner(gpax2, circProfile->Radius - circProfile->WallThickness);
				Handle(Geom_Circle) hInner = GC_MakeCircle(inner);
				TopoDS_Edge innerEdge = BRepBuilderAPI_MakeEdge(hInner);
				BRepBuilderAPI_MakeWire innerWire;
				innerEdge.Reverse();
				innerWire.Add(innerEdge);
				
				faceBlder.Add(innerWire);
			}
			pFace = new TopoDS_Face();
			*pFace = faceBlder.Face();
		}


		void XbimFace::Init(IfcRectangleHollowProfileDef^ rectProfile)
		{
			if (rectProfile->XDim <= 0 || rectProfile->YDim <= 0)
			{
				XbimGeometryCreator::logger->WarnFormat("WW014:Invalid IfcRectangleProfileDef: #{0}, XDim = {1}, YDim = {2}. Face discarded", rectProfile->EntityLabel, rectProfile->XDim, rectProfile->YDim);
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

				//make the face
				BRepBuilderAPI_MakeFace faceBlder(wire);
				//calculate hole
				if (rectProfile->WallThickness <= 0)
					XbimGeometryCreator::logger->WarnFormat("WF008: Wall thickness of a rectangle hollow profile #{0} def must be greater than 0, thickness has been ignored.", rectProfile->EntityLabel);
				else
				{
					TopoDS_Wire innerWire;
					builder.MakeWire(innerWire);
					double t = rectProfile->WallThickness;
					gp_Pnt ibl(-xOff+t, -yOff+t, 0);
					gp_Pnt ibr(xOff-t, -yOff+t, 0);
					gp_Pnt itr(xOff-t, yOff-t, 0);
					gp_Pnt itl(-xOff+t, yOff-t, 0);
					TopoDS_Vertex vibl, vibr, vitr, vitl;
					builder.MakeVertex(vibl, ibl, precision);
					builder.MakeVertex(vibr, ibr, precision);
					builder.MakeVertex(vitr, itr, precision);
					builder.MakeVertex(vitl, itl, precision);
					builder.Add(innerWire, BRepBuilderAPI_MakeEdge(vibl, vibr));
					builder.Add(innerWire, BRepBuilderAPI_MakeEdge(vibr, vitr));
					builder.Add(innerWire, BRepBuilderAPI_MakeEdge(vitr, vitl));
					builder.Add(innerWire, BRepBuilderAPI_MakeEdge(vitl, vibl));
					innerWire.Reverse();
					faceBlder.Add(innerWire);
				}				
				pFace = new TopoDS_Face();
				*pFace = faceBlder.Face();
				//apply the position transformation
				pFace->Move(XbimGeomPrim::ToLocation(rectProfile->Position));

			}

		}

		//Builds a face from a Surface
		void XbimFace::Init(IfcSurface ^ surface)
		{
			if (dynamic_cast<IfcPlane^>(surface))
				return Init((IfcPlane^)surface);
			else if (dynamic_cast<IfcSurfaceOfRevolution^>(surface))
				return Init((IfcSurfaceOfRevolution^)surface);
			else if (dynamic_cast<IfcSurfaceOfLinearExtrusion^>(surface))
				return Init((IfcSurfaceOfLinearExtrusion^)surface);
			else if (dynamic_cast<IfcCurveBoundedPlane^>(surface))
				return Init((IfcCurveBoundedPlane^)surface);
			else if (dynamic_cast<IfcRectangularTrimmedSurface^>(surface))
				return Init((IfcRectangularTrimmedSurface^)surface);
			else if (dynamic_cast<IfcBoundedSurface^>(surface))
				throw(gcnew NotImplementedException("XbimFace. Support for BoundedSurface is not implemented,it should be abstract"));
			else
			{
				Type ^ type = surface->GetType();
				throw(gcnew NotImplementedException(String::Format("XbimFace. BuildFace of type {0} is not implemented", type->Name)));
			}

		}

		//Builds a face from a Plane
		void XbimFace::Init(IfcPlane ^ plane)
		{
			gp_Ax3 ax3 = XbimGeomPrim::ToAx3(plane->Position);
			gp_Pln pln(ax3);
			BRepBuilderAPI_MakeFace  builder(pln);
			pFace = new TopoDS_Face();
			*pFace =  builder.Face();
		}
		void XbimFace::Init(IfcSurfaceOfRevolution ^ sRev)
		{
			XbimWire^ curve = gcnew XbimWire(sRev->SweptCurve);
			if (!curve->IsValid || curve->Edges->Count > 1)
			{
				XbimGeometryCreator::logger->ErrorFormat("WF010: Invalid swept curve = #{0}. Found in IfcSurfaceOfRevolution = #{1}, face discarded", sRev->SweptCurve->EntityLabel, sRev->EntityLabel);
				return;
			}
			XbimEdge^ edge = (XbimEdge^)curve->Edges->First;
			TopLoc_Location loc;
			Standard_Real start, end;
			Handle(Geom_Curve) c3d = BRep_Tool::Curve(edge, loc, start, end);

			gp_Pnt origin(sRev->AxisPosition->Location->X, sRev->AxisPosition->Location->Y, sRev->AxisPosition->Location->Z);
			gp_Dir axisDir(0,0,1);
			if (sRev->AxisPosition->Axis != nullptr)
				axisDir = gp_Dir(sRev->AxisPosition->Axis->X, sRev->AxisPosition->Axis->Y, sRev->AxisPosition->Axis->Z);
			gp_Ax1 axis(origin,axisDir);

			Handle(Geom_SurfaceOfRevolution) geomLin(new  Geom_SurfaceOfRevolution(c3d, axis));

			BRepBuilderAPI_MakeFace faceMaker(geomLin, 0, 2* Math::PI, start, end, sRev->ModelOf->ModelFactors->Precision);
			if (faceMaker.IsDone())
			{
				pFace = new TopoDS_Face();
				*pFace = faceMaker.Face();
			}
			else
				XbimGeometryCreator::logger->ErrorFormat("WF011: Invalid swept curve = #{0}. Found in IfcSurfaceOfRevolution = #{1}, face discarded", sRev->SweptCurve->EntityLabel, sRev->EntityLabel);

		}

		void XbimFace::Init(IfcRectangularTrimmedSurface^ def)
		{
			Init(def->BasisSurface); //initialise the plane
			if (IsValid)
			{
				TopLoc_Location loc;
				Handle(Geom_Surface) surface = BRep_Tool::Surface(this, loc);
				Handle(Geom_RectangularTrimmedSurface) geomTrim(new  Geom_RectangularTrimmedSurface(surface,def->U1,def->U2,def->V1,def->V2));

				BRepBuilderAPI_MakeFace faceMaker(geomTrim,def->ModelOf->ModelFactors->Precision);
				if (faceMaker.IsDone())
				{
					pFace = new TopoDS_Face();
					*pFace = faceMaker.Face();
				}
				else
					XbimGeometryCreator::logger->ErrorFormat("WF015: Invalid trimed surface = #{0}. Found in IfcRectangularTrimmedSurface = #{1}, face discarded", def->BasisSurface->EntityLabel, def->EntityLabel);
			}
		}

		void XbimFace::Init(IfcCurveBoundedPlane^ def)
		{
			Init(def->BasisSurface); //initialise the plane
			if (IsValid)
			{
				XbimWire^ outerBound = gcnew XbimWire(def->OuterBoundary);
				BRepBuilderAPI_MakeFace  builder(this);
				builder.Add(outerBound);
				for each (IfcCurve^ innerCurve in def->InnerBoundaries)
				{
					XbimWire^ innerBound = gcnew XbimWire(innerCurve);
					if (innerBound->IsValid)
						builder.Add(innerBound);
					else
						XbimGeometryCreator::logger->ErrorFormat("WF014: Invalid inner bound = #{0}. Found in IfcCurveBoundedPlane = #{1}, face discarded", innerCurve->EntityLabel, def->EntityLabel);

				}
				if (builder.IsDone())
				{
					pFace = new TopoDS_Face();
					*pFace = builder.Face();
				}
				else
					XbimGeometryCreator::logger->ErrorFormat("WF013: Invalid outer bound = #{0}. Found in IfcCurveBoundedPlane = #{1}, face discarded", def->OuterBoundary->EntityLabel, def->EntityLabel);
			}
		}

		void XbimFace::Init(IfcSurfaceOfLinearExtrusion ^ sLin)
		{
			XbimWire^ curve = gcnew XbimWire(sLin->SweptCurve);
			if (!curve->IsValid || curve->Edges->Count > 1)
			{
				XbimGeometryCreator::logger->ErrorFormat("WF001: Invalid swept curve = #{0}. Found in IfcSurfaceOfLinearExtrusion = #{1}, face discarded", sLin->SweptCurve->EntityLabel, sLin->EntityLabel);
				return;
			}
			XbimEdge^ edge = (XbimEdge^)curve->Edges->First;
			TopLoc_Location loc;
			Standard_Real start, end;
			Handle(Geom_Curve) c3d = BRep_Tool::Curve(edge, loc, start, end);
			gp_Vec extrude(sLin->ExtrudedDirection->X, sLin->ExtrudedDirection->Y, sLin->ExtrudedDirection->Z);
			
			Handle(Geom_SurfaceOfLinearExtrusion) geomLin(new  Geom_SurfaceOfLinearExtrusion(c3d, extrude));
			
			BRepBuilderAPI_MakeFace faceMaker(geomLin, start, end, 0, sLin->Depth,sLin->ModelOf->ModelFactors->Precision);
			if (faceMaker.IsDone())
			{
				pFace = new TopoDS_Face();
				*pFace = faceMaker.Face();
			}
			else
				XbimGeometryCreator::logger->ErrorFormat("WF002: Invalid swept curve = #{0}. Found in IfcSurfaceOfLinearExtrusion = #{1}, face discarded", sLin->SweptCurve->EntityLabel, sLin->EntityLabel);


		}

		void  XbimFace::Init(double x, double y, double tolerance)
		{
			XbimWire^ bounds = gcnew XbimWire(x, y, tolerance);
			if (bounds->IsValid)
			{
				BRepBuilderAPI_MakeFace faceMaker(bounds);
				pFace = new TopoDS_Face();
				*pFace = faceMaker.Face();
			}
		}

#pragma region IXbimFace Interface

		IXbimWire^ XbimFace::OuterBound::get()
		{
			if (pFace == nullptr) return nullptr;
			TopoDS_Wire outerWire = BRepTools::OuterWire(*pFace);//get the outer loop
			GC::KeepAlive(this);
			if (outerWire.IsNull()) //check if this is a half space
			{
				return nullptr;
			}
			else
			{
				return gcnew XbimWire(outerWire);
			}
			
		}

		IXbimWireSet^ XbimFace::Bounds::get()
		{
			if (!IsValid) return XbimWireSet::Empty; //return an empty list, avoid using Enumberable::Empty to avoid LINQ dependencies			
			return gcnew XbimWireSet(this);
		}

		IXbimWireSet^ XbimFace::InnerBounds::get()
		{
			if (!IsValid) return XbimWireSet::Empty; //return an empty list, avoid using Enumberable::Empty to avoid LINQ dependencies
			TopoDS_Wire outerWire = BRepTools::OuterWire(*pFace);//get the outer loop
			if (outerWire.IsNull()) return XbimWireSet::Empty;//check if this is a half space
			TopTools_ListOfShape wires;
			for (TopExp_Explorer wireEx(*pFace, TopAbs_WIRE); wireEx.More(); wireEx.Next())
			{
				if (!wireEx.Current().IsEqual(outerWire))
					wires.Append(TopoDS::Wire(wireEx.Current()));
			}
			GC::KeepAlive(this);
			return gcnew XbimWireSet(wires);
		}


		XbimRect3D XbimFace::BoundingBox::get()
		{
			if (pFace == nullptr) return XbimRect3D::Empty;
			Bnd_Box pBox;
			BRepBndLib::Add(*pFace, pBox);
			Standard_Real srXmin, srYmin, srZmin, srXmax, srYmax, srZmax;
			if (pBox.IsVoid()) return XbimRect3D::Empty;
			pBox.Get(srXmin, srYmin, srZmin, srXmax, srYmax, srZmax);
			GC::KeepAlive(this);
			return XbimRect3D(srXmin, srYmin, srZmin, (srXmax - srXmin), (srYmax - srYmin), (srZmax - srZmin));
		}

		IXbimGeometryObject^ XbimFace::Transform(XbimMatrix3D matrix3D)
		{
			BRepBuilderAPI_Copy copier(this);
			BRepBuilderAPI_Transform gTran(copier.Shape(), XbimGeomPrim::ToTransform(matrix3D));
			TopoDS_Face temp = TopoDS::Face(gTran.Shape());
			return gcnew XbimFace(temp);
		}

		bool XbimFace::IsQuadOrTriangle::get()
		{
			if (!IsValid) return false;
			if (!IsPlanar) return false;
			TopExp_Explorer wireEx(*pFace, TopAbs_WIRE);
			wireEx.Next();
			if (wireEx.More()) return false; //it has holes
			TopTools_IndexedMapOfShape map;
			TopExp::MapShapes(*pFace, TopAbs_VERTEX, map);
			GC::KeepAlive(this);
			if (map.Extent() == 4 || map.Extent() == 3) return true;
			return false;
		}


		double XbimFace::Area::get()
		{
			if (IsValid)
			{
				GProp_GProps gProps;
				BRepGProp::SurfaceProperties(*pFace, gProps);
				GC::KeepAlive(this);
				return gProps.Mass();
			}
			else
				return 0;
		}

		double XbimFace::Perimeter::get()
		{
			if (IsValid)
			{
				GProp_GProps gProps;
				BRepGProp::LinearProperties(*pFace, gProps);
				GC::KeepAlive(this);
				return gProps.Mass();
			}
			else
				return 0;

		}

		XbimVector3D XbimFace::Normal::get()
		{
			if (!IsValid) return XbimVector3D();
			BRepGProp_Face prop(*pFace);
			gp_Pnt centre;
			gp_Vec normalDir;
			double u1, u2, v1, v2;
			prop.Bounds(u1, u2, v1, v2);
			prop.Normal((u1 + u2) / 2.0, (v1 + v2) / 2.0, centre, normalDir);
			XbimVector3D vec(normalDir.X(), normalDir.Y(), normalDir.Z());
			vec.Normalize();
			GC::KeepAlive(this);
			return vec;
		}

		XbimPoint3D XbimFace::Location::get()
		{
			if (!IsValid) return XbimPoint3D();
			BRepGProp_Face prop(*pFace);
			gp_Pnt centre;
			gp_Vec normalDir;
			double u1, u2, v1, v2;
			prop.Bounds(u1, u2, v1, v2);
			prop.Normal((u1 + u2) / 2.0, (v1 + v2) / 2.0, centre, normalDir);
			XbimPoint3D loc(centre.X(), centre.Y(), centre.Z());
			GC::KeepAlive(this);
			return loc;
		}

		bool XbimFace::IsPlanar::get()
		{
			if (!IsValid) return false;
			Handle(Geom_Surface) surf = BRep_Tool::Surface(*pFace); //the surface
			Standard_Real tol = BRep_Tool::Tolerance(*pFace);
			GeomLib_IsPlanarSurface ps(surf, tol);
			GC::KeepAlive(this);
			return ps.IsPlanar() == Standard_True; //see if we have a planar or curved surface, if planar we need only worry about one normal

		}

		bool XbimFace::IsPolygonal::get()
		{
			if (!IsValid) return false;
			TopTools_IndexedMapOfShape map;
			TopExp::MapShapes(*pFace, TopAbs_EDGE, map);
			for (Standard_Integer i = 1; i <= map.Extent(); i++)
			{
				Standard_Real start, end;
				Handle(Geom_Curve) c3d = BRep_Tool::Curve(TopoDS::Edge(map(i)), start, end);
				if (!c3d.IsNull())
				{
					Handle(Standard_Type) cType = c3d->DynamicType();
					if (cType != STANDARD_TYPE(Geom_Line))
					{
						if (cType != STANDARD_TYPE(Geom_TrimmedCurve)) return false;
						Handle(Geom_TrimmedCurve) tc = Handle(Geom_TrimmedCurve)::DownCast(c3d);
						Handle(Standard_Type) tcType = tc->DynamicType();
						if (tcType != STANDARD_TYPE(Geom_Line)) return false;
					}
				}
			}
			return true;
		}

#pragma endregion

#pragma region Equality Overrides

		bool XbimFace::Equals(Object^ obj)
		{
			XbimFace^ f = dynamic_cast<XbimFace^>(obj);
			if (f == nullptr) return false;
			return this == f;
		}

		bool XbimFace::Equals(IXbimFace^ obj)
		{
			XbimFace^ f = dynamic_cast<XbimFace^>(obj);
			if (f == nullptr) return false;
			return this == f;
		}

		int XbimFace::GetHashCode()
		{
			if (!IsValid) return 0;
			return pFace->HashCode(Int32::MaxValue);
		}

		bool XbimFace::operator ==(XbimFace^ left, XbimFace^ right)
		{
			// If both are null, or both are same instance, return true.
			if (System::Object::ReferenceEquals(left, right))
				return true;

			// If one is null, but not both, return false.
			if (((Object^)left == nullptr) || ((Object^)right == nullptr))
				return false;
			return  ((const TopoDS_Face&)left).IsEqual(right) == Standard_True;

		}

		bool XbimFace::operator !=(XbimFace^ left, XbimFace^ right)
		{
			return !(left == right);
		}


#pragma endregion


#pragma region Methods

		void XbimFace::Move(IfcAxis2Placement3D^ position)
		{
			if (!IsValid) return;
			gp_Trsf toPos = XbimGeomPrim::ToTransform(position);
			pFace->Move(toPos);
		}

		void XbimFace::Translate(XbimVector3D translation)
		{
			if (!IsValid) return;
			gp_Vec v(translation.X, translation.Y, translation.Z);
			gp_Trsf t;
			t.SetTranslation(v);
			pFace->Move(t);
		}

		void XbimFace::Reverse()
		{
			if (!IsValid) return;
			pFace->Reverse();
		}

		bool XbimFace::Add(IXbimWire^ innerWire)
		{
			if (!IsValid) return false;
			XbimWire^ wire = dynamic_cast<XbimWire^>(innerWire);
			if (wire==nullptr) throw gcnew ArgumentException("Only IXbimWires created bu Xbim.OCC modules are supported", "xbimWire");
			BRepBuilderAPI_MakeFace faceMaker(*pFace);
			faceMaker.Add(wire);
			if (!faceMaker.IsDone()) return false;
			*pFace=faceMaker.Face();
			GC::KeepAlive(this);
			return true;
		}

		
		XbimPoint3D XbimFace::PointAtParameters(double u, double v)
		{
			if (!IsValid) return XbimPoint3D();
			const Handle(Geom_Surface) &surface = BRep_Tool::Surface(*pFace);
			gp_Pnt p;
			surface->D0(u, v, p);
			GC::KeepAlive(this);
			return XbimPoint3D(p.X(), p.Y(), p.Z());
		}

#pragma endregion

	}
}