# Pong en C++ con SDL2 🎮

Este proyecto es una recreación del clásico juego Pong, desarrollado completamente en C++ utilizando la biblioteca SDL2. El objetivo fue implementar una versión funcional desde cero, incorporando menús interactivos, lógica de colisiones, control de estado del juego y renderizado de texto y sonido.

## 🎮 Características principales

- Menú principal interactivo con soporte para mouse.
- Juego pausable con menú de pausa clickeable.
- Movimiento fluido de la paleta del jugador.
- Colisiones entre pelota, bordes y paletas.
- Puntuación en pantalla.
- Efectos de sonido integrados.
- Renderizado de texto usando fuentes personalizadas.

## 🧱 Estructura del proyecto

- `main.cpp`: Punto de entrada del juego.
- `Game`: Clase principal que gestiona el ciclo de vida del juego y su estado.
- `Player`: Controla el comportamiento de la paleta del jugador.
- `Ball`: Implementa la lógica de movimiento y rebotes de la pelota.
- `Menu`: Maneja menús interactivos usando eventos del mouse.
- `Collisions`, `Utils`: Utilidades auxiliares para colisiones y lógica común.

Los recursos del juego (imágenes, sonidos, fuentes) se encuentran en la carpeta `assets`.

## 🔄 Próximas mejoras previstas

- Implementación de una IA básica para el oponente.
- Modo multijugador local (2 jugadores).
- Pantalla de inicio y final de partida.
- Soporte para joystick.
- Portabilidad a otras plataformas.

---

> Este proyecto está diseñado y probado únicamente en Windows.  
> Para ver detalles técnicos y requisitos, consultá el archivo [Compatibilidad y Requisitos](Compatibilidad-y-Requisitos.md).
