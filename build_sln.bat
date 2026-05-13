@echo off
setlocal
cd /d "%~dp0"
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
if errorlevel 1 cmake -S . -B build -G "Visual Studio 16 2019" -A x64
if errorlevel 1 exit /b 1
cmake --build build --config Release
if errorlevel 1 exit /b 1
echo Solution: build\OpenGLWorld.sln
echo Executable: build\Release\OpenGLWorld.exe
