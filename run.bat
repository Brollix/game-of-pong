@echo off
echo ========================================
echo   Compilando Pong con SFML 2.6.2
echo ========================================

REM Verificar si SFML esta instalado
if not exist "SFML-2.6.2" (
    echo.
    echo SFML no encontrado en el proyecto.
    echo.
    echo Deseas descargar e instalar SFML automaticamente? [S/N]
    set /p RESPUESTA="> "
    
    if /i "%RESPUESTA%"=="S" (
        echo.
        echo Ejecutando instalacion automatica...
        call setup.bat
        if errorlevel 1 (
            echo.
            echo No se pudo instalar SFML automaticamente.
            pause
            exit /b 1
        )
    ) else (
        echo.
        echo Instalacion cancelada.
        echo.
        echo Para instalar SFML manualmente:
        echo 1. Ejecuta: ./setup.bat
        echo 2. O descarga desde: https://github.com/SFML/SFML/releases/tag/2.6.2
        echo.
        pause
        exit /b 1
    )
)

REM Configuración de rutas SFML
set SFML_PATH=SFML-2.6.2

echo Usando SFML en: %SFML_PATH%
echo.

REM Crear carpeta bin si no existe
if not exist "bin" mkdir bin

echo Compilando...

REM Compilar con g++
g++ -c main.cpp -I"%SFML_PATH%\include" -std=c++17 -o bin\main.o

if errorlevel 1 (
    echo.
    echo ERROR: Fallo la compilacion.
    echo Verifica que g++ este instalado y accesible desde la terminal.
    echo.
    echo Para instalar MinGW/g++:
    echo 1. Instala MSYS2 desde: https://www.msys2.org/
    echo 2. Ejecuta: pacman -S mingw-w64-x86_64-gcc
    echo.
    pause
    exit /b 1
)

echo Enlazando...

REM Enlazar (modo dinámico)
g++ bin\main.o -o bin\pong.exe -L"%SFML_PATH%\lib" -lsfml-graphics -lsfml-window -lsfml-system -lopengl32 -lwinmm -lgdi32

if errorlevel 1 (
    echo.
    echo ERROR: Fallo el enlazado.
    echo Verifica que las librerias de SFML esten en %SFML_PATH%\lib
    pause
    exit /b 1
)

REM Copiar DLLs necesarias
if exist "%SFML_PATH%\bin\sfml-graphics-2.dll" (
    echo Copiando DLLs de SFML...
    copy /Y "%SFML_PATH%\bin\sfml-graphics-2.dll" bin\ > nul
    copy /Y "%SFML_PATH%\bin\sfml-window-2.dll" bin\ > nul
    copy /Y "%SFML_PATH%\bin\sfml-system-2.dll" bin\ > nul
)

echo.
echo ========================================
echo   Compilacion exitosa!
echo ========================================
echo.
echo Ejecutando juego...
echo.

REM Ejecutar el juego desde la carpeta raiz para que encuentre assets
cd /d "%~dp0"
bin\pong.exe

echo.
echo El juego se cerro.
