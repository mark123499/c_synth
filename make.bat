@echo off

set COMPILER=gcc
set PROGRAM=synth.exe
set SRC=c_synth.c wave_file.c oscillator.c smf_file.c sequencer.c

%COMPILER% -o %PROGRAM% %SRC%
