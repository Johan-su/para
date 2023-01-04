@echo off

set CLANG=clang++
set FLAGS=-O0 -D _DEBUG -g -gcodeview -Wall -Wpedantic -Wextra -Wconversion -Wshadow -Wno-missing-braces -Wno-c++20-designator -Wno-c++17-extensions -Wno-variadic-macros -Wno-gnu-zero-variadic-macro-arguments


%CLANG% ./src/main.cpp %FLAGS% -o para.exe