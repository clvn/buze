# Buzè

Buzé is a full featured modular music tracker, and compatible with a variety of plugin/file formats.

[![Build status](https://ci.appveyor.com/api/projects/status/6din3gg3skaw9ate/branch/master?svg=true)](https://ci.appveyor.com/project/clvn/buze/branch/master) 

![Machine View](https://batman.no/buze/assets/buze-plugins.png)

## Build from source

The project is developed using Visual Studio 2017, the "Desktop development with C++" workload and the following additional components:

- Windows 10 SDK
- Visual C++ ATL support
- MFC and ATL support
- VC++ 2017 v141 toolset

Additional software used during the build:

- [WiX toolset](http://wixtoolset.org/) produces the .msi installer
- [HTML Help Workshop](http://www.microsoft.com/downloads/details.aspx?FamilyID=00535334-c8a6-452f-9aa0-d597d16580cc) produces the .chm help file

### vcpkg

It is recommended to use `vcpkg` to manage dependencies required by the build.

- Download, bootstrap and integrate [vcpkg](https://github.com/Microsoft/vcpkg)
- Download [vcpkg_extras-master.zip](https://github.com/clvn/vcpkg_extras/archive/master.zip) from the [vcpkg_extras](https://github.com/clvn/vcpkg_extras) project, copy the unpacked ports directory contents to vcpkg/ports
- Install dependencies using vcpkg:
	- `vcpkg install zidl:x86-windows dbgenpp:x86-windows zlib:x86-windows libmad:x86-windows libsndfile:x86-windows sqlite3:x86-windows glew:x86-windows portmidi:x86-windows jack2:x86-windows soundtouch:x86-windows buzz-dsplib:x86-windows wtl:x86-windows`
	- `vcpkg install zlib:x64-windows libmad:x64-windows libsndfile:x64-windows sqlite3:x64-windows portmidi:x64-windows jack2:x64-windows buzz-dsplib:x64-windows wtl:x64-windows`

### Boost

Install boost with vcpkg too, or use the [prebuilt binaries](https://sourceforge.net/projects/boost/files/boost-binaries/).

### Paths

For convenience, and to avoid messing with the system PATH environment, create a Directory.build.targets in the solution root with the paths to vcpkg tools and WiX toolset, f.ex:

```xml
	<Project>
	 <PropertyGroup>
	  <ExecutablePath>d:\code\vcpkg\installed\x86-windows\tools;c:\Program Files (x86)\WiX Toolset v3.11\bin\;$(ExecutablePath)</ExecutablePath>
	 </PropertyGroup>
	</Project>
```
