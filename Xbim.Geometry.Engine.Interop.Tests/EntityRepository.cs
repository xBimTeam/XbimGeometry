using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.IO;
using Xbim.Common;
using Xbim.IO.Memory;

namespace Xbim.Geometry.Engine.Interop.Tests
{
    public class EntityRepository<T> : IDisposable where T:IPersistEntity
    {
        MemoryModel model;
        public T Entity;
        public EntityRepository(string name)
        {
            var path = Path.GetFullPath($@"TestFiles\{name}.ifc");
            Assert.IsTrue(File.Exists(path), path);
            model = MemoryModel.OpenRead(path);
            Entity = (T)model.Instances[1];
        }        

        public void Dispose()
        {
            model.Dispose();
        }
    }
}
