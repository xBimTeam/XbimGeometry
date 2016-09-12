"%ProgramFiles(x86)%\Microsoft Visual Studio 14.0\Common7\IDE\devenv.exe" Xbim.Geometry.Nuget.sln /build Release
nuget pack Xbim.Geometry.Nuspec

copy *.nupkg %HOMEPATH%\Dropbox\Nuget
del *.nupkg