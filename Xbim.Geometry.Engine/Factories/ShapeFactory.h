#pragma once
#include "../XbimHandle.h"
#include <TopoDS_Shell.hxx>
#include "./Unmanaged/NShapeFactory.h"
#include "../Services/LoggingService.h"
using namespace Xbim::Geometry::Services;
using namespace Xbim::Common;
using namespace Xbim::Common::Geometry;
using namespace Xbim::Ifc4::Interfaces;
using namespace Xbim::Geometry::Abstractions;
using namespace System::Collections::Generic;

namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{ 
			public ref class ShapeFactory : XbimHandle<NShapeFactory>, IXShapeFactory
			{
			private:				
				IXModelService^ _modelService;			
				TopoDS_Shape Transform(TopoDS_Shape& shape, XbimMatrix3D matrix);
				array<System::Byte>^ CreateWexBimMesh(const TopoDS_Shape& topoShape, double tolerance, bool checkEdges, bool% hasCurves);
			public:
				ShapeFactory(ModelService^ modelService) : XbimHandle(new NShapeFactory(modelService->Timeout))
				{
					_modelService = modelService;
					NLoggingService* logService = new NLoggingService();
					logService->SetLogger(static_cast<WriteLog>(_modelService->LoggingService->LogDelegatePtr.ToPointer()));
					Ptr()->SetLogger(logService);				
				};

				
				TopoDS_Shape NUnifyDomain(const TopoDS_Shape& toFix);
				

				static IXShape^ GetXbimShape(const TopoDS_Shape& shape);

				virtual property IXModelService^ ModelService {IXModelService^ get() { return _modelService; }};
				virtual property IXLoggingService^ LoggingService {IXLoggingService^ get() { return _modelService->LoggingService; }};
				virtual IXShape^ Convert(System::String^ shape);
				virtual IXbimGeometryObject^ ConvertToV5(System::String^ brepStr);
				virtual IXbimGeometryObject^ ConvertToV5(IXShape^ shape);
				virtual System::String^ Convert(IXShape^ shape);
				virtual System::String^ Convert(IXbimGeometryObject^ shape);

				virtual IXShape^ UnifyDomain(IXShape^ toFix);

				virtual IXShape^ Build(IIfcGeometricRepresentationItem^ geomRep);

				virtual IXShape^ Transform(IXShape^ shape, XbimMatrix3D matrix);
				virtual IXShape^ Union(IXShape^ body, IXShape^ addition);
				virtual IXShape^ Cut(IXShape^ body, IXShape^ substraction);
				virtual IXShape^ Union(IXShape^ body, IEnumerable<IXShape^>^ addition);
				virtual IXShape^ Cut(IXShape^ body, IEnumerable<IXShape^>^ substraction);

				virtual IXShape^ RemovePlacement(IXShape^ shape);
				virtual IXShape^ SetPlacement(IXShape^ shape, IIfcObjectPlacement^ placement);


				virtual IXShape^ Moved(IXShape^ shape, IIfcObjectPlacement^ placement, bool invertPlacement);
				virtual IXShape^ Moved(IXShape^ shape, IXLocation^ moveTo);
				virtual IXFace^ Add(IXFace^ toFace, array<IXWire^>^ wires);

				virtual IEnumerable<IXFace^>^ FixFace(IXFace^ face);
				virtual array<System::Byte>^ CreateWexBimMesh(IXShape^ shape, IXMeshFactors^ meshFactors);
				virtual array<System::Byte>^ CreateWexBimMesh(IEnumerable<IXFace^>^ faces, IXMeshFactors^ meshFactors);
				virtual array<System::Byte>^ CreateWexBimMesh(IXShape^ shape, IXMeshFactors^ meshFactors, bool% hasCurves);
				virtual array<System::Byte>^ CreateWexBimMesh(IEnumerable<IXFace^>^ faces, IXMeshFactors^ meshFactors, bool% hasCurves);
				
			};
		}
	}

};
