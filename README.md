# 🎮 Pong - C++ con SFML 2.6.2

Recreación del clásico juego Pong en C++ usando SFML. Incluye menús interactivos, sistema de colisiones, y **instalación automática de dependencias**.

##  Inicio Rápido

```bash
# Clonar el repositorio
git clone https://github.com/TU_USUARIO/game-of-pong.git
cd game-of-pong

# Compilar y ejecutar (SFML se descarga automáticamente)
./run.bat    # Windows con Git Bash
# o
./run.sh     # Linux/MSYS2
```

**¡Solo necesitas tener g++ instalado!** SFML se descarga automáticamente (~30 MB) la primera vez.

---

##  Requisitos

- **g++ (MinGW)** - [¿Cómo instalar?](#instalación-de-g)
- **Git Bash** - Incluido con [Git for Windows](https://git-scm.com/)
- **Conexión a internet** - Solo la primera vez para descargar SFML

---

##  Características

- Instalación automática de SFML  
- Menú principal interactivo (mouse/teclado)  
- Sistema de pausa  
- Física de pelota y colisiones  
- Movimiento fluido con delta time  
- HUD con marcador  
- Fuentes y sprites personalizados

---

##  Controles

| Tecla     | Acción              |
| --------- | ------------------- |
| **W**     | Mover paleta arriba |
| **S**     | Mover paleta abajo  |
| **ESC**   | Pausar juego        |
| **Mouse** | Navegar menús       |
| **Enter** | Seleccionar opción  |

---

## 🔧 Opciones de Compilación

### Opción 1: run.bat/run.sh (Recomendado)

```bash
./run.bat    # Windows
./run.sh     # Linux/MSYS2
```

El script automáticamente:

- Detecta y descarga SFML si no existe
- Compila el código
- Copia las DLLs necesarias
- Ejecuta el juego

##  Estructura del Proyecto

```
game-of-pong/
├── assets/           # Recursos (imágenes, fuentes)
├── src/              # Código fuente (headers)
│   ├── Ball.h       # Física de la pelota
│   ├── Player.h     # Lógica del jugador
│   ├── Game.h       # Game loop principal
│   ├── Menu.h       # Sistema de menús
│   ├── HUD.h        # Interfaz de usuario
│   ├── Collisions.h # Detección de colisiones
│   └── Utils.h      # Utilidades matemáticas
├── main.cpp         # Punto de entrada
├── run.bat/.sh      # Script de compilación
├── setup.bat/.sh    # Instalador de SFML
└── Makefile         # Build system alternativo
```

##  Mejoras Futuras

- [ ] IA para oponente (paleta derecha)
- [ ] Sistema de puntuación funcional
- [ ] Detección de goles
- [ ] Modo 2 jugadores local
- [ ] Efectos de sonido
- [ ] Física de colisiones mejorada
- [ ] Pantalla de victoria/derrota

---

##  Arquitectura del Código

| Archivo        | Descripción                          |
| -------------- | ------------------------------------ |
| `Game.h`       | Game loop, estados, lógica principal |
| `Player.h`     | Paleta del jugador con input         |
| `Ball.h`       | Física y movimiento de la pelota     |
| `Menu.h`       | Sistema de menús interactivos        |
| `HUD.h`        | Interfaz de puntuación               |
| `Collisions.h` | Detección de colisiones AABB         |
| `Utils.h`      | Vector2f y funciones matemáticas     |

---

##  Tecnologías

- **Lenguaje:** C++17
- **Biblioteca:** [SFML 2.6.2](https://www.sfml-dev.org/)
- **Compilador:** GCC/MinGW
- **Build System:** Bash Scripts + Makefile

---

##  Licencia

Proyecto bajo [Licencia MIT](LICENSE). Libre uso y modificación.

SFML está licenciado bajo [zlib/png license](https://www.sfml-dev.org/license.php).

---

##  Notas

- **Compatibilidad:** Windows con MinGW/MSYS2
- **Instalación:** Totalmente automática con `./run.bat`
- **Repositorio ligero:** SFML no se incluye en Git (~30 MB se descargan automáticamente)

---
