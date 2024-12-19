#pragma once
#include "../XbimHandle.h"
#include <XCAFDoc_VisMaterial.hxx>


using namespace Xbim::Common::Geometry;
using namespace Xbim::Geometry::Abstractions;

using namespace System::Collections::Generic;
namespace Xbim
{
	namespace Geometry
	{
		namespace Visual
		{
			public ref class ColourRGB : IEqualityComparer<ColourRGB^>,  IXColourRGB
			{
				double _red;
				double _green;
				double _blue;
			public:
				ColourRGB(const Quantity_Color& cRGB):_red(cRGB.Red()), _green(cRGB.Green()), _blue(cRGB.Blue()) {};
				ColourRGB(double red, double green, double blue) :_red(red), _green(green), _blue(blue) {};
				virtual property double Red { double get() { return _red; }; }
				virtual property double Green{ double get() { return _green; }; }
				virtual property double Blue{ double get() { return _blue; }; }
				virtual  bool Equals(Object^ obj) override;
				virtual  bool Equals(IXColourRGB^ obj)  { return Equals(this, obj); };
				static bool operator == (ColourRGB^ c1, ColourRGB^ c2) { return c1->Equals(c2); };
				static bool operator != (ColourRGB^ c1, ColourRGB^ c2) { return !c1->Equals(c2); };
				
				virtual int GetHashCode() override;
				// Inherited via IEqualityComparer
				virtual bool Equals(ColourRGB^ x, ColourRGB^ y);
				virtual int GetHashCode(ColourRGB^ obj);
				virtual bool Equals(IXColourRGB^ x, IXColourRGB^ y);
				virtual int GetHashCode(IXColourRGB^ obj);
			};
		}
	}
}
