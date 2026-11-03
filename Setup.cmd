@echo off
setlocal
cd /d "%~dp0"
set PYTHONUTF8=1
where py >nul 2>&1
if not errorlevel 1 (
    py -3 "%~dp0tools\setup_assets.py"
    goto finished
)
where python >nul 2>&1
if not errorlevel 1 (
    python "%~dp0tools\setup_assets.py"
    goto finished
)
echo Python is required for the PC setup.
echo Install Python 3.10 or newer from https://www.python.org/downloads/
echo Then double-click Setup.cmd again.
:finished
echo.
pause
