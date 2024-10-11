@echo off
set CURRENT_DIR=%~dp0
set PATH=%CURRENT_DIR%;%PATH%
"%CURRENT_DIR%\devcon" remove HVCPDriver.inf UMDF\HVCPDriver
