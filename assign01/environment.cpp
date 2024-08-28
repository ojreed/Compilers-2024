#include "environment.h"

Environment::Environment(Environment *parent)
  : m_parent(parent) {
  assert(m_parent != this);
}

Environment::~Environment() {
}

// TODO: implement member functions
