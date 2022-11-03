#include "VisualMaterial.h"

using namespace Xbim::Ifc4::MeasureResource;
using namespace Xbim::Ifc4::PresentationAppearanceResource;
namespace Xbim
{
	namespace Geometry
	{
		namespace Visual
		{
			IXColourRGB^ VisualMaterial::AmbientColor::get()
			{
				return gcnew ColourRGB(Ref()->CommonMaterial().AmbientColor);
			}
			void VisualMaterial::AmbientColor::set(IXColourRGB^ colour)
			{
				XCAFDoc_VisMaterialCommon aMatCom = Ref()->CommonMaterial();
				Quantity_Color& c = aMatCom.AmbientColor;
				c.SetValues(colour->Red, colour->Green, colour->Blue, Quantity_TypeOfColor::Quantity_TOC_RGB);
				aMatCom.IsDefined = true;
				Ref()->SetCommonMaterial(aMatCom);
			}

			void VisualMaterial::SetPhysicalBasedRender()
			{
				XCAFDoc_VisMaterialPBR pbrMaterial = Ref()->ConvertToPbrMaterial();
				Ref()->SetPbrMaterial(pbrMaterial);
			}
			void VisualMaterial::SetStyle(IIfcSurfaceStyleElementSelect^ ifcSurfaceStyleElementSelect)
			{
				IIfcSurfaceStyleRendering^ rendering = dynamic_cast<IIfcSurfaceStyleRendering^>(ifcSurfaceStyleElementSelect);
				IIfcSurfaceStyleShading^ shading = dynamic_cast<IIfcSurfaceStyleShading^>(ifcSurfaceStyleElementSelect);
				if (rendering!=nullptr)
				{
					IIfcColourRgb^ surface = rendering->SurfaceColour;
					AmbientColor = gcnew ColourRGB(surface->Red * 0.3, surface->Green * 0.3, surface->Blue * 0.3);
					IIfcColourRgb^ diffuse = dynamic_cast<IIfcColourRgb^>(rendering->DiffuseColour);
					IfcNormalisedRatioMeasure^ nrm = dynamic_cast<IfcNormalisedRatioMeasure^>(rendering->DiffuseColour);
					if (diffuse!=nullptr)
						DiffuseColor = gcnew ColourRGB(diffuse->Red, diffuse->Green, diffuse->Blue);
					else if (nrm != nullptr)
					{
						double ratio = (double)nrm->Value;
						DiffuseColor = gcnew ColourRGB(ratio * surface->Red, ratio * surface->Green, ratio * surface->Blue);
					}
					else //this must be defined or the world is black
						DiffuseColor = gcnew ColourRGB(surface->Red, surface->Green, surface->Blue);

					IIfcColourRgb^ emmissive = dynamic_cast<IIfcColourRgb^>(rendering->ReflectionColour);
					nrm = dynamic_cast<IfcNormalisedRatioMeasure^>(rendering->ReflectionColour);
					if (emmissive!=nullptr)
						EmissiveColor = gcnew ColourRGB(emmissive->Red, emmissive->Green, emmissive->Blue);
					else if (nrm != nullptr)
					{
						double ratio = (double)nrm->Value;
						EmissiveColor = gcnew ColourRGB(ratio * surface->Red, ratio * surface->Green, ratio * surface->Blue);
					}
					else
						EmissiveColor = gcnew ColourRGB(0, 0, 0);

					IIfcColourRgb^ specular = dynamic_cast<IIfcColourRgb^>(rendering->SpecularColour);
					nrm = dynamic_cast<IfcNormalisedRatioMeasure^>(rendering->SpecularColour);
					if (specular!=nullptr)
						SpecularColor = gcnew ColourRGB(specular->Red, specular->Green, specular->Blue);
					else if (nrm!=nullptr)
					{
						double ratio = (double)nrm->Value;
						SpecularColor = gcnew ColourRGB(ratio * surface->Red, ratio * surface->Green, ratio * surface->Blue);
					}
					else
						SpecularColor = gcnew ColourRGB(0.2, 0.2, 0.2);

					IfcSpecularExponent^ se = dynamic_cast<IfcSpecularExponent^>(rendering->SpecularHighlight);
					IfcSpecularRoughness^ sr = dynamic_cast<IfcSpecularRoughness^>(rendering->SpecularHighlight);
					if (se != nullptr)
					{
						double seVal = (double)se->Value;
						Shininess = seVal > 1.0f ? (float)(seVal / 128.0f) : (float)seVal; //correction for BIM tools using values > 1 using SpecularExponent
					}
					else if (sr!=nullptr)
						Shininess = (float)sr->Value;
					Transparency = rendering->Transparency.HasValue ? (float)rendering->Transparency.Value : 0.0f;
					
					return;
				}
				else if (shading!=nullptr)
				{
					IIfcColourRgb^ surface = shading->SurfaceColour;
					AmbientColor = gcnew ColourRGB(surface->Red, surface->Green, surface->Blue);
					Transparency = shading->Transparency.HasValue ? (float)shading->Transparency.Value : 0.0f;
					
					return;
				}
				throw gcnew System::ArgumentException("Unexpected IIfcSurfaceStyleElementSelect type");

			}
			IXColourRGB^ VisualMaterial::DiffuseColor::get()
			{
				return gcnew ColourRGB(Ref()->CommonMaterial().DiffuseColor);
			}

			void VisualMaterial::DiffuseColor::set(IXColourRGB^ colour)
			{
				XCAFDoc_VisMaterialCommon aMatCom = Ref()->CommonMaterial();
				Quantity_Color& c = aMatCom.DiffuseColor;
				c.SetValues(colour->Red, colour->Green, colour->Blue, Quantity_TypeOfColor::Quantity_TOC_RGB);
				aMatCom.IsDefined = true;
				Ref()->SetCommonMaterial(aMatCom);

			}

			IXColourRGB^ VisualMaterial::SpecularColor::get()
			{
				return gcnew ColourRGB(Ref()->CommonMaterial().SpecularColor);
			}
			void VisualMaterial::SpecularColor::set(IXColourRGB^ colour)
			{
				XCAFDoc_VisMaterialCommon aMatCom = Ref()->CommonMaterial();
				Quantity_Color& c = aMatCom.SpecularColor;
				c.SetValues(colour->Red, colour->Green, colour->Blue, Quantity_TypeOfColor::Quantity_TOC_RGB);
				aMatCom.IsDefined = true;
				Ref()->SetCommonMaterial(aMatCom);
			}

			IXColourRGB^ VisualMaterial::EmissiveColor::get()
			{
				return gcnew ColourRGB(Ref()->CommonMaterial().EmissiveColor);
			}
			void VisualMaterial::EmissiveColor::set(IXColourRGB^ colour)
			{
				XCAFDoc_VisMaterialCommon aMatCom = Ref()->CommonMaterial();
				Quantity_Color& c = aMatCom.EmissiveColor;
				c.SetValues(colour->Red, colour->Green, colour->Blue, Quantity_TypeOfColor::Quantity_TOC_RGB);
				aMatCom.IsDefined = true;
				Ref()->SetCommonMaterial(aMatCom);
			}

			float VisualMaterial::Shininess::get()
			{
				return Ref()->CommonMaterial().Shininess;
			}
			void VisualMaterial::Shininess::set(float shininess)
			{

				XCAFDoc_VisMaterialCommon aMatCom = Ref()->CommonMaterial();
				aMatCom.Shininess = shininess;
				aMatCom.IsDefined = true;
				Ref()->SetCommonMaterial(aMatCom);
			}
			float VisualMaterial::Transparency::get()
			{
				return Ref()->CommonMaterial().Transparency;
			}
			void VisualMaterial::Transparency::set(float transparency)
			{
				XCAFDoc_VisMaterialCommon aMatCom = Ref()->CommonMaterial();
				aMatCom.Transparency = transparency;
				aMatCom.IsDefined = true;
				Ref()->SetCommonMaterial(aMatCom);
			}

			bool VisualMaterial::IsDefined::get()
			{
				return Ref()->CommonMaterial().IsDefined;
			}
		}
	}
}
