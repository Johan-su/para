@echo off
set WARNINGS=-Wall -Wpedantic -Wextra -Wconversion -Wshadow -Wimplicit-fallthrough -Wno-language-extension-token -Wno-nested-anon-types -Wno-gnu-anonymous-struct -Wno-gnu-zero-variadic-macro-arguments 
set FLAGS=-O0 -D _CRT_SECURE_NO_WARNINGS -fwrapv -fno-strict-aliasing  -g

set SANITIZE=-fsanitize=address -fsanitize=undefined
set WINLIBS= -l User32.lib -l Gdi32.lib -l Shell32.lib -l Winmm.lib

set LIBS=-l opengl32 -l lib/raylib.lib
set INCLUDES=-I lib/include

if not exist build mkdir build



if not exist build mkdir build
git rev-parse --short HEAD > build\version.txt
SET /P GIT_HASH= < build\version.txt


echo Building src...
clang++ -MJ build/build.json src/build.cpp -o build/para.exe -DGIT_HASH=0x%GIT_HASH% %FLAGS% %INCLUDES% %WARNINGS% %LIBS% %WINLIBS% 

echo [ > compile_commands.json
type build\build.json >> compile_commands.json
echo ] >> compile_commands.json
