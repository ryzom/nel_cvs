@echo off
R:
cd \code\nel\doc
del html\*.* /Q
s:\bin\doxygen nel.dox
