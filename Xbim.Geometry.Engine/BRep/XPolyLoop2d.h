#pragma once
#include "../XbimHandle.h"
#include "XPoint.h"
#include <Poly_Polygon2d.hxx>

using namespace System::Collections::Generic;
using namespace Xbim::Geometry::Abstractions;

namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			public ref class XPolyLoop2d : XbimHandle<Handle(Poly_Polygon2D)>, IXPolyLoop2d
			{

			public:

				XPolyLoop2d(Handle(Poly_Polygon2D) hPointSeq) : XbimHandle(new Handle(Poly_Polygon2D)(hPointSeq)) {};

				ref struct enumerator : System::Collections::Generic::IEnumerator<IXPoint^>
				{
					enumerator(XPolyLoop2d^ myArr)
					{
						colInst = myArr;
						currentIndex = 0;
					}

					virtual bool MoveNext() = System::Collections::Generic::IEnumerator<IXPoint^>::MoveNext
					{

						if (currentIndex < colInst->Ref()->NbNodes())
						{
							currentIndex++;
							return true;
						}
						return false;
					}

						property IXPoint^ Current
					{
						virtual IXPoint^ get() = System::Collections::Generic::IEnumerator<IXPoint^>::Current::get
						{
							return gcnew XPoint(colInst->Ref()->Nodes().Value(currentIndex));
						}
					};

					property Object^ Current2
					{
						virtual Object^ get() = System::Collections::IEnumerator::Current::get
						{
							return gcnew XPoint(colInst->Ref()->Nodes().Value(currentIndex));
						}
					};

					virtual void Reset() = System::Collections::Generic::IEnumerator<IXPoint^>::Reset{}
						~enumerator() {}

					XPolyLoop2d^ colInst;

					int currentIndex;
				};

				virtual System::Collections::Generic::IEnumerator<IXPoint^>^ GetEnumerator()
				{
					return gcnew enumerator(this);
				};

				virtual System::Collections::IEnumerator^ GetEnumerator2() = System::Collections::IEnumerable::GetEnumerator
				{
					return gcnew enumerator(this);
				};

				virtual property int NbPoints {int get() { return OccHandle()->NbNodes(); }}
				virtual IXWire^ BuildWire(double zDim);

			};
		}
	}
}

