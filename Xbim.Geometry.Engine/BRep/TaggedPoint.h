#pragma once
namespace Xbim
{
    namespace Geometry
    {
        namespace BRep
        {

            struct TaggedPoint
            {
            public:
                gp_Pnt point;
                std::string tag;

                TaggedPoint(const gp_Pnt& p, const std::string& t) : point(p), tag(t) {}
            };

        }
    }
}