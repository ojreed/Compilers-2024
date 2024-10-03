#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <cassert>
#include <map>
#include <string>
#include "value.h"

class Environment {
private:
  Environment *m_parent;
  // TODO: representation of environment (map of names to values)
  std::map<std::string,Value> var_map;

  // copy constructor and assignment operator prohibited
  Environment(const Environment &);
  Environment &operator=(const Environment &);

public:
  Environment(Environment *parent = nullptr);
  ~Environment();

  // TODO: add member functions allowing lookup, definition, and assignment
  Value define(std::string var_name);
  Value lookup(std::string var_name);
  Value assign(std::string var_name,int val);
  Value assign(std::string var_name,Value val);
  void bind(std::string fn_name,Value fn);
  Value fn_call(std::string fn_name);
};

#endif // ENVIRONMENT_H
