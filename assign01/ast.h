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
  AST_LNE
};

class ASTTreePrint : public TreePrint {
public:
  ASTTreePrint();
  virtual ~ASTTreePrint();

  virtual std::string node_tag_to_string(int tag) const;
};

#endif // AST_H
