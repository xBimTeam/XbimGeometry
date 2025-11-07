# Xbim.Geometry


XbimGeometry is part of the [Xbim Toolkit](https://github.com/xBimTeam). 

This package is the top level nuget package for the xbim Geometry Engine which includes the xbim Geometry engine itself
plus the interop layer, and the unmanaged OCC geometry kernel, as well as the .NET ModelGeometry.Scene.


It contains the the Geometry Engine and Scene processing, which provide geometric and topological operations 
to enable users to visualise models in 3D models, typically as a Tesselated scene or mesh.

The native Geometry Engine is built around the open source [Open Cascade library](https://github.com/Open-Cascade-SAS/OCCT?tab=readme-ov-file#open-cascade-technology)
which performs much of the boolean operations involved in generating 3D solids. 
This technology is included under a licence which permits the use as part of a larger work, compatible with xbim's open source CDDL licence.

## Getting started

Before using this library you should register the Geometry Engine with the xbim ServiceProvider.

```csharp
	// Either configure the internal Services
	XbimServices.Current.ConfigureServices(opt => opt.AddXbimToolkit(conf => 
		conf.AddGeometryServices()
		));

	// or configure your services and register the provider with xbim:
	
	services.AddXbimToolkit(conf => conf.AddGeometryServices());
	// Once the DI container is built
	XbimServices.Current.UseExternalServiceProvider(serviceProvider);
```

## Usage

```csharp
using System.IO;
using Xbim.Ifc;
using Xbim.ModelGeometry.Scene;

const string fileName = "SampleHouse.ifc";
using (var model = IfcStore.Open(fileName))
{
    var context = new Xbim3DModelContext(model);
    context.CreateContext();
    // Now access tessellated geometry / Save wexbim etc
    var wexBimFilename = Path.ChangeExtension(fileName, "wexBIM");
    using (var wexBiMfile = File.Create(wexBimFilename))
    {
        using (var wexBimBinaryWriter = new BinaryWriter(wexBiMfile))
        {
            model.SaveAsWexBim(wexBimBinaryWriter);
            wexBimBinaryWriter.Close();
        }
        wexBiMfile.Close();
    }
}

```

