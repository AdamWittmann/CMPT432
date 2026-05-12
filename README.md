# C++ Terminator Compiler

A complete multi-phase compiler for the Alan language written in C++, targeting 6502a machine code that executes on SvegOS, with additional Java source code generation and an interactive Terminator-themed web UI. Built for CMPT 432 at Marist College.

---

## Demo

https://github.com/AdamWittmann/CMPT432/releases/tag/v1.0

---

## Overview

The Alan language is a small, strongly-typed language with integer, string, and boolean types, lexical scoping, while loops, if statements, and print output. This compiler takes Alan source code through lexing, parsing, semantic analysis, optimization, and code generation — producing both a 256-byte 6502a executable image and readable Java source code.

---

## Pipeline

```
source.alan
    |
    v
[ Lexer ]              Tokenizes source, auto-fixes recoverable errors
    |
    v tokens
[ Parser ]             Recursive descent LL(1), builds Concrete Syntax Tree (CST)
    |
    v CST
[ Semantic Analyzer ]  Builds AST, scope checks, type checks, symbol table
    |
    v AST
[ Optimizer ]          Constant folding on integer expressions
    |
    v optimized AST
[ Java Generator ]     Emits readable Java source code (.java)
[ Code Generator ]     Emits 6502a machine code, backpatches addresses and jumps
    |
    v
256-byte executable image + AlanProgramN.java
```

---

## Language Grammar

```
Program             ::= Block $
Block               ::= { StatementList }
StatementList       ::= Statement StatementList | epsilon
Statement           ::= PrintStatement | AssignmentStatement | VarDecl
                      | WhileStatement | IfStatement | Block
PrintStatement      ::= print ( Expr )
AssignmentStatement ::= Id = Expr
VarDecl             ::= type Id
WhileStatement      ::= while BooleanExpr Block
IfStatement         ::= if BooleanExpr Block
Expr                ::= IntExpr | StringExpr | BooleanExpr | Id
IntExpr             ::= digit intop Expr | digit
StringExpr          ::= " CharList "
BooleanExpr         ::= ( Expr boolop Expr ) | boolval
Id                  ::= char

type    ::= int | string | boolean
digit   ::= 0-9
char    ::= a-z
boolval ::= true | false
boolop  ::= == | !=
intop   ::= +
```

---

## Building the Compiler

Requires a C++17 compiler.

```bash
make
```

Produces a `./compiler` binary.

---

## Command Line Usage

```bash
./compiler <source_file>
```

Example:

```bash
./compiler tests/test-valid-simple.txt
```

The compiler runs all phases and prints a lex token trace, CST, semantic analysis traces, symbol table, optimizer traces, Java source, and the final 256-byte hex image.

---

## Terminator Web UI

An interactive browser-based UI with a Terminator vision aesthetic — red-on-black with CRT scanlines, hex rain background, targeting reticles, and a boot sequence. Connects to the real C++ compiler via a Flask backend.

### Setup

**1. Build the compiler**
```bash
make
```

**2. Create and activate a virtual environment**
```bash
python -m venv venv
source venv/bin/activate
```

**3. Install dependencies**
```bash
pip install flask flask-cors
```

**4. Start the backend server**
```bash
cd TerminatorGUI
python server.py
```

**5. Open the UI**

Navigate to `http://localhost:5000` in your browser.

### Features

- Full boot sequence on load
- Real-time streaming of compiler output
- Phase filtering — view output from a single phase
- Live metrics — tokens, errors, warnings, optimizations
- 256-byte hex image display
- Java source display
- Targeting reticles and hex rain background

---

## Error Recovery

The lexer automatically fixes certain recoverable errors instead of halting:

| Issue | Fix Applied | Severity |
|---|---|---|
| Unterminated string `"hello` | Inserts closing `"` | Warning |
| Unterminated comment `/* ...` | Closes comment | Warning |
| Missing `$` at end of program | Inserts `$` token | Warning |
| Lone `!` without `=` | Reports error | Error |
| Multi-digit integer `42` | Reports error | Error |

---

## Optimization — Constant Folding

The optimizer evaluates integer expressions at compile time when all operands are literals.

```
3 + 4      -->  7     (folded at compile time)
1 + 2 + 3  -->  6
3 + a      -->  not folded (variable operand)
```

---

## Java Source Code Generation

Each program generates a valid `.java` file in `JavaCode/` that can be compiled and run on the JVM.

```bash
javac JavaCode/AlanProgram1.java
java -cp JavaCode AlanProgram1
```

Alan to Java type mapping:

| Alan | Java |
|---|---|
| `int` | `int` |
| `string` | `String` |
| `boolean` | `boolean` |
| `print(x)` | `System.out.println(x)` |

---

## 6502a Output Format

```
00 | A9 05 8D 06 00 00 00 00
08 | 00 00 00 00 00 00 00 00
...
F8 | 68 65 6C 6C 6F 00 00 00
```

Paste this into SvegOS to execute the program.

---

## Memory Layout

```
[ 0x00 ... code ... BRK | ... static vars ... | ... heap ... 0xFF ]
                         ^                    ^
                     code ends            0xFF, grows left
```

- **Code section** - 6502a opcodes starting at 0x00
- **Static section** - one byte per variable, immediately after BRK
- **Heap** - null-terminated strings growing up from 0xFF

---

## 6502a Syscall Convention

```
LDA <addr>   ; load value into A
TAY          ; transfer A to Y
LDX #<code>  ; syscall code into X
FF           ; system call
```

Syscall codes: `0x01` = print int, `0x02` = print string, `0x03` = print boolean

---

## Errors and Warnings

**Errors** halt compilation:
- Undeclared variable
- Redeclared variable in same scope
- Type mismatch
- Unrecognized character
- Multi-digit integer

**Warnings** allow compilation to continue:
- Variable declared but never used
- Variable declared but never initialized
- Missing `$` end-of-program marker (auto-fixed)
- Unterminated string (auto-fixed)
- Unterminated comment (auto-fixed)

---

## Testing

```bash
make test                                       # run full suite
make test-one FILE=tests/test-valid-simple.txt  # run one test
make test-verbose                               # all tests with full output
```

Test file naming conventions:
- `test-valid-*` expects exit code 0
- `test-error-*` expects a non-zero exit code
- `test-warning-*` expects WARNING in output

---

## Project Structure

```
compiler-project/
├── main.cpp
├── lexer.h / lexer.cpp
├── parser.h / parser.cpp
├── cst_node.h / cst_node.cpp
├── token.h / token.cpp
├── semantic_analyzer.h / semantic_analyzer.cpp
├── symbol_table.h / symbol_table.cpp
├── optimizer.h / optimizer.cpp
├── code_generator.h / code_generator.cpp
├── java_generator.h / java_generator.cpp
├── Makefile
├── JavaCode/                  Generated Java source files
├── TerminatorGUI/
│   ├── server.py              Flask backend
│   ├── terminator_ui.html     Standalone JS demo
│   └── terminator_ui_live.html Real C++ backend UI
└── tests/
    ├── run_tests.sh
    └── test-*.txt
```

---

## Author

Adam Wittmann — Marist College, CMPT 432, Spring 2026
Targets the Alan language grammar as defined by Prof. Alan Labouseur.
