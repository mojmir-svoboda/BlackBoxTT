rem this file is intended for usage from cmd.exe or from explorer (clicking on it)
@echo off

pushd %~dp0

set PATH=C:\Program Files (x86)\CMake\bin;%PATH%

set MSCVER=12
set BITS=64
copy _projects.vs%MSCVER%.%BITS%\Debug\logging.lib logging_x%BITS%_vc%MSCVER%_d.lib
copy _projects.vs%MSCVER%.%BITS%\RelWithDebInfo\logging.lib logging_x%BITS%_vc%MSCVER%.lib
set MSCVER=12
set BITS=32
copy _projects.vs%MSCVER%.%BITS%\Debug\logging.lib logging_vc%MSCVER%_d.lib
copy _projects.vs%MSCVER%.%BITS%\RelWithDebInfo\logging.lib logging_vc%MSCVER%.lib

set MSCVER=14
set BITS=64
copy _projects.vs%MSCVER%.%BITS%\Debug\logging.lib logging_x%BITS%_vc%MSCVER%_d.lib
copy _projects.vs%MSCVER%.%BITS%\RelWithDebInfo\logging.lib logging_x%BITS%_vc%MSCVER%.lib
set MSCVER=14
set BITS=32
copy _projects.vs%MSCVER%.%BITS%\Debug\logging.lib logging_vc%MSCVER%_d.lib
copy _projects.vs%MSCVER%.%BITS%\RelWithDebInfo\logging.lib logging_vc%MSCVER%.lib


goto NOPAUSE

:TERM
pause

:NOPAUSE
popd
