namespace Xbim.Geometry.Abstractions
{
    public interface IXBRepDocumentManager
    {
        /// <summary>
        /// Returns true if the document manager is fully loaded
        /// </summary>
        bool IsLoaded { get; }
        /// <summary>
        /// Number of documents open in the current session
        /// </summary>
        int NbDocumentsInSession { get; }
        /// <summary>
        /// True if the named document is open in the current session
        /// </summary>
        /// <param name="filePath"></param>
        /// <returns></returns>
        int InSession(string filePath);
        /// <summary>
        /// Returns the in session document
        /// </summary>
        /// <param name="docIndex"></param>
        /// <returns></returns>
        IXBRepDocument GetDocument(int docIndex);
        /// <summary>
        /// Returns the document, null if there is an error accessing the document, see logs for error message
        /// </summary>
        /// <param name="filePath"></param>
        /// <returns></returns>
        IXBRepDocument Open(string filePath);
        IXBRepDocument Open(byte[] bytes);
        /// <summary>
        /// Returns the created document, null if there is an error creating the document, see logs for error message
        /// </summary>
        /// <returns></returns>
        IXBRepDocument NewDocument();

        IXBRepDocument NewDocument(double oneMeter, double precision);

        /// <summary>
        /// Save the document
        /// </summary>
        /// <returns></returns>
        bool Save(IXBRepDocument document);
        /// <summary>
        /// 
        /// </summary>
        /// <param name="filePath"></param>
        /// <param name="document"></param>
        /// <returns></returns>
        bool SaveAs(string filePath, IXBRepDocument document);
        /// <summary>
        /// Returns the document as a byte array
        /// </summary>
        /// <param name="document"></param>
        /// <returns></returns>
        byte[] ToArray(IXBRepDocument document);
        /// <summary>
        /// Close the storage document and removes from the session
        /// </summary>
        /// <param name="document"></param>
        void Close(IXBRepDocument document);

        /// <summary>
        /// returns a binary byte array representation of the shape and its location, 
        /// </summary>
        /// <param name="shape"></param>
        /// <param name="withTriangles"></param>
        /// <param name="withNormals"></param>
        /// <returns></returns>
        byte[] ToArray(IXShape shape, bool withTriangles = false, bool withNormals = false);

        IXShape FromArray(byte[] bytes);
    }
}
