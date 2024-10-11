@echo off
set CURRENT_DIR=%~dp0
set PATH="%CURRENT_DIR%";%PATH%
"%CURRENT_DIR%\devcon" install HVCPDriver.inf  UMDF\HVCPDriver
