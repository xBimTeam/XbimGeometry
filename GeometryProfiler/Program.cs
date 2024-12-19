// See https://aka.ms/new-console-template for more information
using Microsoft.Extensions.Logging;
using System.Diagnostics;
using Xbim.Geometry.Engine.Interop;
using Xbim.Ifc4.GeometricModelResource;
using Xbim.Ifc4.GeometryResource;
using Xbim.IO.Memory;

Console.WriteLine("Profiling");
ILoggerFactory loggerFactory = LoggerFactory.Create(builder => builder.AddConsole());
using var csgModel = InitCsgModel();
var modelService = XbimGeometryEngine.CreateModelGeometryService(csgModel, loggerFactory);
var v5Engine = XbimGeometryEngine.CreateGeometryEngineV5(csgModel, loggerFactory);

var cone = csgModel.Instances.OfType<IfcRightCircularCone>().First();
var v5Time = Time(() => v5Engine.CreateSolid(cone, null), 1000);
var v6Time = Time(() => modelService.SolidFactory.Build(cone), 1000);


MemoryModel InitCsgModel()
{
    var m = new MemoryModel(new Xbim.Ifc4.EntityFactoryIfc4());

    using (var txn = m.BeginTransaction("Test"))
    {
        var cylinder = m.Instances.New<IfcRightCircularCone>();
        var p = m.Instances.New<IfcAxis2Placement3D>();
        p.Axis = m.Instances.New<IfcDirection>(d => d.SetXYZ(1, 0, 0));
        p.Location = m.Instances.New<IfcCartesianPoint>(c => c.SetXYZ(0, 0, 0));
        cylinder.Position = p;
        cylinder.BottomRadius = 0.5;
        cylinder.Height = 2;




        txn.Commit();
    }
    return m;
}

TimeSpan Time(Action action, int executionCount = 1)
{
    action();
    var t = Stopwatch.StartNew();
    for (int i = 0; i < executionCount; i++)
    {
        action();
    }
    t.Stop();
    return t.Elapsed;
}