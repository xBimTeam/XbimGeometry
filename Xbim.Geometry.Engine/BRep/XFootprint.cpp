#include "XFootprint.h"
#include "../BRep/XPolyLoop2d.h"
using namespace System;
using namespace System::Globalization;
using namespace System::Text;
using namespace System::Linq;

namespace Xbim
{
	namespace Geometry
	{
		namespace BRep
		{
			SFAGeometryType XFootprint::SfaGeometryType::get()
			{
				if (Ptr()->Bounds.size() == 0) return SFAGeometryType::Empty;
				if (Ptr()->Bounds.size() == 1) 
					return SFAGeometryType::Polygon;
				else 
					return SFAGeometryType::MultiPolygon;
			}

			void XFootprint::Write(BinaryWriter^ binaryWriter)
			{
				if (Ptr()->Bounds.size() == 0) return;
				const UInt32 MULTIPOLYGON = 6;
				const UInt32 POLYGON = 3;
				bool isMultiPolygon = Ptr()->Bounds.size() > 1;
				if (isMultiPolygon)
				{
					binaryWriter->Write(SfaByteOrder);
					binaryWriter->Write(MULTIPOLYGON);
					binaryWriter->Write((unsigned int)Ptr()->Bounds.size());
					for (auto& boundIt = Ptr()->Bounds.cbegin(); boundIt != Ptr()->Bounds.cend(); boundIt++)
					{
						binaryWriter->Write(SfaByteOrder);
						binaryWriter->Write(POLYGON);
						binaryWriter->Write((unsigned int)boundIt->size());
						for (auto& polyIt = boundIt->cbegin(); polyIt != boundIt->cend(); polyIt++)
						{
							binaryWriter->Write((unsigned int)(*polyIt)->NbNodes());
							for (auto& pointIt = (*polyIt)->Nodes().cbegin(); pointIt != (*polyIt)->Nodes().cend(); pointIt++)
							{
								const gp_XY& pnt = pointIt->Coord();
								binaryWriter->Write(pnt.X());
								binaryWriter->Write(pnt.Y());
							}
						}
					}
				}
				else
				{
					//just take the first bound
					auto& bound = Ptr()->Bounds.front();
					binaryWriter->Write(SfaByteOrder);
					binaryWriter->Write(POLYGON);
					binaryWriter->Write((unsigned int)bound.size());
					for (auto& polyIt = bound.cbegin(); polyIt != bound.cend(); polyIt++)
					{
						binaryWriter->Write((unsigned int)(*polyIt)->NbNodes());
						for (auto& pointIt = (*polyIt)->Nodes().cbegin(); pointIt != (*polyIt)->Nodes().cend(); pointIt++)
						{
							const gp_XY& pnt = pointIt->Coord();
							binaryWriter->Write(pnt.X());
							binaryWriter->Write(pnt.Y());
						}
					}
				}
			}
			void XFootprint::Write(TextWriter^ textWriter)
			{
				textWriter->Write(ToString());
			}

			String^ XFootprint::ToString()
			{

				if (Ptr()->Bounds.size() == 0) return "MULTIPOLYGON EMPTY";
				bool isMultiPolygon = Ptr()->Bounds.size() > 1;
				StringBuilder^ sb = gcnew StringBuilder();
				CultureInfo^ culture = CultureInfo::InvariantCulture;
				if (isMultiPolygon)
					sb->Append("MULTIPOLYGON(");
				else
					sb->Append("POLYGON(");
				//write the Polygons
				for (auto& boundIt = Ptr()->Bounds.cbegin(); boundIt != Ptr()->Bounds.cend(); boundIt++)
				{
					//write the rings
					if (isMultiPolygon) sb->Append("(");
					for (auto& polyIt = boundIt->cbegin(); polyIt != boundIt->cend(); polyIt++)
					{
						sb->Append("(");
						//write the points
						const TColgp_Array1OfPnt2d& nodes = (*polyIt)->Nodes();
						for (int i = 1; i <= (*polyIt)->NbNodes(); i++)
						{
							sb->Append(nodes.Value(i).Coord(1).ToString(culture));
							sb->Append(" ");
							sb->Append(nodes.Value(i).Coord(2).ToString(culture));
							if (i != nodes.Size())
								sb->Append(", ");
						}

						if (*polyIt != boundIt->back()) //we are not at the last one
						{
							sb->Append(", ");
						}
						sb->Append(")");
					}
					if (*boundIt != Ptr()->Bounds.back()) //we are not at the last one
					{
						sb->Append(", ");
					}
					if (isMultiPolygon) sb->Append(")");
				}
				sb->Append(")");
				return sb->ToString();
			}

			IEnumerable<IEnumerable<IXPolyLoop2d^>^>^ XFootprint::Bounds::get()
			{
				List<IEnumerable<IXPolyLoop2d^>^>^ bounds = gcnew List<IEnumerable<IXPolyLoop2d^>^>();
				for (auto& boundIt = Ptr()->Bounds.cbegin(); boundIt != Ptr()->Bounds.cend(); boundIt++)
				{
					List<IXPolyLoop2d^>^ loops = gcnew List<IXPolyLoop2d^>();
					bounds->Add(loops);
					for (auto& polyIt = boundIt->cbegin(); polyIt != boundIt->cend(); polyIt++)
					{
						loops->Add(gcnew XPolyLoop2d(*polyIt));
					}
				}
				return bounds;
			}
		}
	}
}