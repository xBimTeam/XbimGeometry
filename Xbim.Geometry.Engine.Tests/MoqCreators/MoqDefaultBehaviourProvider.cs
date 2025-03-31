using Moq;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using System.Reflection;
using System.Text;
using Xbim.Common;
using Xbim.Common.Metadata;
using Xbim.Ifc4;

namespace Xbim.Geometry.Engine.Tests
{
    public class MoqDefaultBehaviourProvider: LookupOrFallbackDefaultValueProvider
    {
        private static ExpressMetaData metaData = ExpressMetaData.GetMetadata(new EntityFactoryIfc4());
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
            //base.Register(typeof(ExpressType), (type, mock) =>
            // {
            //     Type t = mock.GetType().GetGenericArguments().First();
            //     var ets = metaData.ExpressTypesImplementing(t).FirstOrDefault();
            //     return metaData.ExpressType(ets.);
            // }
            //);
        }
    }
   
}
