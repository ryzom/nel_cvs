@echo off

echo **** > null
mkdir tag 2> null
mkdir output 2> null
mkdir cmb 2> null
mkdir bbox 2> null

REM Force setup of the small bank to avoid a warning
call ..\smallbank\0_setup.bat

rm null
