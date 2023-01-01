#include "XbimGeometryCreatorV6.h"
#include "XbimWire.h"
#include "XbimFace.h"
#include "XbimSolid.h"
#include "XbimSolidSet.h"
#include "XbimOccWriter.h"
#include "XbimConvert.h"


#include "./Factories/SolidFactory.h"
#include "./Factories/FaceFactory.h"
#include "./Factories/WireFactory.h"
#include <BRep_Builder.hxx>
#include <Geom_Plane.hxx>
#include <IntAna2d_AnaIntersection.hxx>
#include <gp_Lin2d.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <BRepTools.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepOffsetAPI_MakePipeShell.hxx>
#include <Standard_Type.hxx>
#include <GeomLib.hxx>
#include <Geom2d_Line.hxx>


namespace Xbim
{
	namespace Geometry
	{

#pragma region Logging


		void XbimGeometryCreatorV6::LogInfo(ILogger^ logger, Object^ entity, System::String^ format, ...array<Object^>^ arg)
		{
			throw gcnew System::NotImplementedException();
		}

		void XbimGeometryCreatorV6::LogWarning(ILogger^ logger, Object^ entity, System::String^ format, ...array<Object^>^ arg)
		{
			throw gcnew System::NotImplementedException();
		}

		void XbimGeometryCreatorV6::LogError(ILogger^ logger, Object^ entity, System::String^ format, ...array<Object^>^ arg)
		{
			throw gcnew System::NotImplementedException();
		}

		void XbimGeometryCreatorV6::LogDebug(ILogger^ logger, Object^ entity, System::String^ format, ...array<Object^>^ arg)
		{
			throw gcnew System::NotImplementedException();
		}

#pragma endregion

#pragma region Creators

		IXbimGeometryObject^ XbimGeometryCreatorV6::Create(IIfcGeometricRepresentationItem^ geomRep, ILogger^)
		{
			return Create(geomRep, nullptr, nullptr);
		}

		IXShape^ XbimGeometryCreatorV6::Build(IIfcGeometricRepresentationItem^ geomRep)
		{

			IXbimGeometryObject^ geomObj = Create(geomRep, nullptr, nullptr);
			return XbimGeometryObject::ToXShape(geomObj);
		}


		IXbimGeometryObject^ XbimGeometryCreatorV6::Create(IIfcGeometricRepresentationItem^ geomRep, IIfcAxis2Placement3D^ objectLocation, ILogger^)
		{

			try
			{
				if (geomRep == nullptr)
				{
					throw gcnew System::NullReferenceException("Geometry Representation Item cannot be null");
				}
				IIfcSweptAreaSolid^ sweptAreaSolid = dynamic_cast<IIfcSweptAreaSolid^>(geomRep);
				if (sweptAreaSolid != nullptr)
				{
					if (dynamic_cast<IIfcCompositeProfileDef^>(sweptAreaSolid->SweptArea)) //handle these as composite solids
					{
						XbimSolidSet^ solidset = (XbimSolidSet^)CreateSolidSet(sweptAreaSolid, Logger());
						if (objectLocation != nullptr) solidset->Move(objectLocation);
						return Trim(solidset);
					}
					else
					{
						XbimSolid^ solid = (XbimSolid^)CreateSolid((IIfcSweptAreaSolid^)geomRep, Logger());
						if (objectLocation != nullptr) solid->Move(objectLocation);
						return solid;
					}
				}
				else if (dynamic_cast<IIfcManifoldSolidBrep^>(geomRep))
				{
					XbimCompound^ comp = gcnew XbimCompound((IIfcManifoldSolidBrep^)geomRep, Logger());
					if (objectLocation != nullptr) comp->Move(objectLocation);
					return comp;
				}
				else if (dynamic_cast<IIfcSweptDiskSolid^>(geomRep))
				{
					XbimSolid^ solid = (XbimSolid^)CreateSolid((IIfcSweptDiskSolid^)geomRep, Logger());
					if (objectLocation != nullptr) solid->Move(objectLocation);
					return solid;
				}
				else if (dynamic_cast<IIfcBooleanClippingResult^>(geomRep))
				{
					XbimSolidSet^ solidSet = gcnew XbimSolidSet((IIfcBooleanClippingResult^)geomRep, this, Logger());
					if (objectLocation != nullptr) solidSet->Move(objectLocation);
					return Trim(solidSet);
				}
				else if (dynamic_cast<IIfcBooleanResult^>(geomRep))
				{
					XbimSolidSet^ solidSet = gcnew XbimSolidSet((IIfcBooleanResult^)geomRep, this, Logger());
					if (objectLocation != nullptr) solidSet->Move(objectLocation);
					return Trim(solidSet);
				}
				else if (dynamic_cast<IIfcFaceBasedSurfaceModel^>(geomRep))
				{
					XbimCompound^ comp = (XbimCompound^)CreateSurfaceModel((IIfcFaceBasedSurfaceModel^)geomRep, Logger());
					if (objectLocation != nullptr) comp->Move(objectLocation);
					return comp;
				}
				else if (dynamic_cast<IIfcShellBasedSurfaceModel^>(geomRep))
				{
					XbimCompound^ comp = (XbimCompound^)CreateSurfaceModel((IIfcShellBasedSurfaceModel^)geomRep, Logger());
					if (objectLocation != nullptr) comp->Move(objectLocation);
					return comp;
				}
				else if (dynamic_cast<IIfcTriangulatedFaceSet^>(geomRep))
				{
					XbimCompound^ comp = (XbimCompound^)CreateSurfaceModel((IIfcTriangulatedFaceSet^)geomRep, Logger());
					if (objectLocation != nullptr) comp->Move(objectLocation);
					return comp;
				}
				else if (dynamic_cast<IIfcPolygonalFaceSet^>(geomRep))
				{
					IIfcPolygonalFaceSet^ polySet = (IIfcPolygonalFaceSet^)geomRep;
					
					if (polySet->Closed.HasValue && polySet->Closed.Value)
					{
						XbimSolidSet^ ss = gcnew XbimSolidSet(GetSolidFactory()->BuildPolygonalFaceSet(polySet));
						if (objectLocation != nullptr) ss->Move(objectLocation);
						return ss;
					}
					else
					{
						CheckClosedStatus checkClosed;
						XbimShell^ comp = gcnew XbimShell(GetShellFactory()->BuildPolygonalFaceSet(polySet, checkClosed));
						if (objectLocation != nullptr) comp->Move(objectLocation);
						return comp;
					}
				}
				else if (dynamic_cast<IIfcTessellatedFaceSet^>(geomRep))
				{
					XbimCompound^ comp = (XbimCompound^)CreateSurfaceModel((IIfcTessellatedFaceSet^)geomRep, Logger());
					if (objectLocation != nullptr) comp->Move(objectLocation);
					return comp;
				}
				else if (dynamic_cast<IIfcSectionedSpine^>(geomRep))
				{
					XbimSolid^ solid = (XbimSolid^)CreateSolid((IIfcSectionedSpine^)geomRep, Logger());
					if (objectLocation != nullptr) solid->Move(objectLocation);
					return solid;
				}
				else if (dynamic_cast<IIfcHalfSpaceSolid^>(geomRep))
				{
					XbimSolid^ solid = (XbimSolid^)CreateSolid((IIfcHalfSpaceSolid^)geomRep, Logger());
					if (objectLocation != nullptr) solid->Move(objectLocation);
					return solid;
				}
				else if (dynamic_cast<IIfcCurve^>(geomRep))
				{
					XbimWire^ wire = (XbimWire^)CreateWire((IIfcCurve^)geomRep, Logger());
					if (objectLocation != nullptr) wire->Move(objectLocation);
					return wire;
				}
				else if (dynamic_cast<IIfcCompositeCurveSegment^>(geomRep))
				{
					XbimWire^ wire = (XbimWire^)CreateWire((IIfcCompositeCurveSegment^)geomRep, Logger());
					if (objectLocation != nullptr) wire->Move(objectLocation);
					return wire;
				}
				else if (dynamic_cast<IIfcBoundingBox^>(geomRep))
				{
					XbimSolid^ solid = (XbimSolid^)CreateSolid((IIfcBoundingBox^)geomRep, Logger());
					if (objectLocation != nullptr) solid->Move(objectLocation);
					return solid;
				}
				else if (dynamic_cast<IIfcSurface^>(geomRep))
				{
					XbimFace^ face = (XbimFace^)CreateFace((IIfcSurface^)geomRep, Logger());
					if (objectLocation != nullptr) face->Move(objectLocation);
					return face;
				}
				else if (dynamic_cast<IIfcCsgPrimitive3D^>(geomRep))
				{
					return CreateSolid(static_cast<IIfcCsgPrimitive3D^>(geomRep),Logger());
				}
				else if (dynamic_cast<IIfcCsgSolid^>(geomRep))
				{
					XbimSolidSet^ solidSet = (XbimSolidSet^)CreateSolidSet((IIfcCsgSolid^)geomRep, Logger());
					if (objectLocation != nullptr) solidSet->Move(objectLocation);
					return Trim(solidSet);
				}
				else if (dynamic_cast<IIfcSphere^>(geomRep))
				{
					XbimSolid^ solid = (XbimSolid^)CreateSolid((IIfcSphere^)geomRep, Logger());
					if (objectLocation != nullptr) solid->Move(objectLocation);
					return solid;
				}
				else if (dynamic_cast<IIfcGeometricSet^>(geomRep))
				{
					if (objectLocation != nullptr) LogError(Logger(), geomRep, "Move is not implemented for IIfcGeometricSet");
					return CreateGeometricSet((IIfcGeometricSet^)geomRep, Logger());
				}
				throw gcnew System::NotSupportedException("Geometry Representation Item is not supported");
			}
			catch (System::Exception^ e)
			{
				throw RaiseGeometryServiceException("Geometry Service Error", geomRep, e);
			}

		}


#pragma endregion

#pragma region Point creation
		IXbimPoint^ XbimGeometryCreatorV6::CreatePoint(double x, double y, double z, double tolerance)
		{
			return gcnew XbimPoint3DWithTolerance(x, y, z, tolerance);
		}

		IXbimPoint^ XbimGeometryCreatorV6::CreatePoint(IIfcCartesianPoint^ p)
		{
			return gcnew XbimPoint3DWithTolerance(XbimPoint3D(p->X, p->Y, p->Z), p->Model->ModelFactors->Precision);
		}

		IXbimPoint^ XbimGeometryCreatorV6::CreatePoint(IIfcPointOnCurve^ p, ILogger^)
		{
			return gcnew XbimPoint3DWithTolerance(p, Logger());
		}

		IXbimPoint^ XbimGeometryCreatorV6::CreatePoint(IIfcPointOnSurface^ p, ILogger^)
		{
			return gcnew XbimPoint3DWithTolerance(p, Logger());
		}

		IXbimPoint^ XbimGeometryCreatorV6::CreatePoint(XbimPoint3D p, double tolerance)
		{
			return gcnew XbimPoint3DWithTolerance(p, tolerance);
		}

		IXbimPoint^ XbimGeometryCreatorV6::CreatePoint(IIfcPoint^ pt)
		{
			if (dynamic_cast<IIfcCartesianPoint^>(pt)) return CreatePoint((IIfcCartesianPoint^)pt);
			else if (dynamic_cast<IIfcPointOnCurve^>(pt)) return CreatePoint((IIfcPointOnCurve^)pt);
			else if (dynamic_cast<IIfcPointOnSurface^>(pt)) return CreatePoint((IIfcPointOnSurface^)pt);
			else throw RaiseGeometryServiceException("Geometry Representation Type is not implemented", pt);

		}
#pragma endregion

#pragma region Curve Creation
		IXbimCurve^ XbimGeometryCreatorV6::CreateCurve(IIfcCurve^ curve, ILogger^)
		{
			//if (Is3D(curve))
			return gcnew XbimCurve(curve, Logger());
			//else
			//	return gcnew XbimCurve2D(curve, Logger());
		}
		IXbimCurve^ XbimGeometryCreatorV6::CreateCurve(IIfcPolyline^ curve, ILogger^)
		{
			if (Is3D(curve))
				return gcnew XbimCurve(curve, Logger());
			else
				return gcnew XbimCurve2D(curve, Logger());
		}
		IXbimCurve^ XbimGeometryCreatorV6::CreateCurve(IIfcCircle^ curve, ILogger^)
		{
			if (Is3D(curve))
				return gcnew XbimCurve(curve, Logger());
			else
				return gcnew XbimCurve2D(curve, Logger());
		}
		IXbimCurve^ XbimGeometryCreatorV6::CreateCurve(IIfcEllipse^ curve, ILogger^)
		{
			if (Is3D(curve))
				return gcnew XbimCurve(curve, Logger());
			else
				return gcnew XbimCurve2D(curve, Logger());
		}
		IXbimCurve^ XbimGeometryCreatorV6::CreateCurve(IIfcLine^ curve, ILogger^)
		{
			if (Is3D(curve))
				return gcnew XbimCurve(curve, Logger());
			else
				return gcnew XbimCurve2D(curve, Logger());
		}
		IXbimCurve^ XbimGeometryCreatorV6::CreateCurve(IIfcTrimmedCurve^ curve, ILogger^)
		{
			if (Is3D(curve))
				return gcnew XbimCurve(curve, Logger());
			else
				return gcnew XbimCurve2D(curve, Logger());
		}
		IXbimCurve^ XbimGeometryCreatorV6::CreateCurve(IIfcRationalBSplineCurveWithKnots^ curve, ILogger^)
		{
			if (Is3D(curve))
				return gcnew XbimCurve(curve, Logger());
			else
				return gcnew XbimCurve2D(curve, Logger());
		}
		IXbimCurve^ XbimGeometryCreatorV6::CreateCurve(IIfcBSplineCurveWithKnots^ curve, ILogger^)
		{
			if (Is3D(curve))
				return gcnew XbimCurve(curve, Logger());
			else
				return gcnew XbimCurve2D(curve, Logger());
		}

		IXbimCurve^ XbimGeometryCreatorV6::CreateCurve(IIfcOffsetCurve3D^ curve, ILogger^)
		{
			return gcnew XbimCurve(curve, Logger());
		}

		IXbimCurve^ XbimGeometryCreatorV6::CreateCurve(IIfcOffsetCurve2D^ curve, ILogger^)
		{
			return gcnew XbimCurve2D(curve, Logger());
		}
#pragma endregion

#pragma region Wire Creation
		IXbimWire^ XbimGeometryCreatorV6::CreateWire(IIfcCurve^ curve, ILogger^)
		{
			IIfcPolyline^ pline = dynamic_cast<IIfcPolyline^>(curve);
			IIfcCompositeCurve^ composite = dynamic_cast<IIfcCompositeCurve^>(curve);
			IIfcIndexedPolyCurve^ poly = dynamic_cast<IIfcIndexedPolyCurve^>(curve);
			if (composite != nullptr)
				return gcnew XbimWire(composite, Logger());
			else if (poly != nullptr)
				return gcnew XbimWire(poly, Logger());
			else if (pline != nullptr)
				return gcnew XbimWire(pline, Logger());
			else
				return gcnew XbimWire(curve, Logger());
		}

		IXbimWire^ XbimGeometryCreatorV6::CreateWire(IIfcCompositeCurveSegment^ compCurveSeg, ILogger^)
		{
			return gcnew XbimWire(compCurveSeg, Logger());
		}
#pragma endregion

#pragma region Face creation

		IXbimFace^ XbimGeometryCreatorV6::CreateFace(IXbimWire^ wire, ILogger^)
		{
			XbimWire^ w = (XbimWire^)wire;
			return gcnew XbimFace(wire, wire->IsPlanar, w->MaxTolerance, 0, Logger());
		};

		IXbimFace^ XbimGeometryCreatorV6::CreateFace(IIfcProfileDef^ profile, ILogger^)
		{
			return gcnew XbimFace(profile, Logger());
		};

		IXbimFace^ XbimGeometryCreatorV6::CreateFace(IIfcCompositeCurve^ cCurve, ILogger^)
		{

			//nb this is not a very sensible function and should be deprecated
			//there are too many uncertainties, the composite curve may not be closed, it might not fit on a surface, it may be 2 or 3d
			//this has been implemented but has the same restrictions as the V5 interface and should be avoided in future work
			TopoDS_Wire wire = this->GetWireFactory()->BuildWire(cCurve, false);
			if (wire.IsNull())
				throw RaiseGeometryServiceException("Could not buld composite curve", cCurve);
			BRepBuilderAPI_MakeFace faceMaker(wire);
			if (!faceMaker.IsDone())
				throw RaiseGeometryServiceException("Face not buld for composite curve", cCurve);
			return gcnew XbimFace(faceMaker.Face());
		};

		IXbimFace^ XbimGeometryCreatorV6::CreateFace(IIfcPolyline^ pline, ILogger^)
		{
			return gcnew XbimFace(pline, Logger());
		};

		IXbimFace^ XbimGeometryCreatorV6::CreateFace(IIfcPolyLoop^ loop, ILogger^)
		{
			return gcnew XbimFace(loop, Logger());
		};

		IXbimFace^ XbimGeometryCreatorV6::CreateFace(IIfcSurface^ surface, ILogger^)
		{
			return gcnew XbimFace(surface, Logger());
		};

		IXbimFace^ XbimGeometryCreatorV6::CreateFace(IIfcPlane^ plane, ILogger^)
		{
			return gcnew XbimFace(plane, Logger());
		};


#pragma endregion

#pragma region Shell Creation
		IXbimShell^ XbimGeometryCreatorV6::CreateShell(IIfcOpenShell^ shell, ILogger^)
		{
			return gcnew XbimShell(shell, Logger());
		}

		IXbimShell^ XbimGeometryCreatorV6::CreateShell(IIfcConnectedFaceSet^ shell, ILogger^)
		{
			return gcnew XbimShell(shell, Logger());
		};

		IXbimShell^ XbimGeometryCreatorV6::CreateShell(IIfcSurfaceOfLinearExtrusion^ linExt, ILogger^)
		{
			return gcnew XbimShell(linExt, Logger());
		}

#pragma endregion

#pragma region Solid Creation

		IXbimSolid^ XbimGeometryCreatorV6::CreateSolid(IIfcSweptAreaSolid^ IIfcSolid, ILogger^)
		{
			IIfcExtrudedAreaSolid^ eas = dynamic_cast<IIfcExtrudedAreaSolid^>(IIfcSolid);
			if (eas != nullptr) return CreateSolid(eas, Logger());
			IIfcRevolvedAreaSolid^ ras = dynamic_cast<IIfcRevolvedAreaSolid^>(IIfcSolid);
			if (ras != nullptr) return CreateSolid(ras, Logger());
			IIfcSurfaceCurveSweptAreaSolid^ scas = dynamic_cast<IIfcSurfaceCurveSweptAreaSolid^>(IIfcSolid);
			if (scas != nullptr) return CreateSolid(scas, Logger());
			throw gcnew System::NotImplementedException(System::String::Format("Swept Solid of Type {0} in entity #{1} is not implemented", IIfcSolid->GetType()->Name, IIfcSolid->EntityLabel));

		};

		IXbimSolid^ XbimGeometryCreatorV6::CreateSolid(IIfcTriangulatedFaceSet^ shell, ILogger^)
		{
			return gcnew XbimSolid(shell, Logger());
		}
		IXbimSolid^ XbimGeometryCreatorV6::CreateSolid(IIfcShellBasedSurfaceModel^ ifcSurface, ILogger^)
		{
			return gcnew XbimSolid(ifcSurface, Logger());
		}
		IXbimSolid^ XbimGeometryCreatorV6::CreateSolid(IIfcFaceBasedSurfaceModel^ ifcSurface, ILogger^)
		{
			return gcnew XbimSolid(ifcSurface, Logger());
		}

		IXbimSolid^ XbimGeometryCreatorV6::CreateSolid(IIfcExtrudedAreaSolid^ IIfcSolid, ILogger^)
		{
			return gcnew XbimSolid(IIfcSolid, Logger());
			//return gcnew XbimSolid(GetSolidFactory()->BuildExtrudedAreaSolid(IIfcSolid));
		};
		IXbimSolid^ XbimGeometryCreatorV6::CreateSolid(IIfcPolygonalFaceSet^ shell, ILogger^)
		{
			if (shell->Closed.HasValue && shell->Closed.Value)
				return gcnew XbimSolid(GetSolidFactory()->BuildPolygonalFaceSet(shell));
			else
				throw RaiseGeometryServiceException("IfcPolygonalFaceSet is not closed, use CreateSurfaceModel() method");
		}

		IXbimSolid^ XbimGeometryCreatorV6::CreateSolid(IIfcRevolvedAreaSolid^ IIfcSolid, ILogger^)
		{
			return gcnew XbimSolid(IIfcSolid, Logger());
		};

		IXbimSolid^ XbimGeometryCreatorV6::CreateSolid(IIfcSweptDiskSolid^ IIfcSolid, ILogger^)
		{
			return gcnew XbimSolid(IIfcSolid, Logger());
		};

		IXbimSolid^ XbimGeometryCreatorV6::CreateSolid(IIfcBoundingBox^ IIfcSolid, ILogger^)
		{
			return gcnew XbimSolid(IIfcSolid, Logger());
		};

		IXbimSolid^ XbimGeometryCreatorV6::CreateSolid(IIfcSurfaceCurveSweptAreaSolid^ IIfcSolid, ILogger^)
		{
			return gcnew XbimSolid(IIfcSolid, Logger());
		};

		[[deprecated("Please use CreateSolidSet")]]
		IXbimSolid^ XbimGeometryCreatorV6::CreateSolid(IIfcBooleanResult^ IIfcSolid, ILogger^)
		{
			XbimSolidSet^ ss = gcnew XbimSolidSet(IIfcSolid, Logger());
			return Enumerable::FirstOrDefault(ss);
		};
		IXbimSolid^ XbimGeometryCreatorV6::CreateSolid(IIfcSweptDiskSolidPolygonal^ ifcSolid, ILogger^)
		{
			return gcnew XbimSolid(ifcSolid, Logger());
		}
		IXbimSolid^ XbimGeometryCreatorV6::CreateSolid(IIfcRevolvedAreaSolidTapered^ ifcSolid, ILogger^)
		{
			return gcnew XbimSolid(ifcSolid, Logger());
		}
		IXbimSolid^ XbimGeometryCreatorV6::CreateSolid(IIfcFixedReferenceSweptAreaSolid^ ifcSolid, ILogger^)
		{
			return gcnew XbimSolid(ifcSolid, Logger());
		}
		IXbimSolid^ XbimGeometryCreatorV6::CreateSolid(IIfcAdvancedBrep^ ifcSolid, ILogger^)
		{
			return gcnew XbimSolid(GetSolidFactory()->BuildAdvancedBrep(ifcSolid));
		}
		IXbimSolid^ XbimGeometryCreatorV6::CreateSolid(IIfcAdvancedBrepWithVoids^ ifcSolid, ILogger^)
		{
			XbimCompound^ comp = gcnew XbimCompound((IIfcAdvancedBrepWithVoids^)ifcSolid, Logger());
			if (comp->Solids->Count > 0)
				return comp->Solids->First;
			else
				return gcnew XbimSolid();
		}
		IXbimSolid^ XbimGeometryCreatorV6::CreateSolid(IIfcSectionedSpine^ ifcSolid, ILogger^)
		{
			return gcnew XbimSolid(ifcSolid, Logger());
		}


		[[deprecated("Please use CreateSolidSet")]]
		IXbimSolid^ XbimGeometryCreatorV6::CreateSolid(IIfcBooleanClippingResult^ IIfcSolid, ILogger^)
		{
			return Enumerable::FirstOrDefault(gcnew XbimSolidSet(IIfcSolid, Logger()));
		};

		IXbimSolidSet^ XbimGeometryCreatorV6::CreateSolidSet(IIfcBooleanClippingResult^ IIfcSolid, ILogger^)
		{
			return gcnew XbimSolidSet(IIfcSolid, Logger());
		};


		IXbimSolid^ XbimGeometryCreatorV6::CreateSolid(IIfcHalfSpaceSolid^ IIfcSolid, ILogger^)
		{
			return gcnew XbimSolid(IIfcSolid, Logger());
		};


		IXbimSolid^ XbimGeometryCreatorV6::CreateSolid(IIfcBoxedHalfSpace^ IIfcSolid, ILogger^)
		{
			return gcnew XbimSolid(IIfcSolid, Logger());
		};

		IXbimSolid^ XbimGeometryCreatorV6::CreateSolid(IIfcPolygonalBoundedHalfSpace^ IIfcSolid, ILogger^)
		{
			return gcnew XbimSolid(IIfcSolid, Logger());
		};



		IXbimSolid^ XbimGeometryCreatorV6::CreateSolid(IIfcCsgPrimitive3D^ IIfcSolid, ILogger^)
		{
			return gcnew XbimSolid(GetSolidFactory()->BuildCsgPrimitive3D(IIfcSolid));
		};


		IXbimSolid^ XbimGeometryCreatorV6::CreateSolid(IIfcSphere^ IIfcSolid, ILogger^)
		{
			return gcnew XbimSolid(IIfcSolid, Logger());
		};

		IXbimSolid^ XbimGeometryCreatorV6::CreateSolid(IIfcBlock^ IIfcSolid, ILogger^)
		{
			return gcnew XbimSolid(GetSolidFactory()->BuildBlock(IIfcSolid));
		};

		IXbimSolid^ XbimGeometryCreatorV6::CreateSolid(IIfcRightCircularCylinder^ IIfcSolid, ILogger^)
		{
			return gcnew XbimSolid(GetSolidFactory()->BuildRightCircularCylinder(IIfcSolid));
		};

		IXbimSolid^ XbimGeometryCreatorV6::CreateSolid(IIfcRightCircularCone^ IIfcSolid, ILogger^)
		{
			return gcnew XbimSolid(GetSolidFactory()->BuildRightCircularCone(IIfcSolid));
		};

		IXbimSolid^ XbimGeometryCreatorV6::CreateSolid(IIfcRectangularPyramid^ IIfcSolid, ILogger^)
		{
			return gcnew XbimSolid(GetSolidFactory()->BuildRectangularPyramid(IIfcSolid));
		};

#pragma endregion

#pragma region Solid Set Creation



		IXbimSolidSet^ XbimGeometryCreatorV6::CreateSolidSet() {
			return gcnew XbimSolidSet(this);
		};

		IXbimSolidSet^ XbimGeometryCreatorV6::CreateSolidSet(IIfcSweptAreaSolid^ IIfcSolid, ILogger^)
		{
			return gcnew XbimSolidSet(IIfcSolid, Logger());
			/*IIfcExtrudedAreaSolid^ eas = dynamic_cast<IIfcExtrudedAreaSolid^>(IIfcSolid);
			if (eas != nullptr) return CreateSolidSet(eas, Logger());
			IIfcRevolvedAreaSolid^ ras = dynamic_cast<IIfcRevolvedAreaSolid^>(IIfcSolid);
			if (ras != nullptr) return CreateSolidSet(ras, Logger());
			IIfcSurfaceCurveSweptAreaSolid^ scas = dynamic_cast<IIfcSurfaceCurveSweptAreaSolid^>(IIfcSolid);
			if (scas != nullptr) return CreateSolidSet(scas, Logger());
			throw gcnew System::NotImplementedException(System::String::Format("Swept Solid of Type {0} in entity #{1} is not implemented", IIfcSolid->GetType()->Name, IIfcSolid->EntityLabel));*/

		};
		IXbimSolidSet^ XbimGeometryCreatorV6::CreateSolidSet(IIfcExtrudedAreaSolid^ IIfcSolid, ILogger^)
		{
			return gcnew XbimSolidSet(IIfcSolid, Logger());
		};

		IXbimSolidSet^ XbimGeometryCreatorV6::CreateSolidSet(IIfcRevolvedAreaSolid^ IIfcSolid, ILogger^)
		{
			return gcnew XbimSolidSet(IIfcSolid, Logger());
		};

		IXbimSolidSet^ XbimGeometryCreatorV6::CreateSolidSet(IIfcSurfaceCurveSweptAreaSolid^ IIfcSolid, ILogger^)
		{
			return gcnew XbimSolidSet(IIfcSolid, Logger());
		}
		IXbimSolidSet^ XbimGeometryCreatorV6::CreateSolidSet(IIfcTriangulatedFaceSet^ shell, ILogger^)
		{
			return gcnew XbimSolidSet(shell, Logger());
		}
		IXbimSolidSet^ XbimGeometryCreatorV6::CreateSolidSet(IIfcPolygonalFaceSet^ shell, ILogger^)
		{
			if (shell->Closed.HasValue && shell->Closed.Value)
				return gcnew XbimSolidSet(shell, Logger());
			else
				throw RaiseGeometryServiceException("IfcPolygonalFaceSet is not closed, use CreateSurfaceModel() method");
		}

		IXbimSolidSet^ XbimGeometryCreatorV6::CreateSolidSet(IIfcShellBasedSurfaceModel^ ifcSurface, ILogger^)
		{
			return gcnew XbimSolidSet(ifcSurface, Logger());
		}
		IXbimSolidSet^ XbimGeometryCreatorV6::CreateSolidSet(IIfcFaceBasedSurfaceModel^ ifcSurface, ILogger^)
		{
			return gcnew XbimSolidSet(ifcSurface, Logger());
		}

		IXbimSolidSet^ XbimGeometryCreatorV6::CreateSolidSet(IIfcBooleanResult^ boolOp, ILogger^)
		{
			return gcnew XbimSolidSet(boolOp, Logger());
		};

		IXbimSolidSet^ XbimGeometryCreatorV6::CreateSolidSet(IIfcBooleanOperand^ IIfcSolid, ILogger^)
		{
			return gcnew XbimSolidSet(IIfcSolid, Logger());
			//ensure operands get treated correctly
			//if (dynamic_cast<IIfcBooleanClippingResult^>(IIfcSolid))
			//	return gcnew XbimSolidSet((IIfcBooleanClippingResult^)IIfcSolid, Logger());
			//else if (dynamic_cast<IIfcBooleanResult^>(IIfcSolid))
			//	return gcnew XbimSolidSet((IIfcBooleanResult^)IIfcSolid, Logger());
			//else if (dynamic_cast<IIfcCsgSolid^>(IIfcSolid))
			//	return gcnew XbimSolidSet((IIfcCsgSolid^)IIfcSolid, Logger());
			//else if (dynamic_cast<IIfcManifoldSolidBrep^>(IIfcSolid)) //these must be single volume solid
			//	return gcnew XbimSolidSet(gcnew XbimSolid((IIfcManifoldSolidBrep^)IIfcSolid, Logger()));
			//else if (dynamic_cast<IIfcSweptAreaSolid^>(IIfcSolid)) //these must be single volume solid
			//	return gcnew XbimSolidSet(gcnew XbimSolid((IIfcSweptAreaSolid^)IIfcSolid, Logger()));
			//else if (dynamic_cast<IIfcSweptDiskSolid^>(IIfcSolid))
			//	return gcnew XbimSolidSet(gcnew XbimSolid((IIfcSweptDiskSolid^)IIfcSolid, Logger()));
			//else if (dynamic_cast<IIfcHalfSpaceSolid^>(IIfcSolid))
			//	return gcnew  XbimSolidSet(gcnew XbimSolid((IIfcHalfSpaceSolid^)IIfcSolid, Logger()));
			//else if (dynamic_cast<IIfcCsgPrimitive3D^>(IIfcSolid))
			//	return gcnew XbimSolidSet(gcnew XbimSolid((IIfcCsgPrimitive3D^)IIfcSolid, Logger()));
			//throw gcnew System::Exception(System::String::Format("Boolean Operand with Type {0} is not implemented", IIfcSolid->GetType()->Name));
		};

		IXbimSolidSet^ XbimGeometryCreatorV6::CreateSolidSet(IIfcManifoldSolidBrep^ IIfcSolid, ILogger^)
		{
			return gcnew XbimSolidSet(IIfcSolid, Logger());
		};

		IXbimSolidSet^ XbimGeometryCreatorV6::CreateSolidSet(IIfcFacetedBrep^ IIfcSolid, ILogger^)
		{
			return gcnew XbimSolidSet(GetSolidFactory()->BuildFacetedBrep(IIfcSolid));
		};

		IXbimSolidSet^ XbimGeometryCreatorV6::CreateSolidSet(IIfcFacetedBrepWithVoids^ IIfcSolid, ILogger^)
		{
			return gcnew XbimSolidSet(IIfcSolid, Logger());
		};

		IXbimSolidSet^ XbimGeometryCreatorV6::CreateSolidSet(IIfcClosedShell^ IIfcSolid, ILogger^)
		{
			CheckClosedStatus isCheckedClosed;
			TopoDS_Shell shell = GetShellFactory()->BuildClosedShell(IIfcSolid, isCheckedClosed);
			//for backward compatibility, a shell is just made into a solid and a warning is issued, fixed in V6
			if (isCheckedClosed == CheckedNotClosed)
				this->LogWarning("IfcClosedShell definition has not defined a shell that is closed", IIfcSolid);
			TopoDS_Solid solid = GetSolidFactory()->EXEC_NATIVE->MakeSolid(shell);
			return gcnew XbimSolidSet(solid);
		};


		IXbimSolidSet^ XbimGeometryCreatorV6::CreateSolidSet(IIfcCsgSolid^ IIfcSolid, ILogger^)
		{
			return gcnew XbimSolidSet(IIfcSolid, Logger());
		};
#pragma endregion

#pragma region Surface Model Creation
		//Surface Models containing one or more shells or solids
		IXbimGeometryObjectSet^ XbimGeometryCreatorV6::CreateSurfaceModel(IIfcShellBasedSurfaceModel^ IIfcSurface, ILogger^)
		{
			return gcnew XbimCompound(IIfcSurface, Logger());
		};

		IXbimGeometryObjectSet^ XbimGeometryCreatorV6::CreateSurfaceModel(IIfcFaceBasedSurfaceModel^ IIfcSurface, ILogger^)
		{
			return gcnew XbimCompound(IIfcSurface, Logger());
		};

		IXbimGeometryObjectSet^ XbimGeometryCreatorV6::CreateSurfaceModel(IIfcTessellatedFaceSet^ faceSet, ILogger^)
		{
			IIfcTriangulatedFaceSet^ tfs = dynamic_cast<IIfcTriangulatedFaceSet^>(faceSet);
			if (tfs != nullptr)  return gcnew XbimCompound(tfs, Logger());
			IIfcPolygonalFaceSet^ pfs = dynamic_cast<IIfcPolygonalFaceSet^>(faceSet);
			if (pfs != nullptr)
			{

				return gcnew XbimCompound(pfs, Logger());
			}
			throw gcnew System::Exception(System::String::Format("IIfcTessellatedFaceSet of Type {0} is not implemented", faceSet->GetType()->Name));
		}

#pragma endregion

#pragma region GeometryObject Set Creation

		IXbimGeometryObjectSet^ XbimGeometryCreatorV6::CreateGeometryObjectSet() {
			return gcnew XbimGeometryObjectSet();
		};
		IXbimGeometryObjectSet^ XbimGeometryCreatorV6::CreateGeometricSet(IIfcGeometricSet^ geomSet, ILogger^)
		{
			XbimGeometryObjectSet^ result = gcnew XbimGeometryObjectSet(Enumerable::Count(geomSet->Elements));
			for each (IIfcGeometricSetSelect ^ elem in geomSet->Elements)
			{
				if (dynamic_cast<IIfcPoint^>(elem)) result->Add(CreatePoint((IIfcPoint^)elem));
				else if (dynamic_cast<IIfcCurve^>(elem)) result->Add(CreateWire((IIfcCurve^)elem, Logger()));
				else if (dynamic_cast<IIfcSurface^>(elem)) result->Add(CreateFace((IIfcSurface^)elem, Logger()));
			}
			return result;
		}
#pragma endregion

#pragma region Write Triangulation Functions

		void XbimGeometryCreatorV6::Mesh(IXbimMeshReceiver^ mesh, IXbimGeometryObject^ geometryObject, double precision, double deflection, double angle)
		{
			XbimSetObject^ objSet = dynamic_cast<XbimSetObject^>(geometryObject);
			XbimOccShape^ occObject = dynamic_cast<XbimOccShape^>(geometryObject);
			if (objSet != nullptr)
				objSet->Mesh(mesh, precision, deflection, angle);
			else if (occObject != nullptr)
				occObject->Mesh(mesh, precision, deflection, angle);
			else
				throw RaiseGeometryServiceException("Unsupported geometry type cannot be meshed");
		}

		void XbimGeometryCreatorV6::WriteTriangulation(IXbimMeshReceiver^ mesh, IXbimGeometryObject^ shape, double tolerance, double deflection, double angle)
		{

			XbimOccShape^ xShape = dynamic_cast<XbimOccShape^>(shape);
			if (xShape != nullptr)
			{
				xShape->WriteTriangulation(mesh, tolerance, deflection, angle);
				return;
			}

		}
		void XbimGeometryCreatorV6::WriteTriangulation(TextWriter^ tw, IXbimGeometryObject^ shape, double tolerance, double deflection, double angle)
		{


			XbimOccShape^ xShape = dynamic_cast<XbimOccShape^>(shape);
			if (xShape != nullptr)
			{
				xShape->WriteTriangulation(tw, tolerance, deflection, angle);
				return;
			}

		}

		void XbimGeometryCreatorV6::WriteTriangulation(BinaryWriter^ bw, IXbimGeometryObject^ shape, double tolerance, double deflection, double angle)
		{

			XbimOccShape^ xShape = dynamic_cast<XbimOccShape^>(shape);
			if (xShape != nullptr)
			{
				xShape->WriteTriangulation(bw, tolerance, deflection, angle);
				return;
			}
		}

		XbimShapeGeometry^ XbimGeometryCreatorV6::CreateShapeGeometry(IXbimGeometryObject^ geometryObject, double precision, double deflection, double angle, XbimGeometryType storageType, ILogger^ /*logger*/)
		{
			XbimShapeGeometry^ shapeGeom = gcnew XbimShapeGeometry();

			if (geometryObject->IsSet)
			{
				IEnumerable<IXbimGeometryObject^>^ set = dynamic_cast<IEnumerable<IXbimGeometryObject^>^>(geometryObject);
				if (set != nullptr)
				{
					MemoryStream^ memStream = gcnew MemoryStream(0x4000);
					if (storageType == XbimGeometryType::PolyhedronBinary)
					{
						BRep_Builder builder;
						BinaryWriter^ bw = gcnew BinaryWriter(memStream);
						TopoDS_Compound occCompound;
						builder.MakeCompound(occCompound);
						for each (IXbimGeometryObject ^ geom in set)
						{
							XbimOccShape^ xShape = dynamic_cast<XbimOccShape^>(geom);
							if (xShape != nullptr)
							{
								builder.Add(occCompound, xShape);
							}
						}
						XbimCompound^ compound = gcnew XbimCompound(occCompound, false, precision);
						WriteTriangulation(bw, compound, precision, deflection, angle);
						bw->Close();
						delete bw;
					}
					else //default to text
					{
						throw gcnew System::Exception("Text format no longer supported");
						/*StreamWriter^ tw = gcnew StreamWriter(memStream);
						for each (IXbimGeometryObject ^ geom in set)
						{
							WriteTriangulation(tw, geom, precision, deflection, angle);
						}
						tw->Close();
						delete tw;*/
					}
					memStream->Flush();
					((IXbimShapeGeometryData^)shapeGeom)->ShapeData = memStream->ToArray();
					delete memStream;

					if (shapeGeom->ShapeData->Length > 0)
					{
						((XbimShapeGeometry^)shapeGeom)->BoundingBox = geometryObject->BoundingBox;
						((XbimShapeGeometry^)shapeGeom)->LOD = XbimLOD::LOD_Unspecified,
							((XbimShapeGeometry^)shapeGeom)->Format = storageType;
						return shapeGeom;
					}
				}
			}
			else
			{
				MemoryStream^ memStream = gcnew MemoryStream(0x4000);
				if (storageType == XbimGeometryType::PolyhedronBinary)
				{
					BinaryWriter^ bw = gcnew BinaryWriter(memStream);
					WriteTriangulation(bw, geometryObject, precision, deflection, angle);
					bw->Close();
					delete bw;
				}
				else //default to text
				{
					throw gcnew System::Exception("Text format no longer supported");
					/*TextWriter^ tw = gcnew StreamWriter(memStream);
					WriteTriangulation(tw, geometryObject, precision, deflection, angle);
					tw->Close();
					delete tw;*/
				}
				memStream->Flush();

				((IXbimShapeGeometryData^)shapeGeom)->ShapeData = memStream->ToArray();
				delete memStream;
				if (shapeGeom->ShapeData->Length > 0)
				{
					((XbimShapeGeometry^)shapeGeom)->BoundingBox = geometryObject->BoundingBox;
					((XbimShapeGeometry^)shapeGeom)->LOD = XbimLOD::LOD_Unspecified,
						((XbimShapeGeometry^)shapeGeom)->Format = storageType;
				}
			}

			return shapeGeom;

		}

#pragma endregion

#pragma region Sundry Methods
		XbimMatrix3D XbimGeometryCreatorV6::ToMatrix3D(IIfcObjectPlacement^ objPlacement, ILogger^)
		{
			return XbimConvert::ConvertMatrix3D(objPlacement, Logger());
		}

		IXbimSolidSet^ XbimGeometryCreatorV6::CreateGrid(IIfcGrid^ grid, ILogger^)
		{
			double mm = System::Math::Max(grid->Model->ModelFactors->OneMilliMeter, grid->Model->ModelFactors->Precision * 10);
			double precision = grid->Model->ModelFactors->Precision;


			gp_Vec normal;
			List<System::Tuple<int, XbimCurve2D^>^>^ UCurves = gcnew List<System::Tuple<int, XbimCurve2D^>^>();
			List<System::Tuple<int, XbimCurve2D^>^>^ VCurves = gcnew List<System::Tuple<int, XbimCurve2D^>^>();
			List<System::Tuple<int, XbimCurve2D^>^>^ WCurves = gcnew List<System::Tuple<int, XbimCurve2D^>^>();
			List<XbimPoint3D>^ intersections = gcnew List<XbimPoint3D>();

			for each (IIfcGridAxis ^ axis in grid->UAxes)
			{
				XbimCurve2D^ c = gcnew XbimCurve2D(axis->AxisCurve, Logger());
				if (c->IsValid)
				{
					System::Tuple<int, XbimCurve2D^>^ curveWithTag = System::Tuple::Create<int, XbimCurve2D^>(axis->EntityLabel, c);
					UCurves->Add(curveWithTag);
				}
			}
			for each (IIfcGridAxis ^ axis in grid->VAxes)
			{
				XbimCurve2D^ c = gcnew XbimCurve2D(axis->AxisCurve, Logger());
				if (c->IsValid)
				{
					System::Tuple<int, XbimCurve2D^>^ curveWithTag = System::Tuple::Create<int, XbimCurve2D^>(axis->EntityLabel, c);
					VCurves->Add(curveWithTag);
					for each (System::Tuple<int, XbimCurve2D^> ^ u in UCurves)
						intersections->AddRange(u->Item2->Intersections(c, precision, Logger()));
				}
			}

			for each (IIfcGridAxis ^ axis in grid->WAxes)
			{
				XbimCurve2D^ c = gcnew XbimCurve2D(axis->AxisCurve, Logger());
				if (c->IsValid)
				{
					System::Tuple<int, XbimCurve2D^>^ curveWithTag = System::Tuple::Create<int, XbimCurve2D^>(axis->EntityLabel, c);
					WCurves->Add(curveWithTag);
					for each (System::Tuple<int, XbimCurve2D^> ^ u in UCurves)
						intersections->AddRange(u->Item2->Intersections(c, precision, Logger()));
					for each (System::Tuple<int, XbimCurve2D^> ^ v in VCurves)
						intersections->AddRange(v->Item2->Intersections(c, precision, Logger()));
				}
			}

			XbimRect3D bb = XbimRect3D::Empty;
			//calculate all the bounding box
			for each (XbimPoint3D pt in intersections)
			{
				if (bb.IsEmpty) bb = XbimRect3D(pt);
				else bb.Union(pt);
			}

			if (bb.SizeX < precision || bb.SizeY < precision)
			{
				LogWarning(Logger(), grid, "Extent of grid is near zero. Found "
					+ intersections->Count + " axis intersections having " + (UCurves->Count + VCurves->Count + WCurves->Count) + " grid axis.");
				XbimPoint3D c = bb.Centroid();
				bb = XbimRect3D(c.X - 75 * mm, c.Y - 75 * mm, c.Z - 75 * mm, 150 * mm, 150 * mm, 150 * mm);
			}
			else
				//the box should have a Z of zero so inflate it a bit
				bb = XbimRect3D::Inflate(bb, bb.SizeX * 0.2, bb.SizeY * 0.2, 0);

			//create a bounded planar 

			gp_Lin2d top(gp_Pnt2d(bb.X, bb.Y + bb.SizeY), gp_Dir2d(1, 0));
			gp_Lin2d bottom(gp_Pnt2d(bb.X, bb.Y), gp_Dir2d(1, 0));
			gp_Lin2d left(gp_Pnt2d(bb.X, bb.Y), gp_Dir2d(0, 1));
			gp_Lin2d right(gp_Pnt2d(bb.X + bb.SizeX, bb.Y), gp_Dir2d(0, 1));

			bool failedGridLines = false;
			IEnumerable<System::Tuple<int, XbimCurve2D^>^>^ curves = Enumerable::Concat(Enumerable::Concat(UCurves, VCurves), WCurves);
			BRep_Builder b;
			TopoDS_Compound solidResults;
			b.MakeCompound(solidResults);

			TopoDS_Wire rect75mm = gcnew XbimWire(75 * mm, mm, precision, true);
			Handle(Geom_Plane) gridPlane = new Geom_Plane(gp_Pln());
			for each (System::Tuple<int, XbimCurve2D^> ^ curveWithTag in curves)
			{
				Handle(Geom2d_Curve) hcurve = curveWithTag->Item2;
				IntAna2d_AnaIntersection its;
				if (hcurve->IsKind(STANDARD_TYPE(Geom2d_Line))) //trim the infinite lines
				{
					const Handle(Geom2d_Line) axis = Handle(Geom2d_Line)::DownCast(hcurve);
					gp_Lin2d line2d = axis->Lin2d();
					List<double>^ params = gcnew List<double>();
					its.Perform(line2d, top);
					if (its.NbPoints() > 0) params->Add(its.Point(1).ParamOnFirst());
					its.Perform(line2d, bottom);
					if (its.NbPoints() > 0) params->Add(its.Point(1).ParamOnFirst());

					if (params->Count < 2)
					{
						its.Perform(line2d, left);
						if (its.NbPoints() > 0) params->Add(its.Point(1).ParamOnFirst());
					}
					if (params->Count < 2)
					{
						its.Perform(line2d, right);
						if (its.NbPoints() > 0) params->Add(its.Point(1).ParamOnFirst());
					}
					if (params->Count != 2)
						continue; //give up
					if (isnan(params[0]) || isnan(params[1]))
						continue; //give up
					hcurve = new Geom2d_TrimmedCurve(hcurve, System::Math::Min(params[0], params[1]), System::Math::Max(params[0], params[1]));
				}


				BRepBuilderAPI_PipeError pipeMakerStatus;
				try
				{
					gp_Pnt2d origin;
					gp_Vec2d curveMainDir;
					hcurve->D1(hcurve->FirstParameter(), origin, curveMainDir); //get the start point and line direction
					gp_Vec2d curveTangent = curveMainDir.GetNormal().Normalized();
					curveMainDir.Normalize();

					gp_Ax2 centre(gp_Pnt(origin.X(), origin.Y(), 0), gp_Vec(curveMainDir.X(), curveMainDir.Y(), 0), gp_Vec(curveTangent.X(), curveTangent.Y(), 0)); //create the axis for the rectangular face

					gp_Trsf trsf;
					trsf.SetTransformation(centre, gp_Ax3());
					TopoDS_Wire placedRect75mm = TopoDS::Wire(rect75mm.Moved(TopLoc_Location(trsf)));


					TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(GeomLib::To3d(gp_Ax2(), hcurve));

					TopoDS_Wire spine = BRepBuilderAPI_MakeWire(edge);

					BRepOffsetAPI_MakePipeShell pipeMaker(spine);
					pipeMaker.SetTolerance(precision, precision, 1.0e-2);

					pipeMaker.Add(placedRect75mm, Standard_False, Standard_True);
					pipeMaker.Build();
					pipeMakerStatus = pipeMaker.GetStatus();


					if (pipeMaker.IsDone() && pipeMaker.MakeSolid() && pipeMaker.Shape().ShapeType() == TopAbs_ShapeEnum::TopAbs_SOLID) //a solid is OK
					{

						b.Add(solidResults, pipeMaker.Shape());
					}
					else if (pipeMaker.IsDone()) //fix up from a shell
					{
						TopoDS_Shell shell;
						b.MakeShell(shell);
						{
							//add the other faces to the shell
							for (TopExp_Explorer explr(pipeMaker.Shape(), TopAbs_FACE); explr.More(); explr.Next())
							{
								b.Add(shell, TopoDS::Face(explr.Current()));
							}
							//visit wires not in a face
							for (TopExp_Explorer explr(pipeMaker.Shape(), TopAbs_WIRE, TopAbs_FACE); explr.More(); explr.Next())
							{
								BRepBuilderAPI_MakeFace faceMaker(TopoDS::Wire(explr.Current()), Standard_True);
								b.Add(shell, faceMaker.Face());
							}
							TopoDS_Solid solid;
							b.MakeSolid(solid);
							b.Add(solid, shell);
							b.Add(solidResults, solid);
						}

					}
					else
					{
						failedGridLines = true;
					}

				}
				catch (const Standard_Failure& sf)
				{
					System::String^ err = gcnew System::String(sf.GetMessageString());

					failedGridLines = true;
					LogWarning(Logger(), grid, "Grid axis #{0} caused exception. Status={1}, {2}", curveWithTag->Item1.ToString(), gcnew System::Int32(pipeMakerStatus), err);
					failedGridLines = true;
				}
				catch (...)
				{
					LogWarning(Logger(), grid, "Grid axis #{0} caused internal exception. Status={1}", curveWithTag->Item1.ToString(), gcnew System::Int32(pipeMakerStatus));
					failedGridLines = true;
				}

			}
			if (failedGridLines)
				LogWarning(Logger(), grid, "One or more grid lines has failed to convert successfully");

			return  gcnew XbimSolidSet(solidResults);
		}

		Xbim::Common::Geometry::IXbimGeometryObject^ XbimGeometryCreatorV6::Transformed(Xbim::Common::Geometry::IXbimGeometryObject^ geometryObject, Xbim::Ifc4::Interfaces::IIfcCartesianTransformationOperator^ transformation)
		{
			XbimOccShape^ occShape = dynamic_cast<XbimOccShape^>(geometryObject);
			if (occShape != nullptr)
				return occShape->Transformed(transformation);
			XbimSetObject^ occSet = dynamic_cast<XbimSetObject^>(geometryObject);
			if (occSet != nullptr)
				return occSet->Transformed(transformation);
			return geometryObject;//do nothing
		}

		Xbim::Common::Geometry::IXbimGeometryObject^ XbimGeometryCreatorV6::Moved(IXbimGeometryObject^ geometryObject, IIfcPlacement^ placement)
		{
			XbimOccShape^ occShape = dynamic_cast<XbimOccShape^>(geometryObject);
			if (occShape != nullptr)
				return occShape->Moved(placement);
			XbimSetObject^ occSet = dynamic_cast<XbimSetObject^>(geometryObject);
			if (occSet != nullptr)
				return occSet->Moved(placement);
			return geometryObject;
		}

		Xbim::Common::Geometry::IXbimGeometryObject^ XbimGeometryCreatorV6::Moved(IXbimGeometryObject^ geometryObject, IIfcObjectPlacement^ objectPlacement, ILogger^)
		{
			XbimOccShape^ occShape = dynamic_cast<XbimOccShape^>(geometryObject);
			if (occShape != nullptr)
				return occShape->Moved(objectPlacement, Logger());
			XbimSetObject^ occSet = dynamic_cast<XbimSetObject^>(geometryObject);
			if (occSet != nullptr)
				return occSet->Moved(objectPlacement, Logger());
			return geometryObject;
		}

		IXbimGeometryObject^ XbimGeometryCreatorV6::FromBrep(System::String^ brepStr)
		{
			TopoDS_Shape result;
			BRep_Builder builder;
			Standard_CString cStr = (const char*)(Marshal::StringToHGlobalAnsi(brepStr)).ToPointer();
			try
			{
				std::istringstream iss(cStr);
				BRepTools::Read(result, iss, builder);
				switch (result.ShapeType())
				{
				case TopAbs_VERTEX:
					return gcnew XbimVertex(TopoDS::Vertex(result));
				case TopAbs_EDGE:
					return gcnew XbimEdge(TopoDS::Edge(result));
				case TopAbs_WIRE:
					return gcnew XbimWire(TopoDS::Wire(result));
				case TopAbs_FACE:
					return gcnew XbimFace(TopoDS::Face(result));
				case TopAbs_SHELL:
					return gcnew XbimShell(TopoDS::Shell(result));
				case TopAbs_SOLID:
					return gcnew XbimSolid(TopoDS::Solid(result));
				case TopAbs_COMPOUND:
					return gcnew XbimCompound(TopoDS::Compound(result), true, 1e-5);
				default:
					return nullptr;
				}
			}
			catch (...)
			{
				return nullptr;
			}
			finally
			{
				Marshal::FreeHGlobal(System::IntPtr((void*)cStr));
			}
		}



		System::String^ XbimGeometryCreatorV6::ToBrep(IXbimGeometryObject^ geometryObject)
		{
			XbimGeometryObject^ geom = dynamic_cast<XbimGeometryObject^>(geometryObject);
			if (geom != nullptr)
				return geom->ToBRep;
			else
				return nullptr;
		}

		bool XbimGeometryCreatorV6::Is3D(IIfcCurve^ rep)
		{
			try
			{
				return ((int)rep->Dim == 3);
			}
			catch (System::Exception^)
			{
				return false; //in case the object has no points at all
			}
		}

		IXbimGeometryObject^ XbimGeometryCreatorV6::Trim(XbimSetObject^ geometryObject)
		{
			return geometryObject->Trim();
		}
#pragma endregion

#pragma region BRep Read and Write
		void XbimGeometryCreatorV6::WriteBrep(System::String^ fileName, IXbimGeometryObject^ geometryObject)
		{
			XbimOccWriter::Write(geometryObject, fileName);
		}

		IXbimGeometryObject^ XbimGeometryCreatorV6::ReadBrep(System::String^ filename)
		{
			Standard_CString fName = (const char*)(Marshal::StringToHGlobalAnsi(filename)).ToPointer();
			try
			{
				BRep_Builder builder;
				TopoDS_Shape result;
				BRepTools::Read(result, fName, builder);
				switch (result.ShapeType())
				{
				case TopAbs_VERTEX:
					return gcnew XbimVertex(TopoDS::Vertex(result));
				case TopAbs_EDGE:
					return gcnew XbimEdge(TopoDS::Edge(result));
				case TopAbs_WIRE:
					return gcnew XbimWire(TopoDS::Wire(result));
				case TopAbs_FACE:
					return gcnew XbimFace(TopoDS::Face(result));
				case TopAbs_SHELL:
					return gcnew XbimShell(TopoDS::Shell(result));
				case TopAbs_SOLID:
					return gcnew XbimSolid(TopoDS::Solid(result));
				case TopAbs_COMPOUND:
					return gcnew XbimCompound(TopoDS::Compound(result), true, 1e-5);
				default:
					return nullptr;
				}

			}
			catch (...)
			{
				return nullptr;
			}
			finally
			{
				Marshal::FreeHGlobal(System::IntPtr((void*)fName));
			}
		}
#pragma endregion


#pragma region Ifc Creation

		IIfcFacetedBrep^ XbimGeometryCreatorV6::CreateFacetedBrep(Xbim::Common::IModel^ model, IXbimSolid^ solid)
		{
			throw gcnew System::NotImplementedException();
			// TODO: insert return statement here
		}

#pragma endregion
	}
}