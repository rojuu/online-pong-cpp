@echo off
SETLOCAL
set TARGET=x86

:: SDL2 libraries and include paths 
set SDL2_LIB=C:\SDL2-2.0.5

set SDL2_LIBDIR=%SDL2_LIB%\lib\%TARGET%
set SDL2_LIBINC=%SDL2_LIB%\include

x:
if not defined DevEnvDir (
call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" %TARGET%
)

set CommonCompilerFlags= -Zi /I%SDL2_LIBINC%
set CommonLinkerFlags=/incremental:no /opt:ref /LIBPATH:%SDL2_LIBDIR% SDL2.lib SDL2main.lib

mkdir build
pushd build
robocopy %SDL2_LIBDIR% . *.dll
del *.pdb > NUL 2> NUL
cl  %CommonCompilerFlags% ..\src\main.cpp /link /subsystem:console %CommonLinkerFlags%
popd