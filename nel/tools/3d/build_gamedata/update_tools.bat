REM -- Backup config files
copy cfg\*.cfg cfg\*.bak

REM -- Copy tools
xcopy \\server\code\tools\build_gamedata\*.* . /E /D /Y

REM -- Restore config files
copy cfg\*.bak cfg\*.cfg

REM -- Delete backuped config files
del cfg\*.bak
