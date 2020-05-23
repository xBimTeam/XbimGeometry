//Geometric Processor Factory, builds  entities used for algebraic calculation etc from their Ifc counterparts
#pragma once
#include "../XbimHandle.h"
#include "Unmanaged/NGeomProcFactory.h"
#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Dir.hxx>
#include <gp_Dir2d.hxx>

#include "../Exceptions/XbimGeometryFactoryException.h"
#include <TColgp_SequenceOfPnt2d.hxx>

using namespace Xbim::Ifc4::Interfaces;
using namespace Xbim::Geometry::Exceptions;
using namespace Xbim::Geometry::Abstractions;
namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			public ref class GeomProcFactory : XbimHandle<NGeomProcFactory>
			{
			private:
				IXLoggingService^ LoggingService;
				IXModelService^ ModelService;

			public:
				GeomProcFactory(IXLoggingService^ loggingService, IXModelService^ modelService) : XbimHandle(new NGeomProcFactory())
				{
					LoggingService = loggingService;
					ModelService = modelService;
					NLoggingService* logService = new NLoggingService();
					logService->SetLogger(static_cast<WriteLog>(loggingService->LogDelegatePtr.ToPointer()));
					Ptr()->SetLogger(logService);
				}
				//builds a 3d point, if the Ifc point is 2d the Z coordinate is 0
				gp_Pnt BuildPoint(IIfcCartesianPoint^ ifcPoint);

				//builds a 2d point, if the Ifc point is 3d an exception is thrown
				gp_Pnt2d BuildPoint2d(IIfcCartesianPoint^ ifcPoint);

				//builds a 3d direction, if the Ifc Direction is 2d the Z component is 0
				gp_Dir BuildDirection(IIfcDirection^ ifcDir);

				//builds a 2d direction, if the Ifc Direction is 3d an exception is thrown
				//throws a XbimGeometryFactoryException if the normal of the vector is 0
				gp_Dir2d BuildDirection2d(IIfcDirection^ ifcDir);

				//builds a 3d vector, if the Ifc Vector is 2d the Z component is 0
				//throws a XbimGeometryFactoryException if the normal of the vector is 0
				gp_Vec BuildVector(IIfcVector^ ifcVec);

				//builds a 2d direction, if the Ifc Direction is 3d an exception is thrown
				gp_Vec2d BuildVector2d(IIfcVector^ ifcVec);

				gp_Ax2 BuildAxis2Placement(IIfcAxis2Placement3D^ axis2);
				gp_Ax2d BuildAxis2Placement2d(IIfcAxis2Placement2D^ axis);

				
			};
		}
	}
}
