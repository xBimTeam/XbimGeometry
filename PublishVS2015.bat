"%ProgramFiles(x86)%\Microsoft Visual Studio 14.0\Common7\IDE\devenv.exe" "Xbim.Geometry.Nuget - VS2015.sln" /build Release
nuget pack Xbim.GeometryVS2015.Nuspec

copy *.nupkg %HOMEPATH%\Dropbox\Nuget
del *.nupkg