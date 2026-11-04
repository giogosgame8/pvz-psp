@echo off
setlocal
cd /d "%~dp0"
set PYTHONUTF8=1
if exist "%~dp0Setup.exe" (
    "%~dp0Setup.exe" %*
    exit /b
)
where py >nul 2>&1
if not errorlevel 1 (
    py -3 "%~dp0tools\setup_assets.py" %*
    goto finished
)
where python >nul 2>&1
if not errorlevel 1 (
    python "%~dp0tools\setup_assets.py" %*
    goto finished
)
echo Download the game ZIP from https://github.com/giogosgame8/pvz-psp/releases
echo Extract it and run Setup.exe. Python is not needed for that download.
echo This source-only folder needs Python 3.10 or newer for development.
:finished
echo.
pause
