#include "exceptions.h"
#include "ast.h"

ASTTreePrint::ASTTreePrint() {
}

ASTTreePrint::~ASTTreePrint() {
}

std::string ASTTreePrint::node_tag_to_string(int tag) const {
  switch (tag) {
  case AST_ADD:
    return "ADD";
  case AST_SUB:
    return "SUB";
  case AST_MULTIPLY:
    return "MULTIPLY";
  case AST_DIVIDE:
    return "DIVIDE";
  case AST_VARREF:
    return "VARREF";
  case AST_INT_LITERAL:
    return "INT_LITERAL";
  case AST_UNIT:
    return "UNIT";
  case AST_STATEMENT:
    return "STATEMENT";
  //add cases for other AST node kinds
  case AST_VARDEF:
    return "VARDEF";
  case AST_ASSIGN:
    return "ASSIGN";
  case AST_LOR:
    return "LOGICAL_OR";
  case AST_LAND:
    return "LOGICAL_AND";
  case AST_LL:
    return "LT";
  case AST_LLE:
    return "LTE";
  case AST_LG:
    return "GT";
  case AST_LGE:
    return "GTE";
  case AST_LE:
    return "EQ";
  case AST_LNE:
    return "NEQ";
  //add members for other AST node kinds HW2
  case AST_FUNC:
    return "FUNCTION";
  case AST_IF:
    return "IF" ;
  case AST_ELSE:
    return "ELSE";
  case AST_WHILE:
    return "WHILE";
  case AST_PARAMETER_LIST:
    return "PARAMETER_LIST"; 
  case AST_STATEMENT_LIST:
    return "AST_STATEMENT_LIST";
  case AST_ARG_LIST:
    return "ARG_LIST";    
  case AST_FNCALL:
    return "FNCALL";  
  default:
    RuntimeError::raise("Unknown AST node type %d\n", tag);
  }
}
