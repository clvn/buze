@rem This bat file calls hhc.exe, deletes the chm on error, and returns an
@rem nmake compatible errorlevel. hhc.exe is hard to use from nmake because:
@rem  1) it returns unexpected errorlevel codes (success = 1)
@rem  2) it always produces .chm output even with errors
@echo off
hhc %1
if not ERRORLEVEL 1 goto error
exit /B 0
:error
del %~n1.chm
exit /B 1
