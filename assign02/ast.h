#ifndef AST_H
#define AST_H

#include "treeprint.h"

// AST node tags
enum ASTKind {
  AST_ADD = 2000,
  AST_SUB,
  AST_MULTIPLY,
  AST_DIVIDE,
  AST_VARREF,
  AST_INT_LITERAL,
  AST_UNIT,
  AST_STATEMENT,
  //add members for other AST node kinds
  AST_VARDEF,
  AST_ASSIGN,
  AST_LOR,
  AST_LAND,
  AST_LL,
  AST_LLE,
  AST_LG,
  AST_LGE,
  AST_LE,
  AST_LNE,
  //add members for other AST node kinds HW2
  AST_FUNC, 
  AST_IF,
  AST_ELSE,
  AST_WHILE,
  AST_LBRACK,
  AST_RBRACK,
  AST_COMMA,
  AST_PARAMETER_LIST,
  AST_STATEMENT_LIST,
  AST_ARG_LIST,
  AST_FNCALL
};

class ASTTreePrint : public TreePrint {
public:
  ASTTreePrint();
  virtual ~ASTTreePrint();

  virtual std::string node_tag_to_string(int tag) const;
};

#endif // AST_H
