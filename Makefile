CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic -g
LDLIBS   := -lsqlite3

BUILD    := build
BIN      := $(BUILD)/tsp-analyzer

SOURCES  := $(wildcard src/*.cpp)
OBJECTS  := $(patsubst src/%.cpp,$(BUILD)/%.o,$(SOURCES))

# Parametros qeu podemos sobreescribir en la linea de comandos

.PHONY: all run clean

all: $(BIN)

$(BIN): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDLIBS)

$(BUILD)/%.o: src/%.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD):
	mkdir -p $(BUILD)

# Genera la base de datos, solo se rehace si tsp.sql cambió.
tsp.db: tsp.sql
	rm -f $@
	sqlite3 $@ < $<

# Compila la base si hace falta y corre una instancia de ejemplo.
run: $(BIN) tsp.db
	./$(BIN) instancias/input-150.tsp

clean:
	rm -rf $(BUILD)