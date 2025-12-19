#!/bin/bash

echo "========================================"
echo "  Compilando Pong con SFML 2.6.2"
echo "========================================"

# Verificar si SFML está instalado
if [ ! -d "SFML-2.6.2" ]; then
    echo ""
    echo "SFML no encontrado en el proyecto."
    echo ""
    echo "¿Deseas descargar e instalar SFML automáticamente? [s/N]"
    read -p "> " RESPUESTA
    
    if [[ "$RESPUESTA" =~ ^[Ss]$ ]]; then
        echo ""
        echo "Ejecutando instalación automática..."
        bash setup.sh
        if [ $? -ne 0 ]; then
            echo ""
            echo "No se pudo instalar SFML automáticamente."
            exit 1
        fi
    else
        echo ""
        echo "Instalación cancelada."
        echo ""
        echo "Para instalar SFML manualmente:"
        echo "1. Ejecuta: ./setup.sh"
        echo "2. O descarga desde: https://github.com/SFML/SFML/releases/tag/2.6.2"
        echo ""
        exit 1
    fi
fi

# Configuración de rutas SFML
SFML_PATH="SFML-2.6.2"

echo "Usando SFML en: $SFML_PATH"
echo ""

# Crear carpeta bin si no existe
mkdir -p bin

echo "Compilando..."

# Compilar con g++
g++ -c main.cpp -I"$SFML_PATH/include" -std=c++17 -o bin/main.o

if [ $? -ne 0 ]; then
    echo ""
    echo "ERROR: Falló la compilación."
    echo "Verifica que g++ esté instalado y accesible desde la terminal."
    echo ""
    echo "Para instalar MinGW/g++:"
    echo "1. Instala MSYS2 desde: https://www.msys2.org/"
    echo "2. Ejecuta: pacman -S mingw-w64-x86_64-gcc"
    echo ""
    exit 1
fi

echo "Enlazando..."

# Enlazar (modo dinámico)
g++ bin/main.o -o bin/pong.exe -L"$SFML_PATH/lib" -lsfml-graphics -lsfml-window -lsfml-system -lopengl32 -lwinmm -lgdi32

if [ $? -ne 0 ]; then
    echo ""
    echo "ERROR: Falló el enlazado."
    echo "Verifica que las librerías de SFML estén en $SFML_PATH/lib"
    exit 1
fi

# Copiar DLLs necesarias
if [ -f "$SFML_PATH/bin/sfml-graphics-2.dll" ]; then
    echo "Copiando DLLs de SFML..."
    cp "$SFML_PATH/bin/sfml-graphics-2.dll" bin/
    cp "$SFML_PATH/bin/sfml-window-2.dll" bin/
    cp "$SFML_PATH/bin/sfml-system-2.dll" bin/
fi

echo ""
echo "========================================"
echo "  Compilación exitosa!"
echo "========================================"
echo ""
echo "Ejecutando juego..."
echo ""

# Ejecutar el juego
./bin/pong.exe

echo ""
echo "El juego se cerró."

