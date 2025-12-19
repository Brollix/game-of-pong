@echo off
echo ========================================
echo   Configuracion automatica de SFML
echo ========================================
echo.

REM Verificar si SFML ya esta instalado
if exist "SFML-2.6.2" (
    echo SFML ya esta instalado en: SFML-2.6.2
    echo.
    echo Si quieres reinstalar, elimina la carpeta SFML-2.6.2 y ejecuta este script de nuevo.
    pause
    exit /b 0
)

echo Descargando SFML 2.6.2 para MinGW (GCC 13.1.0)...
echo.

REM URL de descarga de SFML (GitHub Releases)
set SFML_URL=https://github.com/SFML/SFML/releases/download/2.6.2/SFML-2.6.2-windows-gcc-13.1.0-mingw-64-bit.zip
set SFML_ZIP=SFML-2.6.2.zip

REM Descargar SFML usando curl (incluido en Git Bash y Windows 10+)
echo Descargando desde GitHub...
curl -L -o %SFML_ZIP% %SFML_URL%

if errorlevel 1 (
    echo.
    echo ERROR: No se pudo descargar SFML.
    echo Verifica tu conexion a internet o descarga manualmente desde:
    echo https://github.com/SFML/SFML/releases/tag/2.6.2
    echo.
    pause
    exit /b 1
)

echo.
echo Descarga completada. Descomprimiendo...
echo.

REM Descomprimir usando PowerShell (incluido en Windows 10+)
powershell -Command "Expand-Archive -Path '%SFML_ZIP%' -DestinationPath '.' -Force"

if errorlevel 1 (
    echo.
    echo ERROR: No se pudo descomprimir SFML.
    echo Intenta descomprimir manualmente el archivo %SFML_ZIP%
    echo.
    pause
    exit /b 1
)

REM Renombrar la carpeta descomprimida
for /d %%i in (SFML-2.6.2-*) do (
    move "%%i" "SFML-2.6.2" > nul
)

REM Eliminar el archivo ZIP
del %SFML_ZIP%

echo.
echo ========================================
echo   SFML instalado correctamente!
echo ========================================
echo.
echo Ubicacion: %CD%\SFML-2.6.2
echo.
echo Ahora puedes compilar y ejecutar el juego con:
echo   ./run.bat
echo.
pause

