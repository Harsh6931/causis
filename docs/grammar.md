# causis Grammar v1

> This is the canonical v1 spec; see [PLAN.md](../PLAN.md) Section 0 for project scope.

This document defines the v1 grammar for causis source files.

The grammar is written in an EBNF-like form. Literal keywords and punctuation
are shown in quotes.

---

## 1. Program

```ebnf
program             := declaration* EOF ;

declaration         := world_decl
                     | robot_decl
                     | target_decl
                     | obstacle_decl
                     | behavior_decl ;
```

---

## 2. Top-Level Declarations

```ebnf
world_decl          := "world" integer integer ";" ;

robot_decl          := "robot" identifier "at" integer integer ";" ;

target_decl         := "target" identifier "at" integer integer ";" ;

obstacle_decl       := "obstacle" "at" integer integer ";" ;

behavior_decl       := "behavior" identifier "{" event_block* "}" ;

event_block         := every_tick_block ;

every_tick_block    := "every" "tick" block ;
```

v1 supports exactly one event block kind: `every tick`.

---

## 3. Statements

```ebnf
block               := "{" statement* "}" ;

statement           := if_stmt
                     | block
                     | expr_stmt ;

if_stmt             := "if" expression block else_clause? ;

else_clause         := "else" block ;

expr_stmt           := expression ";" ;
```

---

## 4. Expressions

```ebnf
expression          := assignment ;

assignment          := identifier "=" assignment
                     | equality ;

equality            := comparison ( ( "==" | "!=" ) comparison )* ;

comparison          := term ( ( "<" | "<=" | ">" | ">=" ) term )* ;

term                := factor ( ( "+" | "-" ) factor )* ;

factor              := unary ( ( "*" | "/" ) unary )* ;

unary               := ( "!" | "-" ) unary
                     | call ;

call                := primary ( "(" arguments? ")" )* ;

arguments           := expression ( "," expression )* ;

primary             := integer
                     | "true"
                     | "false"
                     | identifier
                     | "(" expression ")" ;
```

Assignment is right-associative. Other binary operators are left-associative.

---

## 5. Lexical Grammar

```ebnf
identifier          := identifier_start identifier_part* ;
identifier_start    := letter | "_" ;
identifier_part     := letter | digit | "_" ;

integer             := digit+ ;

letter              := "a"..."z" | "A"..."Z" ;
digit               := "0"..."9" ;
```

---

## 6. Comments and Whitespace

```ebnf
line_comment        := "//" characters_until_newline ;
whitespace          := " " | "\t" | "\r" | "\n" ;
```

Whitespace and line comments are ignored except where they separate tokens.

Block comments are not part of v1.

---

## 7. Reserved Words

```text
world
robot
target
obstacle
behavior
every
tick
if
else
at
true
false
```

Reserved words cannot be used as identifiers.

---

## 8. Built-in Calls

The grammar parses built-ins as ordinary calls. Semantic analysis determines
whether the call is valid.

```text
move_up()
move_down()
move_left()
move_right()
move_forward()
move_toward(target)
turn_left()
turn_right()
stop()
distance_to(target)
obstacle_ahead()
collision()
```
