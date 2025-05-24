# Hulk-Full_Compiler 🚀  

A compiler for the **HULK** programming language made with **C**, featuring lexer, parser, AST generation, and LLVM code generation.  

## **Requirements** 📋  

### **📦 Dependencies**  
- **Flex** (lexical analysis)  
- **Bison** (syntax analysis)  
- **LLVM** (IR code generation)  
- **Clang** (compiling generated code)  
- **GCC/Clang** (C compiler)  

### **📌 Linux (Ubuntu/Debian) Installation**  
```bash
sudo apt update
sudo apt install flex bison llvm clang
```
## **Project Structure** 📂

```
├── ast/ # AST nodes
│ ├── ast.c
│ └── ast.h
├── code_generation/ # LLVM codegen
│ ├── llvm_builtins.c
│ ├── llvm_builtins.h
│ ├── llvm_codegen.c
│ ├── llvm_codegen.h
│ ├── llvm_core.c
│ ├── llvm_core.h
│ ├── llvm_operators.c
│ ├── llvm_operators.h
│ ├── llvm_scope.c
│ ├── llvm_scope.h
│ ├── llvm_string.c
│ └── llvm_string.h
├── lexer/ # Lexer
│ └── lexer.l
├── parser/ # Parser
│ └── parser.y
├── scope/ #scope
│ ├──context.c
│ ├──scope.c
│ └──scope.h
├── semantic_check/ #semantic check
│ ├──cond_loop_checking.c
│ ├──basic_checking.c
│ ├──function_checking.c
│ ├──semantic.c
│ ├──semantic.h
│ ├──type_checking.c
│ ├──unification.c
│ └──variable_checking.c
├── type/ #types
│ ├──type.c
│ └──type.h
├── utils/ #utilities
│ ├──utils.c
│ └──utils.h
├── visitor/ #visitor
│ ├──visitor.c
│ └──visitor.h
│
├── main.c # Entry point
├── Makefile # Build automation
├──  README.md
└──  script.hulk
```

## **Usage** 🛠

### 🔨 Build the project
```bash
make compile
```
### ▶️ Run the compiler (generates output.ll)
```bash
make execute
```
### 🧹 Clean generated files
```bash
make clean
```

### 📝 Testing the compiler
```

1. Run `make compile` to compile.

2. The compiler will generate output.ll (LLVM IR).

3. If output.ll exists, `make execute` will compile and execute it.
```

## **Git Commit & Branch Strategy** 💻

We follow a structured Git workflow to maintain clean commit history and effective collaboration:

```bash
# Create new feature branch
git checkout -b prefix/your-feature-name
```
### Possible prefixes 📝


| Prefix      | Description                          | Example                          |
|-------------|--------------------------------------|----------------------------------|
| `feat/`     | New feature                          | `feat/add type inference`       |
| `fix/`      | Bug fix                              | `fix/resolve segfault in parser`|
| `refactor/` | Code improvement (no behavior change)| `refactor/AST node structure`   |
| `doc/`      | Documentation changes                | `doc/update API reference`      |
