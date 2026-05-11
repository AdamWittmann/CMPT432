# Alan Compiler

A complete four-phase compiler for the Alan language written in C++, targeting 6502a machine code that executes on SvegOS. Built for CMPT 432 at Marist College.

---

## Overview

The Alan language is a small, strongly-typed language with integer, string, and boolean types, lexical scoping, while loops, if statements, and print output. This compiler takes Alan source code and produces a 256-byte executable image conforming to the 6502a instruction set.

---

## Pipeline

```
source.alan
    |
    v
[ Lexer ]           Tokenizes source, handles comments, reports errors/warnings
    |
    v tokens
[ Parser ]          Recursive descent LL(1), builds Concrete Syntax Tree (CST)
    |
    v CST
[ Semantic Analyzer ]  Builds AST, scope checks, type checks, symbol table
    |
    v AST
[ Code Generator ]  Emits 6502a machine code, backpatches addresses and jumps
    |
    v
256-byte executable image
```

---

## Language Grammar

```
Program            ::= Block $
Block              ::= { StatementList }
StatementList      ::= Statement StatementList | epsilon
Statement          ::= PrintStatement | AssignmentStatement | VarDecl
                     | WhileStatement | IfStatement | Block
PrintStatement     ::= print ( Expr )
AssignmentStatement ::= Id = Expr
VarDecl            ::= type Id
WhileStatement     ::= while BooleanExpr Block
IfStatement        ::= if BooleanExpr Block
Expr               ::= IntExpr | StringExpr | BooleanExpr | Id
IntExpr            ::= digit intop Expr | digit
StringExpr         ::= " CharList "
BooleanExpr        ::= ( Expr boolop Expr ) | boolval
Id                 ::= char

type    ::= int | string | boolean
digit   ::= 0-9
char    ::= a-z
boolval ::= true | false
boolop  ::= == | !=
intop   ::= +
```

---

## Building

Requires a C++17 compiler.

```bash
make
```

Produces a `./compiler` binary.

---

## Usage

```bash
./compiler <source_file>
```

Example:

```bash
./compiler tests/test-valid-simple.txt
```

The compiler runs all four phases and prints a lex token trace, CST, semantic analysis results, symbol table, and the final 256-byte hex image.

---

## Output Format

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
- Unterminated string or comment
- Unrecognized character

**Warnings** allow compilation to continue:
- Variable declared but never used
- Variable declared but never initialized
- Missing `$` end-of-program marker
- Multi-digit integer (only single digits are supported)

---

## Testing

```bash
make test                                    # run full suite
make test-one FILE=tests/test-valid-simple.txt  # run one test
make test-verbose                            # all tests with full output
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
├── code_generator.h / code_generator.cpp
├── Makefile
└── tests/
    ├── run_tests.sh
    └── test-*.txt
```

---

## Author

Adam Wittmann — Marist College, CMPT 432, Spring 2026
Targets the Alan language grammar as defined by Prof. Alan Labouseur.
