@echo off

set COMPILER=gcc
set PROGRAM=synth.exe
set SRC=c_synth.c

%COMPILER% -o %PROGRAM% %SRC%
