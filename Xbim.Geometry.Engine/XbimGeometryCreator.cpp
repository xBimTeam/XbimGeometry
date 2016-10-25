// This is the main DLL file.

#include "XbimGeometryCreator.h"

#include "XbimFace.h"
#include "XbimSolid.h"
#include "XbimCompound.h"
#include "XbimSolidSet.h"
#include "XbimGeometryObjectSet.h"
#include "XbimCurve.h"
#include "XbimCurve2D.h"
#include "XbimConvert.h"
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
#include <BRepOffsetAPI_MakePipe.hxx>
#include <Geom_Circle.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <Geom_Plane.hxx>
#include <Geom_RectangularTrimmedSurface.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <gp_Lin2d.hxx>
#include <Geom2d_Line.hxx>
#include <IntAna2d_AnaIntersection.hxx>
using System::Runtime::InteropServices::Marshal;

using namespace  System::Threading;
using namespace  System::Linq;


namespace Xbim
{
	namespace Geometry
	{

		void XbimGeometryCreator::LogInfo(Object^ entity, String^ format, ...array<Object^>^ arg)
		{
			String^ msg = String::Format(format, arg);
			IPersistEntity^ ifcEntity = dynamic_cast<IPersistEntity^>(entity);
			if (ifcEntity!=nullptr)
				logger->InfoFormat("GeomEngine: #{0}={1} [{2}]", ifcEntity->EntityLabel, ifcEntity->GetType()->Name, msg);
			else
				logger->InfoFormat("GeomEngine: {0} [{1}]", entity->GetType()->Name, msg);
		}

		void XbimGeometryCreator::LogWarning(Object^ entity, String^ format, ...array<Object^>^ arg)
		{
			String^ msg = String::Format(format, arg);
			IPersistEntity^ ifcEntity = dynamic_cast<IPersistEntity^>(entity);
			if (ifcEntity != nullptr)
				logger->WarnFormat("GeomEngine: #{0}={1} [{2}]", ifcEntity->EntityLabel, ifcEntity->GetType()->Name, msg);
			else
				logger->WarnFormat("GeomEngine: {0} [{1}]", entity->GetType()->Name, msg);
		}

		void XbimGeometryCreator::LogDebug(Object^ entity, String^ format, ...array<Object^>^ arg)
		{
			String^ msg = String::Format(format, arg);
			IPersistEntity^ ifcEntity = dynamic_cast<IPersistEntity^>(entity);
			if (ifcEntity != nullptr)
				logger->DebugFormat("GeomEngine: #{0}={1} [{2}]", ifcEntity->EntityLabel, ifcEntity->GetType()->Name, msg);
			else
				logger->DebugFormat("GeomEngine: {0} [{1}]", entity->GetType()->Name, msg);
		}

		void XbimGeometryCreator::LogError(Object^ entity, String^ format, ...array<Object^>^ arg)
		{
			String^ msg = String::Format(format, arg);
			IPersistEntity^ ifcEntity = dynamic_cast<IPersistEntity^>(entity);
			if (ifcEntity != nullptr)
				logger->ErrorFormat("GeomEngine: #{0}={1} [{2}]", ifcEntity->EntityLabel, ifcEntity->GetType()->Name, msg);
			else
				logger->ErrorFormat("GeomEngine: {0} [{1}]", entity->GetType()->Name, msg);
		}

#pragma region  Creation
		
		void XbimGeometryCreator::Mesh(IXbimMeshReceiver^ mesh, IXbimGeometryObject^ geometryObject, double precision, double deflection, double angle)
		{			
			XbimSetObject^ objSet = dynamic_cast<XbimSetObject^>(geometryObject);
			XbimOccShape^ occObject = dynamic_cast<XbimOccShape^>(geometryObject);
			if(objSet!=nullptr)
				objSet->Mesh(mesh, precision, deflection, angle);
			else if (occObject != nullptr)
				occObject->Mesh(mesh, precision, deflection, angle);
			else
				throw gcnew Exception("Unsupported geometry type cannot be meshed");			
		}


		IXbimGeometryObject^ XbimGeometryCreator::Create(IIfcGeometricRepresentationItem^ geomRep)
		{
			return Create(geomRep, nullptr);
		}

		bool XbimGeometryCreator::Is3D(IIfcCurve^ rep)
		{
			return (rep->Dim == Xbim::Ifc4::GeometryResource::IfcDimensionCount(3));
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
						return Trim(solidset);
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
					return Trim(solidSet);
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
				else if (dynamic_cast<IIfcTriangulatedFaceSet^>(geomRep))
				{
					XbimCompound^ comp = (XbimCompound^)CreateSurfaceModel((IIfcTriangulatedFaceSet^)geomRep);
					if (objectLocation != nullptr) comp->Move(objectLocation);
					return comp;
				}
				else if (dynamic_cast<IIfcSectionedSpine^>(geomRep))
				{
					XbimSolid^ solid = (XbimSolid^)CreateSolid((IIfcSectionedSpine^)geomRep);
					if (objectLocation != nullptr) solid->Move(objectLocation);
					return solid;
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
					if (objectLocation != nullptr) LogError(geomRep, "Move is not implemented for IIfcGeometricSet");
					return CreateGeometricSet((IIfcGeometricSet^)geomRep);
				}
			}
			catch (Standard_Failure e)
			{
				String^ err = gcnew String(Standard_Failure::Caught()->GetMessageString());
				LogError(geomRep, "Error creating geometry representation of type {0}, {1}", geomRep->GetType()->Name, err);
				return XbimGeometryObjectSet::Empty;
			}
			LogError(geomRep,"Geometry Representation of Type {0} is not implemented", geomRep->GetType()->Name);
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
		//		const Handle(Poly_Triangulation)& mesh = BRep_Tool::Triangulation(face, loc);
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
		//				Handle(Poly_PolygonOnTriangulation) edgeMesh = BRep_Tool::PolygonOnTriangulation(edge, mesh, loc);
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
		//					Handle(Poly_PolygonOnTriangulation) edgeMesh = BRep_Tool::PolygonOnTriangulation(edge, mesh, loc);
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

		void XbimGeometryCreator::WriteTriangulation(IXbimMeshReceiver^ mesh, IXbimGeometryObject^ shape, double tolerance, double deflection, double angle)
		{

			XbimOccShape^ xShape = dynamic_cast<XbimOccShape^>(shape);
			if (xShape != nullptr)
			{
				xShape->WriteTriangulation(mesh, tolerance, deflection, angle);
				return;
			}

		}
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
			XbimPoint3D centroid = body->BoundingBox.Centroid();
			for each (IIfcBooleanOperand^ bOp in clips)
			{
				IIfcHalfSpaceSolid^ hs = dynamic_cast<IIfcHalfSpaceSolid^>(bOp);
				
				if (hs!=nullptr) //special case for IIfcHalfSpaceSolid to keep extrusion to the minimum
				{
					XbimSolid^ s = gcnew XbimSolid(hs, maxLen, centroid);
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
			double precision = mf->Precision;
		
			//if (solidSet->Count >0) //do large ops all in one go
			//{
				return body->Cut(solidSet, precision);
			//}
			//else
			//{
			//	IXbimSolidSet^ r = gcnew XbimSolidSet(body);
			//	for each (XbimSolid^ s in solidSet)	
			//		r = r->Cut(s, precision);

			//	//BRepTools::Write((XbimSolid^)(r->First), "d:\\tmp\\r");			
			//	return r;
			//}
			
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
				LogWarning(fOp, "Boolean result has an empty shape in the first operand");
				return XbimSolidSet::Empty;
			}

			if (!right->IsValid)
			{
				LogWarning(sOp, "Boolean result has an empty shape in the second operand");
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
				LogWarning(clip,"Error performing boolean operation, {0}. The second operand has been ignored", xbimE->Message);
				return left;
			}

			XbimSolidSet^ xbimSolidSet = dynamic_cast<XbimSolidSet^>(result);
			if (xbimSolidSet == nullptr)
			{
				LogWarning(clip, "Error performing boolean operation. The second operand has been ignored");
				return left;
			}
			else
				return xbimSolidSet;
		}

#pragma endregion
#pragma region Support for curves
		IXbimCurve^ XbimGeometryCreator::CreateCurve(IIfcCurve^ curve)
		{
			if (Is3D(curve))
				return gcnew XbimCurve(curve);
			else
				return gcnew XbimCurve2D(curve);
		}
		IXbimCurve^ XbimGeometryCreator::CreateCurve(IIfcPolyline^ curve)
		{
			if (Is3D(curve))
				return gcnew XbimCurve(curve);
			else
				return gcnew XbimCurve2D(curve);
		}
		IXbimCurve^ XbimGeometryCreator::CreateCurve(IIfcCircle^ curve)
		{
			if (Is3D(curve))
				return gcnew XbimCurve(curve);
			else
				return gcnew XbimCurve2D(curve);
		}
		IXbimCurve^ XbimGeometryCreator::CreateCurve(IIfcEllipse^ curve)
		{
			if (Is3D(curve))
				return gcnew XbimCurve(curve);
			else
				return gcnew XbimCurve2D(curve);
		}
		IXbimCurve^ XbimGeometryCreator::CreateCurve(IIfcLine^ curve)
		{
			if (Is3D(curve))
				return gcnew XbimCurve(curve);
			else
				return gcnew XbimCurve2D(curve);
		}
		IXbimCurve^ XbimGeometryCreator::CreateCurve(IIfcTrimmedCurve^ curve)
		{
			if(Is3D(curve))
				return gcnew XbimCurve(curve);
			else
				return gcnew XbimCurve2D(curve);
		}
		IXbimCurve^ XbimGeometryCreator::CreateCurve(IIfcRationalBSplineCurveWithKnots^ curve)
		{
			if (Is3D(curve))
				return gcnew XbimCurve(curve);
			else
				return gcnew XbimCurve2D(curve);
		}
		IXbimCurve^ XbimGeometryCreator::CreateCurve(IIfcBSplineCurveWithKnots^ curve)
		{
			if (Is3D(curve))
				return gcnew XbimCurve(curve);
			else
				return gcnew XbimCurve2D(curve);
		}

		IXbimCurve^ XbimGeometryCreator::CreateCurve(IIfcOffsetCurve3D^ curve)
		{			
			return gcnew XbimCurve(curve);		
		}

		IXbimCurve^ XbimGeometryCreator::CreateCurve(IIfcOffsetCurve2D^ curve)
		{		
			return gcnew XbimCurve2D(curve);
		}
#pragma endregion

		//Ifc4 interfaces
		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcSweptDiskSolidPolygonal^ ifcSolid)
		{
			return gcnew XbimSolid(ifcSolid);
		}
		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcRevolvedAreaSolidTapered^ ifcSolid)
		{
			return gcnew XbimSolid(ifcSolid);
		}
		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcFixedReferenceSweptAreaSolid^ ifcSolid)
		{
			return gcnew XbimSolid(ifcSolid);
		}
		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcAdvancedBrep^ ifcSolid)
		{
			XbimCompound^ comp = gcnew XbimCompound((IIfcAdvancedBrep^)ifcSolid);
			if (comp->Solids->Count > 0)
				return comp->Solids->First;
			else
				return gcnew XbimSolid();
		}
		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcAdvancedBrepWithVoids^ ifcSolid)
		{
			XbimCompound^ comp = gcnew XbimCompound((IIfcAdvancedBrepWithVoids^)ifcSolid);
			if (comp->Solids->Count > 0)
				return comp->Solids->First;
			else
				return gcnew XbimSolid();
		}
		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcSectionedSpine^ ifcSolid)
		{
			return gcnew XbimSolid(ifcSolid);
		}
		IXbimGeometryObjectSet^ XbimGeometryCreator::CreateSurfaceModel(IIfcTriangulatedFaceSet^ faceSet)
		{
			return gcnew XbimCompound(faceSet);
			
		}
		XbimMatrix3D XbimGeometryCreator::ToMatrix3D(IIfcObjectPlacement ^ objPlacement)
		{
			return XbimConvert::ConvertMatrix3D(objPlacement);
		}

		IXbimSolidSet^ XbimGeometryCreator::CreateGrid(IIfcGrid^ grid)
		{
			double mm = grid->Model->ModelFactors->OneMilliMeter;
			double precision = grid->Model->ModelFactors->Precision;
			XbimSolidSet^ solids = gcnew XbimSolidSet();
			gp_Pnt origin;
			gp_Vec normal;
			List<XbimCurve2D^>^ UCurves = gcnew List<XbimCurve2D^>();
			List<XbimCurve2D^>^ VCurves = gcnew List<XbimCurve2D^>();
			List<XbimCurve2D^>^ WCurves = gcnew List<XbimCurve2D^>();
			List<XbimPoint3D>^ intersections = gcnew List<XbimPoint3D>();

			for each (IIfcGridAxis^ axis in grid->UAxes)
			{
				XbimCurve2D^ c = gcnew XbimCurve2D(axis->AxisCurve);
				UCurves->Add(c);					
			}			
			for each (IIfcGridAxis^ axis in grid->VAxes)
			{				
				XbimCurve2D^ c = gcnew XbimCurve2D(axis->AxisCurve);
				VCurves->Add(c);
				for each (XbimCurve2D^ u in UCurves)
					intersections->AddRange(u->Intersections(c, precision));	
				
			}
			
			for each (IIfcGridAxis^ axis in grid->WAxes)
			{
				XbimCurve2D^ c = gcnew XbimCurve2D(axis->AxisCurve);
				WCurves->Add(c);
				for each (XbimCurve2D^ u in UCurves)
					intersections->AddRange(u->Intersections(c, precision));
				for each (XbimCurve2D^ v in VCurves)
					intersections->AddRange(v->Intersections(c, precision));
			}

			XbimRect3D bb = XbimRect3D::Empty;
			//calculate all the bounding box
			for each (XbimPoint3D pt in intersections)
			{
				if (bb.IsEmpty) bb = XbimRect3D(pt);
				else bb.Union(pt);
			}
			//the box should have a Z of zero so inflate it a bit
			bb = XbimRect3D::Inflate(bb, bb.SizeX*0.2, bb.SizeY*0.2, 0);

			//create a bounded planar 

			gp_Lin2d top(gp_Pnt2d(bb.X, bb.Y + bb.SizeY) , gp_Dir2d(1,0));
			gp_Lin2d bottom(gp_Pnt2d(bb.X, bb.Y), gp_Dir2d(1,0));
			gp_Lin2d left(gp_Pnt2d(bb.X, bb.Y), gp_Dir2d(0, 1));
			gp_Lin2d right(gp_Pnt2d(bb.X+bb.SizeX, bb.Y), gp_Dir2d(0, 1));
			
			IEnumerable<XbimCurve2D^>^ curves = Enumerable::Concat(Enumerable::Concat(UCurves, VCurves), WCurves);			
			for each (XbimCurve2D^ curve in curves)
			{
			    Handle(Geom2d_Curve) hcurve = curve;
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
						if(its.NbPoints() > 0) params->Add(its.Point(1).ParamOnFirst());
					}
					if (params->Count < 2)
					{
						its.Perform(line2d, right);
						if (its.NbPoints() > 0) params->Add(its.Point(1).ParamOnFirst());
					}
					if (params->Count != 2) continue;//give up
					hcurve = new Geom2d_TrimmedCurve(hcurve, Math::Min(params[0], params[1]), Math::Max(params[0], params[1]));
				}
				
				gp_Pnt2d origin;
				gp_Vec2d curveMainDir;
				
				hcurve->D1(hcurve->FirstParameter(), origin, curveMainDir); //get the start point and line direction
				gp_Vec2d curveTangent = curveMainDir.GetNormal();
				//gp_Dir v1 = gp::DX2d().IsParallel(normal, Precision::Angular()) ? gp::DY() : gp::DX();
				gp_Ax2 centre(gp_Pnt(origin.X(), origin.Y(), 0), gp_Vec(curveMainDir.X(), curveMainDir.Y(), 0), gp_Vec(curveTangent.X(), curveTangent.Y(), 0)); //create the axis for the rectangular face
				XbimWire^ xrect = gcnew XbimWire(75 * mm, mm / 10, precision, true);
				TopoDS_Wire rect = xrect;
				gp_Trsf trsf;
				trsf.SetTransformation(centre, gp_Ax3());
				rect.Move(TopLoc_Location(trsf));
				XbimFace^ profile = gcnew XbimFace(BRepBuilderAPI_MakeFace(rect, Standard_True));
				XbimCurve2D^ xCurve = gcnew XbimCurve2D(hcurve);
				XbimEdge^ edge = gcnew XbimEdge(xCurve);			
				TopoDS_Wire spine = BRepBuilderAPI_MakeWire(edge);
				BRepOffsetAPI_MakePipe axisMaker(spine, profile);
				axisMaker.Build();
				solids->Add(gcnew XbimSolid(TopoDS::Solid(axisMaker.Shape())));
			}
			return solids;
		}

		Xbim::Common::Geometry::IXbimGeometryObject ^ XbimGeometryCreator::Transformed(Xbim::Common::Geometry::IXbimGeometryObject ^geometryObject, Xbim::Ifc4::Interfaces::IIfcCartesianTransformationOperator ^transformation)
		{
			XbimOccShape^ occShape = dynamic_cast<XbimOccShape^>(geometryObject);
			if (occShape != nullptr)
				return occShape->Transformed(transformation);
			XbimSetObject^ occSet = dynamic_cast<XbimSetObject^>(geometryObject);
			if (occSet != nullptr)
				return occSet->Transformed(transformation);
			return geometryObject;//do nothing
		}

		Xbim::Common::Geometry::IXbimGeometryObject ^ XbimGeometryCreator::Moved(IXbimGeometryObject ^geometryObject, IIfcPlacement ^placement)
		{
			XbimOccShape^ occShape = dynamic_cast<XbimOccShape^>(geometryObject);
			if (occShape != nullptr)
				return occShape->Moved(placement);
			XbimSetObject^ occSet = dynamic_cast<XbimSetObject^>(geometryObject);
			if (occSet != nullptr)
				return occSet->Moved(placement);
			return geometryObject;
		}

		Xbim::Common::Geometry::IXbimGeometryObject ^ XbimGeometryCreator::Moved(IXbimGeometryObject ^geometryObject, IIfcObjectPlacement ^objectPlacement)
		{
			XbimOccShape^ occShape = dynamic_cast<XbimOccShape^>(geometryObject);
			if (occShape != nullptr)
				return occShape->Moved(objectPlacement);
			XbimSetObject^ occSet = dynamic_cast<XbimSetObject^>(geometryObject);
			if (occSet != nullptr)
				return occSet->Moved(objectPlacement);
			return geometryObject;
		}

		IXbimGeometryObject ^ XbimGeometryCreator::FromBrep(String ^ brepStr)
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
				Marshal::FreeHGlobal(IntPtr((void*)cStr));
			}
		}

		

		String ^ XbimGeometryCreator::ToBrep(IXbimGeometryObject ^ geometryObject)
		{
			XbimGeometryObject^ geom = dynamic_cast<XbimGeometryObject^>(geometryObject);
			if (geom != nullptr)
				return geom->ToBRep;
			else
				return nullptr;
		}

		IXbimGeometryObject ^ XbimGeometryCreator::Trim(XbimSetObject ^geometryObject)
		{						
			return geometryObject->Trim();
		}
	}
}