#ifndef INTERP_H
#define INTERP_H

#include "value.h"
#include "environment.h"
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
};

#endif // INTERP_H
