
using System;
using System.Diagnostics;
using System.IO;

using Xbim.Common;
using Xbim.IO.Memory;
using Xunit;
using FluentAssertions;
namespace Xbim.Geometry.Engine.Tests
{
    public class EntityRepository<T> : IDisposable where T : IPersistEntity
    {
        MemoryModel model;
        public T Entity;

        public MemoryModel Model { get => model; private set => model = value; }

#pragma warning disable CS8618 // Non-nullable field must contain a non-null value when exiting constructor. Consider declaring as nullable.
        public EntityRepository(string name)
#pragma warning restore CS8618 // Non-nullable field must contain a non-null value when exiting constructor. Consider declaring as nullable.
        {
            var path = Path.GetFullPath($@"Testfiles\{name}.ifc");
            File.Exists(path).Should().BeTrue();
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

        public TInstance? Instance<TInstance>(int label) where TInstance : class, IPersistEntity
        {
            return Model.Instances[label] as TInstance;
           
        }
        public void Dispose()
        {
            Model.Dispose();
        }
    }
}
