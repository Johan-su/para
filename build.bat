@echo off
set CLANG=clang++
set WARNINGS=-Wall -Wpedantic -Wextra -Wconversion -Wshadow -Wno-c++20-designator -Wno-c++17-extensions -Wno-gnu-anonymous-struct -Wno-nested-anon-types
set FLAGS=-O0 -D _DEBUG -D _CRT_SECURE_NO_WARNINGS  -g -gcodeview %WARNINGS%

@REM set SANITIZE=-fsanitize=address -fsanitize=undefined
set WINLIB= -l User32.lib -l Gdi32.lib -l Shell32.lib -l Winmm.lib -l ucrt.lib

if not exist build mkdir build

cd ./build

%CLANG% ../src/main.cpp %FLAGS% -l ../lib/raylib.lib %WINLIB%  -I ../lib/include -nostdlib -o para.exe
%CLANG% ../src/parse.cpp %SANITIZE% %FLAGS% -o para.exe

cd ..