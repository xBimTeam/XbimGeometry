#pragma once
#include "../XbimHandle.h"
#include <gp_Trsf.hxx>

#include "XPoint.h"
#include "XQuaternion.h"


namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			
			using namespace System::Collections::Generic;
			using namespace System::Text::Json;
			using namespace System::Text::Json::Serialization;
			using namespace Xbim::Geometry::Abstractions;

			public ref class XLocation : XbimHandle<gp_Trsf>, IXLocation
			{
			private:

			public:
				///creates an empty location
				XLocation() : XbimHandle(new gp_Trsf()) {}
				XLocation(double tx, double ty, double tz, double sc, double qw, double qx, double qy, double qz): XbimHandle(new gp_Trsf())
				{
					Ref().SetScaleFactor(sc);
					Ref().SetRotationPart(gp_Quaternion(qx, qy, qz, qw));
					Ref().SetTranslationPart(gp_Vec(tx,ty,tz));
				}
				XLocation(const gp_Trsf& transform) : XbimHandle(new gp_Trsf(transform)) {}

				virtual property double Scale {double get() { return Ref().ScaleFactor(); }; }
				virtual property IXPoint^ Translation {IXPoint^ get() { return gcnew XPoint(Ref().TranslationPart()); }; }
				virtual property IXQuaternion^ Rotation {IXQuaternion^ get() { return gcnew Xbim::Geometry::BRep::XQuarternion(Ref().GetRotation()); }; }
				virtual IXLocation^ Multiplied(IXLocation^ location);
				
				virtual property bool IsIdentity {bool get() { return Ref().Form() == gp_TrsfForm::gp_Identity; }}
				virtual IXLocation^ Inverted() { return gcnew XLocation(Ref().Inverted()); }
				virtual IXLocation^ ScaledBy(double scale);
			};
		}
	}
}

