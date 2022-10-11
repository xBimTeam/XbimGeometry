#pragma once

#include <IMeshTools_Parameters.hxx>
#include <math.h>
using namespace Xbim::Geometry::Abstractions;
namespace Xbim
{
	namespace Geometry
	{
		namespace Services
		{
			public ref class MeshFactors : IXMeshFactors
			{
			private:
				double _oneMeter ;
				double _deflection;
				double _angle;
				double _relative;
				double _internalVerticesMode;
				double _controlSurfaceDeflection;
				double _tolerance;

			public:
				MeshFactors(double oneMeter, double tolerance) : _oneMeter(oneMeter), _tolerance(tolerance)
				{
					SetGranularity(MeshGranularity::Normal);
				};
				virtual property double OneMeter {double get() { return _oneMeter; } void set(double meter) { _oneMeter = meter; }  }
				virtual property double LinearDefection {double get() { return _deflection; } void set(double linDef) { _deflection = linDef; }; }
				virtual property double AngularDeflection {double get() { return _angle; } void set(double angDef) { _angle = angDef; }; }
				virtual property bool Relative { bool get() { return _relative; }; void set(bool val) { _relative = val; }; }
				virtual property bool InternalVerticesMode { bool get() { return _internalVerticesMode; }; void set(bool val) { _internalVerticesMode = val; }; }
				virtual property bool ControlSurfaceDeflection { bool get() { return _controlSurfaceDeflection; }; void set(bool val) { _controlSurfaceDeflection = val; }; }
				virtual property double Tolerance {double get() { return _tolerance; } void set(double tolerance) { _tolerance = tolerance; }; }
				virtual IXMeshFactors^ SetGranularity(MeshGranularity granularity);
			};
		}
	}
}
