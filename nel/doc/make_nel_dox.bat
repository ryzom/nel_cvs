@echo off
R:
cd \code\nel\doc
del html\*.* /Q
doxygen nel.dox
