r:
cd \code\nel\tools\leveldesign
cvs -d :pserver:besson@server:/home/cvsroot update
cd \code\nel\tools\3d\ligo
cvs -d :pserver:besson@server:/home/cvsroot update
cd \code\ryzom\src
cvs -d :pserver:besson@server:/home/cvsroot update

xcopy "\\stephanec\gamedev\lib\*.*" "R:\distrib\gamedev\lib" /Y
cd \code\nel\tools\leveldesign
