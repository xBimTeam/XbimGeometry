#include "CompoundFactory.h"

#include <BRep_Builder.hxx>
#include "../BRep/XCompound.h"
#include "../BRep/XEdge.h"
#include "../BRep/XSolid.h"
#include "../BRep/XFace.h"
#include "../BRep/XShell.h"
#include "../BRep/XVertex.h"
#include "../BRep/XWire.h"

using namespace Xbim::Geometry::BRep;

namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			IXCompound^ CompoundFactory::CreateEmpty()
			{
				BRep_Builder builder;
				TopoDS_Compound compound;
				builder.MakeCompound(compound);
				return gcnew XCompound(compound);
			}
			IXCompound^ CompoundFactory::CreateFrom(IEnumerable<IXShape^>^ shapes)
			{
				BRep_Builder builder;
				TopoDS_Compound compound;
				builder.MakeCompound(compound);
				for each (IXShape ^ shape in shapes)
				{	
					const TopoDS_Shape& topoShape = static_cast<XShape^>(shape)->GetTopoShape();
					builder.Add(compound, topoShape);
				}
				return gcnew XCompound(compound);

			}
			
		}
	}
}