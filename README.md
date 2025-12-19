# Pong - C++ con SFML 2.6.2

Recreación moderna del clásico juego Pong con **sistema de IA avanzado**, redes neuronales, algoritmos genéticos y torneos evolutivos. Incluye oponente AI con Q-Learning, sistema de entrenamiento poblacional y persistencia de modelos.

## Inicio Rápido

```bash
# Clonar el repositorio
git clone https://github.com/TU_USUARIO/game-of-pong.git
cd game-of-pong

# Compilar y ejecutar (SFML se descarga automáticamente)
./run.bat    # Windows con Git Bash
# o
./run.sh     # Linux/MSYS2
```

**Solo necesitas g++ instalado.** SFML se descarga automáticamente (~30 MB) la primera vez.

## Requisitos

- **g++ (MinGW)** con soporte C++17
- **Git Bash** - Incluido con [Git for Windows](https://git-scm.com/)
- **Conexión a internet** - Solo la primera vez para descargar SFML

## Características

- **Oponente AI con red neuronal** - Q-Learning con exploración/explotación
- **Sistema de torneos evolutivos** - Poblaciones de IA compitiendo y evolucionando
- **Algoritmos genéticos** - Selección, crossover y mutación de hiperparámetros
- **Persistencia de modelos** - Guardar/cargar redes neuronales entrenadas
- **Instalación automática de SFML** - Sin configuración manual
- **Menú interactivo** - Navegación con mouse y teclado
- **Sistema de pausa** - Control total del flujo del juego
- **Física realista** - Colisiones y movimiento fluido con delta time

## Controles

| Tecla     | Acción                 |
| --------- | ---------------------- |
| **W**     | Mover paleta arriba    |
| **S**     | Mover paleta abajo     |
| **ESC**   | Pausar juego/menú      |
| **SPACE** | Pausar/reanudar torneo |
| **Mouse** | Navegar menús          |
| **Enter** | Seleccionar opción     |

## Opciones de Compilación

### Opción 1: run.bat/run.sh (Recomendado)

```bash
./run.bat    # Windows
./run.sh     # Linux/MSYS2
```

El script automáticamente detecta SFML, descarga si es necesario, compila el código, copia DLLs y ejecuta el juego.

### Opción 2: Makefile

```bash
make        # Compilar
make run    # Compilar y ejecutar
make clean  # Limpiar binarios
```

## Estructura del Proyecto

```
game-of-pong/
├── assets/                  # Recursos (imágenes, fuentes)
├── src/                     # Código fuente
│   ├── Game.h              # Game loop, estados, integración de torneos
│   ├── AIPlayer.h          # Oponente AI con red neuronal
│   ├── NeuralNetwork.h     # Implementación de red neuronal feedforward
│   ├── QLearningAgent.h    # Agente de Q-Learning
│   ├── TournamentManager.h # Orquestación de torneos evolutivos
│   ├── AIPopulation.h      # Gestión de población y algoritmos genéticos
│   ├── TournamentMatch.h   # Simulador de partidas (visual/headless)
│   ├── ModelSaver.h        # Persistencia de modelos
│   ├── Ball.h              # Física de la pelota
│   ├── Player.h            # Lógica del jugador
│   ├── Menu.h              # Sistema de menús
│   ├── HUD.h               # Interfaz de usuario y estadísticas
│   ├── Collisions.h        # Detección de colisiones AABB
│   └── Utils.h             # Utilidades matemáticas
├── models/                  # Modelos guardados (generados)
├── main.cpp                # Punto de entrada
├── run.bat/.sh             # Script de compilación automática
└── TOURNAMENT_SYSTEM.md    # Documentación detallada del sistema de torneos
```

## Sistema de Torneos

El juego incluye un sistema completo de torneos evolutivos donde poblaciones de IA compiten y evolucionan:

- **Algoritmo genético** - Selección elitista, crossover uniforme, mutación adaptativa
- **Round-robin tournaments** - Cada IA juega contra todas las demás
- **Evolución de hiperparámetros** - Learning rate, epsilon decay, tamaño de capa oculta, etc.
- **Evaluación de fitness** - Combinación de win rate y calidad de aprendizaje
- **Guardado automático** - Top 3 modelos por generación

Ver [TOURNAMENT_SYSTEM.md](TOURNAMENT_SYSTEM.md) para documentación completa de arquitectura, configuración y uso.

## Tecnologías

- **Lenguaje:** C++17
- **Biblioteca gráfica:** [SFML 2.6.2](https://www.sfml-dev.org/)
- **Machine Learning:** Red neuronal feedforward, Q-Learning, algoritmos genéticos
- **Compilador:** GCC/MinGW 15.2.0+
- **Build System:** Bash Scripts + Makefile

## Licencia

Proyecto bajo [Licencia MIT](LICENSE). Libre uso y modificación.

SFML está licenciado bajo [zlib/png license](https://www.sfml-dev.org/license.php).
