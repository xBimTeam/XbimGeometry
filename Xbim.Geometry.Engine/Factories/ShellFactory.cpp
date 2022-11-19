#include "ShellFactory.h"
#include <vector>
using namespace System::Collections::Generic;
using namespace System::Linq;

namespace Xbim
{
	namespace Geometry
	{
		namespace Factories
		{
			TopoDS_Shell ShellFactory::BuildConnectedFaceSet(IIfcConnectedFaceSet^ faceSet)
			{
				//process into native structure
				std::vector<std::vector<std::vector<double>>> faceMesh;
				for each (IIfcFace ^ face in  faceSet->CfsFaces)
				{
					std::vector<std::vector<double>> faceLoops;
					
					for each (IIfcFaceBound ^ bound in face->Bounds) //build all the loops
					{
						IIfcPolyLoop^ polyloop = dynamic_cast<IIfcPolyLoop^>(bound->Bound);
						if (polyloop != nullptr)
						{
							IEnumerable<IIfcCartesianPoint^>^ polygon = polyloop->Polygon;
							if (!bound->Orientation) polygon = Enumerable::Reverse(polyloop->Polygon);
							std::vector<double> loop;							
							for each (IIfcCartesianPoint ^ cp in Enumerable::Concat(polyloop->Polygon, Enumerable::Take(polyloop->Polygon, 1)))
							{
								loop.push_back(cp->X);
								loop.push_back(cp->Y);
								if (2 == (int)cp->Dim) loop.push_back(0); else loop.push_back(cp->Z);
							}
							if (loop.size() < 4) //its not a min of a closed triangle
								LoggingService->LogDebug("Bound #" + bound->EntityLabel.ToString() + " is not a valid boundary");
							else
								faceLoops.push_back(loop);
						}
					}
					faceMesh.push_back(faceLoops);
				}
				return Ptr()->BuildConnectedFaceSet(faceMesh, ModelService->Precision, ModelService->OneMillimeter);
			}
		}
	}
}

