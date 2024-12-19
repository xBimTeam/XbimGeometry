#pragma once
#include "FactoryBase.h"
#include <TopoDS_Shell.hxx>
#include "./Unmanaged/NShapeFactory.h"
#include "../Services/LoggingService.h"
using namespace Xbim::Geometry::Services;
using namespace Xbim::Common;
using namespace Xbim::Common::Geometry;
using namespace Xbim::Ifc4::Interfaces;
using namespace Xbim::Geometry::Abstractions;

namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{ 
			public ref class ShapeFactory : FactoryBase<NShapeFactory>, IXShapeFactory
			{
			private:				
				TopoDS_Shape Transform(TopoDS_Shape& shape, XbimMatrix3D matrix);
				int max_retry_attempts = 2;
				
				
			public:
				ShapeFactory(Xbim::Geometry::Services::ModelGeometryService^ modelService) : FactoryBase(modelService, new NShapeFactory(modelService->Timeout)) {}

				
				TopoDS_Shape NUnifyDomain(const TopoDS_Shape& toFix);
				TopoDS_Shape BuildClosedShell(IIfcClosedShell^ closedShell);

				static IXShape^ GetXbimShape(const TopoDS_Shape& shape);

				virtual IXShape^ Convert(System::String^ shape);
				/*virtual IXbimGeometryObject^ ConvertToV5(System::String^ brepStr);
				virtual IXbimGeometryObject^ ConvertToV5(IXShape^ shape);*/
				virtual System::String^ Convert(IXShape^ shape);
				virtual System::String^ Convert(IXbimGeometryObject^ shape);

				virtual IXShape^ UnifyDomain(IXShape^ toFix);

				virtual IXShape^ Build(IIfcGeometricRepresentationItem^ geomRep);

				virtual IXShape^ Transform(IXShape^ shape, XbimMatrix3D matrix);
				virtual IXShape^ Union(IXShape^ body, IXShape^ addition);
				virtual IXShape^ Cut(IXShape^ body, IXShape^ substraction);
				virtual IXShape^ Union(IXShape^ body, System::Collections::Generic::IEnumerable<IXShape^>^ addition);
				virtual IXShape^ Cut(IXShape^ body, System::Collections::Generic::IEnumerable<IXShape^>^ substraction);

				virtual IXShape^ RemovePlacement(IXShape^ shape);
				virtual IXShape^ SetPlacement(IXShape^ shape, IIfcObjectPlacement^ placement);


				virtual IXShape^ Moved(IXShape^ shape, IIfcObjectPlacement^ placement, bool invertPlacement);
				virtual IXShape^ Moved(IXShape^ shape, IXLocation^ moveTo);
				virtual IXFace^ Add(IXFace^ toFace, array<IXWire^>^ wires);

				virtual System::Collections::Generic::IEnumerable<IXFace^>^ FixFace(IXFace^ face);
				

				
			};
		}
	}

};
