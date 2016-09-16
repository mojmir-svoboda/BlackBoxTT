rem this file is intended for usage from cmd.exe or from explorer (clicking on it)
@echo off

pushd %~dp0

set PATH=C:\Program Files (x86)\CMake\bin;C:\Program Files\CMake\bin;%PATH%

set MSCVER=14

set INSTDIR=%CD%\vs%MSCVER%.64\Release
mkdir _projects.vs%MSCVER%.64
cd _projects.vs%MSCVER%.64
call "C:\Program Files (x86)\Microsoft Visual Studio %MSCVER%.0\VC\vcvarsall.bat" x86_amd64
if %errorlevel% neq 0 goto TERM
cmake -G "Visual Studio %MSCVER% Win64" -DOPTION_BUILD_SHARED=ON -DCMAKE_INSTALL_PREFIX:PATH=%INSTDIR% ..
if %errorlevel% neq 0 goto TERM
devenv logging.sln /build RelWithDebInfo /project INSTALL
if %errorlevel% neq 0 goto TERM
cd ..

set INSTDIR=%CD%\vs%MSCVER%.64\Debug
mkdir _projects.vs%MSCVER%.64.Debug
cd _projects.vs%MSCVER%.64.Debug
call "C:\Program Files (x86)\Microsoft Visual Studio %MSCVER%.0\VC\vcvarsall.bat" x86_amd64
if %errorlevel% neq 0 goto TERM
cmake -G "Visual Studio %MSCVER% Win64" -DOPTION_BUILD_SHARED=ON -DCMAKE_INSTALL_PREFIX:PATH=%INSTDIR% ..
if %errorlevel% neq 0 goto TERM
devenv logging.sln /build Debug /project INSTALL
if %errorlevel% neq 0 goto TERM
cd ..


set INSTDIR=%CD%\vs%MSCVER%.32\Release
mkdir _projects.vs%MSCVER%.32
cd _projects.vs%MSCVER%.32
call "C:\Program Files (x86)\Microsoft Visual Studio %MSCVER%.0\VC\vcvarsall.bat" x86
if %errorlevel% neq 0 goto TERM
cmake -G "Visual Studio %MSCVER%" -DOPTION_BUILD_SHARED=ON -DCMAKE_INSTALL_PREFIX:PATH=%INSTDIR% ..
if %errorlevel% neq 0 goto TERM
devenv logging.sln /build RelWithDebInfo /project INSTALL
if %errorlevel% neq 0 goto TERM
cd ..

set INSTDIR=%CD%\vs%MSCVER%.32\Debug
mkdir _projects.vs%MSCVER%.32.Debug
cd _projects.vs%MSCVER%.32.Debug
call "C:\Program Files (x86)\Microsoft Visual Studio %MSCVER%.0\VC\vcvarsall.bat" x86
if %errorlevel% neq 0 goto TERM
cmake -G "Visual Studio %MSCVER%" -DOPTION_BUILD_SHARED=ON -DCMAKE_INSTALL_PREFIX:PATH=%INSTDIR% ..
if %errorlevel% neq 0 goto TERM
devenv logging.sln /build Debug /project INSTALL
if %errorlevel% neq 0 goto TERM
cd ..

goto NOPAUSE

:TERM
pause

:NOPAUSE
popd
