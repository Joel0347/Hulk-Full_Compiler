# Hulk-Full_Compiler 🚀  

A compiler for the **HULK** programming language made with C language, featuring lexer, parser, AST generation, and LLVM code generation.  

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
│ ├── llvm_gen.c
│ └── llvm_gen.h
├── lexer/ # Lexer
│ └── lexer.l
├── parser/ # Parser
│ └── parser.y
├── scope/ #scope
| ├──scope.c
| └──scope.h
├── semantic_check/ #semantic check
| ├──semantic.c
| └──semantic.h
├── type/ #types
| ├──type.c
| └──type.h
├── visitor/ #visitor
| ├──visitor.c
| └──visitor.h
|
├── main.c # Entry point
├── Makefile # Build automation
├── README.md
└── script.hulk #HULK code to compile
```

## **Usage** 🛠

### 🔨 Build the project
```bash
make build
```
### ▶️ Run the compiler (generates output.ll)
```bash
make run
```
### 🧹 Clean generated files
```bash
make clean
```

### 📝 Testing the compiler
```

1. Run make build to compile.

2. The compiler will generate output.ll (LLVM IR).

3. If output.ll exists, make run will compile and execute it.
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
