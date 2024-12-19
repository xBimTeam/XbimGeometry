#pragma once
#include "./Unmanaged/NWexBimMeshFactory.h"
#include "FactoryBase.h" 
namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			public ref class WexBimMeshFactory : FactoryBase<NWexBimMeshFactory>, IXWexBimMeshFactory
			{
			public: 
				WexBimMeshFactory(Xbim::Geometry::Services::ModelGeometryService^ modelService) : FactoryBase(modelService, new NWexBimMeshFactory()) {}
				virtual array<System::Byte>^ CreateWexBimMesh(IXShape^ shape, IXAxisAlignedBoundingBox^% bounds);
				virtual array<System::Byte>^ CreateWexBimMesh(IXShape^ shape, IXMeshFactors^ meshFactors, double scale, IXAxisAlignedBoundingBox^% bounds);
				virtual array<System::Byte>^ CreateWexBimMesh(IXShape^ shape, double tolerance, double linearDeflection, double angularDeflection, double scale, IXAxisAlignedBoundingBox^% bounds);
				virtual array<System::Byte>^ CreateWexBimMesh(IXShape^ shape, IXAxisAlignedBoundingBox^% bounds, bool% hasCurves);
				virtual array<System::Byte>^ CreateWexBimMesh(IXShape^ shape, IXMeshFactors^ meshFactors, double scale, IXAxisAlignedBoundingBox^% bounds, bool% hasCurves);
				virtual array<System::Byte>^ CreateWexBimMesh(IXShape^ shape, double tolerance, double linearDeflection, double angularDeflection, double scale, IXAxisAlignedBoundingBox^% bounds, bool% hasCurves);

			private:
				array<System::Byte>^ createWexBimMesh(IXShape^ shape, IXMeshFactors^ meshFactors, double scale, IXAxisAlignedBoundingBox^% bounds, bool% hasCurvesbool, bool checkEdges, bool cleanBefore, bool cleanAfter);
				array<System::Byte>^ createWexBimMesh(IXShape^ shape, double tolerance, double linearDeflection, double angularDeflection, double scale, IXAxisAlignedBoundingBox^% bounds, bool% hasCurvesbool, bool checkEdges, bool cleanBefore, bool cleanAfter);
			};

		}
	}
}

