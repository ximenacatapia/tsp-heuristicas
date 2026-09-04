CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic -g
LDLIBS   := -lsqlite3

BUILD    := build
BIN      := $(BUILD)/tsp-analyzer
TEST_BIN := $(BUILD)/tests

# Fuentes del sistema, sin main.cpp: el binario de pruebas las enlaza y no
# puede tener dos funciones main.
LIB_SRC   := $(filter-out src/main.cpp,$(wildcard src/*.cpp))
LIB_OBJ   := $(patsubst src/%.cpp,$(BUILD)/%.o,$(LIB_SRC))
TEST_SRC  := $(wildcard tests/*.cpp)
TEST_OBJ  := $(patsubst tests/%.cpp,$(BUILD)/%.o,$(TEST_SRC))

# Parámetros sobrescribibles desde la línea de comandos:
#
#   make run TSP=instancias/input-150.tsp
#   make run TSP=/otra/ruta/mia.tsp DB=/otra/ruta/tsp.db
#
# TSP no tiene valor por omisión: el Makefile no debe decidir qué instancia
# se corre.
TSP :=
DB  := tsp.db
SQL := tsp.sql

.PHONY: all run test clean

all: $(BIN)

$(BIN): $(LIB_OBJ) $(BUILD)/main.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDLIBS)

$(TEST_BIN): $(LIB_OBJ) $(TEST_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDLIBS)

$(BUILD)/%.o: src/%.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD)/%.o: tests/%.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD):
	mkdir -p $(BUILD)

# Genera la base a partir del volcado SQL. Sólo se rehace si el volcado
# cambió. No hace falta para las pruebas: ésas crean sus propias bases.
$(DB): $(SQL)
	rm -f $@
	sqlite3 $@ < $<

# Las pruebas no dependen de ninguna base ni instancia del repositorio.
test: $(TEST_BIN)
	./$(TEST_BIN)

run: $(BIN) $(DB)
	@if [ -z "$(TSP)" ]; then \
		echo "Specify the instance:  make run TSP=instances/input-40.tsp" ; \
		echo "Available in instances/:"; \
		ls -1 instancias/*.tsp 2>/dev/null | sed 's/^/  /' || echo "  (none)"; \
		exit 1; \
	fi
	./$(BIN) $(TSP) $(DB)

clean:
	rm -rf $(BUILD)