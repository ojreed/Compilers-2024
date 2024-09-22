#ifndef INTERP_H
#define INTERP_H

#include "value.h"
#include "environment.h"
#include <set>
class Node;
class Location;

class Interpreter {
private:
  Node *m_ast;

public:
  Interpreter(Node *ast_to_adopt);
  ~Interpreter();

  void analyze();
  Value execute();

private:
  // TODO: private member functions
  std::vector<std::string> find_vars(Node *root);
  Value exec_node(Environment* env,Node*);
  static Value intrinsic_readint(Value args[], unsigned num_args, const Location &loc, Interpreter *interp);
  static Value intrinsic_print(Value args[], unsigned num_args, const Location &loc, Interpreter *interp);
  static Value intrinsic_println(Value args[], unsigned num_args, const Location &loc, Interpreter *interp);
  void analyze_scope(std::set<std::string>& seen_vars, Node *root);
};

#endif // INTERP_H
