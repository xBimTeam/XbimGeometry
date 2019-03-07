Branch | Build Status  | MyGet | NuGet
------ | ------- | --- | --- |
Master | [![Build Status](https://dev.azure.com/xBIMTeam/xBIMToolkit/_apis/build/status/xBimTeam.XbimGeometry?branchName=master)](https://dev.azure.com/xBIMTeam/xBIMToolkit/_build/latest?definitionId=3&branchName=master) | ![master](https://img.shields.io/myget/xbim-master/v/Xbim.Geometry.svg) | ![](https://img.shields.io/nuget/v/Xbim.Geometry.svg)
Develop | [![Build Status](https://dev.azure.com/xBIMTeam/xBIMToolkit/_apis/build/status/xBimTeam.XbimGeometry?branchName=develop)](https://dev.azure.com/xBIMTeam/xBIMToolkit/_build/latest?definitionId=3&branchName=develop) | ![](https://img.shields.io/myget/xbim-develop/vpre/Xbim.Geometry.svg) | -


# XbimGeometry

XbimGeometry is part of the [Xbim Toolkit](https://github.com/xBimTeam/XbimEssentials). 

It contains the the Geometry Engine and Scene processing, which provide geometric and topological operations 
to enable users to visualise models in 3D models, typically as a Tesselated scene or mesh.

The native Geometry Engine is built around the open source [Open Cascade 7.3 library](https://www.opencascade.com/content/overview)
which performs much of the boolean operations involve in generating 3D solids. 
This technology is included under a licence which permits the use as part of a larger work, compatible with our open source CDDL licence.

## Compilation

**Visual Studio 2017 is recommended.**
Prior versions of Visual Studio are unlikely to work on this solution.

The [free VS 2017 Community Edition](https://visualstudio.microsoft.com/downloads/) will be fine. 

In order to compile this solution which includes C++ projects you'll need the following additional 
components installed:

- Visual C++ Core desktop features
- VC++ 2017 v141 tools
- Windows 10 SDK (10.0.17134.0) 

You'll also need patience as it can take up to 30 minutes to compile the C++ libraries.

The XBIM toolkit [uses the NuGet](https://www.nuget.org/packages/Xbim.Geometry/) for the management of our published packages.
We have custom MyGet feeds for the *master* and *develop* branches of the solution which are automatically
updated during our CI builds. The [nuget.config](nuget.config) file should automatically add these feeds for you.


## Acknowledgements
We'd like to acknowledge OpenCascade for the use of their library, which is permitted under clause 6 of [their
Licence](https://www.opencascade.com/content/licensing). 

The XbimTeam wishes to thank [JetBrains](https://www.jetbrains.com/) for supporting the XbimToolkit project 
with free open source [Resharper](https://www.jetbrains.com/resharper/) licenses.

Thanks also to Microsoft Azure DevOps for the use of [Azure Pipelines](https://azure.microsoft.com/en-us/services/devops/pipelines/) 
to automate our builds.

## Getting Involved

If you'd like to get involved and contribute to this project, please read the [CONTRIBUTING ](https://github.com/xBimTeam/XbimEssentials/blob/master/CONTRIBUTING.md)
page or contact the Project Coordinators @CBenghi and @martin1cerny.
