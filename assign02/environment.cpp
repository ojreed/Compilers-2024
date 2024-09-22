#include "environment.h"
#include "exceptions.h"

Environment::Environment(Environment *parent)
  : m_parent(parent) {
  assert(m_parent != this);
}

Environment::~Environment() {
}

// TODO: implement member functions
Value Environment::define(std::string var_name){
  var_map[var_name] = Value(0);
  return  var_map[var_name];
}

Value Environment::lookup(std::string var_name){
  if (var_map.count(var_name) == 0){
    return m_parent->lookup(var_name);
  }
  return var_map[var_name];
}

Value Environment::assign(std::string var_name,int val){
  if (var_map.count(var_name) == 0){
    return m_parent->assign(var_name,val);
  }
  var_map[var_name]= Value(val);
  return  var_map[var_name];
}

Value Environment::assign(std::string var_name,Value val){
  if (var_map.count(var_name) == 0){
    return m_parent->assign(var_name,val);
  }
  var_map[var_name] = val;
  return  var_map[var_name];
}

void Environment::bind(std::string fn_name, Value fn){
  if (fn.get_kind() != VALUE_INTRINSIC_FN && fn.get_kind() != VALUE_FUNCTION) {
    RuntimeError::raise("Bound object %s is not a function",fn_name.c_str());
  }
  var_map[fn_name]= fn;
}

Value Environment::fn_call(std::string fn_name, Value args[], unsigned num_args, 
  const Location &loc, Interpreter *interp){
  if (var_map.count(fn_name) == 0){
    return m_parent->fn_call(fn_name, args, num_args, loc, interp);
  }
  Value v_fn = var_map[fn_name];
  if (v_fn.get_kind() != VALUE_INTRINSIC_FN && v_fn.get_kind() != VALUE_FUNCTION) {
    RuntimeError::raise("Bound object %s is not a function",fn_name.c_str());
  } else if (v_fn.get_kind() == VALUE_INTRINSIC_FN) {
    IntrinsicFn fn = v_fn.get_intrinsic_fn();
    return Value(fn(args, num_args, loc,  interp));
  } else if (v_fn.get_kind() == VALUE_FUNCTION) {
    return Value(0);
  }
  return Value(0);
}