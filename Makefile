# Makefile para Pong SFML
# Para compilar desde bash: make && make run

CXX = g++
CXXFLAGS = -std=c++17 -Wall
TARGET = bin/pong.exe
SRC = main.cpp

# SFML siempre se instala localmente
SFML_PATH = SFML-2.6.2

INCLUDES = -I$(SFML_PATH)/include
LIBS = -L$(SFML_PATH)/lib -lsfml-graphics -lsfml-window -lsfml-system -lopengl32 -lwinmm -lgdi32

.PHONY: all run clean setup check-sfml

all: check-sfml $(TARGET)

# Verificar si SFML está instalado
check-sfml:
	@if [ ! -d "$(SFML_PATH)" ]; then \
		echo ""; \
		echo "SFML no encontrado."; \
		echo "Ejecutando instalación automática..."; \
		echo ""; \
		bash setup.sh; \
		if [ $$? -ne 0 ]; then \
			echo ""; \
			echo "Error: No se pudo instalar SFML automáticamente."; \
			exit 1; \
		fi; \
	fi

$(TARGET): $(SRC)
	@echo "========================================="
	@echo "  Compilando Pong con SFML 2.6.2"
	@echo "========================================="
	@mkdir -p bin
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SRC) -o $(TARGET) $(LIBS)
	@if [ -f "$(SFML_PATH)/bin/sfml-graphics-2.dll" ]; then \
		echo "Copiando DLLs de SFML..."; \
		cp $(SFML_PATH)/bin/sfml-graphics-2.dll bin/ 2>/dev/null || true; \
		cp $(SFML_PATH)/bin/sfml-window-2.dll bin/ 2>/dev/null || true; \
		cp $(SFML_PATH)/bin/sfml-system-2.dll bin/ 2>/dev/null || true; \
	fi
	@echo "Compilación exitosa!"

run: $(TARGET)
	@echo "Ejecutando juego..."
	@./$(TARGET)

setup:
	@bash setup.sh

clean:
	rm -rf bin/*.o bin/*.exe

clean-all: clean
	rm -rf SFML-2.6.2 SFML-*.zip

help:
	@echo "Comandos disponibles:"
	@echo "  make          - Compila el juego (instala SFML si es necesario)"
	@echo "  make run      - Compila y ejecuta el juego"
	@echo "  make setup    - Instala SFML manualmente"
	@echo "  make clean    - Limpia archivos compilados"
	@echo "  make clean-all - Limpia todo incluyendo SFML"
	@echo "  make help     - Muestra esta ayuda"
