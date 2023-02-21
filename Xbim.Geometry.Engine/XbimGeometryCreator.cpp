// This is the main DLL file.

#include "XbimGeometryCreator.h"
#include <BRepMesh_IncrementalMesh.hxx>
#include <Poly_PolygonOnTriangulation.hxx>
#include <Poly_Triangulation.hxx>
#include <TopLoc_Location.hxx>
#include <BRep_Tool.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <BRepTools.hxx>
#include <BRep_Builder.hxx>
#include <BRepOffsetAPI_MakePipe.hxx>
#include <BRepOffsetAPI_MakePipeShell.hxx>
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
#include <GeomLib.hxx>
#include "XbimMesh.h"


#include "XbimFace.h"
#include "XbimSolid.h"
#include "XbimCompound.h"
#include "XbimSolidSet.h"
#include "XbimGeometryObjectSet.h"
#include "XbimCurve.h"
#include "XbimCurve2D.h"
#include "XbimConvert.h"
#include "XbimPoint3DWithTolerance.h"
#include "XbimOccWriter.h"

using namespace  System::Threading;

using namespace  System::IO;

namespace Xbim
{
	namespace Geometry
	{
#pragma warning( push )
#pragma warning( disable : 4691)

		void XbimGeometryCreator::WriteBrep(System::String^ fileName, IXbimGeometryObject^ geometryObject)
		{
			XbimOccWriter::Write(geometryObject, fileName);
		}

		IXbimGeometryObject^ XbimGeometryCreator::ReadBrep(System::String^ filename)
		{
			Standard_CString fName = (const char*)(System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(filename)).ToPointer();
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
				System::Runtime::InteropServices::Marshal::FreeHGlobal(System::IntPtr((void*)fName));
			}
		}

		XbimGeometryCreator::XbimGeometryCreator(IModel^ model, ILoggerFactory^ loggerFactory)
		{
			_logger = LoggerFactoryExtensions::CreateLogger<XbimGeometryCreator^>(loggerFactory);
			
			Dictionary<System::String^, System::Object^>^ scope = gcnew Dictionary<System::String^, System::Object^>();
			scope->Add("OriginatingSystem", model->Header->FileName->OriginatingSystem);
			scope->Add("CreatedBy", model->Header->CreatingApplication);
			scope->Add("IfcVersion", model->Header->SchemaVersion);
			_loggerScope = _logger->BeginScope(scope);
			
			_modelService = gcnew ModelGeometryService(model, loggerFactory);
			Xbim::Geometry::Abstractions::Extensions::IXModelExtensions::AddTagValue(model, "ModelGeometryService", _modelService);
		}



		void XbimGeometryCreator::LogInfo(ILogger^ logger, Object^ entity, System::String^ format, ...array<Object^>^ arg)
		{
			Log(logger, LogLevel::Information, entity, format, arg);
		}

		void XbimGeometryCreator::LogWarning(ILogger^ logger, Object^ entity, System::String^ format, ...array<Object^>^ arg)
		{
			Log(logger, LogLevel::Warning, entity, format, arg);

		}

		void XbimGeometryCreator::LogDebug(ILogger^ logger, Object^ entity, System::String^ format, ...array<Object^>^ arg)
		{
			Log(logger, LogLevel::Debug, entity, format, arg);
		}

		void XbimGeometryCreator::LogError(ILogger^ logger, Object^ entity, System::String^ format, ...array<Object^>^ arg)
		{
			Log(logger, LogLevel::Error, entity, format, arg);

		}

		void XbimGeometryCreator::Log(ILogger^ logger, LogLevel logLevel, Object^ entity, System::String^ format, ...array<Object^>^ args)
		{
			if (!logger->IsEnabled(logLevel))
				return;

			
			if (entity == nullptr)
			{
				LoggerExtensions::Log(logger, logLevel, "GeomEngine: - " + format, args);
			}
			else
			{
				IPersistEntity^ ifcEntity = dynamic_cast<IPersistEntity^>(entity);

				Dictionary<System::String^, System::Object^>^ scope = gcnew Dictionary<System::String^, System::Object^>();

				if (ifcEntity != nullptr)
				{
					scope->Add("ifcEntityLabel", ifcEntity->EntityLabel);
				}
				scope->Add("ifcType", entity->GetType()->Name);

				System::IDisposable^ logScope = logger->BeginScope(scope);
				try
				{
					LoggerExtensions::Log(logger, logLevel, "GeomEngine: - " + format, args);
				}
				finally
				{
					if (logScope != nullptr)
						delete logScope;
				}
			}
		}

#pragma warning( pop)

#pragma region  Creation

		void XbimGeometryCreator::Mesh(IXbimMeshReceiver^ mesh, IXbimGeometryObject^ geometryObject, double precision, double deflection, double angle)
		{
			XbimSetObject^ objSet = dynamic_cast<XbimSetObject^>(geometryObject);
			XbimOccShape^ occObject = dynamic_cast<XbimOccShape^>(geometryObject);
			if (objSet != nullptr)
				objSet->Mesh(mesh, precision, deflection, angle);
			else if (occObject != nullptr)
				occObject->Mesh(mesh, precision, deflection, angle);
			else
				throw gcnew System::Exception("Unsupported geometry type cannot be meshed");
		}


		IXbimGeometryObject^ XbimGeometryCreator::Create(IIfcGeometricRepresentationItem^ geomRep, ILogger^)
		{
			return Create(geomRep, nullptr, _logger);
		}

		bool XbimGeometryCreator::Is3D(IIfcCurve^ rep)
		{
			try
			{
				return (rep->Dim == Xbim::Ifc4::GeometryResource::IfcDimensionCount(3));
			}
			catch (System::Exception^)
			{
				return false; //in case the object has no points at all
			}

		}

		IXbimGeometryObject^ XbimGeometryCreator::Create(IIfcGeometricRepresentationItem^ geomRep, IIfcAxis2Placement3D^ objectLocation, ILogger^)
		{
			if (geomRep == nullptr)
			{
				LogError(_logger, geomRep, "Argument error: XbimGeometryCreator::Create,  Geometry Representation Item cannot be null");
				return nullptr;
			}
			try
			{
				IIfcSweptAreaSolid^ sweptAreaSolid = dynamic_cast<IIfcSweptAreaSolid^>(geomRep);
				if (sweptAreaSolid != nullptr)
				{
					if (dynamic_cast<IIfcCompositeProfileDef^>(sweptAreaSolid->SweptArea)) //handle these as composite solids
					{
						XbimSolidSet^ solidset = (XbimSolidSet^)CreateSolidSet(sweptAreaSolid, _logger);
						if (objectLocation != nullptr) solidset->Move(objectLocation);
						return Trim(solidset);
					}
					else
					{
						XbimSolid^ solid = (XbimSolid^)CreateSolid((IIfcSweptAreaSolid^)geomRep, _logger);
						if (objectLocation != nullptr) solid->Move(objectLocation);
						return solid;
					}
				}
				else if (dynamic_cast<IIfcManifoldSolidBrep^>(geomRep))
				{
					XbimCompound^ comp = gcnew XbimCompound((IIfcManifoldSolidBrep^)geomRep, _logger);
					if (objectLocation != nullptr) comp->Move(objectLocation);
					return comp;
				}
				else if (dynamic_cast<IIfcSweptDiskSolid^>(geomRep))
				{
					XbimSolid^ solid = (XbimSolid^)CreateSolid((IIfcSweptDiskSolid^)geomRep, _logger);
					if (objectLocation != nullptr) solid->Move(objectLocation);
					return solid;
				}
				else if (dynamic_cast<IIfcBooleanClippingResult^>(geomRep))
				{
					XbimSolidSet^ solidSet = gcnew XbimSolidSet((IIfcBooleanClippingResult^)geomRep, _modelService, _logger);
					if (objectLocation != nullptr) solidSet->Move(objectLocation);
					return Trim(solidSet);
				}
				else if (dynamic_cast<IIfcBooleanResult^>(geomRep))
				{
					XbimSolidSet^ solidSet = gcnew XbimSolidSet((IIfcBooleanResult^)geomRep, _modelService, _logger);
					if (objectLocation != nullptr) solidSet->Move(objectLocation);
					return Trim(solidSet);
				}
				else if (dynamic_cast<IIfcFaceBasedSurfaceModel^>(geomRep))
				{
					XbimCompound^ comp = (XbimCompound^)CreateSurfaceModel((IIfcFaceBasedSurfaceModel^)geomRep, _logger);
					if (objectLocation != nullptr) comp->Move(objectLocation);
					return comp;
				}
				else if (dynamic_cast<IIfcShellBasedSurfaceModel^>(geomRep))
				{
					XbimCompound^ comp = (XbimCompound^)CreateSurfaceModel((IIfcShellBasedSurfaceModel^)geomRep, _logger);
					if (objectLocation != nullptr) comp->Move(objectLocation);
					return comp;
				}
				else if (dynamic_cast<IIfcTriangulatedFaceSet^>(geomRep))
				{
					XbimCompound^ comp = (XbimCompound^)CreateSurfaceModel((IIfcTriangulatedFaceSet^)geomRep, _logger);
					if (objectLocation != nullptr) comp->Move(objectLocation);
					return comp;
				}
				else if (dynamic_cast<IIfcPolygonalFaceSet^>(geomRep))
				{
					IIfcPolygonalFaceSet^ polySet = (IIfcPolygonalFaceSet^)geomRep;
					if (polySet->Closed.HasValue && polySet->Closed.Value)
					{
						XbimSolidSet^ ss = (XbimSolidSet^)CreateSolidSet(polySet, _logger);
						if (objectLocation != nullptr) ss->Move(objectLocation);
						return ss;
					}
					else
					{
						XbimCompound^ comp = (XbimCompound^)CreateSurfaceModel((IIfcPolygonalFaceSet^)geomRep, _logger);
						if (objectLocation != nullptr) comp->Move(objectLocation);
						return comp;
					}
				}
				else if (dynamic_cast<IIfcSectionedSpine^>(geomRep))
				{
					XbimSolid^ solid = (XbimSolid^)CreateSolid((IIfcSectionedSpine^)geomRep, _logger);
					if (objectLocation != nullptr) solid->Move(objectLocation);
					return solid;
				}
				else if (dynamic_cast<IIfcHalfSpaceSolid^>(geomRep))
				{
					XbimSolid^ solid = (XbimSolid^)CreateSolid((IIfcHalfSpaceSolid^)geomRep, _logger);
					if (objectLocation != nullptr) solid->Move(objectLocation);
					return solid;
				}
				else if (dynamic_cast<IIfcCurve^>(geomRep))
				{
					XbimWire^ wire = (XbimWire^)CreateWire((IIfcCurve^)geomRep, _logger);
					if (objectLocation != nullptr) wire->Move(objectLocation);
					return wire;
				}
				else if (dynamic_cast<IIfcCompositeCurveSegment^>(geomRep))
				{
					XbimWire^ wire = (XbimWire^)CreateWire((IIfcCompositeCurveSegment^)geomRep, _logger);
					if (objectLocation != nullptr) wire->Move(objectLocation);
					return wire;
				}
				else if (dynamic_cast<IIfcBoundingBox^>(geomRep))
				{
					XbimSolid^ solid = (XbimSolid^)CreateSolid((IIfcBoundingBox^)geomRep, _logger);
					if (objectLocation != nullptr) solid->Move(objectLocation);
					return solid;
				}
				else if (dynamic_cast<IIfcSurface^>(geomRep))
				{
					XbimFace^ face = (XbimFace^)CreateFace((IIfcSurface^)geomRep, _logger);
					if (objectLocation != nullptr) face->Move(objectLocation);
					return face;
				}
				else if (dynamic_cast<IIfcCsgSolid^>(geomRep))
				{
					XbimSolidSet^ solidSet = (XbimSolidSet^)CreateSolidSet((IIfcCsgSolid^)geomRep, _logger);
					if (objectLocation != nullptr) solidSet->Move(objectLocation);
					return Trim(solidSet);
				}
				else if (dynamic_cast<IIfcSphere^>(geomRep))
				{
					XbimSolid^ solid = (XbimSolid^)CreateSolid((IIfcSphere^)geomRep, _logger);
					if (objectLocation != nullptr) solid->Move(objectLocation);
					return solid;
				}
				else if (dynamic_cast<IIfcGeometricSet^>(geomRep))
				{
					if (objectLocation != nullptr) LogError(_logger, geomRep, "Move is not implemented for IIfcGeometricSet");
					return CreateGeometricSet((IIfcGeometricSet^)geomRep, _logger);
				}
			}
			catch (const Standard_Failure& exc)
			{
				System::String^ err = gcnew System::String(exc.GetMessageString());
				LogError(_logger, geomRep, "Error creating geometry #{ifcEntityLabel} representation of type {ifcType}, {occError}", geomRep->GetType()->Name, err, geomRep->EntityLabel);
				return XbimGeometryObjectSet::Empty;
			}
			//catch ()
			catch (...)
			{
				throw gcnew System::Exception(System::String::Format("General Error Creating {ifcType}, #{ifcEntityLabel}", geomRep->GetType()->Name, geomRep->EntityLabel));
			}
			LogError(_logger, geomRep, "Geometry Representation of Type {ifcType} is not implemented", geomRep->GetType()->Name);
			return XbimGeometryObjectSet::Empty;
		}

		/*XbimMesh^ XbimGeometryCreator::CreateMeshGeometry(IXbimGeometryObject^ geometryObject, double precision, double deflection, double angle)
		{
			XbimShapeGeometry^ shapeGeom = CreateShapeGeometry(geometryObject, precision, deflection,angle, XbimGeometryType::PolyhedronBinary, nullptr);
			XbimMesh^ mesh = gcnew XbimMesh((((IXbimShapeGeometryData^)shapeGeom)->ShapeData));
			return mesh;
		}*/

		XbimShapeGeometry^ XbimGeometryCreator::CreateShapeGeometry(IXbimGeometryObject^ geometryObject, double precision, double deflection, double angle, XbimGeometryType storageType, ILogger^ /*logger*/)
		{
			XbimShapeGeometry^ shapeGeom = gcnew XbimShapeGeometry();

			if (geometryObject->IsSet)
			{
				System::Collections::Generic::IEnumerable<IXbimGeometryObject^>^ set = dynamic_cast<System::Collections::Generic::IEnumerable<IXbimGeometryObject^>^>(geometryObject);
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

		IXbimGeometryObjectSet^ XbimGeometryCreator::CreateGeometricSet(IIfcGeometricSet^ geomSet, ILogger^)
		{
			XbimGeometryObjectSet^ result = gcnew XbimGeometryObjectSet(System::Linq::Enumerable::Count(geomSet->Elements));
			for each (IIfcGeometricSetSelect ^ elem in geomSet->Elements)
			{
				if (dynamic_cast<IIfcPoint^>(elem)) result->Add(CreatePoint((IIfcPoint^)elem));
				else if (dynamic_cast<IIfcCurve^>(elem)) result->Add(CreateWire((IIfcCurve^)elem, _logger));
				else if (dynamic_cast<IIfcSurface^>(elem)) result->Add(CreateFace((IIfcSurface^)elem, _logger));
			}
			return result;
		}

		IXbimPoint^ XbimGeometryCreator::CreatePoint(double x, double y, double z, double tolerance)
		{
			return gcnew XbimPoint3DWithTolerance(x, y, z, tolerance);
		}

		IXbimPoint^ XbimGeometryCreator::CreatePoint(IIfcCartesianPoint^ p)
		{
			return gcnew XbimPoint3DWithTolerance(XbimPoint3D(p->X, p->Y, p->Z), p->Model->ModelFactors->Precision);
		}

		IXbimPoint^ XbimGeometryCreator::CreatePoint(IIfcPointOnCurve^ p, ILogger^)
		{
			return gcnew XbimPoint3DWithTolerance(p, _logger);
		}

		IXbimPoint^ XbimGeometryCreator::CreatePoint(IIfcPointOnSurface^ p, ILogger^)
		{
			return gcnew XbimPoint3DWithTolerance(p, _logger);
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
			else throw gcnew System::NotImplementedException(System::String::Format("Geometry Representation of Type {0} in entity #{1} is not implemented", pt->GetType()->Name, pt->EntityLabel));

		}

#pragma endregion

#pragma region Wire Creation
		IXbimWire^ XbimGeometryCreator::CreateWire(IIfcCurve^ curve, ILogger^)
		{
			IIfcPolyline^ pline = dynamic_cast<IIfcPolyline^>(curve);
			IIfcCompositeCurve^ composite = dynamic_cast<IIfcCompositeCurve^>(curve);
			IIfcIndexedPolyCurve^ poly = dynamic_cast<IIfcIndexedPolyCurve^>(curve);
			if (composite != nullptr)
				return gcnew XbimWire(composite, _logger);
			else if (poly != nullptr)
				return gcnew XbimWire(poly, _logger);
			else if (pline != nullptr)
				return gcnew XbimWire(pline, _logger);
			else
				return gcnew XbimWire(curve, _logger);
		}

		IXbimWire^ XbimGeometryCreator::CreateWire(IIfcCompositeCurveSegment^ compCurveSeg, ILogger^)
		{
			return gcnew XbimWire(compCurveSeg, _logger);
		}
#pragma endregion



#pragma region Face creation
		IXbimFace^ XbimGeometryCreator::CreateFace(IXbimWire^ wire, ILogger^)
		{
			XbimWire^ w = (XbimWire^)wire;
			return gcnew XbimFace(wire, wire->IsPlanar, w->MaxTolerance, 0, _logger);
		};

		IXbimFace^ XbimGeometryCreator::CreateFace(IIfcProfileDef^ profile, ILogger^)
		{
			return gcnew XbimFace(profile, _logger);
		};

		IXbimFace^ XbimGeometryCreator::CreateFace(IIfcCompositeCurve^ cCurve, ILogger^)
		{
			return gcnew XbimFace(cCurve, _logger);
		};

		IXbimFace^ XbimGeometryCreator::CreateFace(IIfcPolyline^ pline, ILogger^)
		{
			return gcnew XbimFace(pline, _logger);
		};

		IXbimFace^ XbimGeometryCreator::CreateFace(IIfcPolyLoop^ loop, ILogger^)
		{
			return gcnew XbimFace(loop, _logger);
		};

		IXbimFace^ XbimGeometryCreator::CreateFace(IIfcSurface^ surface, ILogger^)
		{
			return gcnew XbimFace(surface, _logger);
		};

		IXbimFace^ XbimGeometryCreator::CreateFace(IIfcPlane^ plane, ILogger^)
		{
			return gcnew XbimFace(plane, _logger);
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
		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcSweptAreaSolid^ IIfcSolid, ILogger^)
		{
			IIfcExtrudedAreaSolid^ eas = dynamic_cast<IIfcExtrudedAreaSolid^>(IIfcSolid);
			if (eas != nullptr) return CreateSolid(eas, _logger);
			IIfcRevolvedAreaSolid^ ras = dynamic_cast<IIfcRevolvedAreaSolid^>(IIfcSolid);
			if (ras != nullptr) return CreateSolid(ras, _logger);
			IIfcSurfaceCurveSweptAreaSolid^ scas = dynamic_cast<IIfcSurfaceCurveSweptAreaSolid^>(IIfcSolid);
			if (scas != nullptr) return CreateSolid(scas, _logger);
			throw gcnew System::NotImplementedException(System::String::Format("Swept Solid of Type {0} in entity #{1} is not implemented", IIfcSolid->GetType()->Name, IIfcSolid->EntityLabel));

		};

		IXbimSolidSet^ XbimGeometryCreator::CreateSolidSet(IIfcSweptAreaSolid^ IIfcSolid, ILogger^)
		{
			IIfcExtrudedAreaSolid^ eas = dynamic_cast<IIfcExtrudedAreaSolid^>(IIfcSolid);
			if (eas != nullptr) return CreateSolidSet(eas, _logger);
			IIfcRevolvedAreaSolid^ ras = dynamic_cast<IIfcRevolvedAreaSolid^>(IIfcSolid);
			if (ras != nullptr) return CreateSolidSet(ras, _logger);
			IIfcSurfaceCurveSweptAreaSolid^ scas = dynamic_cast<IIfcSurfaceCurveSweptAreaSolid^>(IIfcSolid);
			if (scas != nullptr) return CreateSolidSet(scas, _logger);
			throw gcnew System::NotImplementedException(System::String::Format("Swept Solid of Type {0} in entity #{1} is not implemented", IIfcSolid->GetType()->Name, IIfcSolid->EntityLabel));

		};
		IXbimSolidSet^ XbimGeometryCreator::CreateSolidSet(IIfcExtrudedAreaSolid^ IIfcSolid, ILogger^)
		{
			return gcnew XbimSolidSet(IIfcSolid, _logger);
		};

		IXbimSolidSet^ XbimGeometryCreator::CreateSolidSet(IIfcRevolvedAreaSolid^ IIfcSolid, ILogger^)
		{
			return gcnew XbimSolidSet(IIfcSolid, _logger);
		};

		IXbimSolidSet^ XbimGeometryCreator::CreateSolidSet(IIfcSurfaceCurveSweptAreaSolid^ IIfcSolid, ILogger^)
		{
			return gcnew XbimSolidSet(IIfcSolid, _logger);
		}
		IXbimSolidSet^ XbimGeometryCreator::CreateSolidSet(IIfcTriangulatedFaceSet^ shell, ILogger^)
		{
			return gcnew XbimSolidSet(shell, _logger);
		}
		IXbimSolidSet^ XbimGeometryCreator::CreateSolidSet(IIfcPolygonalFaceSet^ shell, ILogger^)
		{
			return gcnew XbimSolidSet(shell, _logger);
		}

		IXbimSolidSet^ XbimGeometryCreator::CreateSolidSet(IIfcShellBasedSurfaceModel^ ifcSurface, ILogger^)
		{
			return gcnew XbimSolidSet(ifcSurface, _logger);
		}
		IXbimSolidSet^ XbimGeometryCreator::CreateSolidSet(IIfcFaceBasedSurfaceModel^ ifcSurface, ILogger^)
		{
			return gcnew XbimSolidSet(ifcSurface, _logger);
		}
		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcTriangulatedFaceSet^ shell, ILogger^)
		{
			return gcnew XbimSolid(shell, _logger);
		}
		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcShellBasedSurfaceModel^ ifcSurface, ILogger^)
		{
			return gcnew XbimSolid(ifcSurface, _logger);
		}
		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcFaceBasedSurfaceModel^ ifcSurface, ILogger^)
		{
			return gcnew XbimSolid(ifcSurface, _logger);
		}

		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcExtrudedAreaSolid^ IIfcSolid, ILogger^)
		{
			return gcnew XbimSolid(IIfcSolid, _logger);
		};

		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcRevolvedAreaSolid^ IIfcSolid, ILogger^)
		{
			return gcnew XbimSolid(IIfcSolid, _logger);
		};

		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcSweptDiskSolid^ IIfcSolid, ILogger^)
		{
			return gcnew XbimSolid(IIfcSolid, _logger);
		};

		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcBoundingBox^ IIfcSolid, ILogger^)
		{
			return gcnew XbimSolid(IIfcSolid, _logger);
		};

		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcSurfaceCurveSweptAreaSolid^ IIfcSolid, ILogger^)
		{
			return gcnew XbimSolid(IIfcSolid, _logger);
		};

		[[deprecated("Please use CreateSolidSet")]]
		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcBooleanResult^ IIfcSolid, ILogger^)
		{
			XbimSolidSet^ ss = gcnew XbimSolidSet(IIfcSolid, _logger);
			return Enumerable::FirstOrDefault(ss);
		};

		IXbimSolidSet^ XbimGeometryCreator::CreateSolidSet(IIfcBooleanOperand^ IIfcSolid, ILogger^)
		{
			//ensure operands get treated correctly
			if (dynamic_cast<IIfcBooleanClippingResult^>(IIfcSolid))
				return gcnew XbimSolidSet((IIfcBooleanClippingResult^)IIfcSolid, _logger);
			else if (dynamic_cast<IIfcBooleanResult^>(IIfcSolid))
				return gcnew XbimSolidSet((IIfcBooleanResult^)IIfcSolid, _logger);
			else if (dynamic_cast<IIfcCsgSolid^>(IIfcSolid))
				return gcnew XbimSolidSet((IIfcCsgSolid^)IIfcSolid, _logger);
			else if (dynamic_cast<IIfcManifoldSolidBrep^>(IIfcSolid)) //these must be single volume solid
				return gcnew XbimSolidSet(gcnew XbimSolid((IIfcManifoldSolidBrep^)IIfcSolid, _logger));
			else if (dynamic_cast<IIfcSweptAreaSolid^>(IIfcSolid)) //these must be single volume solid
				return gcnew XbimSolidSet(gcnew XbimSolid((IIfcSweptAreaSolid^)IIfcSolid, _logger));
			else if (dynamic_cast<IIfcSweptDiskSolid^>(IIfcSolid))
				return gcnew XbimSolidSet(gcnew XbimSolid((IIfcSweptDiskSolid^)IIfcSolid, _logger));
			else if (dynamic_cast<IIfcHalfSpaceSolid^>(IIfcSolid))
				return gcnew  XbimSolidSet(gcnew XbimSolid((IIfcHalfSpaceSolid^)IIfcSolid, _logger));
			else if (dynamic_cast<IIfcCsgPrimitive3D^>(IIfcSolid))
				return gcnew XbimSolidSet(gcnew XbimSolid((IIfcCsgPrimitive3D^)IIfcSolid, _logger));
			throw gcnew System::Exception(System::String::Format("Boolean Operand with Type {0} is not implemented", IIfcSolid->GetType()->Name));
		};

		[[deprecated("Please use CreateSolidSet")]]
		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcBooleanClippingResult^ IIfcSolid, ILogger^)
		{
			return Enumerable::FirstOrDefault(gcnew XbimSolidSet(IIfcSolid, _logger));
		};

		IXbimSolidSet^ XbimGeometryCreator::CreateSolidSet(IIfcBooleanClippingResult^ IIfcSolid, ILogger^)
		{
			return gcnew XbimSolidSet(IIfcSolid, _logger);
		};


		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcHalfSpaceSolid^ IIfcSolid, ILogger^)
		{
			return gcnew XbimSolid(IIfcSolid, _logger);
		};


		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcBoxedHalfSpace^ IIfcSolid, ILogger^)
		{
			return gcnew XbimSolid(IIfcSolid, _logger);
		};

		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcPolygonalBoundedHalfSpace^ IIfcSolid, ILogger^)
		{
			return gcnew XbimSolid(IIfcSolid, _logger);
		};

		IXbimSolidSet^ XbimGeometryCreator::CreateSolidSet(IIfcManifoldSolidBrep^ IIfcSolid, ILogger^)
		{
			return gcnew XbimSolidSet(IIfcSolid, _logger);
		};

		IXbimSolidSet^ XbimGeometryCreator::CreateSolidSet(IIfcFacetedBrep^ IIfcSolid, ILogger^)
		{
			return gcnew XbimSolidSet(IIfcSolid, _logger);
		};

		IXbimSolidSet^ XbimGeometryCreator::CreateSolidSet(IIfcFacetedBrepWithVoids^ IIfcSolid, ILogger^)
		{
			return gcnew XbimSolidSet(IIfcSolid, _logger);
		};

		IXbimSolidSet^ XbimGeometryCreator::CreateSolidSet(IIfcClosedShell^ IIfcSolid, ILogger^)
		{
			return gcnew XbimSolidSet(IIfcSolid, _logger);
		};

		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcCsgPrimitive3D^ IIfcSolid, ILogger^)
		{
			return gcnew XbimSolid(IIfcSolid, _logger);
		};

		IXbimSolidSet^ XbimGeometryCreator::CreateSolidSet(IIfcCsgSolid^ IIfcSolid, ILogger^)
		{
			return gcnew XbimSolidSet(IIfcSolid, _logger);
		};

		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcSphere^ IIfcSolid, ILogger^)
		{
			return gcnew XbimSolid(IIfcSolid, _logger);
		};

		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcBlock^ IIfcSolid, ILogger^)
		{
			return gcnew XbimSolid(IIfcSolid, _logger);
		};

		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcRightCircularCylinder^ IIfcSolid, ILogger^)
		{
			return gcnew XbimSolid(IIfcSolid, _logger);
		};

		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcRightCircularCone^ IIfcSolid, ILogger^)
		{
			return gcnew XbimSolid(IIfcSolid, _logger);
		};

		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcRectangularPyramid^ IIfcSolid, ILogger^)
		{
			return gcnew XbimSolid(IIfcSolid, _logger);
		};




		//Surface Models containing one or more shells or solids
		IXbimGeometryObjectSet^ XbimGeometryCreator::CreateSurfaceModel(IIfcShellBasedSurfaceModel^ IIfcSurface, ILogger^)
		{
			return gcnew XbimCompound(IIfcSurface, _logger);
		};

		IXbimGeometryObjectSet^ XbimGeometryCreator::CreateSurfaceModel(IIfcFaceBasedSurfaceModel^ IIfcSurface, ILogger^)
		{
			return gcnew XbimCompound(IIfcSurface, _logger);
		};

		IXbimShell^ XbimGeometryCreator::CreateShell(IIfcOpenShell^ shell, ILogger^)
		{
			return gcnew XbimShell(shell, _logger);
		}

		IXbimShell^ XbimGeometryCreator::CreateShell(IIfcConnectedFaceSet^ shell, ILogger^)
		{
			return gcnew XbimShell(shell, _logger);
		};

		IXbimShell^ XbimGeometryCreator::CreateShell(IIfcSurfaceOfLinearExtrusion^ linExt, ILogger^)
		{
			return gcnew XbimShell(linExt, _logger);
		}

		IXbimSolidSet^ XbimGeometryCreator::CreateSolidSet(IIfcBooleanResult^ boolOp, ILogger^)
		{
			return gcnew XbimSolidSet(boolOp, _logger);
		};

#pragma endregion

#pragma region IIfcFacetedBrep Conversions

		IIfcFacetedBrep^ XbimGeometryCreator::CreateFacetedBrep(IModel^ /*model*/, IXbimSolid^ /*solid*/)
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
			return gcnew XbimSolidSet(_modelService);
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


#pragma endregion
#pragma region Support for curves
		IXbimCurve^ XbimGeometryCreator::CreateCurve(IIfcCurve^ curve, ILogger^)
		{
			//if (Is3D(curve))
			return gcnew XbimCurve(curve, _logger);
			//else
			//	return gcnew XbimCurve2D(curve, _logger);
		}
		IXbimCurve^ XbimGeometryCreator::CreateCurve(IIfcPolyline^ curve, ILogger^)
		{
			if (Is3D(curve))
				return gcnew XbimCurve(curve, _logger);
			else
				return gcnew XbimCurve2D(curve, _logger);
		}
		IXbimCurve^ XbimGeometryCreator::CreateCurve(IIfcCircle^ curve, ILogger^)
		{
			if (Is3D(curve))
				return gcnew XbimCurve(curve, _logger);
			else
				return gcnew XbimCurve2D(curve, _logger);
		}
		IXbimCurve^ XbimGeometryCreator::CreateCurve(IIfcEllipse^ curve, ILogger^)
		{
			if (Is3D(curve))
				return gcnew XbimCurve(curve, _logger);
			else
				return gcnew XbimCurve2D(curve, _logger);
		}
		IXbimCurve^ XbimGeometryCreator::CreateCurve(IIfcLine^ curve, ILogger^)
		{
			if (Is3D(curve))
				return gcnew XbimCurve(curve, _logger);
			else
				return gcnew XbimCurve2D(curve, _logger);
		}
		IXbimCurve^ XbimGeometryCreator::CreateCurve(IIfcTrimmedCurve^ curve, ILogger^)
		{
			if (Is3D(curve))
				return gcnew XbimCurve(curve, _logger);
			else
				return gcnew XbimCurve2D(curve, _logger);
		}
		IXbimCurve^ XbimGeometryCreator::CreateCurve(IIfcRationalBSplineCurveWithKnots^ curve, ILogger^)
		{
			if (Is3D(curve))
				return gcnew XbimCurve(curve, _logger);
			else
				return gcnew XbimCurve2D(curve, _logger);
		}
		IXbimCurve^ XbimGeometryCreator::CreateCurve(IIfcBSplineCurveWithKnots^ curve, ILogger^)
		{
			if (Is3D(curve))
				return gcnew XbimCurve(curve, _logger);
			else
				return gcnew XbimCurve2D(curve, _logger);
		}

		IXbimCurve^ XbimGeometryCreator::CreateCurve(IIfcOffsetCurve3D^ curve, ILogger^)
		{
			return gcnew XbimCurve(curve, _logger);
		}

		IXbimCurve^ XbimGeometryCreator::CreateCurve(IIfcOffsetCurve2D^ curve, ILogger^)
		{
			return gcnew XbimCurve2D(curve, _logger);
		}
#pragma endregion

		//Ifc4 interfaces
		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcSweptDiskSolidPolygonal^ ifcSolid, ILogger^)
		{
			return gcnew XbimSolid(ifcSolid, _logger);
		}
		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcRevolvedAreaSolidTapered^ ifcSolid, ILogger^)
		{
			return gcnew XbimSolid(ifcSolid, _logger);
		}
		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcFixedReferenceSweptAreaSolid^ ifcSolid, ILogger^)
		{
			return gcnew XbimSolid(ifcSolid, _logger);
		}
		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcAdvancedBrep^ ifcSolid, ILogger^)
		{
			XbimCompound^ comp = gcnew XbimCompound((IIfcAdvancedBrep^)ifcSolid, _logger);
			if (comp->Solids->Count > 0)
				return comp->Solids->First;
			else
				return gcnew XbimSolid();
		}
		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcAdvancedBrepWithVoids^ ifcSolid, ILogger^)
		{
			XbimCompound^ comp = gcnew XbimCompound((IIfcAdvancedBrepWithVoids^)ifcSolid, _logger);
			if (comp->Solids->Count > 0)
				return comp->Solids->First;
			else
				return gcnew XbimSolid();
		}
		IXbimSolid^ XbimGeometryCreator::CreateSolid(IIfcSectionedSpine^ ifcSolid, ILogger^)
		{
			return gcnew XbimSolid(ifcSolid, _logger);
		}

		IXbimGeometryObjectSet^ XbimGeometryCreator::CreateSurfaceModel(IIfcTessellatedFaceSet^ faceSet, ILogger^)
		{
			IIfcTriangulatedFaceSet^ tfs = dynamic_cast<IIfcTriangulatedFaceSet^>(faceSet);
			if (tfs != nullptr)  return gcnew XbimCompound(tfs, _logger);
			IIfcPolygonalFaceSet^ pfs = dynamic_cast<IIfcPolygonalFaceSet^>(faceSet);
			if (pfs != nullptr)
			{

				return gcnew XbimCompound(pfs, _logger);
			}
			throw gcnew System::Exception(System::String::Format("IIfcTessellatedFaceSet of Type {0} is not implemented", faceSet->GetType()->Name));
		}



		XbimMatrix3D XbimGeometryCreator::ToMatrix3D(IIfcObjectPlacement^ objPlacement, ILogger^)
		{
			return XbimConvert::ConvertMatrix3D(objPlacement, _logger);
		}

		IXbimSolidSet^ XbimGeometryCreator::CreateGrid(IIfcGrid^ grid, ILogger^)
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
				XbimCurve2D^ c = gcnew XbimCurve2D(axis->AxisCurve, _logger);
				if (c->IsValid)
				{
					System::Tuple<int, XbimCurve2D^>^ curveWithTag = System::Tuple::Create<int, XbimCurve2D^>(axis->EntityLabel, c);
					UCurves->Add(curveWithTag);
				}
			}
			for each (IIfcGridAxis ^ axis in grid->VAxes)
			{
				XbimCurve2D^ c = gcnew XbimCurve2D(axis->AxisCurve, _logger);
				if (c->IsValid)
				{
					System::Tuple<int, XbimCurve2D^>^ curveWithTag = System::Tuple::Create<int, XbimCurve2D^>(axis->EntityLabel, c);
					VCurves->Add(curveWithTag);
					for each (System::Tuple<int, XbimCurve2D^> ^ u in UCurves)
						intersections->AddRange(u->Item2->Intersections(c, precision, _logger));
				}
			}

			for each (IIfcGridAxis ^ axis in grid->WAxes)
			{
				XbimCurve2D^ c = gcnew XbimCurve2D(axis->AxisCurve, _logger);
				if (c->IsValid)
				{
					System::Tuple<int, XbimCurve2D^>^ curveWithTag = System::Tuple::Create<int, XbimCurve2D^>(axis->EntityLabel, c);
					WCurves->Add(curveWithTag);
					for each (System::Tuple<int, XbimCurve2D^> ^ u in UCurves)
						intersections->AddRange(u->Item2->Intersections(c, precision, _logger));
					for each (System::Tuple<int, XbimCurve2D^> ^ v in VCurves)
						intersections->AddRange(v->Item2->Intersections(c, precision, _logger));
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
				LogWarning(_logger, grid, "Extent of grid is near zero. Found "
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
			System::Collections::Generic::IEnumerable<System::Tuple<int, XbimCurve2D^>^>^ curves = Enumerable::Concat(Enumerable::Concat(UCurves, VCurves), WCurves);
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
					LogWarning(_logger, grid, "Grid axis #{ifcEntityLabel} caused exception. Status={geomStatus}, {occError}", curveWithTag->Item1.ToString(), gcnew System::Int32(pipeMakerStatus), err);
					failedGridLines = true;
				}
				catch (...)
				{
					LogWarning(_logger, grid, "Grid axis #{ifcEntityLabel} caused internal exception. Status={geomStatus}", curveWithTag->Item1.ToString(), gcnew System::Int32(pipeMakerStatus));
					failedGridLines = true;
				}

			}
			if (failedGridLines)
				LogWarning(_logger, grid, "One or more grid lines has failed to convert successfully");

			return  gcnew XbimSolidSet(solidResults);
		}

		Xbim::Common::Geometry::IXbimGeometryObject^ XbimGeometryCreator::Transformed(Xbim::Common::Geometry::IXbimGeometryObject^ geometryObject, Xbim::Ifc4::Interfaces::IIfcCartesianTransformationOperator^ transformation)
		{
			XbimOccShape^ occShape = dynamic_cast<XbimOccShape^>(geometryObject);
			if (occShape != nullptr)
				return occShape->Transformed(transformation);
			XbimSetObject^ occSet = dynamic_cast<XbimSetObject^>(geometryObject);
			if (occSet != nullptr)
				return occSet->Transformed(transformation);
			return geometryObject;//do nothing
		}

		Xbim::Common::Geometry::IXbimGeometryObject^ XbimGeometryCreator::Moved(IXbimGeometryObject^ geometryObject, IIfcPlacement^ placement)
		{
			XbimOccShape^ occShape = dynamic_cast<XbimOccShape^>(geometryObject);
			if (occShape != nullptr)
				return occShape->Moved(placement);
			XbimSetObject^ occSet = dynamic_cast<XbimSetObject^>(geometryObject);
			if (occSet != nullptr)
				return occSet->Moved(placement);
			return geometryObject;
		}

		Xbim::Common::Geometry::IXbimGeometryObject^ XbimGeometryCreator::Moved(IXbimGeometryObject^ geometryObject, IIfcObjectPlacement^ objectPlacement, ILogger^)
		{
			XbimOccShape^ occShape = dynamic_cast<XbimOccShape^>(geometryObject);
			if (occShape != nullptr)
				return occShape->Moved(objectPlacement, _logger);
			XbimSetObject^ occSet = dynamic_cast<XbimSetObject^>(geometryObject);
			if (occSet != nullptr)
				return occSet->Moved(objectPlacement, _logger);
			return geometryObject;
		}

		IXbimGeometryObject^ XbimGeometryCreator::FromBrep(System::String^ brepStr)
		{
			TopoDS_Shape result;
			BRep_Builder builder;
			Standard_CString cStr = (const char*)(System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(brepStr)).ToPointer();
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
				System::Runtime::InteropServices::Marshal::FreeHGlobal(System::IntPtr((void*)cStr));
			}
		}



		System::String^ XbimGeometryCreator::ToBrep(IXbimGeometryObject^ geometryObject)
		{
			XbimGeometryObject^ geom = dynamic_cast<XbimGeometryObject^>(geometryObject);
			if (geom != nullptr)
				return geom->ToBRep;
			else
				return nullptr;
		}

		IXbimGeometryObject^ XbimGeometryCreator::Trim(XbimSetObject^ geometryObject)
		{
			return geometryObject->Trim();
		}
	}
}