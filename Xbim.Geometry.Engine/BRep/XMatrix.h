
#pragma once
#include <gp_Trsf.hxx>
#include <gp_GTrsf.hxx>
#include "../XbimHandle.h"
#include <Graphic3d_Mat4d.hxx>
using namespace Xbim::Geometry::Abstractions;
namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			public ref class XMatrix : XbimHandle<Graphic3d_Mat4d>, IXMatrix
			{
			public:
				XMatrix() : XbimHandle(new Graphic3d_Mat4d())
				{}
				XMatrix(const gp_Trsf& trsf) : XbimHandle(new Graphic3d_Mat4d())
				{
					trsf.GetMat4(Ref());
				}
				XMatrix(const gp_GTrsf& gTrsf) : XbimHandle(new Graphic3d_Mat4d()) { gTrsf.GetMat4(Ref()); }
				XMatrix(const gp_Mat& mat, const gp_Vec& offset, const gp_XYZ& scale) : XbimHandle(new Graphic3d_Mat4d())
				{
					for (int r = 0; r < 4; r++)
						for (int c = 0; c < 4; c++)
							Ref().SetValue(r, c, mat.Value(r, c));

					Ref().SetValue(0, 3, scale.X());
					Ref().SetValue(1, 3, scale.Y());
					Ref().SetValue(2, 3, scale.Z());
					Ref().SetValue(3, 0, offset.X());
					Ref().SetValue(3, 1, offset.Y());
					Ref().SetValue(3, 2, offset.Z());
				}
				virtual property bool  IsIdentity { bool get() { return Ref().IsIdentity(); }; }

				virtual property double M11 {      double get() { return Ref().GetValue(0, 0); }; }
				virtual property double M12 {      double get() { return Ref().GetValue(0, 1); }; }
				virtual property double M13 {      double get() { return Ref().GetValue(0, 2); }; }
				virtual property double ScaleX {      double get() { return Ref().GetValue(0, 3); }; }
				virtual property double M21 {      double get() { return Ref().GetValue(1, 0); }; }
				virtual property double M22 {      double get() { return Ref().GetValue(1, 1); }; }
				virtual property double M23 {      double get() { return Ref().GetValue(1, 2); }; }
				virtual property double ScaleY {      double get() { return Ref().GetValue(1, 3); }; }
				virtual property double M31 {      double get() { return Ref().GetValue(2, 0); }; }
				virtual property double M32 {      double get() { return Ref().GetValue(2, 1); }; }
				virtual property double M33 {      double get() { return Ref().GetValue(2, 2); }; }
				virtual property double ScaleZ {      double get() { return Ref().GetValue(2, 3); }; }
				virtual property double M44 {         double get() { return Ref().GetValue(3, 3); }; }
				virtual property double OffsetX {  double get() { return Ref().GetValue(3, 0); }; }
				virtual property double OffsetY {  double get() { return Ref().GetValue(3, 1); }; }
				virtual property double OffsetZ {  double get() { return Ref().GetValue(3, 2); }; }

				gp_GTrsf Transform()
				{
					gp_GTrsf gTrsf; 
					gTrsf.SetMat4(Ref()); 
					return gTrsf;
				};
				void SetScale(double x, double y, double z)
				{
					Ref().SetValue(0, 3, x);
					Ref().SetValue(1, 3, y);
					Ref().SetValue(2, 3, z);
				};
				virtual property array<double>^ Values {
					array<double>^ get()
					{
						array<double>^ values = gcnew array<double>(16);
						int i = 0;
						for (int r = 0; r < 4; r++)
							for (int c = 0; c < 4; c++)
								values[i++] = Ref().GetValue(r, c);
						return values;
					};
				}
			};
		}
	}
}
