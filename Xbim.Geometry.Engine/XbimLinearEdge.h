#pragma once
#include "XbimPoint3DWithTolerance.h"

namespace Xbim
{
	namespace Geometry
	{
		ref class XbimLinearEdge : IXbimEdge
		{
		private:
			XbimPoint3DWithTolerance^ start;
			XbimPoint3DWithTolerance^ end;
			
		public:

#pragma region Constructors
			//XbimLinearEdge();
			XbimLinearEdge(XbimPoint3DWithTolerance^ start, XbimPoint3DWithTolerance^ end);
#pragma endregion
#pragma region destructors

			~XbimLinearEdge(){ }
			!XbimLinearEdge(){ }

#pragma endregion
#pragma region Equality Overrides
			virtual bool Equals(Object^ v) override;
			virtual int GetHashCode() override;
			static bool operator ==(XbimLinearEdge^ left, XbimLinearEdge^ right);
			static bool operator !=(XbimLinearEdge^ left, XbimLinearEdge^ right);
			virtual bool Equals(IXbimEdge^ e);
#pragma endregion

#pragma region IXbim Edge Interfaces
			virtual property bool IsValid{bool get() { return true; }; }
			virtual property bool IsSet{bool get() { return false; }; }
			virtual property  XbimGeometryObjectType GeometryType{XbimGeometryObjectType  get() { return XbimGeometryObjectType::XbimEdgeType; }; }
			virtual property IXbimVertex^ EdgeStart{IXbimVertex^ get(){ return start; }; }
			virtual property IXbimVertex^ EdgeEnd{IXbimVertex^ get(){ return end; }; }
			virtual property double Length{double get(); }
			virtual property IXbimCurve^ EdgeGeometry{IXbimCurve^ get(){ return nullptr; }; }
			virtual property XbimRect3D BoundingBox{XbimRect3D get(); }
			virtual IXbimGeometryObject^ Transform(XbimMatrix3D matrix3D) ;
			virtual IXbimGeometryObject^ TransformShallow(XbimMatrix3D matrix3D);
			virtual property String^  ToBRep{String^ get(); }
#pragma endregion
		};
	}
}

