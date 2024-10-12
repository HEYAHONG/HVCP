@echo off
set CURRENT_DIR=%~dp0
set PATH=%CURRENT_DIR%;%PATH%
pushd "%CURRENT_DIR%"
"%CURRENT_DIR%\devcon" remove HVCPDriver.inf UMDF\HVCPDriver
popd
