REM ************ Upload LevelDesign Tools



REM --- Directories

mkdir \\server\code\tools
mkdir \\server\code\tools\leveldesign



REM --- Upload Binaries

REM - WorldEditor

xcopy r:\code\nel\tools\3d\ligo\WorldEditor_Exe\releasedebug\WorldEditor.exe "\\server\code\tools\leveldesign" /D /Y
xcopy r:\code\nel\lib\WorldEditor_release_debug.dll "\\server\code\tools\leveldesign" /D /Y
xcopy r:\code\nel\lib\nel_drv_opengl_win_rd.dll "\\server\code\tools\leveldesign" /D /Y

REM - Georges

xcopy r:\code\nel\tools\leveldesign\georges_exe\georges_release_debug.dll "\\server\code\tools\leveldesign" /D /Y
xcopy r:\code\nel\tools\leveldesign\georges_exe\releasedebug\georges_exe.exe "\\server\code\tools\leveldesign" /D /Y

REM - LogicEditor

xcopy r:\code\nel\tools\leveldesign\logic_editor_exe\logic_editor_rd.dll "\\server\code\tools\leveldesign" /D /Y
xcopy r:\code\nel\tools\leveldesign\logic_editor_exe\releasedebug\logic_editor_exe.exe "\\server\code\tools\leveldesign" /D /Y

REM - Master

xcopy r:\code\nel\tools\leveldesign\master\releasedebug\master.exe "\\server\code\tools\leveldesign" /D /Y



REM --- Backup the build

xcopy "\\server\code\tools\leveldesign\*.*" "\\server\code\tools\leveldesign backup\" /E
ren_date "\\server\code\tools\leveldesign backup" "\\server\code\tools\leveldesign"
