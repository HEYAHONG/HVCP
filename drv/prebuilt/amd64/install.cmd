@echo off
set CURRENT_DIR=%~dp0
set PATH="%CURRENT_DIR%";%PATH%
pushd "%CURRENT_DIR%"
"%CURRENT_DIR%\devcon" install HVCPDriver.inf  UMDF\HVCPDriver
popd
