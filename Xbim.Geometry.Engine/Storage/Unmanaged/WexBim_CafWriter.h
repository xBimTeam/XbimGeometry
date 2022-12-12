
#ifndef _WexBim_CafWriter_HeaderFiler
#define _WexBim_CafWriter_HeaderFiler

#include <TColStd_IndexedDataMapOfStringString.hxx>
#include <TColStd_MapOfAsciiString.hxx>
#include <TDF_LabelSequence.hxx>
#include <RWMesh_CoordinateSystemConverter.hxx>
#include <XCAFPrs_Style.hxx>
#include <vector>
class NWexBimMesh;
class NFaceMeshIterator;
class TDocStd_Document;

//! WexBim writer context from XCAF document.
//! 

class WexBim_CafWriter : public Standard_Transient
{
	DEFINE_STANDARD_RTTIEXT(WexBim_CafWriter, Standard_Transient)
public:

	//! Main constructor.
	//! @param theFile     [in] path to output WexBim file
	Standard_EXPORT WexBim_CafWriter(
		const Quantity_ColorRGBA& defaultColour, 
		double oneMeter, 
		double linearDefection, 
		double angularDeflection, 
		double precision);

	//! Destructor.
	Standard_EXPORT virtual ~WexBim_CafWriter();

	//! Return transformation from OCCT to WexBim coordinate system.
	const RWMesh_CoordinateSystemConverter& CoordinateSystemConverter() const { return myCSTrsf; }

	//! Return transformation from OCCT to WexBim coordinate system.
	RWMesh_CoordinateSystemConverter& ChangeCoordinateSystemConverter() { return myCSTrsf; }

	//! Set transformation from OCCT to WexBim coordinate system.
	void SetCoordinateSystemConverter(const RWMesh_CoordinateSystemConverter& theConverter) { myCSTrsf = theConverter; }

	//! Return default material definition to be used for nodes with only color defined.
	const XCAFPrs_Style& DefaultStyle() const { return myDefaultStyle; }



	//! Set default material definition to be used for nodes with only color defined.
	void SetDefaultStyle(const XCAFPrs_Style& theStyle) { myDefaultStyle = theStyle; }

	//! Write WexBim file and associated binary file.
	//! Triangulation data should be precomputed within shapes!
	//! @param theDocument    [in] input document
	//! @param theRootLabels  [in] list of root shapes to export
	//! @param theFileInfo    [in] map with file metadata to put into WexBim header section
	//! @return FALSE on file writing failure
	Standard_EXPORT virtual bool Perform(const Handle(TDocStd_Document)& theDocument, std::ostream& outputStrm,
		const TDF_LabelSequence& theRootLabels,
		const TColStd_IndexedDataMapOfStringString& theFileInfo);

	//! Write WexBim file and associated binary file.
	//! Triangulation data should be precomputed within shapes!
	//! @param theDocument    [in] input document
	//! @param theFileInfo    [in] map with file metadata to put into WexBim header section
	//! @return FALSE on file writing failure
	Standard_EXPORT virtual bool Perform(const Handle(TDocStd_Document)& theDocument, std::ostream& outputStrm,
		const TColStd_IndexedDataMapOfStringString& theFileInfo);

protected:

	//! Write binary data file with triangulation data.
	//! Triangulation data should be precomputed within shapes!
	//! @param theDocument    [in] input document
	//! @param theRootLabels  [in] list of root shapes to export
	//! @return FALSE on file writing failure
	Standard_EXPORT virtual bool writeBinData(const Handle(TDocStd_Document)& theDocument, std::ostream& outputStrm,
		const TDF_LabelSequence& theRootLabels,
		const TColStd_IndexedDataMapOfStringString& theFileInfo);

protected:


	//! Write mesh nodes into binary file.
	//! @param theWexBimFace [out] WexBim face definition
	//! @param theBinFile  [out] output file to write into
	//! @param theFaceIter [in]  current face to write
	//! @param theAccessorNb [in] [out] last accessor index
	Standard_EXPORT virtual void saveNodes(NWexBimMesh& theMesh, const NFaceMeshIterator& theFaceIter, std::vector<int>& pointIndexMap) const;



	//! Write mesh indexes into binary file.
	//! @param theWexBimFace [out] WexBim face definition
	//! @param theBinFile  [out] output file to write into
	//! @param theFaceIter [in]  current face to write
	//! @param theAccessorNb [in] [out] last accessor index
	Standard_EXPORT virtual void saveIndicesAndNormals(NWexBimMesh& wexBimMesh,  NFaceMeshIterator& theFaceIter, std::vector<int>& pointIndexMap);


protected:


protected:
	RWMesh_CoordinateSystemConverter              myCSTrsf;            //!< transformation from OCCT to WexBim coordinate system
	XCAFPrs_Style                                 myDefaultStyle;      //!< default material definition to be used for nodes with only color defined
	double                                        _oneMeter;
	double										  _linearDeflection;
	double										  _angularDeflection;
	double									      _precision;
};

#endif // _WexBim_CafWriter_HeaderFiler
