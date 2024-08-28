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

  // copy constructor and assignment operator prohibited
  Environment(const Environment &);
  Environment &operator=(const Environment &);

public:
  Environment(Environment *parent = nullptr);
  ~Environment();

  // TODO: add member functions allowing lookup, definition, and assignment
};

#endif // ENVIRONMENT_H
