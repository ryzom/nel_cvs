@echo off

REM PAUSE ?
IF EXIST c:\pause.tag (
del c:\pause.tag
echo BUILD PAUSED
pause
echo BUILD RESUMED
)
