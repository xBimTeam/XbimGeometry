using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.IO;
using Xbim.Common;
using Xbim.IO.Memory;

namespace Xbim.Geometry.Engine.Interop.Tests
{
    public class EntityRepository<T> : IDisposable where T : IPersistEntity
    {
        MemoryModel model;
        public T Entity;
        public EntityRepository(string name, bool inRadians = false)
        {
            var path = Path.GetFullPath($@"TestFiles\{name}.ifc");
            Assert.IsTrue(File.Exists(path), path);
            model = MemoryModel.OpenRead(path);
            if (inRadians)
                model.ModelFactors.Initialise(1, 1e-3, 1e-5);
            else
                model.ModelFactors.Initialise(Math.PI / 180, 1e-3, 1e-5);
            Entity = (T)model.Instances[1];
        }

        public void Dispose()
        {
            model.Dispose();
        }
    }
}
