@echo off
R:
cd \code\server\doc
del html\*.* /Q
s:\bin\doxygen server.dox
S:\bin\hhc html\index.hhp
copy html\index.chm s:\doc\server.chm
