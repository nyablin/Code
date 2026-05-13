@echo off
setlocal
cd /d "%~dp0"
set BUILD_DIR=build_vs
cmake -S . -B "%BUILD_DIR%"
if errorlevel 1 exit /b 1
> "%BUILD_DIR%\OpenGLWorld.slnx" (
echo ^<Solution^>
echo   ^<Configurations^>
echo     ^<Platform Name="x64" /^>
echo   ^</Configurations^>
echo   ^<Project Path="OpenGLWorld.vcxproj" Id="845e92b7-48be-3561-a2fb-9ab224e71e5a" /^>
echo   ^<Project Path="ZERO_CHECK.vcxproj" Id="6968d9c7-701f-3309-9fd8-27554f037a75" /^>
echo   ^<Project Path="_deps/glfw-build/src/glfw.vcxproj" Id="c1160f31-9bfb-3508-b7f8-6bee1d151e27" /^>
echo ^</Solution^>
)
cmake --build "%BUILD_DIR%" --config Release
if errorlevel 1 exit /b 1
if exist "%BUILD_DIR%\OpenGLWorld.sln" echo Solution: %BUILD_DIR%\OpenGLWorld.sln
if exist "%BUILD_DIR%\OpenGLWorld.slnx" echo Solution: %BUILD_DIR%\OpenGLWorld.slnx
echo Executable: bin\OpenGLWorld.exe
