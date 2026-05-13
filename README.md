# OpenGLWorld
Небольшой C++/OpenGL проект с процедурной voxel-генерацией, физикой игрока, блоками и инвентарём.
## Возможности
- Генерация блокового мира чанками вокруг игрока.
- Холмы, камень, земля, трава, снег, ресурсы и простые деревья.
- Ломание и постановка блоков через raycast от камеры.
- Физика игрока: гравитация, прыжок, sprint, коллизия с блоками и ступеньки.
- Небо с отдельным шейдером, градиентом, свечением и диском солнца.
- Инвентарь из пяти слотов и hotbar.
- Переключение wireframe-режима.
## Сборка
Требования:
- CMake 3.24 или новее.
- Компилятор C++17.
- Драйвер с поддержкой OpenGL 3.3.
Команды:
```powershell
cmake -S . -B build
cmake --build build --config Release
.\bin\OpenGLWorld.exe
```
Или через готовый батник для Visual Studio solution:
```powershell
.\build_sln.bat
```
Он создаёт `build_vs\OpenGLWorld.sln`, `build_vs\OpenGLWorld.slnx` и `bin\OpenGLWorld.exe`.
Установка инструментов сборки через `winget`:
```powershell
.\install_framework.bat
```
Для Ninja или Makefile путь обычно будет:
```powershell
.\build\OpenGLWorld.exe
```
Быстрая проверка OpenGL-контекста без открытия игрового окна:
```powershell
.\bin\OpenGLWorld.exe --smoke
```
## Управление
- `W`, `A`, `S`, `D` - движение камеры.
- `Space` - прыжок.
- `Shift` - ускорение.
- Мышь - обзор.
- `1`-`5` - выбрать слот инвентаря.
- ЛКМ - сломать блок.
- ПКМ - поставить выбранный блок.
- `R` - пересоздать мир и сбросить инвентарь.
- `F` - wireframe.
- `Esc` - выход.
## Структура
```text
CMakeLists.txt
README.md
src/main.cpp
```
GLFW загружается через CMake `FetchContent`, поэтому отдельная установка GLFW не нужна.
