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

			public ref class XLocation : XbimHandle<gp_Trsf>, IXLocation, IXMatrix
			{
			private:

			public:
				///creates an empty location
				XLocation() : XbimHandle(new gp_Trsf())
				{}
				XLocation(double tx, double ty, double tz, double sc, double qw, double qx, double qy, double qz) : XbimHandle(new gp_Trsf())
				{
					Ref().SetScaleFactor(sc);
					Ref().SetRotationPart(gp_Quaternion(qx, qy, qz, qw));
					Ref().SetTranslationPart(gp_Vec(tx, ty, tz));
				}
				XLocation(const gp_Trsf& transform) : XbimHandle(new gp_Trsf(transform))
				{}

				virtual property double Scale {double get() { return Ref().ScaleFactor(); }; }
				virtual property IXPoint^ Translation {IXPoint^ get() { return gcnew XPoint(Ref().TranslationPart()); }; }
				virtual property IXQuaternion^ Rotation {IXQuaternion^ get() { return gcnew Xbim::Geometry::BRep::XQuarternion(Ref().GetRotation()); }; }
				virtual IXLocation^ Multiplied(IXLocation^ location);

				virtual property bool IsIdentity {bool get()
				{
					return Ref().Form() == gp_TrsfForm::gp_Identity;
				}}
				virtual void SetTranslation(double x, double y, double z) { Ref().SetTranslationPart(gp_Vec(x, y, z)); }
				virtual IXLocation^ Inverted() { return gcnew XLocation(Ref().Inverted()); }
				virtual IXLocation^ ScaledBy(double scale);
				virtual IXMatrix^ Multiply(IXMatrix^ matrix);
				virtual property double M11 {      double get() { return Ref().HVectorialPart().Value(1, 1); }; }
				virtual property double M12 {      double get() { return Ref().HVectorialPart().Value(1, 2); }; }
				virtual property double M13 {      double get() { return Ref().HVectorialPart().Value(1, 3); }; }
				virtual property double OffsetX {      double get() { return Ref().TranslationPart().X(); }; }
				virtual property double M21 {      double get() { return Ref().HVectorialPart().Value(2, 1); }; }
				virtual property double M22 {      double get() { return Ref().HVectorialPart().Value(2, 2); }; }
				virtual property double M23 {      double get() { return Ref().HVectorialPart().Value(2, 3); }; }
				virtual property double OffsetY {      double get() { return Ref().TranslationPart().Y(); }; }
				virtual property double M31 {      double get() { return Ref().HVectorialPart().Value(3, 1); }; }
				virtual property double M32 {      double get() { return Ref().HVectorialPart().Value(3, 2); }; }
				virtual property double M33 {      double get() { return Ref().HVectorialPart().Value(3, 3); }; }
				virtual property double OffsetZ {      double get() { return Ref().TranslationPart().Z(); }; }
				virtual property double ScaleX {  double get() { return  Ref().ScaleFactor(); }; }
				virtual property double ScaleY {  double get() { return Ref().ScaleFactor(); }; }
				virtual property double ScaleZ {  double get() { return Ref().ScaleFactor(); }; }
				virtual property double M44 {double get() { return 1.0; }; }
				virtual property array<double>^ Values {
					array<double>^ get()
					{
						array<double>^ values = gcnew array<double>(16) { M11, M12, M13, ScaleX, M21, M22, M23, ScaleY, M31, M32, M33, ScaleZ, OffsetX, OffsetY, OffsetZ, M44};
						return values;
					};
				}
				virtual array<System::Byte>^ ToByteArray()
				{
					auto ms = gcnew System::IO::MemoryStream(16 * sizeof(double));
					auto bw = gcnew System::IO::BinaryWriter(ms);
					for each (auto val in Values)
					{
						bw->Write(val);
					}
					auto bytes = ms->ToArray();
					delete bw;
					delete ms;
					return bytes;
				}
			};
		}
	}
}

