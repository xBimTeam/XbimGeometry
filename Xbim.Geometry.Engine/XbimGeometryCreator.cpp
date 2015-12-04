// This is the main DLL file.

#include "XbimGeometryCreator.h"
#include "XbimFace.h"
#include "XbimSolid.h"
#include "XbimCompound.h"
#include "XbimFacetedSolid.h"
#include "XbimSolidSet.h"
#include "XbimGeometryObjectSet.h"

#include "XbimPoint3DWithTolerance.h"
#include <BRepMesh_IncrementalMesh.hxx>
#include <Poly_PolygonOnTriangulation.hxx>
#include <Poly_Triangulation.hxx>
#include <TopLoc_Location.hxx>
#include <BRep_Tool.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <BRepTools.hxx>
#include <ShapeFix_ShapeTolerance.hxx>
#include <BRep_Builder.hxx>
using namespace  System::Threading;
using namespace  System::Linq;
using namespace Xbim::Common;
namespace Xbim
{
	namespace Geometry
	{
		
 
#pragma region Point Creation


		IXbimGeometryObject^ XbimGeometryCreator::Create(IIfcGeometricRepresentationItem^ geomRep)
		{
			return Create(geomRep, nullptr);
		}


		IXbimGeometryObject^ XbimGeometryCreator::Create(IIfcGeometricRepresentationItem^ geomRep, IIfcAxis2Placement3D^ objectLocation)
		{
			try
			{
				IIfcSweptAreaSolid^ sweptAreaSolid = dynamic_cast<IIfcSweptAreaSolid^>(geomRep);
				if (sweptAreaSolid != nullptr)
				{
					if (dynamic_cast<IIfcCompositeProfileDef^>(sweptAreaSolid->SweptArea)) //handle these as composite solids
					{
						XbimSolidSet^ solidset = (XbimSolidSet^)CreateSolidSet(sweptAreaSolid);
						if (objectLocation != nullptr) solidset->Move(objectLocation);
						return solidset;
					}
					else
					{
						XbimSolid^ solid = (XbimSolid^)CreateSolid((IIfcSweptAreaSolid^)geomRep);
						if (objectLocation != nullptr) solid->Move(objectLocation);
						return solid;
					}
				}
				else if (dynamic_cast<IIfcManifoldSolidBrep^>(geomRep))
				{
					XbimCompound^ comp = gcnew XbimCompound((IIfcManifoldSolidBrep^)geomRep);
					if (objectLocation != nullptr) comp->Move(objectLocation);
					
					return comp;
				}				
				else if (dynamic_cast<IIfcSweptDiskSolid^>(geomRep))
				{
					XbimSolid^ solid = (XbimSolid^)CreateSolid((IIfcSweptDiskSolid^)geomRep);
					if (objectLocation != nullptr) solid->Move(objectLocation);
					return solid;
				}
				else if (dynamic_cast<IIfcBooleanResult^>(geomRep))
				{
					XbimSolidSet^ solidSet = (XbimSolidSet^) CreateBooleanResult((IIfcBooleanResult^)geomRep);
					//BRepTools::Write((XbimSolid^)(solidSet->First), "d:\\tmp\\s");
					if (objectLocation != nullptr) solidSet->Move(objectLocation);
					return solidSet;
				}				
				/*else if (dynamic_cast<IIfcBooleanResult^>(geomRep))
				{
					XbimSolidSet^ solidSet = (XbimSolidSet^)CreateSolidSet((IIfcBooleanResult^)geomRep);
					if (objectLocation != nullptr) solidSet->Move(objectLocation);
					return solidSet;
				}*/
				else if (dynamic_cast<IIfcFaceBasedSurfaceModel^>(geomRep))
				{
					XbimCompound^ comp = (XbimCompound^)CreateSurfaceModel((IIfcFaceBasedSurfaceModel^)geomRep);
					if (objectLocation != nullptr) comp->Move(objectLocation);
					return comp;
				} 
				else if (dynamic_cast<IIfcShellBasedSurfaceModel^>(geomRep))
				{
					XbimCompound^ comp = (XbimCompound^)CreateSurfaceModel((IIfcShellBasedSurfaceModel^)geomRep);
					if (objectLocation != nullptr) comp->Move(objectLocation);
					return comp;
				}
				else if (dynamic_cast<IIfcHalfSpaceSolid ^>(geomRep))
				{
					XbimSolid^ solid = (XbimSolid^)CreateSolid((IIfcHalfSpaceSolid^)geomRep);
					if (objectLocation != nullptr) solid->Move(objectLocation);
					return solid;
				}
				else if (dynamic_cast<IIfcCurve^>(geomRep))
				{
					XbimWire^ wire = (XbimWire^)CreateWire((IIfcCurve^)geomRep);
					if (objectLocation != nullptr) wire->Move(objectLocation);
					return wire;
				}	
				else if (dynamic_cast<IIfcCompositeCurveSegment ^>(geomRep))
				{
					XbimWire^ wire = (XbimWire^)CreateWire((IIfcCompositeCurveSegment^)geomRep);
					if (objectLocation != nullptr) wire->Move(objectLocation);
					return wire;
				}					
				else if (dynamic_cast<IIfcBoundingBox^>(geomRep))
				{
					XbimSolid^ solid = (XbimSolid^)CreateSolid((IIfcBoundingBox^)geomRep);
					if (objectLocation != nullptr) solid->Move(objectLocation);
					return solid;
				}
				else if (dynamic_cast<IIfcSurface^>(geomRep))
				{
					XbimFace^ face = (XbimFace^)CreateFace((IIfcSurface^)geomRep);
					if (objectLocation != nullptr) face->Move(objectLocation);
					return face;
				}				
				else if (dynamic_cast<IIfcCsgSolid^>(geomRep))
				{
					XbimSolid^ solid = (XbimSolid^)CreateSolid((IIfcCsgSolid^)geomRep);
					if (objectLocation != nullptr) solid->Move(objectLocation);
					return solid;
				}
				else if (dynamic_cast<IIfcGeometricSet^>(geomRep))
				{
					if (objectLocation != nullptr) Logger->Error("Move is not implemented for IIfcGeometricSet");
					return CreateGeometricSet((IIfcGeometricSet^)geomRep);
				}
			}
			catch (...)
			{
				Logger->ErrorFormat("EG001: Unknown error creating geometry representation of type {0} in entity #{1}", geomRep->GetType()->Name, geomRep->EntityLabel);
				return XbimGeometryObjectSet::Empty;
			}
			Logger->ErrorFormat("EG002: Geometry Representation of Type {0} in entity #{1} is not implemented", geomRep->GetType()->Name, geomRep->EntityLabel);
			return XbimGeometryObjectSet::Empty;
		}

		XbimShapeGeometry^ XbimGeometryCreator::CreateShapeGeometry(IXbimGeometryObject^ geometryObject, double precision, double deflection, double angle, XbimGeometryType storageType)
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
						for each (IXbimGeometryObject^ geom in set)
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
						TextWriter^ tw = gcnew StreamWriter(memStream);
						for each (IXbimGeometryObject^ geom in set)
						{
							WriteTriangulation(tw, geom, precision, deflection, angle);
						}
						tw->Close();
						delete tw;
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
					TextWriter^ tw = gcnew StreamWriter(memStream);
					WriteTriangulation(tw, geometryObject, precision, deflection, angle);
					tw->Close();
					delete tw;
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

		IXbimGeometryObjectSet^ XbimGeometryCreator::CreateGeometricSet(IIfcGeometricSet^ geomSet)
		{
			XbimGeometryObjectSet^ result = gcnew XbimGeometryObjectSet(Enumerable::Count(geomSet->Elements));
			for each (IIfcGeometricSetSelect^ elem in geomSet->Elements)
			{	
				if (dynamic_cast<IIfcPoint^>(elem)) result->Add(CreatePoint((IIfcPoint^)elem));
				else if (dynamic_cast<IIfcCurve^>(elem)) result->Add(CreateWire((IIfcCurve^)elem));
				else if (dynamic_cast<IIfcSurface^>(elem)) result->Add(CreateFace((IIfcSurface^)elem));
			}
			return result;
		}

		IXbimPoint^ XbimGeometryCreator::CreatePoint(double x, double y, double z, double tolerance)
		{
			return gcnew XbimPoint3DWithTolerance(x, y, z, tolerance);
		}

		IXbimPoint^ XbimGeometryCreator::CreatePoint(IIfcCartesianPoint^ p)
		{
			return gcnew XbimPoint3DWithTolerance(XbimPoint3D(p->X,p->Y,p->Z), p->Model->ModelFactors->Precision);
		}
		
		IXbimPoint^ XbimGeometryCreator::CreatePoint(IIfcPointOnCurve^ p)
		{
			return gcnew XbimPoint3DWithTolerance(p);
		}

		IXbimPoint^ XbimGeometryCreator::CreatePoint(IIfcPointOnSurface^ p)
		{
			return gcnew XbimPoint3DWithTolerance(p);
		}

		IXbimPoint^ XbimGeometryCreator::CreatePoint(XbimPoint3D p, double tolerance)
		{
			return gcnew XbimPoint3DWithTolerance(p, tolerance);
		}

		IXbimPoint^ XbimGeometryCreator::CreatePoint(IIfcPoint^ pt)
		{
			if (dynamic_cast<IIfcCartesianPoint^>(pt)) return CreatePoint((IIfcCartesianPoint^)pt);
			else if (dynamic_cast<IIfcPointOnCurve^>(pt)) return CreatePoint((IIfcPointOnCurve^)pt);
			else if (dynamic_cast<IIfcPointOnSurface^>(pt)) return CreatePoint((IIfcPointOnSurface^)pt);
			else throw gcnew NotImplementedException(String::Format("Geometry Representation of Type {0} in entity #{1} is not implemented", pt->GetType()->Name, pt->EntityLabel));

		}

#pragma endregion

#pragma region Wire Creation
		IXbimWire^ XbimGeometryCreator::CreateWire(IIfcCurve^ curve)
		{
			return gcnew XbimWire(curve);
		}

		IXbimWire^ XbimGeometryCreator::CreateWire(IIfcCompositeCurveSegment^ compCurveSeg)
		{
			return gcnew XbimWire(compCurveSeg);
		}
#pragma endregion



#pragma region Face creation
		IXbimFace^ XbimGeometryCreator::CreateFace(IXbimWire ^ wire)
		{
			return gcnew XbimFace(wire);
		};

		IXbimFace^ XbimGeometryCreator::CreateFace(IIfcProfileDef ^ profile)
		{
			return gcnew XbimFace(profile);
		};

		IXbimFace^ XbimGeometryCreator::CreateFace(IIfcCompositeCurve ^ cCurve)
		{
			return gcnew XbimFace(cCurve);
		};

		IXbimFace^ XbimGeometryCreator::CreateFace(IIfcPolyline ^ pline)
		{
			return gcnew XbimFace(pline);
		};

		IXbimFace^ XbimGeometryCreator::CreateFace(IIfcPolyLoop ^ loop)
		{
			return gcnew XbimFace(loop);
		};

		IXbimFace^ XbimGeometryCreator::CreateFace(IIfcSurface ^ surface)
		{
			return gcnew XbimFace(surface);
		};

		IXbimFace^ XbimGeometryCreator::CreateFace(IIfcPlane ^ plane)
		{
			return gcnew XbimFace(plane);
		};


#pragma endregion

		//Solid creation 
#pragma region Solids Creation
		

		/*IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcGeometricRepresentationItem^ IIfcSolid)
		{
			IIfcSolidModel^ sm = dynamic_cast<IIfcSolidModel^>(IIfcSolid);
			if (sm != nullptr) return CreateSolid(sm);

			throw gcnew NotImplementedException(String::Format("Swept Solid of Type {0} in entity #{1} is not implemented", IIfcSolid->GetType()->Name, IIfcSolid->EntityLabel));

		}*/

		/*IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcSolidModel^ solid)
		{
			IIfcSweptAreaSolid^ extrudeArea = dynamic_cast<IIfcSweptAreaSolid^>(solid);
			if (extrudeArea) return CreateSolid(extrudeArea);
			IIfcManifoldSolidBrep^ ms = dynamic_cast<IIfcManifoldSolidBrep^>(solid);
			if (ms != nullptr) return CreateSolidSet(ms);
			IIfcSweptDiskSolid^ sd = dynamic_cast<IIfcSweptDiskSolid^>(solid);
			if (sd != nullptr) return CreateSolid(sd);

			throw gcnew NotImplementedException(String::Format("Solid of Type {0} in entity #{1} is not implemented", solid->GetType()->Name, solid->EntityLabel));

		}*/
		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcSweptAreaSolid^ IIfcSolid)
		{
			IIfcExtrudedAreaSolid^ eas = dynamic_cast<IIfcExtrudedAreaSolid^>(IIfcSolid);
			if (eas != nullptr) return CreateSolid(eas);
			IIfcRevolvedAreaSolid^ ras = dynamic_cast<IIfcRevolvedAreaSolid^>(IIfcSolid);
			if (ras != nullptr) return CreateSolid(ras);
			IIfcSurfaceCurveSweptAreaSolid^ scas = dynamic_cast<IIfcSurfaceCurveSweptAreaSolid^>(IIfcSolid);
			if (scas != nullptr) return CreateSolid(scas);
			throw gcnew NotImplementedException(String::Format("Swept Solid of Type {0} in entity #{1} is not implemented", IIfcSolid->GetType()->Name, IIfcSolid->EntityLabel));
	
		};

		IXbimSolidSet^ XbimGeometryCreator::CreateSolidSet(IIfcSweptAreaSolid^ IIfcSolid)
		{
			IIfcExtrudedAreaSolid^ eas = dynamic_cast<IIfcExtrudedAreaSolid^>(IIfcSolid);
			if (eas != nullptr) return CreateSolidSet(eas);
			IIfcRevolvedAreaSolid^ ras = dynamic_cast<IIfcRevolvedAreaSolid^>(IIfcSolid);
			if (ras != nullptr) return CreateSolidSet(ras);
			IIfcSurfaceCurveSweptAreaSolid^ scas = dynamic_cast<IIfcSurfaceCurveSweptAreaSolid^>(IIfcSolid);
			if (scas != nullptr) return CreateSolidSet(scas);
			throw gcnew NotImplementedException(String::Format("Swept Solid of Type {0} in entity #{1} is not implemented", IIfcSolid->GetType()->Name, IIfcSolid->EntityLabel));

		};
		IXbimSolidSet^ XbimGeometryCreator::CreateSolidSet(IIfcExtrudedAreaSolid^ IIfcSolid)
		{
			return gcnew XbimSolidSet(IIfcSolid);
		};

		IXbimSolidSet^ XbimGeometryCreator::CreateSolidSet(IIfcRevolvedAreaSolid^ IIfcSolid)
		{
			return gcnew XbimSolidSet(IIfcSolid);
		};

		IXbimSolidSet^ XbimGeometryCreator::CreateSolidSet(IIfcSurfaceCurveSweptAreaSolid^ IIfcSolid)
		{
			return gcnew XbimSolidSet(IIfcSolid);
		};
		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcExtrudedAreaSolid^ IIfcSolid)
		{
			return gcnew XbimSolid(IIfcSolid);
		};

		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcRevolvedAreaSolid^ IIfcSolid)
		{
			return gcnew XbimSolid(IIfcSolid);
		};

		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcSweptDiskSolid^ IIfcSolid)
		{
			return gcnew XbimSolid(IIfcSolid);
		};

		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcBoundingBox^ IIfcSolid)
		{
			return gcnew XbimSolid(IIfcSolid);
		};

		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcSurfaceCurveSweptAreaSolid^ IIfcSolid)
		{
			return gcnew XbimSolid(IIfcSolid);
		};



		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcBooleanOperand^ IIfcSolid)
		{
			return gcnew XbimSolid(IIfcSolid);
		};
		
		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcBooleanClippingResult^ IIfcSolid)
		{
			return gcnew XbimSolid(IIfcSolid);
		};

		

		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcHalfSpaceSolid^ IIfcSolid)
		{
			return gcnew XbimSolid(IIfcSolid);
		};


		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcBoxedHalfSpace^ IIfcSolid)
		{
			return gcnew XbimSolid(IIfcSolid);
		};

		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcPolygonalBoundedHalfSpace^ IIfcSolid)
		{
			return gcnew XbimSolid(IIfcSolid);
		};

		IXbimSolidSet^ XbimGeometryCreator::CreateSolidSet(IIfcManifoldSolidBrep^ IIfcSolid)
		{
			return gcnew XbimSolidSet(IIfcSolid);
		};

		IXbimSolidSet^ XbimGeometryCreator::CreateSolidSet(IIfcFacetedBrep^ IIfcSolid)
		{
			return gcnew XbimSolidSet(IIfcSolid);
		};

		IXbimSolidSet^ XbimGeometryCreator::CreateSolidSet(IIfcFacetedBrepWithVoids^ IIfcSolid)
		{
			return gcnew XbimSolidSet(IIfcSolid);
		};

		IXbimSolidSet^ XbimGeometryCreator::CreateSolidSet(IIfcClosedShell^ IIfcSolid)
		{
			return gcnew XbimSolidSet(IIfcSolid);
		};

		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcCsgPrimitive3D^ IIfcSolid)
		{
			return gcnew XbimSolid(IIfcSolid);
		};

		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcCsgSolid^ IIfcSolid)
		{
			return gcnew XbimSolid(IIfcSolid);
		};

		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcSphere^ IIfcSolid)
		{
			return gcnew XbimSolid(IIfcSolid);
		};

		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcBlock^ IIfcSolid)
		{
			return gcnew XbimSolid(IIfcSolid);
		};

		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcRightCircularCylinder^ IIfcSolid)
		{
			return gcnew XbimSolid(IIfcSolid);
		};

		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcRightCircularCone^ IIfcSolid)
		{
			return gcnew XbimSolid(IIfcSolid);
		};

		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcRectangularPyramid^ IIfcSolid)
		{
			return gcnew XbimSolid(IIfcSolid);
		};

#ifdef USE_CARVE_CSG

		IXbimSolid^ XbimGeometryCreator::CreateSolid(IXbimSolid^ from)
		{
			XbimFacetedSolid^ faceted = dynamic_cast<XbimFacetedSolid^>(from);
			if (faceted != nullptr) return faceted->ConvertToXbimSolid();
			XbimSolid^ solid = dynamic_cast<XbimSolid^>(from);
			if (faceted != nullptr) return solid;
			throw gcnew NotImplementedException(String::Format("Solid conversion of Type {0} is not implemented", from->GetType()->Name));

		}

		IXbimSolid^ XbimGeometryCreator::CreateFacetedSolid(IIfcBooleanClippingResult^ IIfcSolid)
		{
			return gcnew XbimFacetedSolid(IIfcSolid);
		};
#endif // USE_CARVE_CSG


		//Surface Models containing one or more shells or solids
		IXbimGeometryObjectSet^ XbimGeometryCreator::CreateSurfaceModel(IIfcShellBasedSurfaceModel^ IIfcSurface)
		{
			return gcnew XbimCompound(IIfcSurface);
		};

		IXbimGeometryObjectSet^ XbimGeometryCreator::CreateSurfaceModel(IIfcFaceBasedSurfaceModel^ IIfcSurface)
		{
			return gcnew XbimCompound(IIfcSurface);
		};

		IXbimShell^  XbimGeometryCreator::CreateShell(IIfcOpenShell^ shell)
		{
			return gcnew XbimShell(shell); 
		}

		IXbimShell^ XbimGeometryCreator::CreateShell(IIfcConnectedFaceSet^ shell)
		{
			return gcnew XbimShell(shell);
		};

		IXbimShell^ XbimGeometryCreator::CreateShell(IIfcSurfaceOfLinearExtrusion^ linExt)
		{
			return gcnew XbimShell(linExt);
		}

		IXbimSolidSet^ XbimGeometryCreator::CreateSolidSet(IIfcBooleanResult^ boolOp)
		{
			return gcnew XbimSolidSet(boolOp);
		};

#pragma endregion

#pragma region IIfcFacetedBrep Conversions

	IIfcFacetedBrep^ XbimGeometryCreator::CreateFacetedBrep(IModel^ model, IXbimSolid^ solid)
		{	
		//	XbimSolid^ xSolid = dynamic_cast<XbimSolid^>(solid);
		//	ITransaction^ txn = model->CurrentTransaction;
		//	if (txn == nullptr)
		//		txn = model->BeginTransaction("Add Brep");			
		//	IIfcFacetedBrep^ bRep = model->Instances->New<IIfcFacetedBrep^>();
		//	bRep->Outer = model->Instances->New<IIfcClosedShell^>();
		//	IIfcClosedShell^ cs = bRep->Outer;
	
		//	Monitor::Enter(xSolid);
		//	try
		//	{
		//		BRepMesh_IncrementalMesh incrementalMesh(xSolid, model->ModelFactors->DeflectionTolerance, Standard_False); //triangulate the first time				
		//	}
		//	finally
		//	{
		//		Monitor::Exit(xSolid);
		//	}
		//	Dictionary<IXbimPoint^, IIfcCartesianPoint^>^ pointMap = gcnew Dictionary<IXbimPoint^, IIfcCartesianPoint^>();
		//	for each (XbimFace^ face in xSolid->Faces)
		//	{
		//		TopLoc_Location loc;
		//		const Handle_Poly_Triangulation& mesh = BRep_Tool::Triangulation(face, loc);
		//		const TColgp_Array1OfPnt & nodes = mesh->Nodes();
		//	
		//		//If the face is planar we only need to write out the bounding edges
		//		if (face->IsPlanar)
		//		{
		//			IIfcFace^ fc = model->Instances->New<IIfcFace^>();
		//			IIfcFaceOuterBound^ fo = model->Instances->New<IIfcFaceOuterBound^>();
		//			fc->Bounds->Add(fo);
		//			IIfcPolyLoop^ outerLoop = model->Instances->New<IIfcPolyLoop^>();
		//			fo->Bound = outerLoop;
		//			for each (XbimEdge^ edge in face->OuterBound->Edges)
		//			{
		//				Handle_Poly_PolygonOnTriangulation edgeMesh = BRep_Tool::PolygonOnTriangulation(edge, mesh, loc);
		//				bool reverse = edge->IsReversed;
		//				int numNodes = edgeMesh->NbNodes(); //nb we skip the last node
		//				for (Standard_Integer i = reverse ? numNodes : 1; reverse ? i > 1:i < numNodes; reverse ? i-- : i++)
		//				{
		//					gp_XYZ p = nodes.Value(edgeMesh->Nodes().Value(i)).XYZ();
		//					loc.Transformation().Transforms(p);
		//					IIfcCartesianPoint^ cp;
		//					IXbimPoint^ pt = CreatePoint(p.X(), p.Y(), p.Z(), model->ModelFactors->Precision);
		//					if (!pointMap->TryGetValue(pt, cp))
		//					{
		//						cp = model->Instances->New<IIfcCartesianPoint^>();
		//						cp->SetXYZ(p.X(), p.Y(), p.Z());
		//						pointMap->Add(pt, cp);
		//					}
		//					outerLoop->Polygon->Add(cp);
		//				}
		//			}
		//			//now we have to do any inner bounds
		//			for each (XbimWire^ innerBound in face->InnerBounds)
		//			{
		//				
		//				IIfcFaceBound^ fi = model->Instances->New<IIfcFaceBound^>();
		//				fc->Bounds->Add(fi);
		//				IIfcPolyLoop^ innerLoop = model->Instances->New<IIfcPolyLoop^>();
		//				fi->Bound = innerLoop;
		//				for each (XbimEdge^ edge in innerBound->Edges)
		//				{
		//					Handle_Poly_PolygonOnTriangulation edgeMesh = BRep_Tool::PolygonOnTriangulation(edge, mesh, loc);
		//					bool reverse = edge->IsReversed;
		//					int numNodes = edgeMesh->NbNodes(); //nb we skip the last node
		//					for (Standard_Integer i = reverse ? numNodes : 1; reverse ? i > 1:i < numNodes; reverse ? i-- : i++)
		//					{
		//						gp_XYZ p = nodes.Value(edgeMesh->Nodes().Value(i)).XYZ();
		//						loc.Transformation().Transforms(p);
		//						IIfcCartesianPoint^ cp;
		//						IXbimPoint^ pt = CreatePoint(p.X(), p.Y(), p.Z(), model->ModelFactors->Precision);
		//						if (!pointMap->TryGetValue(pt, cp))
		//						{
		//							cp = model->Instances->New<IIfcCartesianPoint^>();
		//							cp->SetXYZ(p.X(), p.Y(), p.Z());
		//							pointMap->Add(pt, cp);
		//						}
		//						innerLoop->Polygon->Add(cp);
		//					}
		//				}
		//			}
		//			cs->CfsFaces->Add(fc);
		//		}
		//		else //it is a curved surface so we need to use the triangulation of the surface
		//		{
		//			const Poly_Array1OfTriangle& triangles = mesh->Triangles();
		//			Standard_Integer nbTriangles = mesh->NbTriangles();
		//			bool faceReversed = face->IsReversed;
		//			Standard_Integer t[3];
		//			for (Standard_Integer i = 1; i <= nbTriangles; i++) //add each triangle as a face
		//			{
		//				if (faceReversed) //get normals in the correct order of triangulation
		//					triangles.Value(i).Get(t[2], t[1], t[0]); 
		//				else
		//					triangles.Value(i).Get(t[0], t[1], t[2]);
		//				IIfcFace^ fc = model->Instances->New<IIfcFace^>();
		//				IIfcFaceOuterBound^ fo = model->Instances->New<IIfcFaceOuterBound^>();
		//				fc->Bounds->Add(fo);
		//				IIfcPolyLoop^ outerLoop = model->Instances->New<IIfcPolyLoop^>();
		//				fo->Bound = outerLoop;
		//				for (size_t j = 0; j < 3; j++)
		//				{
		//					gp_XYZ p = nodes.Value(t[j]).XYZ();
		//					loc.Transformation().Transforms(p);
		//					IIfcCartesianPoint^ cp;
		//					IXbimPoint^ pt = CreatePoint(p.X(), p.Y(), p.Z(), model->ModelFactors->Precision);
		//					if (!pointMap->TryGetValue(pt, cp))
		//					{
		//						cp = model->Instances->New<IIfcCartesianPoint^>();
		//						cp->SetXYZ(p.X(), p.Y(), p.Z());
		//						pointMap->Add(pt, cp);
		//					}
		//					outerLoop->Polygon->Add(cp);
		//				}
		//				cs->CfsFaces->Add(fc);
		//			}	
		//		}
		//	}
		//	if (txn!=nullptr) txn->Commit();
		//	return bRep;
			return nullptr;
		}


#pragma endregion



#pragma endregion


		IXbimSolidSet^ XbimGeometryCreator::CreateSolidSet() {
			return gcnew XbimSolidSet();
		};

		IXbimGeometryObjectSet^ XbimGeometryCreator::CreateGeometryObjectSet() {
			return gcnew XbimGeometryObjectSet();
		};

#pragma region Write Functions

		void XbimGeometryCreator::WriteTriangulation(TextWriter^ tw, IXbimGeometryObject^ shape, double tolerance, double deflection, double angle)
		{


			XbimOccShape^ xShape = dynamic_cast<XbimOccShape^>(shape);
			if (xShape != nullptr)
			{
				xShape->WriteTriangulation(tw, tolerance, deflection, angle);
				return;
			}
			
		}

		void XbimGeometryCreator::WriteTriangulation(BinaryWriter^ bw, IXbimGeometryObject^ shape, double tolerance, double deflection, double angle)
		{

			XbimOccShape^ xShape = dynamic_cast<XbimOccShape^>(shape);
			if (xShape != nullptr)
			{
				xShape->WriteTriangulation(bw, tolerance, deflection, angle);
				return;
			}

		}


		IXbimSolidSet^ XbimGeometryCreator::CreateBooleanResult(IIfcBooleanResult^ clip)
		{
			IModelFactors^ mf = clip->Model->ModelFactors;
			
#ifdef OCC_6_9_SUPPORTED			
			
			List<IIfcBooleanOperand^>^ clips = gcnew List<IIfcBooleanOperand^>();
			
			IXbimSolidSet^ solidSet = gcnew XbimSolidSet();
			XbimSolid^ body = XbimSolid::BuildClippingList(clip, clips);
			double maxLen = body->BoundingBox.Length();
			for each (IIfcBooleanOperand^ bOp in clips)
			{
				IIfcHalfSpaceSolid^ hs = dynamic_cast<IIfcHalfSpaceSolid^>(bOp);
				
				if (hs!=nullptr) //special case for IIfcHalfSpaceSolid to keep extrusion to the minimum
				{
					XbimSolid^ s = gcnew XbimSolid(hs, maxLen);
				    if (s->IsValid) solidSet->Add(s); 
				}
				else
				{
					XbimSolid^ s = gcnew XbimSolid(bOp);
					if (s->IsValid) solidSet->Add(s);
				}
			}
			//BRepTools::Write(body, "d:\\tmp\\b");
			ShapeFix_ShapeTolerance FTol;
			double precision = Math::Max(mf->OneMilliMeter/100, mf->Precision); //set the precision to 100th mm but never less than precision
				
			if (solidSet->Count > 5) //do large ops all in one go
			{
				return body->Cut(solidSet, precision);
			}
			else
			{
				IXbimSolidSet^ r = gcnew XbimSolidSet(body);
				for each (XbimSolid^ s in solidSet)	r = r->Cut(s, precision);

				//BRepTools::Write((XbimSolid^)(r->First), "d:\\tmp\\r");			
				return r;
			}
			
#endif
			IIfcBooleanOperand^ fOp = clip->FirstOperand;
			IIfcBooleanOperand^ sOp = clip->SecondOperand;
			IXbimSolidSet^ left;
			IXbimSolidSet^ right;
			if (dynamic_cast<IIfcBooleanResult^>(fOp))
				left = CreateBooleanResult((IIfcBooleanResult^)fOp);
			else
			{
				left = gcnew XbimSolidSet(); 
				XbimSolid^ l = gcnew XbimSolid(fOp);
				if (l->IsValid)	left->Add(l);
			}
			if (dynamic_cast<IIfcBooleanResult^>(sOp))
				right = CreateBooleanResult((IIfcBooleanResult^)sOp);
			else
			{
				right = gcnew XbimSolidSet();
				XbimSolid^ r = gcnew XbimSolid(sOp);
				if (r->IsValid)	right->Add(r);
			}

			if (!left->IsValid)
			{
				//XbimGeometryCreator::logger->WarnFormat("WS006: IIfcBooleanResult #{0} with invalid first operand", clip->EntityLabel);
				return XbimSolidSet::Empty;
			}

			if (!right->IsValid)
			{
				XbimGeometryCreator::logger->WarnFormat("WS007: IIfcBooleanResult #{0} has an empty shape in the second operand", clip->EntityLabel);
				return left;
			}

			IXbimGeometryObject^ result;
			try
			{
				switch (clip->Operator)
				{
				case IfcBooleanOperator::UNION:
					result = left->Union(right, mf->PrecisionBoolean);
					break;
				case IfcBooleanOperator::INTERSECTION:
					result = left->Intersection(right, mf->PrecisionBoolean);
					break;
				case IfcBooleanOperator::DIFFERENCE:
					result = left->Cut(right, mf->PrecisionBoolean);
					break;
				}
			}
			catch (Exception^ xbimE)
			{
				XbimGeometryCreator::logger->ErrorFormat("ES001: Error performing boolean operation for entity #{0}={1}\n{2}. The first operand has been used, the operation has been ignored", clip->EntityLabel, clip->GetType()->Name, xbimE->Message);
				return left;
			}

			XbimSolidSet^ xbimSolidSet = dynamic_cast<XbimSolidSet^>(result);
			if (xbimSolidSet == nullptr)
			{
				XbimGeometryCreator::logger->ErrorFormat("ES002: Error performing boolean operation for entity #{0}={1}. The first operand has been used, the operation has been ignored", clip->EntityLabel, clip->GetType()->Name);	
				return left;
			}
			else
				return xbimSolidSet;
		}
	
#pragma endregion
		//Ifc4 interfaces
		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcSweptDiskSolidPolygonal^ ifcSolid)
		{
			throw gcnew NotImplementedException();
		}
		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcRevolvedAreaSolidTapered^ ifcSolid)
		{
			throw gcnew NotImplementedException();
		}
		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcFixedReferenceSweptAreaSolid^ ifcSolid)
		{
			throw gcnew NotImplementedException();
		}
		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcAdvancedBrep^ ifcSolid)
		{
			throw gcnew NotImplementedException();
		}
		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcAdvancedBrepWithVoids^ ifcSolid)
		{
			throw gcnew NotImplementedException();
		}
		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcSectionedSpine^ ifcSolid)
		{
			throw gcnew NotImplementedException();
		}
		IXbimShell^ XbimGeometryCreator::CreateShell(IIfcTriangulatedFaceSet^ shell)
		{
			throw gcnew NotImplementedException();
		}

	}
}