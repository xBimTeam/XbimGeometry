#pragma once
using namespace System;
namespace Xbim
{
    namespace Geometry
    {
        namespace Exceptions
        {
            public ref struct  XbimGeometryFactoryException :
                public Exception
            {
            public:
                XbimGeometryFactoryException() {};
                XbimGeometryFactoryException(String^ message) : Exception(message) { };
                XbimGeometryFactoryException(String^ message, Exception^ inner) : Exception(message, inner) {};
            };
        }
    }
}


