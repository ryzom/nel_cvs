@echo off
echo Generating config_file.lex.cpp
flex -f -8 -Pcf -Scfflex.skl -oconfig_file.lex.cpp config_file.lex

echo Generating config_file.yacc.cpp
set BISON_SIMPLE=cfbison.simple
bison -d -p cf -o config_file.yacc.cpp config_file.yacc
pause
