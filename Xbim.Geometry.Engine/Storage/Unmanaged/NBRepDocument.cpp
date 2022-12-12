#include "NBRepDocument.h"
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_VisMaterialTool.hxx>
#include <TDF_Tool.hxx>
#include <TDataStd_Name.hxx>

TDF_Label NBRepDocument::FindVisualMaterial(const Handle(TDocStd_Document)& theDoc, const TCollection_AsciiString& theKey)
{
    Handle(XCAFDoc_VisMaterialTool) aMatTool = XCAFDoc_DocumentTool::VisMaterialTool(theDoc->Main());
    TDF_Label aMatLab;
    TDF_Tool::Label(theDoc->GetData(), theKey, aMatLab);
    if (!aMatLab.IsNull())
    {
        return aMatTool->IsMaterial(aMatLab) ? aMatLab : TDF_Label();
    }

    TDF_LabelSequence aLabels;
    aMatTool->GetMaterials(aLabels);
    for (TDF_LabelSequence::Iterator aLabIter(aLabels); aLabIter.More(); aLabIter.Next())
    {
        Handle(TDataStd_Name) aNodeName;
        if (aLabIter.Value().FindAttribute(TDataStd_Name::GetID(), aNodeName)
            && aNodeName->Get().IsEqual(theKey))
        {
            return aLabIter.Value();
        }
    }
    return TDF_Label();
}
