## 🪟 Compatibilidad

Este juego fue creado y probado en **Windows**, y está pensado para usarse en ese sistema.

## 🔧 ¿Qué necesitás?

- Visual Studio 2022 o similar, con soporte para C++
- Las librerías SDL2 (y sus complementos) instaladas
- Los archivos **DLL** de SDL deben estar en la misma carpeta que el `.exe`

## ▶️ ¿Cómo se corre?

1. Abrí el proyecto en Visual Studio (`sdl-pong-game.sln`)
2. Compilá (`Ctrl + Shift + B`)
3. Asegurate de copiar los archivos `.dll` de SDL (como `SDL2.dll`, `SDL2_image.dll`, etc.) a la misma carpeta donde se genera el `.exe` (por ejemplo, `Debug/` o `Release/`)
4. Ejecutá (`F5` o doble clic en el `.exe`)

La carpeta `assets/` también debe estar al lado del ejecutable para que el juego pueda cargar las imágenes, sonidos y fuentes.

> 💡 Si al ejecutar el juego ves errores de "missing DLL", es porque los archivos `.dll` no están en la misma carpeta que el `.exe`.
