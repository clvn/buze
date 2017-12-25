// This bat file calls hhc.exe, deletes the chm on error, and returns an
// nmake compatible errorlevel. hhc.exe is hard to use from nmake because:
//  1) it returns unexpected errorlevel codes (success = 1)
//  2) it always produces .chm output even with errors

var inputFile = WScript.arguments(0);
var WShell = WScript.CreateObject("WScript.Shell")
var errorCode = WShell.Run("hhc " + inputFile, undefined, true);

if (errorCode == 1) {
	WScript.Quit(0);
} else {
	var outputFile = (inputFile.substr(0, inputFile.lastIndexOf('.')) || input) + ".chm";
	WShell.Run("del " + outputFile, undefined, true);
	WScript.Quit(1);
}
/*
@echo off
hhc %1
if not ERRORLEVEL 1 goto error
exit /B 0
:error
del %~n1.chm
exit /B 1
*/