#pragma once
#include <XCAFPrs_DocumentNode.hxx>
#include <TDF_Tool.hxx>

struct ShapeNode
{
	TCollection_AsciiString Id;
	TDF_Label ShapeLabel;
	XCAFPrs_Style Style;
	TopLoc_Location Location;

	ShapeNode()
	{

	}
	ShapeNode(const XCAFPrs_DocumentNode& docNode)
	{
		TDF_Tool::Entry(docNode.RefLabel, Id);
		ShapeLabel = docNode.RefLabel;
		Style = docNode.Style;
		Location = docNode.Location;
	}
	ShapeNode& operator=(const XCAFPrs_DocumentNode& docNode)
	{
		TDF_Tool::Entry(docNode.RefLabel, Id);
		ShapeLabel = docNode.RefLabel;
		Style = docNode.Style;
		Location = docNode.Location;
		return *this;
	}
	bool operator() (const ShapeNode& lhs, const ShapeNode& rhs) const
	{
		return lhs.Id < rhs.Id;
	}
	bool operator==(const ShapeNode& other) const
	{
		return Id == other.Id;
	}
	std::size_t operator()(const ShapeNode& node) const
	{
		return std::hash<std::string>{}(node.Id.ToCString());
	}

	struct ShapeNodeHashFunction
	{
		size_t operator()(const ShapeNode& node) const
		{
			return std::hash<std::string>{}(node.Id.ToCString());
		}
	};

};


