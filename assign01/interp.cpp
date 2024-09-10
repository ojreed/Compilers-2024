#include <cassert>
#include <algorithm>
#include <memory>
#include <set>
#include <string>
#include "ast.h"
#include "node.h"
#include "exceptions.h"
#include "function.h"
#include "interp.h"

Interpreter::Interpreter(Node *ast_to_adopt)
  : m_ast(ast_to_adopt) {
}

Interpreter::~Interpreter() {
  delete m_ast;
}

void Interpreter::analyze() {
  // TODO: implement
  int statements = m_ast->get_num_kids();
  std::set<std::string> seen_vars;
  for (int c = 0; c < statements; c++){
    Node* statment = m_ast->get_kid(c);
    if (statment->get_kid(0)->get_tag() == AST_VARDEF) {//if our statment is a var declaration
      seen_vars.insert(statment->get_kid(0)->get_kid(0)->get_str());//save the name of the declared var
    } else {
      std::vector<std::string> stmt_vars = find_vars(statment);
      for (const auto& var : stmt_vars) {
        if (seen_vars.find(var) == seen_vars.end()) {
          SemanticError::raise(statment->get_loc(), "Undefined variable '%s'", var.c_str());
        }
      }
    }
  }
}

std::vector<std::string> Interpreter::find_vars(Node *root){
  int children = root->get_num_kids();
  std::vector<std::string> vars_found;
  for (int c = 0; c < children; c++){
    Node* child = root->get_kid(c);
    if (child->get_tag() == AST_VARREF) {
      vars_found.push_back(child->get_str());
    }
    std::vector<std::string> new_vars = find_vars(child);
    vars_found.insert(vars_found.end(), new_vars.begin(), new_vars.end());
  }
  return vars_found;
}

Value Interpreter::execute() {
  // TODO: implement
  Value result;
  return result;
}

