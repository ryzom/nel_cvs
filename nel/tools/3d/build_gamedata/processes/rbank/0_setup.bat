@echo off

echo **** > null
mkdir tag 2> null
mkdir output 2> null
mkdir cmb 2> null
mkdir bbox 2> null

REM Force setup of the small bank to avoid a warning
cd ..\smallbank
call 0_setup.bat
cd ..\rbank

rm null
