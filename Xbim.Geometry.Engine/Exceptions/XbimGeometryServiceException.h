#pragma once

namespace Xbim
{
    namespace Geometry
    {
        namespace Exceptions
        {
            public ref struct  XbimGeometryServiceException :
                public System::Exception
            {
            public:
                XbimGeometryServiceException() {};
                XbimGeometryServiceException(System::String^ message) : System::Exception(message) { };
                XbimGeometryServiceException(System::String^ message, System::Exception^ inner) : System::Exception(message, inner) {};
            };
        }
    }
}

