param($installPath, $toolsPath, $package, $project)

$propertyName = "CopyToOutputDirectory"

$item32 = $project.ProjectItems.Item("x86\Xbim.Geometry.Engine32.dll")
if ($item32  -ne $null) 
{ 
	$property = $item32.Properties.Item($propertyName)
	if ($property -ne $null) 
	{ 
		$property.Value = 2
	}
}

$item64 = $project.ProjectItems.Item("x64\Xbim.Geometry.Engine64.dll")
if ($item64  -ne $null) 
{ 
	$property = $item64.Properties.Item($propertyName)
	if ($property -ne $null) 
	{ 
		$property.Value = 2
	}
}
    
