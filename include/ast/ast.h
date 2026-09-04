#pragma once

#include "lexer/token.h"

#include <memory>
#include <string>
#include <vector>

namespace causis::ast {

struct Expr {    // base class for all expressions (expressions are somethiing that produce value)
    int line{1};      // this class is for all others inheritance
    int column{1};

    virtual ~Expr() = default;  //handle deerived object destructor
};

struct IntLiteralExpr : Expr { //integer literal expression
    int value{0};
};

struct BoolLiteralExpr : Expr {  
    bool value{false};
};

struct VariableExpr : Expr {     // ideentifier being read as expression
    std::string name;
};

struct BinaryExpr : Expr {   //reperesent operation with 2 operand (1+2)
    lexer::TokenType op{};
    std::unique_ptr<Expr> left;  // both side can be any expression
    std::unique_ptr<Expr> right;   // due to this AST is a tree structure
};

struct UnaryExpr : Expr {  //one expression (-5)
    lexer::TokenType op{};
    std::unique_ptr<Expr> operand;
};

struct CallExpr : Expr {  //function call (print(1+2)), built in call
    std::string callee;
    std::vector<std::unique_ptr<Expr>> arguments;
};

struct AssignExpr : Expr {  //assigning value to variable (x = 1+2)
    std::string name;
    std::unique_ptr<Expr> value;
};

struct Stmt { //something that perform action /control flow
    int line{1};
    int column{1};

    virtual ~Stmt() = default;
};

struct ExprStmt : Stmt {
    std::unique_ptr<Expr> expression; // turn expression into statement
};

struct BlockStmt : Stmt {
    std::vector<std::unique_ptr<Stmt>> statements; // list of statements
};

struct IfStmt : Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> then_branch;
    std::unique_ptr<Stmt> else_branch;
};

struct EveryTickStmt : Stmt {
    std::unique_ptr<BlockStmt> body;
};

struct Decl {       //declaring a new entity (world, robot, target, obstacle)
    int line{1};
    int column{1};

    virtual ~Decl() = default;
};

struct WorldDecl : Decl {
    int width{0};
    int height{0};
};

struct RobotDecl : Decl {
    std::string name;
    int x{0};
    int y{0};
};

struct TargetDecl : Decl {  // target x at 2,2;
    std::string name;
    int x{0};
    int y{0};
};

struct ObstacleDecl : Decl {
    int x{0};
    int y{0};
};

struct BehaviorDecl : Decl {           // behavior block for robot (robot1)
    std::string robot_name;
    std::vector<std::unique_ptr<EveryTickStmt>> event_blocks;
};

struct Program {
    std::vector<std::unique_ptr<Decl>> declarations;   // root of the entire AST
};

std::string print_program(const Program& program, int indent = 0);

} // namespace causis::ast
