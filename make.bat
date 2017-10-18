@echo off

set COMPILER=gcc
set PROGRAM=synth.exe
set SRC=c_synth.c wave_file.c oscillator.c

%COMPILER% -o %PROGRAM% %SRC%
