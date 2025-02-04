@echo off
set CLANG=clang++
set WARNINGS=-Wall -Wpedantic -Wextra -Wconversion -Wshadow -Wno-c++20-designator -Wno-c++17-extensions -Wno-gnu-anonymous-struct -Wno-nested-anon-types
set FLAGS=-O0 -D _CRT_SECURE_NO_WARNINGS -fwrapv -fno-strict-aliasing  -g %WARNINGS%

set SANITIZE=-fsanitize=address -fsanitize=undefined
set WINLIB= -l User32.lib -l Gdi32.lib -l Shell32.lib -l Winmm.lib

if not exist build mkdir build

pushd build

%CLANG% -E -Wno-macro-redefined ../src/meta.cpp > ../src/generated.cpp
%CLANG% %FLAGS% -l ../lib/raylib.lib %WINLIB% -I ../lib/include  ../src/main.cpp -o para.exe

popd build