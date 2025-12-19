#!/bin/bash

echo "========================================"
echo "  Configuración automática de SFML"
echo "========================================"
echo ""

# Verificar si SFML ya está instalado
if [ -d "SFML-2.6.2" ]; then
    echo "SFML ya está instalado en: SFML-2.6.2"
    echo ""
    echo "Si quieres reinstalar, elimina la carpeta SFML-2.6.2 y ejecuta este script de nuevo."
    exit 0
fi

echo "Descargando SFML 2.6.2 para MinGW (GCC 13.1.0)..."
echo ""

# URL de descarga de SFML (GitHub Releases)
SFML_URL="https://github.com/SFML/SFML/releases/download/2.6.2/SFML-2.6.2-windows-gcc-13.1.0-mingw-64-bit.zip"
SFML_ZIP="SFML-2.6.2.zip"

# Descargar SFML usando curl o wget
echo "Descargando desde GitHub..."
if command -v curl &> /dev/null; then
    curl -L -o "$SFML_ZIP" "$SFML_URL"
elif command -v wget &> /dev/null; then
    wget -O "$SFML_ZIP" "$SFML_URL"
else
    echo ""
    echo "ERROR: No se encontró curl ni wget."
    echo "Instala uno de estos comandos o descarga SFML manualmente desde:"
    echo "https://github.com/SFML/SFML/releases/tag/2.6.2"
    echo ""
    exit 1
fi

if [ $? -ne 0 ]; then
    echo ""
    echo "ERROR: No se pudo descargar SFML."
    echo "Verifica tu conexión a internet o descarga manualmente desde:"
    echo "https://github.com/SFML/SFML/releases/tag/2.6.2"
    echo ""
    exit 1
fi

echo ""
echo "Descarga completada. Descomprimiendo..."
echo ""

# Descomprimir usando unzip
if command -v unzip &> /dev/null; then
    unzip -q "$SFML_ZIP"
else
    echo "ERROR: No se encontró el comando unzip."
    echo "Instala unzip o descomprime manualmente el archivo $SFML_ZIP"
    echo ""
    exit 1
fi

if [ $? -ne 0 ]; then
    echo ""
    echo "ERROR: No se pudo descomprimir SFML."
    echo "Intenta descomprimir manualmente el archivo $SFML_ZIP"
    echo ""
    exit 1
fi

# Renombrar la carpeta descomprimida
for dir in SFML-2.6.2-*; do
    if [ -d "$dir" ]; then
        mv "$dir" "SFML-2.6.2"
        break
    fi
done

# Eliminar el archivo ZIP
rm "$SFML_ZIP"

echo ""
echo "========================================"
echo "  SFML instalado correctamente!"
echo "========================================"
echo ""
echo "Ubicación: $(pwd)/SFML-2.6.2"
echo ""
echo "Ahora puedes compilar y ejecutar el juego con:"
echo "  ./run.sh"
echo ""

