@echo off

REM PAUSE ?
IF EXIST c:\pause.tag (
echo BUILD PAUSED
pause
echo BUILD RESUMED
del c:\pause.tag
)
