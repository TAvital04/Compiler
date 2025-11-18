echo off

gcc -O2 -std=c11 -o lex lex.c
lex "test_%1.txt"

gcc -O2 -std=c11 -o parsercodegen parsercodegen_complete.c
parsercodegen

echo.

gcc -O2 -Wall -std=c11 -o vm vm.c
vm elf.txt