Prequisites:
	- Installed WiX tools, added to PATH
	- Open VS command prompt

setup.wxs is the WiX installer definition source file.

Visual Studio 2012 Express prohibits extensions such as WiX, so instead the
installer is created by a makefile:

nmake -f setup.nmake
