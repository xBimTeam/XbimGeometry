#pragma once
#include "FactoryBase.h"
#include "./Unmanaged//NVertexFactory.h"

namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			public ref class VertexFactory : FactoryBase<NVertexFactory>, IXVertexFactory
			{

			public:
#pragma region Interface implementation
				virtual IXVertex^ Build(double x, double y, double z);
				
#pragma endregion



				VertexFactory(Xbim::Geometry::Services::ModelGeometryService^ modelService) : FactoryBase(modelService, new NVertexFactory()) {}
				TopoDS_Vertex Build(IIfcCartesianPoint^ ifcCartesianPoint);
				bool IsGeometricallySame(const TopoDS_Vertex& vertex1, const TopoDS_Vertex& vertex2);
			};

		}
	}
}