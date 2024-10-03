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
  } else if (root->get_tag() == AST_FUNC){
    seen_vars.insert(root->get_kid(0)->get_str());
    unsigned num_args = 0;
    if (root->get_num_kids() == 3){
      num_args = root->get_kid(1)->get_num_kids(); //number of children in arglist
    }
    seen_vars = std::set<std::string>(seen_vars);
    for (unsigned i = 0; i < num_args; i+=1){
      //store evaluated arg for each arg in arglist
      seen_vars.insert(root->get_kid(1)->get_kid(i)->get_str());
    }
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
  if (num_args != 0)
    EvaluationError::raise(
      loc, "Wrong number of arguments passed to print function");
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
    if (!rhs.is_numeric() || !lhs.is_numeric() ){
      EvaluationError::raise(node->get_loc(), "Contitional output is non-numeric");
    }
    return Value(lhs.get_ival()+rhs.get_ival());
  } else if (node->get_tag() == AST_SUB){
    Value lhs = exec_node(env,node->get_kid(0));
    Value rhs = exec_node(env,node->get_kid(1));
    if (!rhs.is_numeric() || !lhs.is_numeric() ){
      EvaluationError::raise(node->get_loc(), "Contitional output is non-numeric");
    }
    return Value(lhs.get_ival()-rhs.get_ival());
  } else if (node->get_tag() == AST_MULTIPLY){
    Value lhs = exec_node(env,node->get_kid(0));
    Value rhs = exec_node(env,node->get_kid(1));
    if (!rhs.is_numeric() || !lhs.is_numeric() ){
      EvaluationError::raise(node->get_loc(), "Contitional output is non-numeric");
    }
    return Value(lhs.get_ival()*rhs.get_ival());
  } else if (node->get_tag() == AST_DIVIDE){
    Value lhs = exec_node(env,node->get_kid(0));
    Value rhs = exec_node(env,node->get_kid(1));
    if (!rhs.is_numeric() || !lhs.is_numeric() ){
      EvaluationError::raise(node->get_loc(), "Contitional output is non-numeric");
    }
    if (rhs.get_ival() == 0) {
      EvaluationError::raise(node->get_loc(),"Attempted to divide by zero");
    } 
    return Value(lhs.get_ival()/rhs.get_ival());
  } else if (node->get_tag() == AST_LOR){ //START: LOGIC OPS
    Value lhs = exec_node(env,node->get_kid(0));
    if (!lhs.is_numeric()){
      EvaluationError::raise(node->get_loc(), "Contitional output is non-numeric");
    }
    if (lhs.get_ival()) { //shortcut for or
      return Value(1);
    }
    Value rhs = exec_node(env,node->get_kid(1));
    if (!rhs.is_numeric()){
      EvaluationError::raise(node->get_loc(), "Contitional output is non-numeric");
    }
    return Value(lhs.get_ival()||rhs.get_ival());
  } else if (node->get_tag() == AST_LAND){
    Value lhs = exec_node(env,node->get_kid(0));
    if (!lhs.is_numeric()){
      EvaluationError::raise(node->get_loc(), "Contitional output is non-numeric");
    }
    if (!lhs.get_ival()) { //shortcut for and
      return Value(0);
    }
    Value rhs = exec_node(env,node->get_kid(1));
    if (!rhs.is_numeric()){
      EvaluationError::raise(node->get_loc(), "Contitional output is non-numeric");
    }
    return Value(lhs.get_ival()&&rhs.get_ival());
  } else if (node->get_tag() == AST_LL){
    Value lhs = exec_node(env,node->get_kid(0));
    Value rhs = exec_node(env,node->get_kid(1));
    if (!rhs.is_numeric() || !lhs.is_numeric() ){
      EvaluationError::raise(node->get_loc(), "Contitional output is non-numeric");
    }
    return Value(lhs.get_ival()<rhs.get_ival());
  } else if (node->get_tag() == AST_LLE){
    Value lhs = exec_node(env,node->get_kid(0));
    Value rhs = exec_node(env,node->get_kid(1));
    if (!rhs.is_numeric() || !lhs.is_numeric() ){
      EvaluationError::raise(node->get_loc(), "Contitional output is non-numeric");
    }
    return Value(lhs.get_ival()<=rhs.get_ival());
  } else if (node->get_tag() == AST_LG){
    Value lhs = exec_node(env,node->get_kid(0));
    Value rhs = exec_node(env,node->get_kid(1));
    if (!rhs.is_numeric() || !lhs.is_numeric() ){
      EvaluationError::raise(node->get_loc(), "Contitional output is non-numeric");
    }
    return Value(lhs.get_ival()>rhs.get_ival());
  } else if (node->get_tag() == AST_LGE){
    Value lhs = exec_node(env,node->get_kid(0));
    Value rhs = exec_node(env,node->get_kid(1));
    if (!rhs.is_numeric() || !lhs.is_numeric() ){
      EvaluationError::raise(node->get_loc(), "Contitional output is non-numeric");
    }
    return Value(lhs.get_ival()>=rhs.get_ival());
  } else if (node->get_tag() == AST_LE){
    Value lhs = exec_node(env,node->get_kid(0));
    Value rhs = exec_node(env,node->get_kid(1));
    if (!rhs.is_numeric() || !lhs.is_numeric() ){
      EvaluationError::raise(node->get_loc(), "Contitional output is non-numeric");
    }
    return Value(lhs.get_ival()==rhs.get_ival());
  } else if (node->get_tag() == AST_LNE){
    Value lhs = exec_node(env,node->get_kid(0));
    Value rhs = exec_node(env,node->get_kid(1));
    if (!rhs.is_numeric() || !lhs.is_numeric() ){
      EvaluationError::raise(node->get_loc(), "Contitional output is non-numeric");
    }
    return Value(lhs.get_ival()!=rhs.get_ival());
  } else if (node->get_tag() == AST_IF){ //Start: control flow
    Value condition = exec_node(env,node->get_kid(0));
    if (!condition.is_numeric()){
      EvaluationError::raise(node->get_loc(), "Contitional output is non-numeric");
    }
    if (condition.get_ival() != 0) {
      exec_node(env,node->get_kid(1)); //evaluate the IF side AST_SLIST
    } else if (node->get_num_kids() == 3){
      exec_node(env,node->get_kid(2)->get_kid(0)); //evaluate the ELSE side AST_SLIST (if it exists)
    }
    return Value(0);
  } else if (node->get_tag() == AST_WHILE){ 
    //pre validate condition
    Value condition = exec_node(env,node->get_kid(0));
    if (!condition.is_numeric()){
      EvaluationError::raise(node->get_loc(), "Contitional output is non-numeric");
    }
    while (condition.get_ival() != 0) {
      exec_node(env,node->get_kid(1)); //do loop
      //validate condition
      condition = exec_node(env,node->get_kid(0));
      if (!condition.is_numeric()){
        EvaluationError::raise(node->get_loc(), "Contitional output is non-numeric");
      }
    } 
    return Value(0);
  } else if (node->get_tag() == AST_STATEMENT_LIST){
    Value result;
    int statements = node->get_num_kids();
    Environment* child_env = new Environment(env); //setup a new child env for inside blocks
    for (int c = 0; c < statements; c++){ //for every statment in our code we what to evaluate that statment
      Node* statment = node->get_kid(c);
      result = exec_node(child_env,statment); //store result in a value every iteration --> return at end
    }
    delete child_env; //destroy child env
    return result;
  } else if (node->get_tag() == AST_FUNC) {
    std::string fn_name = node->get_kid(0)->get_str();//get function name from def

    int a = (node->get_num_kids() == 3) ? 2 : 1;
    std::vector<std::string> params;
    if (a == 2) { //if there is a parameter list
      //generate a list of function parameter strings
      Node* param_list = node->get_kid(a-1);
      for (unsigned i = 0; i< param_list->get_num_kids(); i++){
        params.push_back(param_list->get_kid(i)->get_str());
      }
    }
    Node* stmt_list = node->get_kid(a); //get function body

    //assemble function and bind
    Value fn_value = new Function(fn_name,params,env,stmt_list);
    env->bind(fn_name,fn_value);
    return Value(0); 
  } else if (node->get_tag() == AST_FNCALL) {
    Value output;
    Environment* fn_call_env = new Environment(env);
    std::string fn_name = node->get_kid(0)->get_str(); //function identifier 
    Value v_fn = env->fn_call(fn_name);
    unsigned num_args = 0;
    if (node->get_num_kids() > 1){
      num_args = node->get_kid(1)->get_num_kids(); //number of children in arglist
    }
    Value args[num_args]; 
    for (unsigned i = 0; i < num_args; i+=1){
      //store evaluated arg for each arg in arglist
      args[i] = exec_node(fn_call_env,node->get_kid(1)->get_kid(i));
    }
    const Location &loc = node->get_loc(); 
    Interpreter *interp = this;
    if (v_fn.get_kind() == VALUE_INTRINSIC_FN){
      IntrinsicFn fn = v_fn.get_intrinsic_fn();
      output = fn(args,num_args,loc,interp);
    } else {
      Node* entry_point = v_fn.get_function()->get_body();
      std::vector<std::string> p_names = v_fn.get_function()->get_params();
      if (p_names.size() != num_args) {
        EvaluationError::raise(node->get_loc(), "Incorect number of function argumnets. Expected %ld, given %d.",p_names.size(),num_args);
      }
      Environment* body_env = new Environment(v_fn.get_function()->get_parent_env());
      for (unsigned i = 0; i < num_args; i+=1){
        body_env->define(p_names[i]);
        body_env->assign(p_names[i],args[i]);
      }
      output = exec_node(body_env,entry_point);
      delete body_env;
    } 
    delete fn_call_env;
    return output;
  } 
  return Value(0);
}


