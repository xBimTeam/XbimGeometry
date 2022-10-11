#include "MeshFactors.h"
using namespace System;
namespace Xbim
{
	namespace Geometry
	{
		namespace Services
		{
			IXMeshFactors^ Xbim::Geometry::Services::MeshFactors::SetGranularity(MeshGranularity granularity)
			{
				switch (granularity)
				{
				
				case Xbim::Geometry::Abstractions::MeshGranularity::Normal:
					LinearDefection = 30 * OneMeter / 1000; // 30 mm
					AngularDeflection = 0.523598776; //30 degrees is normal, a circle will have 360/(30/2)=24 segments
					break;
				case Xbim::Geometry::Abstractions::MeshGranularity::Fine:
					LinearDefection = 12 * OneMeter / 1000; // 12 mm;
					AngularDeflection = 0.26179938;//15 degrees is fine a circle will have 360/(15/2)=48 segments
					break;
				case Xbim::Geometry::Abstractions::MeshGranularity::Course:
				default:
					LinearDefection = 50 * OneMeter / 1000; // 50 mm
					AngularDeflection = 0.698132; //40 degrees is course, a small radius circle will have 360/(40/2)= 18 segments
					break;
				}
				return this;
			}
		}
	}
}