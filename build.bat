@echo off
set RUSTC=rustc

if not exist build mkdir build

cd ./build

%RUSTC% ../src/para.rs -o para.exe

cd ..