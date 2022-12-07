using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using Xbim.Common;
using Xbim.IO.Memory;

namespace Xbim.Geometry.Engine.Interop.Tests
{
    public class EntityRepository<T> : IDisposable where T : IPersistEntity
    {
        MemoryModel model;
        public T Entity;

        public MemoryModel Model { get => model; private set => model = value; }

        public EntityRepository(string name)
        {
            var path = Path.GetFullPath($@"Testfiles\{name}.ifc");
            Assert.IsTrue(File.Exists(path), $"File not found '{path}'");
            Debug.WriteLine($"Opening '{path}' for tests.");
            Model = MemoryModel.OpenRead(path);

            Entity = (T)Model.Instances[1];
        }

        public EntityRepository(string name, double millimeter, double precision, bool inRadians) : this(name)
        {
            if (inRadians)
                Model.ModelFactors.Initialise(1, millimeter / 1000, precision);
            else
                Model.ModelFactors.Initialise(Math.PI / 180, millimeter / 1000, precision);
        }

        public TInstance Instance<TInstance>(int label) where TInstance : class, IPersistEntity
        {
            var instance = Model.Instances[label] as TInstance;
            return instance;
        }
        public void Dispose()
        {
            Model.Dispose();
        }
    }
}
