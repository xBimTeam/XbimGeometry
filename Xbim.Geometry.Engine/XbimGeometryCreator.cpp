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
using namespace Xbim::Common;
using namespace Xbim::XbimExtensions::Interfaces;
namespace Xbim
{
	namespace Geometry
	{
		
 
#pragma region Point Creation
		void XbimGeometryCreator::LogDebug(Object^ entity, String^ format, ...array<Object^>^ arg)
		{
			String^ msg = String::Format(format, arg);
			IPersistIfcEntity^ ifcEntity = dynamic_cast<IPersistIfcEntity^>(entity);
			if (ifcEntity != nullptr)
				logger->DebugFormat("GeomEngine: #{0}={1} [{2}]", ifcEntity->EntityLabel, ifcEntity->GetType()->Name, msg);
			else
				logger->DebugFormat("GeomEngine: {0} [{1}]", entity->GetType()->Name, msg);
		}

		void XbimGeometryCreator::LogWarning(Object^ entity, String^ format, ...array<Object^>^ arg)
		{
			String^ msg = String::Format(format, arg);
			IPersistIfcEntity^ ifcEntity = dynamic_cast<IPersistIfcEntity^>(entity);
			if (ifcEntity != nullptr)
				logger->WarnFormat("GeomEngine: #{0}={1} [{2}]", ifcEntity->EntityLabel, ifcEntity->GetType()->Name, msg);
			else
				logger->WarnFormat("GeomEngine: {0} [{1}]", entity->GetType()->Name, msg);
		}


		IXbimGeometryObject^ XbimGeometryCreator::Create(IfcGeometricRepresentationItem^ geomRep)
		{
			return Create(geomRep, nullptr);
		}


		IXbimGeometryObject^ XbimGeometryCreator::Create(IfcGeometricRepresentationItem^ geomRep, IfcAxis2Placement3D^ objectLocation)
		{
			try
			{
				IfcSweptAreaSolid^ sweptAreaSolid = dynamic_cast<IfcSweptAreaSolid^>(geomRep);
				if (sweptAreaSolid != nullptr)
				{
					if (dynamic_cast<IfcCompositeProfileDef^>(sweptAreaSolid->SweptArea)) //handle these as composite solids
					{
						XbimSolidSet^ solidset = (XbimSolidSet^)CreateSolidSet(sweptAreaSolid);
						if (objectLocation != nullptr) solidset->Move(objectLocation);
						return solidset;
					}
					else
					{
						XbimSolid^ solid = (XbimSolid^)CreateSolid((IfcSweptAreaSolid^)geomRep);
						if (objectLocation != nullptr) solid->Move(objectLocation);
						return solid;
					}
				}
				else if (dynamic_cast<IfcManifoldSolidBrep^>(geomRep))
				{
					XbimCompound^ comp = gcnew XbimCompound((IfcManifoldSolidBrep^)geomRep);
					if (objectLocation != nullptr) comp->Move(objectLocation);
					
					return comp;
				}				
				else if (dynamic_cast<IfcSweptDiskSolid^>(geomRep))
				{
					XbimSolid^ solid = (XbimSolid^)CreateSolid((IfcSweptDiskSolid^)geomRep);
					if (objectLocation != nullptr) solid->Move(objectLocation);
					return solid;
				}
				else if (dynamic_cast<IfcBooleanResult^>(geomRep))
				{
					XbimSolidSet^ solidSet = (XbimSolidSet^) CreateBooleanResult((IfcBooleanResult^)geomRep);
					//BRepTools::Write((XbimSolid^)(solidSet->First), "d:\\tmp\\s");
					if (objectLocation != nullptr) solidSet->Move(objectLocation);
					return solidSet;
				}				
				/*else if (dynamic_cast<IfcBooleanResult^>(geomRep))
				{
					XbimSolidSet^ solidSet = (XbimSolidSet^)CreateSolidSet((IfcBooleanResult^)geomRep);
					if (objectLocation != nullptr) solidSet->Move(objectLocation);
					return solidSet;
				}*/
				else if (dynamic_cast<IfcFaceBasedSurfaceModel^>(geomRep))
				{
					XbimCompound^ comp = (XbimCompound^)CreateSurfaceModel((IfcFaceBasedSurfaceModel^)geomRep);
					if (objectLocation != nullptr) comp->Move(objectLocation);
					return comp;
				} 
				else if (dynamic_cast<IfcShellBasedSurfaceModel^>(geomRep))
				{
					XbimCompound^ comp = (XbimCompound^)CreateSurfaceModel((IfcShellBasedSurfaceModel^)geomRep);
					if (objectLocation != nullptr) comp->Move(objectLocation);
					return comp;
				}
				else if (dynamic_cast<IfcHalfSpaceSolid ^>(geomRep))
				{
					XbimSolid^ solid = (XbimSolid^)CreateSolid((IfcHalfSpaceSolid^)geomRep);
					if (objectLocation != nullptr) solid->Move(objectLocation);
					return solid;
				}
				else if (dynamic_cast<IfcCurve^>(geomRep))
				{
					XbimWire^ wire = (XbimWire^)CreateWire((IfcCurve^)geomRep);
					if (objectLocation != nullptr) wire->Move(objectLocation);
					return wire;
				}	
				else if (dynamic_cast<IfcCompositeCurveSegment ^>(geomRep))
				{
					XbimWire^ wire = (XbimWire^)CreateWire((IfcCompositeCurveSegment^)geomRep);
					if (objectLocation != nullptr) wire->Move(objectLocation);
					return wire;
				}					
				else if (dynamic_cast<IfcBoundingBox^>(geomRep))
				{
					XbimSolid^ solid = (XbimSolid^)CreateSolid((IfcBoundingBox^)geomRep);
					if (objectLocation != nullptr) solid->Move(objectLocation);
					return solid;
				}
				else if (dynamic_cast<IfcSurface^>(geomRep))
				{
					XbimFace^ face = (XbimFace^)CreateFace((IfcSurface^)geomRep);
					if (objectLocation != nullptr) face->Move(objectLocation);
					return face;
				}				
				else if (dynamic_cast<IfcCsgSolid^>(geomRep))
				{
					XbimSolid^ solid = (XbimSolid^)CreateSolid((IfcCsgSolid^)geomRep);
					if (objectLocation != nullptr) solid->Move(objectLocation);
					return solid;
				}
				else if (dynamic_cast<IfcGeometricSet^>(geomRep))
				{
					if (objectLocation != nullptr) Logger->Error("Move is not implemented for IfcGeometricSet");
					return CreateGeometricSet((IfcGeometricSet^)geomRep);
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

		IXbimShapeGeometryData^ XbimGeometryCreator::CreateShapeGeometry(IXbimGeometryObject^ geometryObject, double precision, double deflection, double angle, XbimGeometryType storageType)
		{
			IXbimShapeGeometryData^ shapeGeom = gcnew XbimShapeGeometry();
			
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
					shapeGeom->ShapeData = memStream->ToArray();
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
				shapeGeom->ShapeData = memStream->ToArray();
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

		IXbimGeometryObjectSet^ XbimGeometryCreator::CreateGeometricSet(IfcGeometricSet^ geomSet)
		{
			XbimGeometryObjectSet^ result = gcnew XbimGeometryObjectSet(geomSet->Elements->Count);
			for each (IfcGeometricSetSelect^ elem in geomSet->Elements)
			{	
				if (dynamic_cast<IfcPoint^>(elem)) result->Add(CreatePoint((IfcPoint^)elem));
				else if (dynamic_cast<IfcCurve^>(elem)) result->Add(CreateWire((IfcCurve^)elem));
				else if (dynamic_cast<IfcSurface^>(elem)) result->Add(CreateFace((IfcSurface^)elem));
			}
			return result;
		}

		IXbimPoint^ XbimGeometryCreator::CreatePoint(double x, double y, double z, double tolerance)
		{
			return gcnew XbimPoint3DWithTolerance(x, y, z, tolerance);
		}

		IXbimPoint^ XbimGeometryCreator::CreatePoint(IfcCartesianPoint^ p)
		{
			return gcnew XbimPoint3DWithTolerance(p->XbimPoint3D(), p->ModelOf->ModelFactors->Precision);
		}
		
		IXbimPoint^ XbimGeometryCreator::CreatePoint(IfcPointOnCurve^ p)
		{
			return gcnew XbimPoint3DWithTolerance(p);
		}

		IXbimPoint^ XbimGeometryCreator::CreatePoint(IfcPointOnSurface^ p)
		{
			return gcnew XbimPoint3DWithTolerance(p);
		}

		IXbimPoint^ XbimGeometryCreator::CreatePoint(XbimPoint3D p, double tolerance)
		{
			return gcnew XbimPoint3DWithTolerance(p, tolerance);
		}

		IXbimPoint^ XbimGeometryCreator::CreatePoint(IfcPoint^ pt)
		{
			if (dynamic_cast<IfcCartesianPoint^>(pt)) return CreatePoint((IfcCartesianPoint^)pt);
			else if (dynamic_cast<IfcPointOnCurve^>(pt)) return CreatePoint((IfcPointOnCurve^)pt);
			else if (dynamic_cast<IfcPointOnSurface^>(pt)) return CreatePoint((IfcPointOnSurface^)pt);
			else throw gcnew NotImplementedException(String::Format("Geometry Representation of Type {0} in entity #{1} is not implemented", pt->GetType()->Name, pt->EntityLabel));

		}

#pragma endregion

#pragma region Wire Creation
		IXbimWire^ XbimGeometryCreator::CreateWire(IfcCurve^ curve)
		{
			return gcnew XbimWire(curve);
		}

		IXbimWire^ XbimGeometryCreator::CreateWire(IfcCompositeCurveSegment^ compCurveSeg)
		{
			return gcnew XbimWire(compCurveSeg);
		}
#pragma endregion



#pragma region Face creation
		IXbimFace^ XbimGeometryCreator::CreateFace(IXbimWire ^ wire)
		{
			return gcnew XbimFace(wire);
		};

		IXbimFace^ XbimGeometryCreator::CreateFace(IfcProfileDef ^ profile)
		{
			return gcnew XbimFace(profile);
		};

		IXbimFace^ XbimGeometryCreator::CreateFace(IfcCompositeCurve ^ cCurve)
		{
			return gcnew XbimFace(cCurve);
		};

		IXbimFace^ XbimGeometryCreator::CreateFace(IfcPolyline ^ pline)
		{
			return gcnew XbimFace(pline);
		};

		IXbimFace^ XbimGeometryCreator::CreateFace(IfcPolyLoop ^ loop)
		{
			return gcnew XbimFace(loop);
		};

		IXbimFace^ XbimGeometryCreator::CreateFace(IfcSurface ^ surface)
		{
			return gcnew XbimFace(surface);
		};

		IXbimFace^ XbimGeometryCreator::CreateFace(IfcPlane ^ plane)
		{
			return gcnew XbimFace(plane);
		};


#pragma endregion

		//Solid creation 
#pragma region Solids Creation
		

		/*IXbimSolid^ XbimGeometryCreator::CreateSolid(IfcGeometricRepresentationItem^ ifcSolid)
		{
			IfcSolidModel^ sm = dynamic_cast<IfcSolidModel^>(ifcSolid);
			if (sm != nullptr) return CreateSolid(sm);

			throw gcnew NotImplementedException(String::Format("Swept Solid of Type {0} in entity #{1} is not implemented", ifcSolid->GetType()->Name, ifcSolid->EntityLabel));

		}*/

		/*IXbimSolid^ XbimGeometryCreator::CreateSolid(IfcSolidModel^ solid)
		{
			IfcSweptAreaSolid^ extrudeArea = dynamic_cast<IfcSweptAreaSolid^>(solid);
			if (extrudeArea) return CreateSolid(extrudeArea);
			IfcManifoldSolidBrep^ ms = dynamic_cast<IfcManifoldSolidBrep^>(solid);
			if (ms != nullptr) return CreateSolidSet(ms);
			IfcSweptDiskSolid^ sd = dynamic_cast<IfcSweptDiskSolid^>(solid);
			if (sd != nullptr) return CreateSolid(sd);

			throw gcnew NotImplementedException(String::Format("Solid of Type {0} in entity #{1} is not implemented", solid->GetType()->Name, solid->EntityLabel));

		}*/
		IXbimSolid^ XbimGeometryCreator::CreateSolid(IfcSweptAreaSolid^ ifcSolid)
		{
			IfcExtrudedAreaSolid^ eas = dynamic_cast<IfcExtrudedAreaSolid^>(ifcSolid);
			if (eas != nullptr) return CreateSolid(eas);
			IfcRevolvedAreaSolid^ ras = dynamic_cast<IfcRevolvedAreaSolid^>(ifcSolid);
			if (ras != nullptr) return CreateSolid(ras);
			IfcSurfaceCurveSweptAreaSolid^ scas = dynamic_cast<IfcSurfaceCurveSweptAreaSolid^>(ifcSolid);
			if (scas != nullptr) return CreateSolid(scas);
			throw gcnew NotImplementedException(String::Format("Swept Solid of Type {0} in entity #{1} is not implemented", ifcSolid->GetType()->Name, ifcSolid->EntityLabel));
	
		};

		IXbimSolidSet^ XbimGeometryCreator::CreateSolidSet(IfcSweptAreaSolid^ ifcSolid)
		{
			IfcExtrudedAreaSolid^ eas = dynamic_cast<IfcExtrudedAreaSolid^>(ifcSolid);
			if (eas != nullptr) return CreateSolidSet(eas);
			IfcRevolvedAreaSolid^ ras = dynamic_cast<IfcRevolvedAreaSolid^>(ifcSolid);
			if (ras != nullptr) return CreateSolidSet(ras);
			IfcSurfaceCurveSweptAreaSolid^ scas = dynamic_cast<IfcSurfaceCurveSweptAreaSolid^>(ifcSolid);
			if (scas != nullptr) return CreateSolidSet(scas);
			throw gcnew NotImplementedException(String::Format("Swept Solid of Type {0} in entity #{1} is not implemented", ifcSolid->GetType()->Name, ifcSolid->EntityLabel));

		};
		IXbimSolidSet^ XbimGeometryCreator::CreateSolidSet(IfcExtrudedAreaSolid^ ifcSolid)
		{
			return gcnew XbimSolidSet(ifcSolid);
		};

		IXbimSolidSet^ XbimGeometryCreator::CreateSolidSet(IfcRevolvedAreaSolid^ ifcSolid)
		{
			return gcnew XbimSolidSet(ifcSolid);
		};

		IXbimSolidSet^ XbimGeometryCreator::CreateSolidSet(IfcSurfaceCurveSweptAreaSolid^ ifcSolid)
		{
			return gcnew XbimSolidSet(ifcSolid);
		};
		IXbimSolid^ XbimGeometryCreator::CreateSolid(IfcExtrudedAreaSolid^ ifcSolid)
		{
			return gcnew XbimSolid(ifcSolid);
		};

		IXbimSolid^ XbimGeometryCreator::CreateSolid(IfcRevolvedAreaSolid^ ifcSolid)
		{
			return gcnew XbimSolid(ifcSolid);
		};

		IXbimSolid^ XbimGeometryCreator::CreateSolid(IfcSweptDiskSolid^ ifcSolid)
		{
			return gcnew XbimSolid(ifcSolid);
		};

		IXbimSolid^ XbimGeometryCreator::CreateSolid(IfcBoundingBox^ ifcSolid)
		{
			return gcnew XbimSolid(ifcSolid);
		};

		IXbimSolid^ XbimGeometryCreator::CreateSolid(IfcSurfaceCurveSweptAreaSolid^ ifcSolid)
		{
			return gcnew XbimSolid(ifcSolid);
		};



		IXbimSolid^ XbimGeometryCreator::CreateSolid(IfcBooleanOperand^ ifcSolid)
		{
			return gcnew XbimSolid(ifcSolid);
		};
		
		IXbimSolid^ XbimGeometryCreator::CreateSolid(IfcBooleanClippingResult^ ifcSolid)
		{
			return gcnew XbimSolid(ifcSolid);
		};

		

		IXbimSolid^ XbimGeometryCreator::CreateSolid(IfcHalfSpaceSolid^ ifcSolid)
		{
			return gcnew XbimSolid(ifcSolid);
		};


		IXbimSolid^ XbimGeometryCreator::CreateSolid(IfcBoxedHalfSpace^ ifcSolid)
		{
			return gcnew XbimSolid(ifcSolid);
		};

		IXbimSolid^ XbimGeometryCreator::CreateSolid(IfcPolygonalBoundedHalfSpace^ ifcSolid)
		{
			return gcnew XbimSolid(ifcSolid);
		};

		IXbimSolidSet^ XbimGeometryCreator::CreateSolidSet(IfcManifoldSolidBrep^ ifcSolid)
		{
			return gcnew XbimSolidSet(ifcSolid);
		};

		IXbimSolidSet^ XbimGeometryCreator::CreateSolidSet(IfcFacetedBrep^ ifcSolid)
		{
			return gcnew XbimSolidSet(ifcSolid);
		};

		IXbimSolidSet^ XbimGeometryCreator::CreateSolidSet(IfcFacetedBrepWithVoids^ ifcSolid)
		{
			return gcnew XbimSolidSet(ifcSolid);
		};

		IXbimSolidSet^ XbimGeometryCreator::CreateSolidSet(IfcClosedShell^ ifcSolid)
		{
			return gcnew XbimSolidSet(ifcSolid);
		};

		IXbimSolid^ XbimGeometryCreator::CreateSolid(IfcCsgPrimitive3D^ ifcSolid)
		{
			return gcnew XbimSolid(ifcSolid);
		};

		IXbimSolid^ XbimGeometryCreator::CreateSolid(IfcCsgSolid^ ifcSolid)
		{
			return gcnew XbimSolid(ifcSolid);
		};

		IXbimSolid^ XbimGeometryCreator::CreateSolid(IfcSphere^ ifcSolid)
		{
			return gcnew XbimSolid(ifcSolid);
		};

		IXbimSolid^ XbimGeometryCreator::CreateSolid(IfcBlock^ ifcSolid)
		{
			return gcnew XbimSolid(ifcSolid);
		};

		IXbimSolid^ XbimGeometryCreator::CreateSolid(IfcRightCircularCylinder^ ifcSolid)
		{
			return gcnew XbimSolid(ifcSolid);
		};

		IXbimSolid^ XbimGeometryCreator::CreateSolid(IfcRightCircularCone^ ifcSolid)
		{
			return gcnew XbimSolid(ifcSolid);
		};

		IXbimSolid^ XbimGeometryCreator::CreateSolid(IfcRectangularPyramid^ ifcSolid)
		{
			return gcnew XbimSolid(ifcSolid);
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

		IXbimSolid^ XbimGeometryCreator::CreateFacetedSolid(IfcBooleanClippingResult^ ifcSolid)
		{
			return gcnew XbimFacetedSolid(ifcSolid);
		};
#endif // USE_CARVE_CSG


		//Surface Models containing one or more shells or solids
		IXbimGeometryObjectSet^ XbimGeometryCreator::CreateSurfaceModel(IfcShellBasedSurfaceModel^ ifcSurface)
		{
			return gcnew XbimCompound(ifcSurface);
		};

		IXbimGeometryObjectSet^ XbimGeometryCreator::CreateSurfaceModel(IfcFaceBasedSurfaceModel^ ifcSurface)
		{
			return gcnew XbimCompound(ifcSurface);
		};

		IXbimShell^  XbimGeometryCreator::CreateShell(IfcOpenShell^ shell)
		{
			return gcnew XbimShell(shell); 
		}

		IXbimShell^ XbimGeometryCreator::CreateShell(IfcConnectedFaceSet^ shell)
		{
			return gcnew XbimShell(shell);
		};

		IXbimShell^ XbimGeometryCreator::CreateShell(IfcSurfaceOfLinearExtrusion^ linExt)
		{
			return gcnew XbimShell(linExt);
		}

		IXbimSolidSet^ XbimGeometryCreator::CreateSolidSet(IfcBooleanResult^ boolOp)
		{
			return gcnew XbimSolidSet(boolOp);
		};

#pragma endregion

#pragma region IfcFacetedBrep Conversions

		IfcFacetedBrep^ XbimGeometryCreator::CreateFacetedBrep(XbimModel^ model, IXbimSolid^ solid)
		{	
			XbimSolid^ xSolid = dynamic_cast<XbimSolid^>(solid);
			XbimReadWriteTransaction^ txn = nullptr;
			if (!model->IsTransacting)
				txn = model->BeginTransaction();			
			IfcFacetedBrep^ bRep = model->Instances->New<IfcFacetedBrep^>();
			bRep->Outer = model->Instances->New<IfcClosedShell^>();
			IfcClosedShell^ cs = bRep->Outer;
	
			Monitor::Enter(xSolid);
			try
			{
				BRepMesh_IncrementalMesh incrementalMesh(xSolid, model->ModelFactors->DeflectionTolerance, Standard_False); //triangulate the first time				
			}
			finally
			{
				Monitor::Exit(xSolid);
			}
			Dictionary<IXbimPoint^, IfcCartesianPoint^>^ pointMap = gcnew Dictionary<IXbimPoint^, IfcCartesianPoint^>();
			for each (XbimFace^ face in xSolid->Faces)
			{
				TopLoc_Location loc;
				const Handle_Poly_Triangulation& mesh = BRep_Tool::Triangulation(face, loc);
				const TColgp_Array1OfPnt & nodes = mesh->Nodes();
			
				//If the face is planar we only need to write out the bounding edges
				if (face->IsPlanar)
				{
					IfcFace^ fc = model->Instances->New<IfcFace^>();
					IfcFaceOuterBound^ fo = model->Instances->New<IfcFaceOuterBound^>();
					fc->Bounds->Add(fo);
					IfcPolyLoop^ outerLoop = model->Instances->New<IfcPolyLoop^>();
					fo->Bound = outerLoop;
					for each (XbimEdge^ edge in face->OuterBound->Edges)
					{
						Handle_Poly_PolygonOnTriangulation edgeMesh = BRep_Tool::PolygonOnTriangulation(edge, mesh, loc);
						bool reverse = edge->IsReversed;
						int numNodes = edgeMesh->NbNodes(); //nb we skip the last node
						for (Standard_Integer i = reverse ? numNodes : 1; reverse ? i > 1:i < numNodes; reverse ? i-- : i++)
						{
							gp_XYZ p = nodes.Value(edgeMesh->Nodes().Value(i)).XYZ();
							loc.Transformation().Transforms(p);
							IfcCartesianPoint^ cp;
							IXbimPoint^ pt = CreatePoint(p.X(), p.Y(), p.Z(), model->ModelFactors->Precision);
							if (!pointMap->TryGetValue(pt, cp))
							{
								cp = model->Instances->New<IfcCartesianPoint^>();
								cp->SetXYZ(p.X(), p.Y(), p.Z());
								pointMap->Add(pt, cp);
							}
							outerLoop->Polygon->Add(cp);
						}
					}
					//now we have to do any inner bounds
					for each (XbimWire^ innerBound in face->InnerBounds)
					{
						
						IfcFaceBound^ fi = model->Instances->New<IfcFaceBound^>();
						fc->Bounds->Add(fi);
						IfcPolyLoop^ innerLoop = model->Instances->New<IfcPolyLoop^>();
						fi->Bound = innerLoop;
						for each (XbimEdge^ edge in innerBound->Edges)
						{
							Handle_Poly_PolygonOnTriangulation edgeMesh = BRep_Tool::PolygonOnTriangulation(edge, mesh, loc);
							bool reverse = edge->IsReversed;
							int numNodes = edgeMesh->NbNodes(); //nb we skip the last node
							for (Standard_Integer i = reverse ? numNodes : 1; reverse ? i > 1:i < numNodes; reverse ? i-- : i++)
							{
								gp_XYZ p = nodes.Value(edgeMesh->Nodes().Value(i)).XYZ();
								loc.Transformation().Transforms(p);
								IfcCartesianPoint^ cp;
								IXbimPoint^ pt = CreatePoint(p.X(), p.Y(), p.Z(), model->ModelFactors->Precision);
								if (!pointMap->TryGetValue(pt, cp))
								{
									cp = model->Instances->New<IfcCartesianPoint^>();
									cp->SetXYZ(p.X(), p.Y(), p.Z());
									pointMap->Add(pt, cp);
								}
								innerLoop->Polygon->Add(cp);
							}
						}
					}
					cs->CfsFaces->Add(fc);
				}
				else //it is a curved surface so we need to use the triangulation of the surface
				{
					const Poly_Array1OfTriangle& triangles = mesh->Triangles();
					Standard_Integer nbTriangles = mesh->NbTriangles();
					bool faceReversed = face->IsReversed;
					Standard_Integer t[3];
					for (Standard_Integer i = 1; i <= nbTriangles; i++) //add each triangle as a face
					{
						if (faceReversed) //get normals in the correct order of triangulation
							triangles.Value(i).Get(t[2], t[1], t[0]); 
						else
							triangles.Value(i).Get(t[0], t[1], t[2]);
						IfcFace^ fc = model->Instances->New<IfcFace^>();
						IfcFaceOuterBound^ fo = model->Instances->New<IfcFaceOuterBound^>();
						fc->Bounds->Add(fo);
						IfcPolyLoop^ outerLoop = model->Instances->New<IfcPolyLoop^>();
						fo->Bound = outerLoop;
						for (size_t j = 0; j < 3; j++)
						{
							gp_XYZ p = nodes.Value(t[j]).XYZ();
							loc.Transformation().Transforms(p);
							IfcCartesianPoint^ cp;
							IXbimPoint^ pt = CreatePoint(p.X(), p.Y(), p.Z(), model->ModelFactors->Precision);
							if (!pointMap->TryGetValue(pt, cp))
							{
								cp = model->Instances->New<IfcCartesianPoint^>();
								cp->SetXYZ(p.X(), p.Y(), p.Z());
								pointMap->Add(pt, cp);
							}
							outerLoop->Polygon->Add(cp);
						}
						cs->CfsFaces->Add(fc);
					}	
				}
			}
			if (txn!=nullptr) txn->Commit();
			return bRep;
		}


#pragma endregion

#pragma region Faceted Mesh creation

#ifdef USE_CARVE_CSG
		IXbimSolid^ XbimGeometryCreator::CreateFacetedSolid(IXbimSolid^ solid, double precision, double deflection)
		{
			return gcnew XbimFacetedSolid(solid, precision, deflection);
		}

		IXbimSolid^ XbimGeometryCreator::CreateFacetedSolid(IXbimSolid^ solid, double precision, double deflection, double angle)
		{
			return gcnew XbimFacetedSolid(solid, precision, deflection, angle);
		}

		IXbimSolid^ XbimGeometryCreator::CreateTriangulatedSolid(IXbimSolid^ solid, double precision, double deflection)
		{
			return gcnew XbimFacetedSolid(solid, precision, deflection, true);
		}

		IXbimSolid^ XbimGeometryCreator::CreateTriangulatedSolid(IXbimSolid^ solid, double precision, double deflection, double angle)
		{
			return gcnew XbimFacetedSolid(solid, precision, deflection, angle, true);
		}
#endif // USE_CARVE_CSG




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
			
#ifdef USE_CARVE_CSG
			XbimFacetedSolid^ fSolid = dynamic_cast<XbimFacetedSolid^>(shape);
			if (fSolid != nullptr)
			{
				fSolid->WriteTriangulation(tw, tolerance, deflection, angle);
				return;
			}
#endif // USE_CARVE_CSG


			XbimOccShape^ xShape = dynamic_cast<XbimOccShape^>(shape);
			if (xShape != nullptr)
			{
				xShape->WriteTriangulation(tw, tolerance, deflection, angle);
				return;
			}
			
		}

		void XbimGeometryCreator::WriteTriangulation(BinaryWriter^ bw, IXbimGeometryObject^ shape, double tolerance, double deflection, double angle)
		{

#ifdef USE_CARVE_CSG
			XbimFacetedSolid^ fSolid = dynamic_cast<XbimFacetedSolid^>(shape);
			if (fSolid != nullptr)
			{
				fSolid->WriteTriangulation(bw, tolerance, deflection, angle);
				return;
			}
#endif // USE_CARVE_CSG


			XbimOccShape^ xShape = dynamic_cast<XbimOccShape^>(shape);
			if (xShape != nullptr)
			{
				xShape->WriteTriangulation(bw, tolerance, deflection, angle);
				return;
			}

		}

#ifdef USE_CARVE_CSG
		IXbimGeometryObject^  XbimGeometryCreator::ReadTriangulation(TextReader^ sr)
		{
			return ReadTriangulation(sr, false);
		}

		IXbimGeometryObject^  XbimGeometryCreator::ReadTriangulation(TextReader^ sr, bool unTriangulate)
		{
			String^ l = sr->ReadLine();
			array<Char>^ space = gcnew array < Char > {' '};
			array<Char>^ comma = gcnew array < Char > {','};
			array<Char>^ slash = gcnew array < Char > {'/'};

			std::vector<vertex_t> vertices;
			std::vector<face_t*> meshFaces;

			while (l != nullptr)
			{
				array<String^>^ toks = l->Split(space, StringSplitOptions::RemoveEmptyEntries);
				if (toks->Length < 2) //skip if invalid line
					continue;
				String^ cmd = toks[0]->ToUpperInvariant();
				if (cmd == "T")
				{
					if (toks->Length > 1) //we have at least one triangle
					{
						for (int i = 1; i < toks->Length; i++) //skip the T
						{
							array<String^>^ indices = toks[i]->Split(comma, StringSplitOptions::RemoveEmptyEntries);
							array<String^>^ a = indices[0]->Split(slash, StringSplitOptions::RemoveEmptyEntries);
							array<String^>^ b = indices[1]->Split(slash, StringSplitOptions::RemoveEmptyEntries);
							array<String^>^ c = indices[2]->Split(slash, StringSplitOptions::RemoveEmptyEntries);

							size_t av = UInt32::Parse(a[0]);
							size_t bv = UInt32::Parse(b[0]);
							size_t cv = UInt32::Parse(c[0]);
							meshFaces.push_back(new face_t(&vertices[av], &vertices[bv], &vertices[cv]));
						}
					}
				}
				else if (cmd == "V")
				{
					for (int i = 1; i < toks->Length; i++)
					{
						array<String^>^ coords = toks[i]->Split(comma, StringSplitOptions::RemoveEmptyEntries);
						vertices.push_back(carve::geom::VECTOR(Double::Parse(coords[0]), Double::Parse(coords[1]), Double::Parse(coords[2])));
					}
				}
				else if (cmd == "N")
				{
					//ignore them
				}
				else if (cmd == "P") //initialise the polyData
				{
					String^ version = toks[1];
					int vCount = Int32::Parse(toks[2]);
					int fCount = Int32::Parse(toks[3]);
					int tCount = Int32::Parse(toks[4]);
					int nCount = Int32::Parse(toks[5]);
					vertices.reserve(vCount);
					meshFaces.reserve(tCount);
				}
				else if (cmd == "F") //initialise the face data
				{
					//do nothing
				}
				else
					Logger->WarnFormat("Illegal Polygon command format '{0}' has been ignored", cmd);
				l = sr->ReadLine(); //get the next line
			}
			std::vector<mesh_t*> newMeshes;
			mesh_t::create(meshFaces.begin(), meshFaces.end(), newMeshes, carve::mesh::MeshOptions());
			XbimFacetedSolid^ solid = gcnew XbimFacetedSolid(new meshset_t(vertices, newMeshes));
			if (unTriangulate)
				int reduced = solid->MergeCoPlanarFaces(0.4 * Math::PI / 180.0); //0.4 of a degree
			return solid;

		}
#endif // USE_CARVE_CSG

		IXbimSolidSet^ XbimGeometryCreator::CreateBooleanResult(IfcBooleanResult^ clip)
		{
			XbimModelFactors^ mf = clip->ModelOf->ModelFactors;
			
#ifdef OCC_6_9_SUPPORTED			
			
			List<IfcBooleanOperand^>^ clips = gcnew List<IfcBooleanOperand^>();
			
			IXbimSolidSet^ solidSet = gcnew XbimSolidSet();
			XbimSolid^ body = XbimSolid::BuildClippingList(clip, clips);
			double maxLen = body->BoundingBox.Length();
			
			for each (IfcBooleanOperand^ bOp in clips)
			{
				IfcHalfSpaceSolid^ hs = dynamic_cast<IfcHalfSpaceSolid^>(bOp);
				
				if (hs!=nullptr) //special case for IfcHalfSpaceSolid to keep extrusion to the minimum
				{
					XbimPoint3D^ ctrd = body->BoundingBox.Centroid();
					XbimSolid^ s = gcnew XbimSolid(hs, maxLen, ctrd);
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
			IfcBooleanOperand^ fOp = clip->FirstOperand;
			IfcBooleanOperand^ sOp = clip->SecondOperand;
			IXbimSolidSet^ left;
			IXbimSolidSet^ right;
			if (dynamic_cast<IfcBooleanResult^>(fOp))
				left = CreateBooleanResult((IfcBooleanResult^)fOp);
			else
			{
				left = gcnew XbimSolidSet(); 
				XbimSolid^ l = gcnew XbimSolid(fOp);
				if (l->IsValid)	left->Add(l);
			}
			if (dynamic_cast<IfcBooleanResult^>(sOp))
				right = CreateBooleanResult((IfcBooleanResult^)sOp);
			else
			{
				right = gcnew XbimSolidSet();
				XbimSolid^ r = gcnew XbimSolid(sOp);
				if (r->IsValid)	right->Add(r);
			}

			if (!left->IsValid)
			{
				//XbimGeometryCreator::logger->WarnFormat("WS006: IfcBooleanResult #{0} with invalid first operand", clip->EntityLabel);
				return XbimSolidSet::Empty;
			}

			if (!right->IsValid)
			{
				XbimGeometryCreator::logger->WarnFormat("WS007: IfcBooleanResult #{0} has an empty shape in the second operand", clip->EntityLabel);
				return left;
			}

			IXbimGeometryObject^ result;
			try
			{
				switch (clip->Operator)
				{
				case IfcBooleanOperator::Union:
					result = left->Union(right, mf->PrecisionBoolean);
					break;
				case IfcBooleanOperator::Intersection:
					result = left->Intersection(right, mf->PrecisionBoolean);
					break;
				case IfcBooleanOperator::Difference:
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

	}
}