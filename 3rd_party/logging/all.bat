rem this file is intended for usage from cmd.exe or from explorer (clicking on it)
@echo off

pushd %~dp0

call build_all_vs2013.bat
call build_all_vs2015.bat
call build_all_vs2017.bat
call install.bat

popd
