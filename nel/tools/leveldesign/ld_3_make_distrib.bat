REM ************ Upload LevelDesign Tools



REM --- Directories

mkdir \\server\code\tools
rmdir /s /q \\server\code\tools\leveldesign
mkdir \\server\code\tools\leveldesign
mkdir \\server\code\tools\leveldesign\zoneBitmaps
mkdir \\server\code\tools\leveldesign\zoneLigos
mkdir \\server\code\tools\leveldesign\zones
mkdir \\server\code\tools\leveldesign\common
mkdir \\server\code\tools\leveldesign\common\dfn

REM --- Upload Binaries

REM - WorldEditor

xcopy "r:\code\nel\tools\3d\ligo\WorldEditor_Exe\releasedebug\WorldEditor_exe.exe" "\\server\code\tools\leveldesign" /D /Y
xcopy "r:\code\nel\lib\WorldEditor_release_debug.dll" "\\server\code\tools\leveldesign" /D /Y
xcopy "r:\code\nel\lib\nel_drv_opengl_win_rd.dll" "\\server\code\tools\leveldesign" /D /Y
xcopy "R:\code\tool\zviewer\ReleaseDebug\zviewer.exe" "\\server\code\tools\leveldesign" /D /Y

REM - Georges

xcopy "r:\code\nel\tools\leveldesign\georges_exe\georges_release_debug.dll" "\\server\code\tools\leveldesign" /D /Y
xcopy "r:\code\nel\tools\leveldesign\georges_exe\releasedebug\georges_exe.exe" "\\server\code\tools\leveldesign" /D /Y

REM - LogicEditor

xcopy "r:\code\nel\tools\leveldesign\logic_editor_exe\logic_editor_rd.dll" "\\server\code\tools\leveldesign" /D /Y
xcopy "r:\code\nel\tools\leveldesign\logic_editor_exe\releasedebug\logic_editor_exe.exe" "\\server\code\tools\leveldesign" /D /Y

REM - Master

xcopy "r:\code\nel\tools\leveldesign\master\releasedebug\master.exe" "\\server\code\tools\leveldesign" /D /Y
xcopy "r:\code\nel\tools\leveldesign\master\n019003l.pfb" "\\server\code\tools\leveldesign" /D /Y
xcopy "r:\code\nel\tools\leveldesign\master\readme.txt" "\\server\code\tools\leveldesign" /D /Y
xcopy "r:\code\nel\tools\leveldesign\test_root\common\dfn\*.*" "\\server\code\tools\leveldesign\common\dfn" /D /Y
xcopy "r:\code\nel\tools\leveldesign\test_root\ligoscape.cfg" "\\server\code\tools\leveldesign" /D /Y
xcopy "r:\code\nel\tools\leveldesign\test_root\zoneBitmaps\_unused_.tga" "\\server\code\tools\leveldesign\zoneBitmaps" /D /Y

REM --- Backup the build

xcopy "\\server\code\tools\leveldesign\*.*" "\\server\code\tools\leveldesign backup\" /E
ren_date "\\server\code\tools\leveldesign backup" "\\server\code\tools\leveldesign"

pause
cd \code\nel\tools\leveldesign