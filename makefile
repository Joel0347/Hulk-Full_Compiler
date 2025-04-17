CC = clang
CFLAGS = -Wall -g -I. $(shell llvm-config --cflags) -O0
LDFLAGS = $(shell llvm-config --ldflags --libs core) -lm
LEXFLAGS = -w
YFLAGS = -d -y -v
LEX = flex
YACC = bison
EXEC = compilador

SRC_DIR = .
AST_DIR = ast
CODE_GEN_DIR = code_generation
LEXER_DIR = lexer
PARSER_DIR = parser

.PHONY: all build run clean

all: build

build: $(EXEC)
	@./$(EXEC)

$(EXEC): lex.yy.o y.tab.o $(AST_DIR)/ast.o $(SRC_DIR)/main.o $(CODE_GEN_DIR)/llvm_gen.o
	@echo "🔗 Enlazando ejecutable..."
	@$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "🔄 Compiling..."

y.tab.c y.tab.h: $(PARSER_DIR)/parser.y
	@echo "Generating parser..."
	@$(YACC) $(YFLAGS) $< || (echo "Bison failed to process parser.y"; exit 1)

lex.yy.c: $(LEXER_DIR)/lexer.l y.tab.h
	@echo "Generating lexer..."
	@$(LEX) $(LEXFLAGS) $< || (echo "Flex failed to process lexer.l"; exit 1)

$(CODE_GEN_DIR)/llvm_gen.o: $(CODE_GEN_DIR)/llvm_gen.c $(CODE_GEN_DIR)/llvm_gen.h $(AST_DIR)/ast.h
	@echo "⚡ Compilando módulo LLVM..."
	@$(CC) $(CFLAGS) -c $< -o $@

$(AST_DIR)/ast.o: $(AST_DIR)/ast.c $(AST_DIR)/ast.h
	@$(CC) $(CFLAGS) -c $< -o $@

%.o: %.c
	@echo "🔨 Compilando $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

run: build
	@if [ -s output.ll ]; then \
		echo "🔄 Compiling output.ll..."; \
		clang output.ll -o program -lm || (echo "❌ clang failed when compiling output.ll"; exit 1); \
		echo "💻 Executing compiled program:"; \
		./program; \
	else \
		echo "⚠️  output.ll does not exist or it is empty - nothing to be executed"; \
	fi

clean:
	@echo "🧹 Cleaning project..."
	@rm -f *.o $(EXEC) y.tab.* lex.yy.c *.output y.* output.ll program
	@rm -f $(AST_DIR)/*.o
	@rm -f $(CODE_GEN_DIR)/*.o