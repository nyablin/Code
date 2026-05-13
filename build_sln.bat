@echo off
setlocal
cd /d "%~dp0"
cmake -S . -B build -G "Visual Studio 16 2019" -A x64
if errorlevel 1 exit /b %errorlevel%
cmake --build build --config Release
if errorlevel 1 exit /b %errorlevel%
echo Solution: build\OpenGLWorld.sln
echo Executable: build\Release\OpenGLWorld.exe
