# Mini-Java compiler and Interpreter

## Overview
The compiler was created as part of a university course and includes all the phases of a compiler besides the *optimisation* step. The collaboration included both combined work, split work and a lot of communication and planning.

Overall the compiler and interpreter supports all of the mini-java language and can compile all the tests found under the `test_files`directory.

### Compiler
The compiler is based on the two step approach, performing tokenisation, building a parse tree, type checking, then performing the second step of intermediate code generation followed by the actual code generation, skipping the optimisation step.

### Interpreter
The interpreter reads the bytecode generated from the Compiler in a stack based approach and performs the various operations based on the bytecode.

## Building
The projects offers a makefile where the first row selects the appropriate test and the `all` command builds the entire compiler.
Besides building the entire project, the makefile splits the build process into several nodes as well as seperating the compiler and the interpreter. The nodes are the lexer, bison, the abstract syntax tree (AST) and the control flow graph (CFG).
