using System;

namespace Xbim.Geometry.Exceptions
{
    public class XbimShapeServiceException : Exception
    {
        public XbimShapeServiceException() { }
        public XbimShapeServiceException(string message) : base(message) { }
        public XbimShapeServiceException(string message, Exception innerException) : base(message, innerException) { }
    }
}
