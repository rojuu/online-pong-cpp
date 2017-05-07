@echo off
SETLOCAL
set TARGET=x86

:: SDL2 libraries and include paths 

if not defined DevEnvDir (
call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" %TARGET%
)

set SDL_INC=%SDL2%\include
set SDL_LIB=%SDL2%\lib\%TARGET%
set ENET_INC=%ENET%\include
set ENET_LIB=%ENET%

set CommonCompilerFlags=/Zi /EHsc /nologo /FC /I%SDL_INC% /I%ENET_INC%

set CommonLinkerFlags=/out:online-pong.exe /DEBUG /LIBPATH:%SDL_LIB% /LIBPATH:%ENET_LIB% SDL2.lib SDL2main.lib enet.lib winmm.lib ws2_32.lib

mkdir build
pushd build
robocopy %SDL_LIB% . *.dll
del *.pdb > NUL 2> NUL
cl  %CommonCompilerFlags% ..\src\*.cpp /link /subsystem:console %CommonLinkerFlags%
popd