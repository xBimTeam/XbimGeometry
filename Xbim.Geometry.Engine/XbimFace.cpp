#include "XbimFace.h"
#include "XbimOccWriter.h"
#include "XbimCurve.h"
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepBuilderAPI_GTransform.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <ShapeFix_ShapeTolerance.hxx>
#include <BRepCheck_Analyzer.hxx>
#include "XbimGeometryCreator.h"
#include "XbimConvert.h" 
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
#include <Geom_BSplineSurface.hxx>
#include <gp_Cylinder.hxx>
#include <BRepFilletAPI_MakeFillet2d.hxx>
#include <ShapeFix_Face.hxx>
#include <ShapeFix_Wire.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <GeomAPI_ProjectPointOnSurf.hxx>
#include <BRepClass_FaceClassifier.hxx>
#include <BRepTopAdaptor_FClass2d.hxx>
#include <BRepBuilderAPI_FindPlane.hxx>
#include <Geom_Plane.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <GProp_PGProps.hxx>
#include <ShapeFix_Edge.hxx>
#include <ShapeAnalysis.hxx>
#include <BRepFill.hxx>
#include <BRepPrimAPI_MakeRevol.hxx>
#include <ShapeAnalysis_Wire.hxx>
#include <ShapeFix_Shape.hxx>
#include <BRepLib.hxx>
#include <ShapeConstruct_ProjectCurveOnSurface.hxx>
#include <GeomProjLib.hxx>
#include <GeomAPI.hxx>
#include <GeomConvert.hxx>


using namespace System::Linq;

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
		bool XbimFace::RemoveDuplicatePoints(TColgp_SequenceOfPnt& polygon, bool assumeClosed, double tol)
		{
			return RemoveDuplicatePoints(polygon, std::vector<int>(), assumeClosed, tol);
		}

		bool XbimFace::RemoveDuplicatePoints(TColgp_SequenceOfPnt& polygon, std::vector<int> handles, bool assumeClosed, double tol)
		{
			// todo: 2021: this code loops more than required through the points to ensure all cleanup is performed
			// a more careful thought about starting indices would speed this up.
			//
			tol *= tol;
			bool firstAndLastPointsCoincide = false;
			while (true) {
				bool removed = false;
				int n = polygon.Length();
				if (!assumeClosed)
					n--;
				// indices of polygon are 1-based
				for (int iCurr = 1; iCurr <= n; ++iCurr) {
					// wrap around to the first point in case of a closed loop
					int iNext = (iCurr % polygon.Length()) + 1;
					double dist = polygon.Value(iCurr).SquareDistance(polygon.Value(iNext));
					if (dist < tol) {
						if (iNext == 1 && iCurr == n) //the first and last point are the same
							firstAndLastPointsCoincide = true;
						// do not remove the first or last point to
						// maintain connectivity with other wires
						if ((assumeClosed && iNext == 1) || (!assumeClosed && iNext == n))
						{
							polygon.Remove(iCurr);
							if (!handles.empty())
								handles.erase(handles.begin() + iCurr - 1);
						}
						else
						{
							polygon.Remove(iNext);
							if (!handles.empty())
								handles.erase(handles.begin() + iNext - 1);
						}
						removed = true;
						break; // every time we remove a point the loop is restarted
					}
				}
				
				// at this stage we have removed overlapping points, so we can look for aligned segments
				// this is still done in the main loop since removing aligned points might result in further
				// overlapping to be discovered
				if (!removed)
				{
					for (int iCurr = 2; iCurr < n; iCurr++) {
						// we are not considering first and last and looping around the end if closed
						gp_Pnt prev = polygon.Value(iCurr - 1);
						gp_Pnt c = polygon.Value(iCurr);
						gp_Pnt next = polygon.Value(iCurr + 1);
						if (AreCollinear(
							prev,
							c,
							next))
						{
							polygon.Remove(iCurr);
							if (!handles.empty())
								handles.erase(handles.begin() + iCurr - 1);
							removed = true;
							break; // every time we remove a point the loop is restarted
						}
					}
				}
				if (!removed) 
					break;
			}
			return firstAndLastPointsCoincide;
		}

		bool XbimFace::AreCollinear(const gp_Pnt& prePoint, const gp_Pnt& midPoint, const gp_Pnt& nextPoint)
		{
			gp_Dir dir1 = gp_Dir(midPoint.X() - prePoint.X(), midPoint.Y() - prePoint.Y(), midPoint.Z() - prePoint.Z());
			gp_Dir dir2 = gp_Dir(nextPoint.X() - midPoint.X(), nextPoint.Y() - midPoint.Y(), nextPoint.Z() - midPoint.Z());
			if (dir1.IsParallel(dir2, Precision::Angular()))
			{
				return true;
			}
			return false;
		}

		XbimFace::XbimFace(XbimPoint3D l, XbimVector3D n, ILogger^ /*logger*/)
		{
			gp_Pln plane(gp_Pnt(l.X, l.Y, l.Z), gp_Dir(n.X, n.Y, n.Z));
			BRepBuilderAPI_MakeFace faceMaker(plane);
			pFace = new TopoDS_Face();
			*pFace = faceMaker.Face();
		}

		XbimFace::XbimFace(XbimVector3D n, ILogger^ /*logger*/)
		{
			gp_Pln plane(gp_Pnt(0, 0, 0), gp_Dir(n.X, n.Y, n.Z));
			BRepBuilderAPI_MakeFace faceMaker(plane);
			pFace = new TopoDS_Face();
			*pFace = faceMaker.Face();
		}

		XbimFace::XbimFace(const TopoDS_Face& face)
		{
			pFace = new TopoDS_Face();
			*pFace = face;
		}

		XbimFace::XbimFace(const TopoDS_Face& face, Object^ tag) : XbimFace(face)
		{
			Tag = tag;
		}

		XbimFace::XbimFace(IIfcProfileDef^ profile, ILogger^ logger)
		{
			Init(profile, logger);
		}


		XbimFace::XbimFace(IIfcSurface^ surface, ILogger^ logger)
		{
			Init(surface, logger);
		}

		XbimFace::XbimFace(IIfcCurveBoundedPlane^ def, ILogger^ logger)
		{
			Init(def, logger);
		}

		XbimFace::XbimFace(IIfcRectangularTrimmedSurface^ def, ILogger^ logger)
		{
			Init(def, logger);
		}

		XbimFace::XbimFace(IIfcPlane^ plane, ILogger^ logger)
		{
			Init(plane, logger);
		}

		XbimFace::XbimFace(IIfcCylindricalSurface^ cylinder, ILogger^ logger)
		{
			Init(cylinder, logger);
		}
		XbimFace::XbimFace(IIfcSurfaceOfLinearExtrusion^ sLin, ILogger^ logger)
		{
			Init(sLin, true, logger);
		}

		XbimFace::XbimFace(IIfcSurfaceOfLinearExtrusion^ sLin, bool useWorkArounds, ILogger^ logger)
		{
			Init(sLin, useWorkArounds, logger);
		}

		XbimFace::XbimFace(IIfcSurfaceOfRevolution^ sRev, ILogger^ logger)
		{
			Init(sRev, logger);
		}
		XbimFace::XbimFace(IIfcCompositeCurve^ cCurve, ILogger^ logger)
		{
			Init(cCurve, logger);
		}

		XbimFace::XbimFace(IIfcPolyline^ pline, ILogger^ logger)
		{
			Init(pline, logger);
		}

		XbimFace::XbimFace(IIfcPolyLoop^ loop, ILogger^ logger)
		{
			Init(loop, logger);
		}


		XbimFace::XbimFace(IXbimWire^ wire, bool isPlanar, double precision, int entityLabel, ILogger^ logger)
		{
			Init(wire, isPlanar, precision, entityLabel, logger);
		}

		XbimFace::XbimFace(IXbimWire^ wire, XbimPoint3D pointOnFace, XbimVector3D faceNormal, ILogger^ logger)
		{
			Init(wire, pointOnFace, faceNormal, logger);
		}

		// todo: 2021: never used in the internal code... I think it could be obsoleted
		XbimFace::XbimFace(IIfcFace^ face, ILogger^ logger, bool useVertexMap, TopTools_DataMapOfIntegerShape& vertexMap)
		{
			Init(face, logger, useVertexMap, vertexMap);
		}

		XbimFace::XbimFace(double x, double y, double tolerance, ILogger^ logger)
		{
			Init(x, y, tolerance, logger);
		}

		XbimFace::XbimFace(IIfcSurface^ surface, XbimWire^ outerBound, IEnumerable<XbimWire^>^ innerBounds, ILogger^ logger)
		{
			Init(surface, logger);
			if (!IsValid) return;

			Handle(Geom_Surface) geomSurface = BRep_Tool::Surface(this);
			BRepBuilderAPI_MakeFace faceMaker(geomSurface, outerBound, Standard_True);
			if (faceMaker.IsDone())
			{
				for each (XbimWire ^ inner in innerBounds)
				{
					faceMaker.Add(inner);
				}
				BRepCheck_Analyzer analyser(faceMaker.Face(), Standard_False);
				if (!analyser.IsValid())
				{
					ShapeFix_Face sfs(faceMaker.Face());
					sfs.Perform();
					*pFace = sfs.Face();
				}
				else
					*pFace = faceMaker.Face();
			}
			else
			{
				delete pFace;
				pFace = nullptr;
			}
		}

		void XbimFace::PutEdgeOnFace(const TopoDS_Edge& Edg,
			const TopoDS_Face& Fac)
		{
			BRep_Builder B;
			TopLoc_Location LocFac;

			Handle(Geom_Surface) S = BRep_Tool::Surface(Fac, LocFac);
			Handle(Standard_Type) styp = S->DynamicType();

			if (styp == STANDARD_TYPE(Geom_RectangularTrimmedSurface)) {
				S = Handle(Geom_RectangularTrimmedSurface)::DownCast(S)->BasisSurface();
				styp = S->DynamicType();
			}

			if (styp == STANDARD_TYPE(Geom_Plane)) {
				return;
			}

			Standard_Real Umin, Umax, Vmin, Vmax;
			BRepTools::UVBounds(Fac, Umin, Umax, Vmin, Vmax);

			Standard_Real f, l;

			Handle(Geom2d_Curve) aC2d = BRep_Tool::CurveOnSurface(Edg, Fac, f, l);
			if (!aC2d.IsNull()) {
				gp_Pnt2d p2d;
				aC2d->D0((f + l) * 0.5, p2d);
				Standard_Boolean IsIn = Standard_True;
				if ((p2d.X() < Umin - Precision::PConfusion()) ||
					(p2d.X() > Umax + Precision::PConfusion()))
					IsIn = Standard_False;
				if ((p2d.Y() < Vmin - Precision::PConfusion()) ||
					(p2d.Y() > Vmax + Precision::PConfusion()))
					IsIn = Standard_False;

				if (IsIn)
					return;
			}

			TopLoc_Location Loc;
			Handle(Geom_Curve) C = BRep_Tool::Curve(Edg, Loc, f, l);
			if (!Loc.IsIdentity()) {
				Handle(Geom_Geometry) GG = C->Transformed(Loc.Transformation());
				C = Handle(Geom_Curve)::DownCast(GG);
			}

			if (C->DynamicType() != STANDARD_TYPE(Geom_TrimmedCurve)) {
				C = new Geom_TrimmedCurve(C, f, l);
			}

			S = BRep_Tool::Surface(Fac);

			Standard_Real TolFirst = -1, TolLast = -1;
			TopoDS_Vertex V1, V2;
			TopExp::Vertices(Edg, V1, V2);
			if (!V1.IsNull())
				TolFirst = BRep_Tool::Tolerance(V1);
			if (!V2.IsNull())
				TolLast = BRep_Tool::Tolerance(V2);

			Standard_Real tol2d = Precision::Confusion();
			Handle(Geom2d_Curve) C2d;
			ShapeConstruct_ProjectCurveOnSurface aToolProj;
			aToolProj.Init(S, tol2d);

			aToolProj.Perform(C, f, l, C2d, TolFirst, TolLast);
			if (C2d.IsNull())
			{
				return;
			}

			gp_Pnt2d pf(C2d->Value(f));
			gp_Pnt2d pl(C2d->Value(l));
			gp_Pnt PF, PL;
			S->D0(pf.X(), pf.Y(), PF);
			S->D0(pl.X(), pl.Y(), PL);
			if (Edg.Orientation() == TopAbs_REVERSED) {
				V1 = TopExp::LastVertex(Edg);
				V1.Reverse();
			}
			else {
				V1 = TopExp::FirstVertex(Edg);
			}
			if (Edg.Orientation() == TopAbs_REVERSED) {
				V2 = TopExp::FirstVertex(Edg);
				V2.Reverse();
			}
			else {
				V2 = TopExp::LastVertex(Edg);
			}

			if (!V1.IsNull() && V2.IsNull()) {
				//Handling of internal vertices
				Standard_Real old1 = BRep_Tool::Tolerance(V1);
				Standard_Real old2 = BRep_Tool::Tolerance(V2);
				gp_Pnt pnt1 = BRep_Tool::Pnt(V1);
				gp_Pnt pnt2 = BRep_Tool::Pnt(V2);
				Standard_Real tol1 = pnt1.Distance(PF);
				Standard_Real tol2 = pnt2.Distance(PL);
				B.UpdateVertex(V1, Max(old1, tol1));
				B.UpdateVertex(V2, Max(old2, tol2));
			}

			if (S->IsUPeriodic()) {
				Standard_Real up = S->UPeriod();
				Standard_Real tolu = Precision::PConfusion();// Epsilon(up);
				Standard_Integer nbtra = 0;
				Standard_Real theUmin = Min(pf.X(), pl.X());
				Standard_Real theUmax = Max(pf.X(), pl.X());

				if (theUmin < Umin - tolu) {
					while (theUmin < Umin - tolu) {
						theUmin += up;
						nbtra++;
					}
				}
				else if (theUmax > Umax + tolu) {
					while (theUmax > Umax + tolu) {
						theUmax -= up;
						nbtra--;
					}
				}

				if (nbtra != 0) {
					C2d->Translate(gp_Vec2d(nbtra * up, 0.));
				}
			}

			if (S->IsVPeriodic()) {
				Standard_Real vp = S->VPeriod();
				Standard_Real tolv = Precision::PConfusion();// Epsilon(vp);
				Standard_Integer nbtra = 0;
				Standard_Real theVmin = Min(pf.Y(), pl.Y());
				Standard_Real theVmax = Max(pf.Y(), pl.Y());

				if (theVmin < Vmin - tolv) {
					while (theVmin < Vmin - tolv) {
						theVmin += vp; theVmax += vp;
						nbtra++;
					}
				}
				else if (theVmax > Vmax + tolv) {
					while (theVmax > Vmax + tolv) {
						theVmax -= vp; theVmin -= vp;
						nbtra--;
					}
				}

				if (nbtra != 0) {
					C2d->Translate(gp_Vec2d(0., nbtra * vp));
				}
			}
			B.UpdateEdge(Edg, C2d, Fac, BRep_Tool::Tolerance(Edg));

			B.SameParameter(Edg, Standard_False);
			BRepLib::SameParameter(Edg, tol2d);
		}



		//NB the wires defined in the facesurface are ignored
		XbimFace::XbimFace(IIfcFaceSurface^ surface, XbimWire^ outerBound, IEnumerable<XbimWire^>^ innerBounds, double tolerance, ILogger^ logger)
		{
			Init(surface->FaceSurface, logger);
			if (!IsValid) return;

			Handle(Geom_Surface) geomSurface = BRep_Tool::Surface(this);
			TopoDS_Wire outer = outerBound;
			//make sure all the pcurves are built	
			ShapeFix_Wire wFix(outer, this, tolerance);
			if (wFix.FixEdgeCurves())
				outer = wFix.Wire();

			BRepBuilderAPI_MakeFace faceMaker;
			faceMaker.Init(geomSurface, false, tolerance);
			faceMaker.Add(outer);
			faceMaker.Build();
			bool reversed = false;
			if (faceMaker.IsDone())
			{
				*pFace = faceMaker.Face();
				try
				{
					reversed = CheckInside();
				}
				catch (Standard_Failure sf)
				{
					String^ err = gcnew String(sf.GetMessageString());
					XbimGeometryCreator::LogWarning(logger, surface, "Failed to create  IfcFaceSurface: " + err);

					delete pFace;
					pFace = nullptr;
					return;
				}

			}
			else
			{
				delete pFace;
				pFace = nullptr;
				return;
			}
			if (Enumerable::Any(innerBounds))
			{
				faceMaker.Init(*pFace);
				BRepBuilderAPI_MakeFace innerFaceMaker;
				for each (XbimWire ^ inner in innerBounds)
				{
					TopoDS_Wire innerWire = inner;
					innerFaceMaker.Init(geomSurface, false, tolerance);
					wFix.Init(inner, this, tolerance);
					if (wFix.FixEdgeCurves())
						innerWire = wFix.Wire();
					innerFaceMaker.Add(innerWire);
					if (innerFaceMaker.IsDone())
					{
						TopoDS_Face F = TopoDS::Face(innerFaceMaker.Face());
						BRepTopAdaptor_FClass2d FClass(F, 0.);
						if (FClass.PerformInfinitePoint() != TopAbs_IN) //need to reverse things, should be outside
						{
							BRep_Builder B;
							TopoDS_Face S = TopoDS::Face(pFace->EmptyCopied());
							TopoDS_Iterator it(innerFaceMaker.Face());
							while (it.More()) {
								B.Add(S, it.Value().Reversed());
								it.Next();
							}
							faceMaker.Add(BRepTools::OuterWire(S));
						}
					}
				}
				*pFace = faceMaker.Face();
			}
			//check orientation
			if (!surface->SameSense) Reverse();
			BRepCheck_Analyzer analyser(this, Standard_True);

			if (!analyser.IsValid())
			{
				analyser.IsValid();
			}
			ShapeFix_Face faceFixer(*pFace);
			if (faceFixer.Perform())
				*pFace = faceFixer.Face();
		}

		//Ensure the wire bounds a space inside
		bool XbimFace::CheckInside()
		{
			if (pFace != nullptr)
			{
				TopoDS_Face F = TopoDS::Face(*pFace);
				BRepTopAdaptor_FClass2d FClass(F, 0.);
				if (FClass.PerformInfinitePoint() == TopAbs_IN) //need to reverse things
				{
					BRep_Builder B;
					TopoDS_Shape S = pFace->EmptyCopied();
					TopoDS_Iterator it(*pFace);
					while (it.More()) {
						B.Add(S, it.Value().Reversed());
						it.Next();
					}
					*pFace = TopoDS::Face(S);
					return true;
				}
			}
			return false;
		}

		void XbimFace::Init(IIfcCompositeCurve^ cCurve, ILogger^ logger)
		{
			XbimWire^ wire = gcnew XbimWire(cCurve, logger, XbimConstraints::Closed | XbimConstraints::NotSelfIntersecting);
			if (wire->IsValid)
			{
				BRepBuilderAPI_MakeFace faceMaker(wire);
				pFace = new TopoDS_Face();
				*pFace = faceMaker.Face();
			}
		}

		void XbimFace::Init(IIfcPolyline^ pline, ILogger^ logger)
		{

			XbimWire^ wire = gcnew XbimWire(pline, logger, XbimConstraints::Closed | XbimConstraints::NotSelfIntersecting);
			if (wire->IsValid)
			{
				BRepBuilderAPI_MakeFace faceMaker(wire);
				pFace = new TopoDS_Face();
				*pFace = faceMaker.Face();
			}
		}

		void XbimFace::Init(IIfcPolyLoop^ polyloop, ILogger^ logger)
		{
			List<IIfcCartesianPoint^>^ polygon = Enumerable::ToList(polyloop->Polygon);
			int originalCount = polygon->Count;
			double tolerance = polyloop->Model->ModelFactors->Precision;
			if (originalCount < 3)
			{
				XbimGeometryCreator::LogWarning(logger, polyloop, "Invalid loop, it has less than three points. Wire discarded");
				return;
			}

			TColgp_SequenceOfPnt pointSeq;

			BRepBuilderAPI_MakeWire wireMaker;
			for (int i = 0; i < originalCount; i++)
			{
				pointSeq.Append(XbimConvert::GetPoint3d(polygon[i]));

			}

			XbimFace::RemoveDuplicatePoints(pointSeq, true, tolerance);

			if (pointSeq.Length() != originalCount)
			{
				XbimGeometryCreator::LogInfo(logger, polyloop, "Polyloop with duplicate points. Ifc rule: first point shall not be repeated at the end of the list. It has been removed");
			}

			if (pointSeq.Length() < 3)
			{
				XbimGeometryCreator::LogWarning(logger, polyloop, "Polyloop with less than 3 points is an empty loop. It has been ignored");
				return;
			}
			//get the basic properties
			TColgp_Array1OfPnt pointArray(1, pointSeq.Length());
			for (int i = 1; i <= pointSeq.Length(); i++)
			{
				pointArray.SetValue(i, pointSeq.Value(i));
			}


			//limit the tolerances for the vertices and edges
			BRepBuilderAPI_MakePolygon polyMaker;
			for (int i = 1; i <= pointSeq.Length(); ++i) {
				polyMaker.Add(pointSeq.Value(i));
			}
			polyMaker.Close();

			if (polyMaker.IsDone())
			{
				bool isPlanar;
				gp_Vec normal = XbimConvert::NewellsNormal(pointArray, isPlanar);
				if (!isPlanar)
				{

					XbimGeometryCreator::LogInfo(logger, polyloop, "Polyloop is a line. Empty loop built");
					return;
				}
				gp_Pnt centre = GProp_PGProps::Barycentre(pointArray);
				gp_Pln thePlane(centre, normal);
				TopoDS_Wire theWire = polyMaker.Wire();
				TopoDS_Face theFace = BRepBuilderAPI_MakeFace(thePlane, theWire, false);

				//limit the tolerances for the vertices and edges
				ShapeFix_ShapeTolerance tolFixer;
				tolFixer.LimitTolerance(theWire, tolerance); //set all tolerances
				//adjust vertex tolerances for bad planar fit if we have anything more than a triangle (which will alway fit a plane)

				if (pointSeq.Length() > 3)
				{
					TopTools_IndexedMapOfShape map;
					TopExp::MapShapes(theWire, TopAbs_EDGE, map);
					ShapeFix_Edge ef;
					bool fixed = false;
					for (int i = 1; i <= map.Extent(); i++)
					{
						const TopoDS_Edge edge = TopoDS::Edge(map(i));
						if (ef.FixVertexTolerance(edge, theFace)) fixed = true;

					}
					if (fixed)
						XbimGeometryCreator::LogInfo(logger, polyloop, "Polyloop is slightly mis-aligned to a plane. It has been adjusted");
				}
				//need to check for self intersecting edges to comply with Ifc rules
				double maxTol = BRep_Tool::MaxTolerance(theWire, TopAbs_VERTEX);
				Handle(ShapeAnalysis_Wire) wa = new ShapeAnalysis_Wire(theWire, theFace, maxTol);

				if (wa->CheckSelfIntersection()) //some edges are self intersecting or not on plane within tolerance, fix them
				{
					ShapeFix_Wire wf;
					wf.Init(wa);
					wf.SetPrecision(tolerance);
					wf.SetMinTolerance(tolerance);
					wf.SetMaxTolerance(maxTol);
					if (!wf.Perform())
					{
						XbimGeometryCreator::LogWarning(logger, polyloop, "Failed to fix self-interecting wire edges");

					}
					else
					{
						theWire = wf.Wire();
						theFace = BRepBuilderAPI_MakeFace(thePlane, theWire, false);
					}

				}
				pFace = new TopoDS_Face();
				*pFace = theFace;

			}
			else
			{
				XbimGeometryCreator::LogWarning(logger, polyloop, "Failed to build Polyloop"); //nothing more to say to the log			
			}

		}

		void XbimFace::Init(IXbimWire^ xbimWire, XbimPoint3D pointOnFace, XbimVector3D faceNormal, ILogger^ /*logger*/)
		{
			if (!dynamic_cast<XbimWire^>(xbimWire))
				throw gcnew ArgumentException("Only IXbimWires created by Xbim.OCC modules are supported", "xbimWire");
			XbimWire^ wire = (XbimWire^)xbimWire;
			if (wire->IsValid && !faceNormal.IsInvalid())
			{
				gp_Pln plane(gp_Pnt(pointOnFace.X, pointOnFace.Y, pointOnFace.Z), gp_Dir(faceNormal.X, faceNormal.Y, faceNormal.Z));
				BRepBuilderAPI_MakeFace faceMaker(plane, wire, Standard_False);
				TopoDS_Face resultFace = faceMaker.Face();
				if (BRepCheck_Analyzer(resultFace, Standard_True).IsValid() == Standard_False)
				{
					ShapeFix_Face faceFixer(faceMaker.Face());
					faceFixer.Perform();
					resultFace = faceFixer.Face();
				}
				pFace = new TopoDS_Face();
				*pFace = resultFace;
			}
			GC::KeepAlive(xbimWire);
		}

		void XbimFace::Init(IXbimWire^ xbimWire, bool isPlanar, double precision, int entityLabel, ILogger^ logger)
		{
			if (!dynamic_cast<XbimWire^>(xbimWire))
				throw gcnew ArgumentException("Only IXbimWires created by Xbim.OCC modules are supported", "xbimWire");
			XbimWire^ wire = (XbimWire^)xbimWire;
			double currentPrecision = precision;
			if (wire->IsValid)
			{
				if (isPlanar)
				{
					int retriedMakePlaneCount = 0;
					//we need to find the correct plane
				makePlane:
					BRepBuilderAPI_FindPlane planeMaker(wire, currentPrecision);
					if (retriedMakePlaneCount < 20 && !planeMaker.Found())
					{
						retriedMakePlaneCount++;
						currentPrecision = 10 * precision * retriedMakePlaneCount;
						goto makePlane;
					}
					if (!planeMaker.Found())
						XbimGeometryCreator::LogWarning(logger, nullptr, "Failure to build planar face due to a non-planar wire, entity #{0}", entityLabel);
					else
					{
						BRepBuilderAPI_MakeFace faceMaker(planeMaker.Plane()->Pln(), wire, Standard_True);
						pFace = new TopoDS_Face();
						*pFace = faceMaker.Face();
					}
				}
				else
				{
					BRepBuilderAPI_MakeFace faceMaker(wire, Standard_False);
					if (faceMaker.IsDone())
					{
						pFace = new TopoDS_Face();
						*pFace = faceMaker.Face();
					}
					else
					{
						XbimGeometryCreator::LogWarning(logger, nullptr, "Failure to build non-planar face, entity #{0}", entityLabel);
					}
				}
			}
			GC::KeepAlive(xbimWire);
		}

		void XbimFace::Init(IIfcFace^ ifcFace, ILogger^ logger, bool useVertexMap, TopTools_DataMapOfIntegerShape& vertexMap)
		{
			double tolerance = ifcFace->Model->ModelFactors->Precision;
			double angularTolerance = 0.00174533; //1 tenth of a degree
			double outerLoopArea = 0;
			ShapeFix_ShapeTolerance tolFixer;
			TopoDS_Face theFace;
			TopoDS_ListOfShape innerBounds;
			for each (IIfcFaceBound ^ bound in ifcFace->Bounds)
			{
				IIfcPolyLoop^ polyloop = dynamic_cast<IIfcPolyLoop^>(bound->Bound);

				if (polyloop == nullptr || !XbimConvert::IsPolygon((IIfcPolyLoop^)bound->Bound))
				{
					XbimGeometryCreator::LogInfo(logger, bound, "Polyloop bound is not a polygon and has been ignored");
					continue; //skip non-polygonal faces
				}
				//List<IIfcCartesianPoint^>^ polygon = Enumerable::ToList(polyloop->Polygon);
				int originalCount = polyloop->Polygon->Count;

				if (originalCount < 3)
				{
					XbimGeometryCreator::LogWarning(logger, polyloop, "Invalid loop, it has less than three points. Wire discarded");
					continue;
				}

				TColgp_SequenceOfPnt pointSeq;
				std::vector<int> handles;
				BRepBuilderAPI_MakeWire wireMaker;
				for (int i = 0; i < originalCount; i++)
				{
					pointSeq.Append(XbimConvert::GetPoint3d(polyloop->Polygon[i]));
					if (useVertexMap) handles.push_back(polyloop->Polygon[i]->EntityLabel);
				}
				if (useVertexMap)
					XbimFace::RemoveDuplicatePoints(pointSeq, handles, true, tolerance);
				else
					XbimFace::RemoveDuplicatePoints(pointSeq, true, tolerance);

				if (pointSeq.Length() != originalCount)
				{
					XbimGeometryCreator::LogInfo(logger, polyloop, "Polyloop with duplicate points. Ifc rule: first point shall not be repeated at the end of the list. It has been removed");
				}

				if (pointSeq.Length() < 3)
				{
					XbimGeometryCreator::LogWarning(logger, polyloop, "Polyloop with less than 3 points is an empty loop. It has been ignored");
					continue;
				}
				//get the basic properties
				TColgp_Array1OfPnt pointArray(1, pointSeq.Length());
				for (int i = 1; i <= pointSeq.Length(); i++)
				{
					pointArray.SetValue(i, pointSeq.Value(i));
				}

				BRep_Builder builder;
				//limit the tolerances for the vertices and edges
				BRepBuilderAPI_MakePolygon polyMaker;
				if (!bound->Orientation) //reverse the points
				{
					for (int i = pointSeq.Length(); i > 0; i--)
					{
						TopoDS_Vertex vertex;
						int entityLabel = handles.at(i - 1);
						if (!vertexMap.Find(entityLabel, vertex))
						{
							builder.MakeVertex(vertex, pointSeq.Value(i), tolerance);
							vertexMap.Bind(entityLabel, vertex);
						}
						polyMaker.Add(vertex);
					}
				}
				else
				{
					for (int i = 1; i <= pointSeq.Length(); ++i)
					{
						TopoDS_Vertex vertex;
						if (useVertexMap)
						{
							int entityLabel = handles.at(i - 1);
							if (!vertexMap.Find(entityLabel, vertex))
							{
								builder.MakeVertex(vertex, pointSeq.Value(i), tolerance);
								vertexMap.Bind(entityLabel, vertex);
							}
						}
						else
							builder.MakeVertex(vertex, pointSeq.Value(i), tolerance);
						polyMaker.Add(vertex);
					}
				}

				polyMaker.Close();


				if (polyMaker.IsDone())
				{

					bool isPlanar;
					gp_Vec normal = XbimConvert::NewellsNormal(pointArray, isPlanar);
					if (!isPlanar)
					{
						XbimGeometryCreator::LogInfo(logger, polyloop, "Polyloop is a line. Empty loop built");
						continue;
					}
					gp_Pnt centre = GProp_PGProps::Barycentre(pointArray);
					gp_Pln thePlane(centre, normal);
					TopoDS_Wire theWire = polyMaker.Wire();
					tolFixer.LimitTolerance(theWire, tolerance); //set all tolerances

					TopoDS_Face aFace = BRepBuilderAPI_MakeFace(thePlane, theWire, false);
					//need to check for self intersecting edges to comply with Ifc rules
					TopTools_IndexedMapOfShape map;
					TopExp::MapShapes(aFace, TopAbs_EDGE, map);
					ShapeFix_Edge ef;
					bool fixed = false;
					for (int i = 1; i <= map.Extent(); i++)
					{
						const TopoDS_Edge edge = TopoDS::Edge(map(i));
						if (ef.FixVertexTolerance(edge, aFace)) fixed = true;
					}
					if (fixed)
						XbimGeometryCreator::LogDebug(logger, ifcFace, "Face bounds are slightly mis-aligned to a plane. It has been adjusted");
					double maxTol = BRep_Tool::MaxTolerance(theWire, TopAbs_VERTEX);
					Handle(ShapeAnalysis_Wire) wa = new ShapeAnalysis_Wire(theWire, aFace, maxTol);

					if (wa->CheckSelfIntersection()) //some edges are self intersecting or not on plane within tolerance, fix them
					{
						ShapeFix_Wire wf;
						wf.Init(wa);
						wf.SetPrecision(tolerance);
						wf.SetMinTolerance(tolerance);
						wf.SetMaxTolerance(std::max(tolerance, maxTol));
						if (!wf.Perform())
						{
							XbimGeometryCreator::LogWarning(logger, polyloop, "Failed to fix self-interecting wire edges");
						}
						else
						{
							theWire = wf.Wire();
							aFace = BRepBuilderAPI_MakeFace(thePlane, theWire, false);
						}

					}

					double area = ShapeAnalysis::ContourArea(theWire);
					if (area > outerLoopArea)
					{
						if (!theFace.IsNull())
						{
							innerBounds.Append(theFace); //save the face to go inside as a loop
						}
						theFace = aFace;
						outerLoopArea = area;
					}
					else
					{
						innerBounds.Append(aFace);
					}
				}
				else
				{
					XbimGeometryCreator::LogWarning(logger, polyloop, "Failed to build Polyloop"); //nothing more to say to the log			
				}
			}
			if (!theFace.IsNull() && innerBounds.Size() > 0)
			{
				//add the other wires to the face
				BRepBuilderAPI_MakeFace faceMaker(theFace);

				TopoDS_ListIteratorOfListOfShape wireIter(innerBounds);
				BRepGProp_Face prop(theFace);
				gp_Pnt centre;
				gp_Vec theFaceNormal;
				double u1, u2, v1, v2;
				prop.Bounds(u1, u2, v1, v2);
				prop.Normal((u1 + u2) / 2.0, (v1 + v2) / 2.0, centre, theFaceNormal);

				while (wireIter.More())
				{
					TopoDS_Face face = TopoDS::Face(wireIter.Value());
					BRepGProp_Face fprop(face);
					gp_Vec innerBoundNormalDir;
					fprop.Bounds(u1, u2, v1, v2);
					fprop.Normal((u1 + u2) / 2.0, (v1 + v2) / 2.0, centre, innerBoundNormalDir);

					if (!theFaceNormal.IsOpposite(innerBoundNormalDir, angularTolerance))
					{

						face.Reverse();
					}
					faceMaker.Add(BRepTools::OuterWire(face));
					wireIter.Next();
				}
				theFace = faceMaker.Face();
				//limit the tolerances for the vertices and edges

				tolFixer.LimitTolerance(theFace, tolerance); //set all tolerances
				//adjust vertex tolerances for bad planar fit if we have anything more than a triangle (which will alway fit a plane)


				TopTools_IndexedMapOfShape map;
				TopExp::MapShapes(theFace, TopAbs_EDGE, map);
				ShapeFix_Edge ef;
				bool fixed = false;
				for (int i = 1; i <= map.Extent(); i++)
				{
					const TopoDS_Edge edge = TopoDS::Edge(map(i));
					if (ef.FixVertexTolerance(edge, theFace)) fixed = true;

				}
				if (fixed)
					XbimGeometryCreator::LogDebug(logger, ifcFace, "Face bounds are slightly mis-aligned to a plane. It has been adjusted");


			}
			else
			{
				tolFixer.LimitTolerance(theFace, tolerance); //set all tolerances
			}
			pFace = new TopoDS_Face();
			*pFace = theFace;
		}

		void XbimFace::Init(IXbimFace^ face, ILogger^ /*logger*/)
		{
			IXbimWire^ outerBound = face->OuterBound;
			XbimWire^ outerWire = dynamic_cast<XbimWire^>(outerBound);
			if (outerWire == nullptr) throw gcnew ArgumentException("Only IXbimWires created by Xbim.OCC modules are supported", "xbimWire");
			XbimVector3D n = face->Normal;
			XbimPoint3D pw = outerWire->Vertices->First->VertexGeometry;
			gp_Pln plane(gp_Pnt(pw.X, pw.Y, pw.Z), gp_Dir(n.X, n.Y, n.Z));
			BRepBuilderAPI_MakeFace faceMaker(plane, outerWire, Standard_False);
			for each (IXbimWire ^ innerBound in face->InnerBounds)
			{
				XbimWire^ innerWire = dynamic_cast<XbimWire^>(innerBound);
				if (innerWire != nullptr)
					faceMaker.Add(innerWire);
			}
			pFace = new TopoDS_Face();
			*pFace = faceMaker.Face();
		}

		void XbimFace::Init(IIfcProfileDef^ profile, ILogger^ logger)
		{
			IIfcArbitraryProfileDefWithVoids^ arbProfDefVoids = dynamic_cast<IIfcArbitraryProfileDefWithVoids^>(profile);
			if (arbProfDefVoids != nullptr) //this is a compound wire so we need to build it at face level
				return Init(arbProfDefVoids, logger);
			IIfcCircleHollowProfileDef^ circHollow = dynamic_cast<IIfcCircleHollowProfileDef^>(profile);
			if (circHollow != nullptr) //this is a compound wire so we need to build it at face level
				return Init(circHollow, logger);
			IIfcRectangleHollowProfileDef^ rectHollow = dynamic_cast<IIfcRectangleHollowProfileDef^>(profile);
			if (rectHollow != nullptr) //this is a compound wire so we need to build it at face level
				return Init(rectHollow, logger);
			if (dynamic_cast<IIfcCompositeProfileDef^>(profile))
				return Init((IIfcCompositeProfileDef^)profile, logger);
			if (dynamic_cast<IIfcDerivedProfileDef^>(profile))
				return Init((IIfcDerivedProfileDef^)profile, logger);
			if (dynamic_cast<IIfcArbitraryOpenProfileDef^>(profile) && !dynamic_cast<IIfcCenterLineProfileDef^>(profile))
				XbimGeometryCreator::LogWarning(logger, profile, "Faces cannot be built with IIfcArbitraryOpenProfileDef, a face requires a closed loop");
			else //it is a standard profile that can be built as a single wire
			{
				XbimWire^ wire = gcnew XbimWire(profile, logger, XbimConstraints::Closed | XbimConstraints::NotSelfIntersecting);
				//need to make sure the wire is ok


				if (wire->IsValid)
				{
					double tolerance = profile->Model->ModelFactors->Precision;
					XbimVector3D n = wire->Normal;
					if (n.IsInvalid()) //it is not an area
					{
						XbimGeometryCreator::LogWarning(logger, profile, "Face cannot be built with a profile that has no area (invalid normal).");
						return;
					}
					else
					{
						XbimPoint3D centre = wire->BaryCentre;
						gp_Pln thePlane(gp_Pnt(centre.X, centre.Y, centre.Z), gp_Vec(n.X, n.Y, n.Z));

						pFace = new TopoDS_Face();
						*pFace = BRepBuilderAPI_MakeFace(thePlane, wire, true);
						Handle(Geom_Plane) planeSurface = new Geom_Plane(thePlane);
						ShapeAnalysis_Wire wireChecker;
						wireChecker.SetSurface(planeSurface);
						wireChecker.Load(wire);
						wireChecker.SetPrecision(tolerance);
						if (wireChecker.CheckSelfIntersection())
						{
							ShapeFix_Shape faceFixer(*pFace);
							faceFixer.SetPrecision(tolerance);
							if (faceFixer.Perform())
							{
								TopoDS_Shape shape = faceFixer.Shape();
								TopTools_IndexedMapOfShape map;
								TopExp::MapShapes(shape, TopAbs_FACE, map);
								if (map.Extent() > 0)
								{
									BRepBuilderAPI_MakeFace faceBlder(TopoDS::Face(map(1)));
									for (int i = 2; i <= map.Extent(); i++)
									{
										faceBlder.Add(BRepTools::OuterWire(TopoDS::Face(map(i))));
									}
									if (faceBlder.IsDone())
									{
										pFace = new TopoDS_Face();
										*pFace = faceBlder.Face();
									}
									else
									{
										XbimGeometryCreator::LogInfo(logger, profile, "Profile could not be built.It has been omitted");
										return;
									}
								}
								else
								{
									XbimGeometryCreator::LogInfo(logger, profile, "Profile could not be built.It has been omitted");
									return;
								}
							}
							else
							{
								XbimGeometryCreator::LogInfo(logger, profile, "Profile could not be built.It has been omitted");
								return;
							}
						}

						//need to check for self intersecting edges to comply with Ifc rules
						TopTools_IndexedMapOfShape map;
						TopExp::MapShapes(*pFace, TopAbs_EDGE, map);
						ShapeFix_Edge ef;

						for (int i = 1; i <= map.Extent(); i++)
						{
							const TopoDS_Edge edge = TopoDS::Edge(map(i));
							ef.FixVertexTolerance(edge, *pFace);
						}
						ShapeFix_ShapeTolerance fTol;
						fTol.LimitTolerance(*pFace, tolerance);
					}
				}

			}
		}

		void XbimFace::Init(IIfcDerivedProfileDef^ profile, ILogger^ logger)
		{
			Init(profile->ParentProfile, logger);
			if (IsValid && !dynamic_cast<IIfcMirroredProfileDef^>(profile))
			{
				gp_Trsf aTrsf = XbimConvert::ToTransform(profile->Operator);
				// pFace->Move(TopLoc_Location(trsf));
				BRepBuilderAPI_Transform aBrepTrsf(*pFace, aTrsf);
				*pFace = TopoDS::Face(aBrepTrsf.Shape());
			}
			if (IsValid && dynamic_cast<IIfcMirroredProfileDef^>(profile))
			{
				//we need to mirror about the Y axis
				gp_Pnt origin(0, 0, 0);
				gp_Dir xDir(0, 1, 0);
				gp_Ax1 mirrorAxis(origin, xDir);
				gp_Trsf aTrsf;
				aTrsf.SetMirror(mirrorAxis);
				BRepBuilderAPI_Transform aBrepTrsf(*pFace, aTrsf);
				*pFace = TopoDS::Face(aBrepTrsf.Shape());
				Reverse();//correct the normal to be correct
			}
		}

		void XbimFace::Init(IIfcArbitraryProfileDefWithVoids^ profile, ILogger^ logger)
		{
			//IfcArbitraryProfileDefWithVoids must be defined by a 2D wire
			if (2 != (int)profile->OuterCurve->Dim)
			{
				XbimGeometryCreator::LogWarning(logger, profile, "Invalid bound. It should be 2D");
			}
			//Z must be up
			double tolerance = profile->Model->ModelFactors->Precision;
			double toleranceMax = profile->Model->ModelFactors->PrecisionMax;
			ShapeFix_ShapeTolerance FTol;
			TopoDS_Face face;
			XbimWire^ loop = gcnew XbimWire(profile->OuterCurve, logger, XbimConstraints::Closed | XbimConstraints::NotSelfIntersecting);
			if (loop->IsValid)
			{
				if (!loop->IsClosed) //we need to close it i
				{
					double oneMilli = profile->Model->ModelFactors->OneMilliMeter;
					XbimFace^ xface = gcnew XbimFace(loop, true, oneMilli, profile->OuterCurve->EntityLabel, logger);
					ShapeFix_Wire wireFixer(loop, xface, profile->Model->ModelFactors->Precision);
					wireFixer.ClosedWireMode() = Standard_True;
					wireFixer.FixGaps2dMode() = Standard_True;
					wireFixer.FixGaps3dMode() = Standard_True;
					wireFixer.ModifyGeometryMode() = Standard_True;
					wireFixer.SetMinTolerance(profile->Model->ModelFactors->Precision);
					wireFixer.SetPrecision(oneMilli);
					wireFixer.SetMaxTolerance(oneMilli * 10);
					Standard_Boolean closed = wireFixer.Perform();
					if (closed)
						loop = gcnew XbimWire(wireFixer.Wire());
				}
				double currentFaceTolerance = tolerance;
			TryBuildFace:
				BRepBuilderAPI_MakeFace faceMaker(loop, true);

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
					XbimGeometryCreator::LogWarning(logger, profile, "Invalid bound, {0}. Face discarded", errMsg);
					return;
				}
				pFace = new TopoDS_Face();
				*pFace = faceMaker.Face();
				XbimVector3D tn = Normal;
				//some models incorrectly output overlapping / intersecting wires, don't process them
				for each (IIfcCurve ^ curve in profile->InnerCurves)
				{
					faceMaker.Init(*pFace); //reset the faceMaker
					XbimWire^ innerWire = gcnew XbimWire(curve, logger, XbimConstraints::Closed | XbimConstraints::NotSelfIntersecting);
					if (!innerWire->IsValid)
					{
						XbimGeometryCreator::LogWarning(logger, profile, "Invalid innerWire. Inner bound ignored", curve->EntityLabel);
						continue;
					}
					if (!innerWire->IsClosed) //we need to close it if we have more thn one edge
					{
						double oneMilli = profile->Model->ModelFactors->OneMilliMeter;
						XbimFace^ xface = gcnew XbimFace(innerWire, true, oneMilli, curve->EntityLabel, logger);
						ShapeFix_Wire wireFixer(innerWire, xface, profile->Model->ModelFactors->Precision);
						wireFixer.ClosedWireMode() = Standard_True;
						wireFixer.FixGaps2dMode() = Standard_True;
						wireFixer.FixGaps3dMode() = Standard_True;
						wireFixer.ModifyGeometryMode() = Standard_True;
						wireFixer.SetMinTolerance(profile->Model->ModelFactors->Precision);
						wireFixer.SetPrecision(oneMilli);
						wireFixer.SetMaxTolerance(oneMilli * 10);
						Standard_Boolean closed = wireFixer.Perform();
						if (closed)
							innerWire = gcnew XbimWire(wireFixer.Wire());
					}
					if (innerWire->IsClosed) //if the loop is not closed it is not a bound
					{
						try //it is possible the inner loop is just a closed wire with zero area when a normal is calculated, this will throw an excpetion and the void is invalid
						{
							XbimVector3D n = innerWire->Normal;
							if (n.IsInvalid())
							{
								XbimGeometryCreator::LogWarning(logger, profile, "Invalid void. Inner bound ignored", curve->EntityLabel);
								continue;
							}
							bool needInvert = n.DotProduct(tn) > 0;
							if (needInvert) //inner wire should be reverse of outer wire
								innerWire->Reverse();
							double currentloopTolerance = tolerance;
						TryBuildLoop:
							faceMaker.Add(innerWire);
							//check the face is ok
							if (BRepCheck_Analyzer(faceMaker.Face(), Standard_True).IsValid() == Standard_False)
							{
								XbimGeometryCreator::LogWarning(logger, profile, "Invalid void. Inner bound ignored", curve->EntityLabel);
								continue;
							}
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
								XbimGeometryCreator::LogWarning(logger, profile, "Invalid void, {0}. IfcCurve #{1} could not be added. Inner bound ignored", errMsg, curve->EntityLabel);
							}
						}
						catch (Exception^ e)
						{
							XbimGeometryCreator::LogWarning(logger, profile, "Invalid profile void, {0}. IfcCurve #{1} could not be added. Inner bound ignored", e->Message, curve->EntityLabel);
						}
						*pFace = faceMaker.Face();
					}
					else
					{
						XbimGeometryCreator::LogWarning(logger, profile, "Invalid void. IfcCurve #{0} is not closed. Inner bound ignored", curve->EntityLabel);
					}
				}
			}
		}
		void XbimFace::Init(IIfcCompositeProfileDef^ compProfile, ILogger^ logger)
		{
			int profileCount = Enumerable::Count(compProfile->Profiles);
			if (profileCount == 0)
			{
				XbimGeometryCreator::LogInfo(logger, compProfile, "A composite profile must have 2 or more profiles, 0 were found. Profile discarded");
				return;
			}
			if (profileCount == 1)
			{
				XbimGeometryCreator::LogInfo(logger, compProfile, "A composite profile must have 2 or more profiles, 1 was found. A prilfe with a single segment has been used");
				Init(Enumerable::First(compProfile->Profiles), logger);
				return;
			}
			XbimFace^ firstFace = gcnew XbimFace(Enumerable::First(compProfile->Profiles), logger);
			BRepBuilderAPI_MakeFace faceBlder(firstFace);
			bool first = true;
			for each (IIfcProfileDef ^ profile in compProfile->Profiles)
			{
				if (!first)
				{
					XbimFace^ face = gcnew XbimFace(profile, logger);
					faceBlder.Add((XbimWire^)face->OuterBound);
					for each (IXbimWire ^ inner in face->InnerBounds)
					{
						faceBlder.Add((XbimWire^)inner);
					}
				}
				else
					first = false;
			}
			if (faceBlder.IsDone())
			{
				pFace = new TopoDS_Face();
				*pFace = faceBlder.Face();
			}
			else
				XbimGeometryCreator::LogInfo(logger, compProfile, "Profile could not be built.It has been omitted");

		}


		//Builds a face from a CircleProfileDef
		void XbimFace::Init(IIfcCircleHollowProfileDef^ circProfile, ILogger^ logger)
		{
			if (circProfile->Radius <= 0)
			{
				XbimGeometryCreator::LogInfo(logger, circProfile, "Circular profile has a radius <= 0. Face discarded");
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

			//make the outer wire
			gp_Circ outer(gpax2, circProfile->Radius);
			Handle(Geom_Circle) hOuter = GC_MakeCircle(outer);
			TopoDS_Edge outerEdge = BRepBuilderAPI_MakeEdge(hOuter);
			BRepBuilderAPI_MakeWire outerWire;
			outerWire.Add(outerEdge);
			double innerRadius = circProfile->Radius - circProfile->WallThickness;
			BRepBuilderAPI_MakeFace faceBlder(outerWire);
			//now add inner wire
			if (innerRadius > 0)
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


		void XbimFace::Init(IIfcRectangleHollowProfileDef^ rectProfile, ILogger^ logger)
		{
			if (rectProfile->XDim <= 0 || rectProfile->YDim <= 0)
			{
				XbimGeometryCreator::LogInfo(logger, rectProfile, "Profile has a dimension <= 0,  XDim = {0}, YDim = {1}. Face ignored", rectProfile->XDim, rectProfile->YDim);
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
				wire.Closed(Standard_True);
				double oRad = rectProfile->OuterFilletRadius.HasValue ? (double)(rectProfile->OuterFilletRadius.Value) : 0.0;
				if (oRad > 0) //consider fillets
				{
					BRepBuilderAPI_MakeFace faceMaker(wire, true);
					BRepFilletAPI_MakeFillet2d filleter(faceMaker.Face());
					for (BRepTools_WireExplorer exp(wire); exp.More(); exp.Next())
					{
						filleter.AddFillet(exp.CurrentVertex(), oRad);
					}
					filleter.Build();
					if (filleter.IsDone())
					{
						TopoDS_Shape shape = filleter.Shape();
						for (TopExp_Explorer exp(shape, TopAbs_WIRE); exp.More();) //just take the first wire
						{
							wire = TopoDS::Wire(exp.Current());
							break;
						}
					}
				}
				//make the face
				BRepBuilderAPI_MakeFace faceBlder(wire);
				//calculate hole
				if (rectProfile->WallThickness <= 0)
					XbimGeometryCreator::LogInfo(logger, rectProfile, "Wall thickness of a rectangle hollow profile must be greater than 0, a solid rectangular profile has been used.");
				else
				{
					TopoDS_Wire innerWire;
					builder.MakeWire(innerWire);
					double t = rectProfile->WallThickness;
					gp_Pnt ibl(-xOff + t, -yOff + t, 0);
					gp_Pnt ibr(xOff - t, -yOff + t, 0);
					gp_Pnt itr(xOff - t, yOff - t, 0);
					gp_Pnt itl(-xOff + t, yOff - t, 0);
					TopoDS_Vertex vibl, vibr, vitr, vitl;
					builder.MakeVertex(vibl, ibl, precision);
					builder.MakeVertex(vibr, ibr, precision);
					builder.MakeVertex(vitr, itr, precision);
					builder.MakeVertex(vitl, itl, precision);
					builder.Add(innerWire, BRepBuilderAPI_MakeEdge(vibl, vibr));
					builder.Add(innerWire, BRepBuilderAPI_MakeEdge(vibr, vitr));
					builder.Add(innerWire, BRepBuilderAPI_MakeEdge(vitr, vitl));
					builder.Add(innerWire, BRepBuilderAPI_MakeEdge(vitl, vibl));

					double iRad = rectProfile->InnerFilletRadius.HasValue ? (double)(rectProfile->InnerFilletRadius.Value) : 0.0;
					if (iRad > 0) //consider fillets
					{
						BRepBuilderAPI_MakeFace faceMaker(innerWire, true);
						BRepFilletAPI_MakeFillet2d filleter(faceMaker.Face());
						for (BRepTools_WireExplorer exp(innerWire); exp.More(); exp.Next())
						{
							filleter.AddFillet(exp.CurrentVertex(), iRad);
						}
						filleter.Build();
						if (filleter.IsDone())
						{
							TopoDS_Shape shape = filleter.Shape();
							for (TopExp_Explorer exp(shape, TopAbs_WIRE); exp.More();) //just take the first wire
							{
								innerWire = TopoDS::Wire(exp.Current());
								break;
							}
						}
					}
					innerWire.Reverse();
					innerWire.Closed(Standard_True);
					faceBlder.Add(innerWire);
				}

				pFace = new TopoDS_Face();
				*pFace = faceBlder.Face();
				//apply the position transformation
				if (rectProfile->Position != nullptr)
					pFace->Move(XbimConvert::ToLocation(rectProfile->Position));
				ShapeFix_ShapeTolerance fTol;
				fTol.LimitTolerance(*pFace, rectProfile->Model->ModelFactors->Precision);
			}

		}

		//Builds a face from a Surface
		void XbimFace::Init(IIfcSurface^ surface, ILogger^ logger)
		{
			if (dynamic_cast<IIfcPlane^>(surface))
				return Init((IIfcPlane^)surface, logger);
			else if (dynamic_cast<IIfcSurfaceOfRevolution^>(surface))
				return Init((IIfcSurfaceOfRevolution^)surface, logger);
			else if (dynamic_cast<IIfcSurfaceOfLinearExtrusion^>(surface))
				return Init((IIfcSurfaceOfLinearExtrusion^)surface, logger);
			else if (dynamic_cast<IIfcCurveBoundedPlane^>(surface))
				return Init((IIfcCurveBoundedPlane^)surface, logger);
			else if (dynamic_cast<IIfcRectangularTrimmedSurface^>(surface))
				return Init((IIfcRectangularTrimmedSurface^)surface, logger);
			else if (dynamic_cast<IIfcBSplineSurface^>(surface))
				return Init((IIfcBSplineSurface^)surface, logger);
			else if (dynamic_cast<IIfcCylindricalSurface^>(surface))
				return Init((IIfcCylindricalSurface^)surface, logger);
			else
			{
				Type^ type = surface->GetType();
				throw(gcnew NotImplementedException(String::Format("XbimFace. BuildFace of type {0} is not implemented", type->Name)));
			}

		}

		void XbimFace::Init(IIfcCylindricalSurface^ surface, ILogger^ /*logger*/)
		{
			gp_Ax3 ax3 = XbimConvert::ToAx3(surface->Position);
			Handle(Geom_CylindricalSurface)   gcs = new Geom_CylindricalSurface(ax3, surface->Radius);
			//gp_Cylinder cylinder(ax3, surface->Radius);
			BRepBuilderAPI_MakeFace  builder;
			builder.Init(gcs, Standard_False, surface->Model->ModelFactors->Precision);
			pFace = new TopoDS_Face();
			*pFace = builder.Face();

			pFace->EmptyCopy();
		}


		void XbimFace::Init(IIfcBSplineSurface^ surface, ILogger^ logger)
		{
			if (dynamic_cast<IIfcBSplineSurfaceWithKnots^>(surface))
				return Init((IIfcBSplineSurfaceWithKnots^)surface, logger);
			Type^ type = surface->GetType();
			throw(gcnew NotImplementedException(String::Format("XbimFace. BuildFace of type {0} is not implemented", type->Name)));
		}

		void XbimFace::Init(IIfcBSplineSurfaceWithKnots^ surface, ILogger^ logger)
		{
			if (dynamic_cast<IIfcRationalBSplineSurfaceWithKnots^>(surface))
				return Init((IIfcRationalBSplineSurfaceWithKnots^)surface, logger);

			List<List<XbimPoint3D>^>^ ifcControlPoints = surface->ControlPoints;
			if (surface->ControlPoints->Count < 2)
			{
				XbimGeometryCreator::LogWarning(logger, surface, "Incorrect number of poles for Bspline surface, it must be at least 2");
				return;
			}
			TColgp_Array2OfPnt poles(1, (Standard_Integer)surface->UUpper + 1, 1, (Standard_Integer)surface->VUpper + 1);

			for (int u = 0; u <= surface->UUpper; u++)
			{
				List<XbimPoint3D>^ uRow = ifcControlPoints[u];
				for (int v = 0; v <= surface->VUpper; v++)
				{
					XbimPoint3D cp = uRow[v];
					poles.SetValue(u + 1, v + 1, gp_Pnt(cp.X, cp.Y, cp.Z));
				}

			}

			TColStd_Array1OfReal uknots(1, (Standard_Integer)surface->KnotUUpper);
			TColStd_Array1OfReal vknots(1, (Standard_Integer)surface->KnotVUpper);
			TColStd_Array1OfInteger uMultiplicities(1, (Standard_Integer)surface->KnotUUpper);
			TColStd_Array1OfInteger vMultiplicities(1, (Standard_Integer)surface->KnotVUpper);
			int i = 1;
			for each (double knot in surface->UKnots)
			{
				uknots.SetValue(i, knot);
				i++;
			}
			i = 1;
			for each (double knot in surface->VKnots)
			{
				vknots.SetValue(i, knot);
				i++;
			}

			i = 1;
			for each (int multiplicity in surface->UMultiplicities)
			{
				uMultiplicities.SetValue(i, multiplicity);
				i++;
			}

			i = 1;
			for each (int multiplicity in surface->VMultiplicities)
			{
				vMultiplicities.SetValue(i, multiplicity);
				i++;
			}

			Handle(Geom_BSplineSurface) hSurface = new Geom_BSplineSurface(poles, uknots, vknots, uMultiplicities, vMultiplicities, (Standard_Integer)surface->UDegree, (Standard_Integer)surface->VDegree);
			BRepBuilderAPI_MakeFace faceMaker(hSurface, 0.1/*surface->Model->ModelFactors->Precision*/);
			pFace = new TopoDS_Face();
			*pFace = faceMaker.Face();
			pFace->EmptyCopy();
			/*ShapeFix_ShapeTolerance FTol;
			FTol.SetTolerance(*pFace, bspline->Model->ModelFactors->Precision, TopAbs_VERTEX);*/
		}
		void XbimFace::Init(IIfcRationalBSplineSurfaceWithKnots^ surface, ILogger^ /*logger*/)
		{
			List<List<XbimPoint3D>^>^ ifcControlPoints = surface->ControlPoints;
			if (surface->ControlPoints->Count < 2) throw gcnew XbimException("Incorrect number of poles for Bspline surface, must be at least 2");
			TColgp_Array2OfPnt poles(1, (Standard_Integer)surface->UUpper + 1, 1, (Standard_Integer)surface->VUpper + 1);

			for (int u = 0; u <= surface->UUpper; u++)
			{
				List<XbimPoint3D>^ uRow = ifcControlPoints[u];
				for (int v = 0; v <= surface->VUpper; v++)
				{
					XbimPoint3D cp = uRow[v];
					poles.SetValue(u + 1, v + 1, gp_Pnt(cp.X, cp.Y, cp.Z));
				}

			}

			List<List<Ifc4::MeasureResource::IfcReal>^>^ ifcWeights = surface->Weights;
			TColStd_Array2OfReal weights(1, (Standard_Integer)surface->UUpper + 1, 1, (Standard_Integer)surface->VUpper + 1);
			for (int u = 0; u <= surface->UUpper; u++)
			{
				List<Ifc4::MeasureResource::IfcReal>^ uRow = ifcWeights[u];
				for (int v = 0; v <= surface->VUpper; v++)
				{
					double r = uRow[v];
					weights.SetValue(u + 1, v + 1, r);
				}
			}

			TColStd_Array1OfReal uknots(1, (Standard_Integer)surface->KnotUUpper);
			TColStd_Array1OfReal vknots(1, (Standard_Integer)surface->KnotVUpper);
			TColStd_Array1OfInteger uMultiplicities(1, (Standard_Integer)surface->KnotUUpper);
			TColStd_Array1OfInteger vMultiplicities(1, (Standard_Integer)surface->KnotVUpper);
			int i = 1;
			for each (double knot in surface->UKnots)
			{
				uknots.SetValue(i, knot);
				i++;
			}
			i = 1;
			for each (double knot in surface->VKnots)
			{
				vknots.SetValue(i, knot);
				i++;
			}

			i = 1;
			for each (int multiplicity in surface->UMultiplicities)
			{
				uMultiplicities.SetValue(i, multiplicity);
				i++;
			}

			i = 1;
			for each (int multiplicity in surface->VMultiplicities)
			{
				vMultiplicities.SetValue(i, multiplicity);
				i++;
			}

			Standard_Integer uDegree = (Standard_Integer)surface->UDegree;
			Standard_Integer vDegree = (Standard_Integer)surface->VDegree;
			Handle(Geom_BSplineSurface) hSurface = new Geom_BSplineSurface(poles, weights, uknots, vknots, uMultiplicities, vMultiplicities, uDegree, vDegree);
			BRepBuilderAPI_MakeFace faceMaker(hSurface, surface->Model->ModelFactors->Precision);
			pFace = new TopoDS_Face();
			*pFace = faceMaker.Face();
			pFace->EmptyCopy(); //remove any edges as we only want a surface
		}

		//Builds a face from a Plane
		void XbimFace::Init(IIfcPlane^ plane, ILogger^ /*logger*/)
		{
			gp_Ax3 ax3 = XbimConvert::ToAx3(plane->Position);
			gp_Pln pln(ax3);
			BRepBuilderAPI_MakeFace  builder(pln);
			pFace = new TopoDS_Face();
			*pFace = builder.Face();
			ShapeFix_ShapeTolerance fTol;
			fTol.LimitTolerance(*pFace, plane->Model->ModelFactors->Precision);
		}
		void XbimFace::Init(IIfcSurfaceOfRevolution^ sRev, ILogger^ logger)
		{
			if (sRev->SweptCurve->ProfileType != IfcProfileTypeEnum::CURVE)
			{
				XbimGeometryCreator::LogWarning(logger, sRev, "Only profiles of type curve are valid in a surface of revolution {0}. Face discarded", sRev->SweptCurve->EntityLabel);
				return;
			}
			XbimEdge^ edge = gcnew XbimEdge(sRev->SweptCurve, logger);
			if (!edge->IsValid)
			{
				XbimGeometryCreator::LogWarning(logger, sRev, "Invalid Swept Curve for IfcSurfaceOfRevolution, face discarded");
				return;
			}
			TopoDS_Edge startEdge = edge;

			gp_Pnt origin(sRev->AxisPosition->Location->X, sRev->AxisPosition->Location->Y, sRev->AxisPosition->Location->Z);
			gp_Dir axisDir(0, 0, 1);
			if (sRev->AxisPosition->Axis != nullptr)
				axisDir = gp_Dir(sRev->AxisPosition->Axis->X, sRev->AxisPosition->Axis->Y, sRev->AxisPosition->Axis->Z);
			gp_Ax1 axis(origin, axisDir);

			BRepPrimAPI_MakeRevol revolutor(startEdge, axis, M_PI * 2);
			if (revolutor.IsDone())
			{
				pFace = new TopoDS_Face();
				*pFace = TopoDS::Face(revolutor.Shape());
				pFace->EmptyCopy();
			}
			else
			{
				XbimGeometryCreator::LogWarning(logger, sRev, "Invalid IfcSurfaceOfRevolution, face discarded");
			}
		}

		void XbimFace::Init(IIfcRectangularTrimmedSurface^ def, ILogger^ logger)
		{
			Init(def->BasisSurface, logger); //initialise the plane
			if (IsValid)
			{
				TopLoc_Location loc;
				Handle(Geom_Surface) surface = BRep_Tool::Surface(this, loc);
				Handle(Geom_RectangularTrimmedSurface) geomTrim(new  Geom_RectangularTrimmedSurface(surface, def->U1, def->U2, def->V1, def->V2));

				BRepBuilderAPI_MakeFace faceMaker(geomTrim, def->Model->ModelFactors->Precision);
				if (faceMaker.IsDone())
				{
					pFace = new TopoDS_Face();
					*pFace = faceMaker.Face();
					pFace->EmptyCopy();
				}
				else
					XbimGeometryCreator::LogWarning(logger, def, "Invalid trimed surface = #{0} in rectangular trimmed surface. Face discarded", def->BasisSurface->EntityLabel);
			}
		}

		void XbimFace::Init(IIfcCurveBoundedPlane^ def, ILogger^ logger)
		{
			Init(def->BasisSurface, logger); //initialise the plane
			if (IsValid)
			{
				XbimWire^ outerBound = gcnew XbimWire(def->OuterBoundary, logger, XbimConstraints::Closed | XbimConstraints::NotSelfIntersecting);
				BRepBuilderAPI_MakeFace  builder(this);
				builder.Add(outerBound);
				for each (IIfcCurve ^ innerCurve in def->InnerBoundaries)
				{
					XbimWire^ innerBound = gcnew XbimWire(innerCurve, logger, XbimConstraints::Closed | XbimConstraints::NotSelfIntersecting);
					if (innerBound->IsValid)
						builder.Add(innerBound);
					else
						XbimGeometryCreator::LogWarning(logger, def, "Invalid inner bound = #{0} found in curve bounded plane. Inner bound ignored", innerCurve->EntityLabel);

				}
				if (builder.IsDone())
				{
					pFace = new TopoDS_Face();
					*pFace = builder.Face();
				}
				else
					XbimGeometryCreator::LogWarning(logger, def, "Invalid outer bound = #{0} found in curve bounded plane. Face discarded", def->OuterBoundary->EntityLabel);
			}
		}
		void XbimFace::Init(IIfcSurfaceOfLinearExtrusion^ sLin, ILogger^ logger)
		{
			return Init(sLin, true, logger);
		}
		/// <summary>
		/// There are several older versions of the Revit Ifc Export that write the first ruled surface edge at the final geometric position, then apply a transformation that displaces it by twice as much
		/// Also they write the extrusion out in TRevit base units (feet) not model units
		/// </summary>
		/// <param name="sLin"></param>
		/// <param name="useWorkArounds"></param>
		/// <param name="logger"></param>
		void XbimFace::Init(IIfcSurfaceOfLinearExtrusion^ sLin, bool /*useWorkArounds*/, ILogger^ logger)
		{
			if (sLin->SweptCurve->ProfileType != IfcProfileTypeEnum::CURVE)
			{
				XbimGeometryCreator::LogWarning(logger, sLin, "Only profiles of type curve are valid in a surface of linearExtrusion {0}. Face discarded", sLin->SweptCurve->EntityLabel);
				return;
			}
			IModelFactors^ mf = sLin->Model->ModelFactors;
			XbimEdge^ xbasisEdge1 = nullptr;
			double tolerance = mf->Precision;
			bool isFixed = false;
			IIfcArbitraryOpenProfileDef^ pDef = dynamic_cast<IIfcArbitraryOpenProfileDef^>(sLin->SweptCurve);

			IIfcTrimmedCurve^ tc = nullptr;
			IIfcCircle^ circle = nullptr;
			IIfcBSplineCurveWithKnots^ bspline = nullptr;
			if (pDef != nullptr)
			{
				tc = dynamic_cast<IIfcTrimmedCurve^>(pDef->Curve);
				if (tc != nullptr)
				{
					circle = dynamic_cast<IIfcCircle^>(tc->BasisCurve);
					bspline = dynamic_cast<IIfcBSplineCurveWithKnots^>(tc->BasisCurve);
				}
			}
			if (mf->ApplyWorkAround("#RevitIncorrectArcCentreSweptCurve") && sLin->Position != nullptr)
			{
				if (circle != nullptr)
				{
					//the centre has been transformed twice, recalculate the centre
					//trim 1 and trim 2 will be cartesian points
					IIfcCartesianPoint^ trim1 = dynamic_cast<IIfcCartesianPoint^>(Enumerable::FirstOrDefault(tc->Trim1));
					IIfcCartesianPoint^ trim2 = dynamic_cast<IIfcCartesianPoint^>(Enumerable::FirstOrDefault(tc->Trim2));

					if (trim1 != nullptr && trim2 != nullptr) {

						gp_Pnt p1 = XbimConvert::GetPoint3d(trim1);
						gp_Pnt p2 = XbimConvert::GetPoint3d(trim2);

						//there are two solutions A, B
						//calc solution A
						double radsq = circle->Radius * circle->Radius;
						double qX = Math::Sqrt(((p2.X() - p1.X()) * (p2.X() - p1.X())) + ((p2.Y() - p1.Y()) * (p2.Y() - p1.Y())));
						double x3 = (p1.X() + p2.X()) / 2;
						double centreX = x3 - Math::Sqrt(radsq - ((qX / 2) * (qX / 2))) * ((p1.Y() - p2.Y()) / qX);

						double qY = Math::Sqrt(((p2.X() - p1.X()) * (p2.X() - p1.X())) + ((p2.Y() - p1.Y()) * (p2.Y() - p1.Y())));

						double y3 = (p1.Y() + p2.Y()) / 2;

						double centreY = y3 - Math::Sqrt(radsq - ((qY / 2) * (qY / 2))) * ((p2.X() - p1.X()) / qY);

						ITransaction^ txn = sLin->Model->BeginTransaction("Fix Centre");

						IIfcPlacement^ p = dynamic_cast<IIfcPlacement^>(circle->Position);

						p->Location->Coordinates[0] = centreX;
						p->Location->Coordinates[1] = centreY;
						p->Location->Coordinates[2] = 0;

						xbasisEdge1 = gcnew XbimEdge(sLin->SweptCurve, logger);
						txn->RollBack();
						isFixed = true;
					}
				}


			}
			if (!isFixed) //just build it
				xbasisEdge1 = gcnew XbimEdge(sLin->SweptCurve, logger);
			if (!xbasisEdge1->IsValid) return;
			try
			{


				gp_Vec extrude = XbimConvert::GetDir3d(sLin->ExtrudedDirection); //we are going to ignore magnitude as the surface should be infinite
				extrude *= sLin->Depth;
				if (mf->ApplyWorkAround("#RevitSweptSurfaceExtrusionInFeet"))
					extrude *= mf->OneFoot;


				TopLoc_Location loc;
				double start, end;
				TopoDS_Edge edge = xbasisEdge1;
				Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, loc, start, end);

				Handle(Geom_SurfaceOfLinearExtrusion) surface = new Geom_SurfaceOfLinearExtrusion(curve, extrude);
				BRepBuilderAPI_MakeFace faceBlder(surface, tolerance);
				if (faceBlder.IsDone())
				{
					pFace = new TopoDS_Face();
					*pFace = faceBlder.Face();
					if (sLin->Position != nullptr)
					{
						if (!(bspline != nullptr && mf->ApplyWorkAround("#RevitIncorrectBsplineSweptCurve"))) //revit does not respect the local placement correctly
						{
							TopLoc_Location newLoc = XbimConvert::ToLocation(sLin->Position);
							pFace->Move(newLoc);
						}
					}
					pFace->EmptyCopy(); //remove the ruled edges
				}



			}
			catch (Standard_Failure f)
			{
				String^ err = gcnew String(f.GetMessageString());
				throw gcnew Exception("General failure in advanced face building: " + err);
			}
		}
		void XbimFace::ReParamCurve(TopoDS_Edge& basisEdge)
		{
			TopLoc_Location L;
			Standard_Real First, Last;

			Handle(Geom_Curve) curve = Handle(Geom_Curve)::DownCast(BRep_Tool::Curve(basisEdge, L, First, Last)->Copy());
			//if ( Abs (First) <= Precision::PConfusion() && Abs (Last - 1.) <= Precision::PConfusion() ) return;
			if (!curve->IsKind(STANDARD_TYPE(Geom_Line))) return;

			ReparamBSpline(curve, First, Last);

			BRep_Builder B;
			B.UpdateEdge(basisEdge, curve, L, Precision::Confusion());
			B.Range(basisEdge, 0., 1);
		}

		void XbimFace::ReparamBSpline(Handle(Geom_Curve)& curve,
			const Standard_Real First,
			const Standard_Real Last)
		{
			Handle(Geom_BSplineCurve) bscurve;
			if (!curve->IsKind(STANDARD_TYPE(Geom_BSplineCurve))) {
				if (curve->FirstParameter() < First || curve->LastParameter() > Last)
					curve = new Geom_TrimmedCurve(curve, First, Last);
				bscurve = GeomConvert::CurveToBSplineCurve(curve, Convert_RationalC1);
			}
			else {
				bscurve = Handle(Geom_BSplineCurve)::DownCast(curve);
				bscurve->Segment(First, Last);
			}

			if (bscurve.IsNull())
				return;

			TColStd_Array1OfReal Knots(1, bscurve->NbKnots());
			bscurve->Knots(Knots);
			BSplCLib::Reparametrize(0., 1., Knots);
			bscurve->SetKnots(Knots);
			curve = bscurve;
		}

		TopoDS_Edge XbimFace::ReParamEdge(TopoDS_Edge& basisEdge)
		{
			TopLoc_Location L;
			Standard_Real First, Last;
			Handle(Geom_Curve) curve = Handle(Geom_Curve)::DownCast(BRep_Tool::Curve(basisEdge, L, First, Last)->Copy());
			if (Abs(First) <= Precision::PConfusion() && Abs(Last - 1.) <= Precision::PConfusion()) return basisEdge;

			Handle(Geom_BSplineCurve) bscurve;
			if (!curve->IsKind(STANDARD_TYPE(Geom_BSplineCurve))) {
				if (curve->FirstParameter() < First || curve->LastParameter() > Last)
					curve = new Geom_TrimmedCurve(curve, First, Last);
				bscurve = GeomConvert::CurveToBSplineCurve(curve, Convert_RationalC1);
			}
			else {
				bscurve = Handle(Geom_BSplineCurve)::DownCast(curve);
				bscurve->Segment(First, Last);
			}
			TColStd_Array1OfReal Knots(1, bscurve->NbKnots());
			bscurve->Knots(Knots);
			BSplCLib::Reparametrize(0., 1., Knots);
			bscurve->SetKnots(Knots);

			BRep_Builder B;
			B.UpdateEdge(basisEdge, bscurve, L, Precision::Confusion());
			B.Range(basisEdge, 0., 1);
			return basisEdge;
		}

		void  XbimFace::Init(double x, double y, double tolerance, ILogger^ /*logger*/)
		{
			XbimWire^ bounds = gcnew XbimWire(x, y, tolerance, false);
			if (bounds->IsValid)
			{
				BRepBuilderAPI_MakeFace faceMaker(bounds);
				pFace = new TopoDS_Face();
				*pFace = faceMaker.Face();
			}
		}

#pragma region IXbimFace Interface

		XbimWire^ XbimFace::OuterWire::get()
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

		XbimWireSet^ XbimFace::Wires::get()
		{
			if (!IsValid) return XbimWireSet::Empty; //return an empty list, avoid using Enumberable::Empty to avoid LINQ dependencies			
			return gcnew XbimWireSet(this);
		}

		XbimWireSet^ XbimFace::InnerWires::get()
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
			BRepBuilderAPI_Transform gTran(copier.Shape(), XbimConvert::ToTransform(matrix3D));
			TopoDS_Face temp = TopoDS::Face(gTran.Shape());
			GC::KeepAlive(this);
			return gcnew XbimFace(temp);
		}

		IXbimGeometryObject^ XbimFace::TransformShallow(XbimMatrix3D matrix3D)
		{
			TopoDS_Face face = TopoDS::Face(pFace->Moved(XbimConvert::ToTransform(matrix3D)));
			GC::KeepAlive(this);
			return gcnew XbimFace(face);
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
			TopoDS_Face face = this;
			BRepGProp_Face prop(face);
			gp_Pnt centre;
			gp_Vec normalDir;
			double u1, u2, v1, v2;
			prop.Bounds(u1, u2, v1, v2);
			prop.Normal((u1 + u2) / 2.0, (v1 + v2) / 2.0, centre, normalDir);
			XbimVector3D vec(normalDir.X(), normalDir.Y(), normalDir.Z());
			GC::KeepAlive(this);
			return vec.Normalized();
		}

		XbimVector3D XbimFace::NormalAt(double u, double v)
		{
			if (!IsValid) return XbimVector3D();
			TopoDS_Face face = this;
			BRepGProp_Face prop(face);
			gp_Pnt pos;
			gp_Vec normalDir;
			/*double u1, u2, v1, v2;
			prop.Bounds(u1, u2, v1, v2);*/
			prop.Normal(u, v, pos, normalDir);
			XbimVector3D vec(normalDir.X(), normalDir.Y(), normalDir.Z());
			return vec.Normalized();

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
			GC::KeepAlive(this);
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

		void XbimFace::SaveAsBrep(String^ fileName)
		{
			if (IsValid)
			{
				XbimOccWriter^ occWriter = gcnew XbimOccWriter();
				occWriter->Write(this, fileName);
			}
		}

#pragma endregion


#pragma region Methods

		void XbimFace::Move(IIfcAxis2Placement3D^ position)
		{
			if (!IsValid) return;
			gp_Trsf toPos = XbimConvert::ToTransform(position);
			pFace->Move(toPos);
		}

		void XbimFace::Move(gp_Trsf transform)
		{
			if (!IsValid) return;
			pFace->Move(transform);
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
			if (wire == nullptr) throw gcnew ArgumentException("Only IXbimWires created bu Xbim.OCC modules are supported", "xbimWire");
			BRepBuilderAPI_MakeFace faceMaker(*pFace);
			faceMaker.Add(wire);
			if (!faceMaker.IsDone()) return false;
			*pFace = faceMaker.Face();
			GC::KeepAlive(this);
			return true;
		}


		XbimPoint3D XbimFace::PointAtParameters(double u, double v)
		{
			if (!IsValid) return XbimPoint3D();
			const Handle(Geom_Surface)& surface = BRep_Tool::Surface(*pFace);
			gp_Pnt p;
			surface->D0(u, v, p);
			GC::KeepAlive(this);
			return XbimPoint3D(p.X(), p.Y(), p.Z());
		}

		Handle(Geom_Surface) XbimFace::GetSurface()
		{
			TopoDS_Face face = this;
			return BRep_Tool::Surface(face);
		}

		void XbimFace::SetLocation(TopLoc_Location loc)
		{
			if (IsValid)
				pFace->Move(loc);
		}

		XbimGeometryObject^ XbimFace::Transformed(IIfcCartesianTransformationOperator^ transformation)
		{
			IIfcCartesianTransformationOperator3DnonUniform^ nonUniform = dynamic_cast<IIfcCartesianTransformationOperator3DnonUniform^>(transformation);
			if (nonUniform != nullptr)
			{
				gp_GTrsf trans = XbimConvert::ToTransform(nonUniform);
				BRepBuilderAPI_GTransform tr(this, trans, Standard_True); //make a copy of underlying shape
				GC::KeepAlive(this);
				return gcnew XbimFace(TopoDS::Face(tr.Shape()), Tag);
			}
			else
			{
				gp_Trsf trans = XbimConvert::ToTransform(transformation);
				BRepBuilderAPI_Transform tr(this, trans, Standard_False); //do not make a copy of underlying shape
				GC::KeepAlive(this);
				return gcnew XbimFace(TopoDS::Face(tr.Shape()), Tag);
			}
		}

		XbimGeometryObject^ XbimFace::Moved(IIfcPlacement^ placement)
		{
			if (!IsValid) return this;
			XbimFace^ copy = gcnew XbimFace(*pFace, Tag); //take a copy of the shape
			TopLoc_Location loc = XbimConvert::ToLocation(placement);
			copy->Move(loc);
			return copy;
		}

		XbimGeometryObject^ XbimFace::Moved(IIfcObjectPlacement^ objectPlacement, ILogger^ logger)
		{
			if (!IsValid) return this;
			XbimFace^ copy = gcnew XbimFace(*pFace, Tag); //take a copy of the shape
			TopLoc_Location loc = XbimConvert::ToLocation(objectPlacement, logger);
			copy->Move(loc);
			return copy;
		}

#pragma endregion

	}
}