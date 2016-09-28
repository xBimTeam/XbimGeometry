@echo on
set src_folder=C:\Users\Steve\Downloads\occt-e8e26df.tar\occt-e8e26df\src
set dst_folder=C:\Users\Steve\Source\Repos\XbimGeometry\Xbim.Geometry.Engine\OCC\src
for /f "tokens=*" %%i in (OCCDirList.txt) DO (
    xcopy /S/Y "%src_folder%\%%i\*.*" "%dst_folder%\%%i\"
)
