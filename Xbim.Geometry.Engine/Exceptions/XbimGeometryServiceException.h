#pragma once
using namespace System;
namespace Xbim
{
    namespace Geometry
    {
        namespace Exceptions
        {
            public ref struct  XbimGeometryServiceException :
                public Exception
            {
            public:
                XbimGeometryServiceException() {};
                XbimGeometryServiceException(String^ message) : Exception(message) { };
                XbimGeometryServiceException(String^ message, Exception^ inner) : Exception(message, inner) {};
            };
        }
    }
}

