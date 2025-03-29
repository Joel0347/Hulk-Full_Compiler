# Makefile para compilador HULK en Windows con estructura src/include
# Versión compatible con cmd.exe

.PHONY: all build run clean

# Configuración de compilador
CC = g++
LEX = win_flex
YACC = win_bison
CFLAGS = -Iinclude -Wall -std=c++11
LDFLAGS = 

# Directorios
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)\obj

# Nombres de archivos
LEX_FILE = $(SRC_DIR)\hulk.l
YACC_FILE = $(SRC_DIR)\hulk.y
SRC_FILES = $(wildcard $(SRC_DIR)\*.cpp)
OBJ_FILES = $(patsubst $(SRC_DIR)\%.cpp,$(OBJ_DIR)\%.o,$(SRC_FILES))
OUTPUT = $(BUILD_DIR)\hulk.exe

# Reglas principales
all: build

build: dirs lexer parser $(OUTPUT)
	@echo Compilación completada

run: build
	@if exist "script.hulk" ( \
		$(OUTPUT) script.hulk \
	) else ( \
		echo Error: No se encontró script.hulk en el directorio actual \
	)

clean:
	@if exist "$(BUILD_DIR)" rmdir /s /q "$(BUILD_DIR)"
	@if exist "$(SRC_DIR)\lex.yy.c" del /q "$(SRC_DIR)\lex.yy.c"
	@if exist "$(SRC_DIR)\hulk.tab.c" del /q "$(SRC_DIR)\hulk.tab.c"
	@if exist "$(SRC_DIR)\hulk.tab.h" del /q "$(SRC_DIR)\hulk.tab.h"

# Reglas de construcción
dirs:
	@if not exist "$(BUILD_DIR)" mkdir "$(BUILD_DIR)"
	@if not exist "$(OBJ_DIR)" mkdir "$(OBJ_DIR)"

lexer:
	$(LEX) -o $(SRC_DIR)\lex.yy.c $(LEX_FILE)

parser:
	$(YACC) -d -o $(SRC_DIR)\hulk.tab.c $(YACC_FILE)
	@if exist "$(SRC_DIR)\hulk.tab.h" move /y "$(SRC_DIR)\hulk.tab.h" "$(INCLUDE_DIR)\hulk.tab.h"

$(OBJ_DIR)\%.o: $(SRC_DIR)\%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)\lex.yy.o: $(SRC_DIR)\lex.yy.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)\hulk.tab.o: $(SRC_DIR)\hulk.tab.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OUTPUT): $(OBJ_FILES) $(OBJ_DIR)\lex.yy.o $(OBJ_DIR)\hulk.tab.o
	$(CC) $(LDFLAGS) -o $@ $^

# Dependencias específicas
$(OBJ_DIR)\main.o: $(INCLUDE_DIR)\ast.h $(INCLUDE_DIR)\hulk.tab.h
$(OBJ_DIR)\ast.o: $(INCLUDE_DIR)\ast.h $(INCLUDE_DIR)\hulk.tab.h
$(SRC_DIR)\lex.yy.c: $(INCLUDE_DIR)\hulk.tab.h