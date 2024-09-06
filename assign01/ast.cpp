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
  // TODO: add cases for other AST node kinds
  case AST_VAR:
    return "VARIABLE";
  case AST_ASSIGN:
    return "ASSIGNMENT";
  case AST_LOR:
    return "L_OR";
  case AST_LAND:
    return "L_AND";
  case AST_LL:
    return "L_LESS";
  case AST_LLE:
    return "L_LESS_EQ";
  case AST_LG:
    return "L_GREAT";
  case AST_LGE:
    return "L_GREAT_EQ";
  case AST_LE:
    return "L_EQ";
  case AST_LNE:
    return "L_NOT_EQ";
  default:
    RuntimeError::raise("Unknown AST node type %d\n", tag);
  }
}
