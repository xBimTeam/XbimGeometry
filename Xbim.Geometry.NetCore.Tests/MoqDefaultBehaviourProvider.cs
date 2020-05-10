using Moq;
using System;
using System.Collections.Generic;
using System.Text;
using Xbim.Common;

namespace Xbim.Geometry.NetCore.Tests
{
    public class MoqDefaultBehaviourProvider: LookupOrFallbackDefaultValueProvider
    {
        public MoqDefaultBehaviourProvider()
        {
            //base.Register(typeof(string), (type, mock) => "?");
            base.Register(typeof(IItemSet<>), (type, mock) =>
            {
                Type lType = typeof(ItemListMoq<>);
                var genType = type.GetGenericArguments()[0];
                Type constructed = lType.MakeGenericType(genType);
                return Activator.CreateInstance(constructed);
            }
            );
        }
    }
   
}
