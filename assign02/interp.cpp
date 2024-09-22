#include <cassert>
#include <algorithm>
#include <memory>
#include <set>
#include <string>
#include <iostream>
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
  std::set<std::string> seen_vars;
  //handle intrinsics
  seen_vars.insert("print");
  seen_vars.insert("println");
  seen_vars.insert("readint");
  analyze_scope(seen_vars,m_ast);
}

void Interpreter::analyze_scope(std::set<std::string>& seen_vars, Node *root) {
  if (root->get_tag() == AST_VARDEF) {//if our statment is a var declaration
    seen_vars.insert(root->get_kid(0)->get_str());//save the name of the declared var
  } else {//if our statment is a var declaration
    if (root->get_tag() == AST_VARREF && seen_vars.find(root->get_str()) == seen_vars.end()) {//if we havent raise an error on it
      SemanticError::raise(root->get_loc(), "Undefined variable '%s'", root->get_str().c_str());
    }
  } 
  int children = root->get_num_kids();
  for (int c = 0; c < children; c++){ 
    Node* child = root->get_kid(c);
    if (child->get_tag() == AST_STATEMENT_LIST) {
      std::set<std::string> local_seen_vars(seen_vars);
      analyze_scope(local_seen_vars,child);
    } else {
      analyze_scope(seen_vars,child);
    }
  }
}


Value Interpreter::intrinsic_print(
  Value args[], unsigned num_args,
  const Location &loc, Interpreter *interp) {
  if (num_args != 1)
    EvaluationError::raise(
      loc, "Wrong number of arguments passed to print function");
  printf("%s", args[0].as_str().c_str());
  return Value();
}

Value Interpreter::intrinsic_println(Value args[], unsigned num_args,
  const Location &loc, Interpreter *interp){
  if (num_args != 1)
    EvaluationError::raise(
      loc, "Wrong number of arguments passed to print function");
  printf("%s\n", args[0].as_str().c_str());
  return Value();
}

Value Interpreter::intrinsic_readint(Value args[], unsigned num_args, 
  const Location &loc, Interpreter *interp){
  int i;
  scanf("%d",&i);
  return Value(i);
}

Value Interpreter::execute() {
  Environment* global = new Environment();
  //Add intrinsics 
  global->bind("print", Value(&intrinsic_print));
  global->bind("println", Value(&intrinsic_println));
  global->bind("readint", Value(&intrinsic_readint));

  Value result;
  int statements = m_ast->get_num_kids();
  std::set<std::string> seen_vars;
  for (int c = 0; c < statements; c++){ //for every statment in our code we what to evaluate that statment
    Node* statment = m_ast->get_kid(c);
    result = exec_node(global,statment); //store result in a value every iteration --> return at end
  }
  delete global;
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
    if (lhs.get_ival()) { //shortcut for or
      return Value(1);
    }
    Value rhs = exec_node(env,node->get_kid(1));
    return Value(lhs.get_ival()||rhs.get_ival());
  } else if (node->get_tag() == AST_LAND){
    Value lhs = exec_node(env,node->get_kid(0));
    if (!lhs.get_ival()) { //shortcut for and
      return Value(0);
    }
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
  } else if (node->get_tag() == AST_IF){ //Start: control flow
    if (exec_node(env,node->get_kid(0)).get_ival() != 0) {
      exec_node(env,node->get_kid(1)); //evaluate the IF side AST_SLIST
    } else if (node->get_num_kids() == 3){
      exec_node(env,node->get_kid(2)->get_kid(0)); //evaluate the ELSE side AST_SLIST (if it exists)
    }
    return Value(0);
  } else if (node->get_tag() == AST_WHILE){ 
    Value cur_val = Value(0);
    while (exec_node(env,node->get_kid(0)).get_ival() != 0) {
      cur_val = exec_node(env,node->get_kid(1));
    } 
    return Value(0);
  } else if (node->get_tag() == AST_STATEMENT_LIST){
    Value result;
    int statements = node->get_num_kids();
    Environment* child_env = new Environment(env);
    for (int c = 0; c < statements; c++){ //for every statment in our code we what to evaluate that statment
      Node* statment = node->get_kid(c);
      result = exec_node(child_env,statment); //store result in a value every iteration --> return at end
    }
    delete child_env;
    return result;
  } else if (node->get_tag() == AST_FUNC) {
    return Value(0); //TODO: for MS2
  } else if (node->get_tag() == AST_FNCALL) {
    std::string fn_name = node->get_kid(0)->get_str(); //function identifier 
    if (node->get_num_kids() > 1){
      unsigned num_args = node->get_kid(1)->get_num_kids(); //number of children in arglist
      Value args[num_args]; 
      for (unsigned i = 0; i < num_args; i+=1){
        //store evaluated arg for each arg in arglist
        args[i] = exec_node(env,node->get_kid(1)->get_kid(i));
      }
      const Location &loc = node->get_loc(); 
      Interpreter *interp = this;
      return env->fn_call(fn_name,args,num_args,loc,interp);
    } else {
      unsigned num_args = 0;
      Value args[num_args]; 
      const Location &loc = node->get_loc(); 
      Interpreter *interp = this;
      return env->fn_call(fn_name,args,num_args,loc,interp);
    }
    
  }
  return Value(0);
}


