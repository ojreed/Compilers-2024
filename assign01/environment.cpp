#include "environment.h"

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
  return var_map[var_name];
}

Value Environment::assign(std::string var_name,int val){
  var_map[var_name]= Value(val);
  return  var_map[var_name];
}

Value Environment::assign(std::string var_name,Value val){
  var_map[var_name]= val;
  return  var_map[var_name];
}