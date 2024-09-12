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
  int statements = m_ast->get_num_kids();
  std::set<std::string> seen_vars;
  for (int c = 0; c < statements; c++){
    Node* statment = m_ast->get_kid(c);
    if (statment->get_kid(0)->get_tag() == AST_VARDEF) {//if our statment is a var declaration
      seen_vars.insert(statment->get_kid(0)->get_kid(0)->get_str());//save the name of the declared var
    } else {
      std::vector<std::string> stmt_vars = find_vars(statment); //creates a vectors of all referenced vars in statement
      for (const auto& var : stmt_vars) {//search to see if we have already declared the referenced var
        if (seen_vars.find(var) == seen_vars.end()) {//if we havent raise an error on it
          SemanticError::raise(statment->get_loc(), "Undefined variable '%s'", var.c_str());
        }
      }
    }
  }
}

//Helper function that recusivly returns var references found in a statment
std::vector<std::string> Interpreter::find_vars(Node *root){
  int children = root->get_num_kids();
  std::vector<std::string> vars_found;
  for (int c = 0; c < children; c++){ //look at all children
    Node* child = root->get_kid(c);
    if (child->get_tag() == AST_VARREF) {
      vars_found.push_back(child->get_str()); //include child var refs
    }
    std::vector<std::string> new_vars = find_vars(child); //recurse to continue search
    vars_found.insert(vars_found.end(), new_vars.begin(), new_vars.end());
  }
  return vars_found; 
}

Value Interpreter::execute() {
  Environment* env = new Environment();
  Value result;
  int statements = m_ast->get_num_kids();
  std::set<std::string> seen_vars;
  for (int c = 0; c < statements; c++){ //for every statment in our code we what to evaluate that statment
    Node* statment = m_ast->get_kid(c);
    result = exec_node(env,statment); //store result in a value every iteration --> return at end
  }
  delete env;
  return result;
}

//takes a given node in the AST and interprets its meaning recursivly
Value Interpreter::exec_node(Environment* env,Node* node){
  if (node->get_tag() == AST_STATEMENT){ //START: FUNDAMENTAL TOKENS
    return exec_node(env,node->get_kid(0));
  } else if (node->get_tag() == AST_INT_LITERAL){
    return Value(std::stoi(node->get_str()));
  } else if (node->get_tag() == AST_VARDEF){
    return env->define(node->get_kid(0)->get_str());
  } else if (node->get_tag() == AST_VARREF){
    return env->lookup(node->get_str());
  }else if (node->get_tag() == AST_ASSIGN){
    Value rhs = exec_node(env,node->get_kid(1));
    return env->assign(node->get_kid(0)->get_str(),rhs);
  } else if (node->get_tag() == AST_ADD){ //START: ARITH OPS
    Value lhs = exec_node(env,node->get_kid(0));
    Value rhs = exec_node(env,node->get_kid(1));
    return Value(lhs.get_ival()+rhs.get_ival());
  } else if (node->get_tag() == AST_SUB){
    Value lhs = exec_node(env,node->get_kid(0));
    Value rhs = exec_node(env,node->get_kid(1));
    return Value(lhs.get_ival()-rhs.get_ival());
  } else if (node->get_tag() == AST_MULTIPLY){
    Value lhs = exec_node(env,node->get_kid(0));
    Value rhs = exec_node(env,node->get_kid(1));
    return Value(lhs.get_ival()*rhs.get_ival());
  } else if (node->get_tag() == AST_DIVIDE){
    Value lhs = exec_node(env,node->get_kid(0));
    Value rhs = exec_node(env,node->get_kid(1));
    if (rhs.get_ival() == 0) {
      EvaluationError::raise(node->get_loc(),"Attempted to divide by zero");
    } 
    return Value(lhs.get_ival()/rhs.get_ival());
  } else if (node->get_tag() == AST_LOR){ //START: LOGIC OPS
    Value lhs = exec_node(env,node->get_kid(0));
    Value rhs = exec_node(env,node->get_kid(1));
    return Value(lhs.get_ival()||rhs.get_ival());
  } else if (node->get_tag() == AST_LAND){
    Value lhs = exec_node(env,node->get_kid(0));
    Value rhs = exec_node(env,node->get_kid(1));
    return Value(lhs.get_ival()&&rhs.get_ival());
  } else if (node->get_tag() == AST_LL){
    Value lhs = exec_node(env,node->get_kid(0));
    Value rhs = exec_node(env,node->get_kid(1));
    return Value(lhs.get_ival()<rhs.get_ival());
  } else if (node->get_tag() == AST_LLE){
    Value lhs = exec_node(env,node->get_kid(0));
    Value rhs = exec_node(env,node->get_kid(1));
    return Value(lhs.get_ival()<=rhs.get_ival());
  } else if (node->get_tag() == AST_LG){
    Value lhs = exec_node(env,node->get_kid(0));
    Value rhs = exec_node(env,node->get_kid(1));
    return Value(lhs.get_ival()>rhs.get_ival());
  } else if (node->get_tag() == AST_LGE){
    Value lhs = exec_node(env,node->get_kid(0));
    Value rhs = exec_node(env,node->get_kid(1));
    return Value(lhs.get_ival()>=rhs.get_ival());
  } else if (node->get_tag() == AST_LE){
    Value lhs = exec_node(env,node->get_kid(0));
    Value rhs = exec_node(env,node->get_kid(1));
    return Value(lhs.get_ival()==rhs.get_ival());
  } else if (node->get_tag() == AST_LNE){
    Value lhs = exec_node(env,node->get_kid(0));
    Value rhs = exec_node(env,node->get_kid(1));
    return Value(lhs.get_ival()!=rhs.get_ival());
  } 
  return Value(0);
}
