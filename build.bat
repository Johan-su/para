@echo off
set CLANG=clang++
set WARNINGS=-Wall -Wpedantic -Wextra -Wconversion -Wshadow -Wno-c++20-designator -Wno-c++17-extensions -Wno-gnu-anonymous-struct -Wno-nested-anon-types
set FLAGS=-O0 -D _DEBUG -D _CRT_SECURE_NO_WARNINGS  -g %WARNINGS%

set SANITIZE=-fsanitize=address -fsanitize=undefined
set WINLIB= -l User32.lib -l Gdi32.lib -l Shell32.lib -l Winmm.lib

if not exist build mkdir build

cd ./build

@REM %CLANG% ../src/main.cpp %FLAGS% %WINLIB% -l opengl32 -l ../lib/raylib.lib  -I ../lib/include -o para.exe
%CLANG% -c ../src/main.cpp %FLAGS% -I ../lib/include
%CLANG% main.o -l ../lib/raylib.lib %FLAGS% %WINLIB% -o para.exe
@REM %CLANG% ../src/parse.cpp %SANITIZE% %FLAGS% -o para.exe

cd ..