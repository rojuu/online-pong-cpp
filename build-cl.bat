@echo off
SETLOCAL
set TARGET=x86

if not defined DevEnvDir (
call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" %TARGET%
)

set SDL_INC=%SDL2%\include
set SDL_LIB=%SDL2%\lib\%TARGET%
set ENET_INC=%ENET%\include
set ENET_LIB=%ENET%

set ENET_LIBFILE=""
IF %TARGET%==x86 ( 
    set ENET_LIBFILE=enet.lib
)
IF %TARGET%==x64 (
    set ENET_LIBFILE=enet64.lib
)

set CommonCompilerFlags=/Zi /EHsc /nologo /FC /I%SDL_INC% /I%ENET_INC%

set CommonLinkerFlags=/DEBUG /LIBPATH:%SDL_LIB% /LIBPATH:%ENET_LIB% SDL2.lib SDL2main.lib %ENET_LIBFILE% winmm.lib ws2_32.lib

mkdir build_cl
pushd build_cl
robocopy %SDL_LIB% . *.dll
cl %CommonCompilerFlags% ..\src\*.cpp /link /subsystem:console %CommonLinkerFlags% /out:online-pong.exe
popd