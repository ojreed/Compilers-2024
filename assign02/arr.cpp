#include "arr.h"

ArrayVal::ArrayVal(std::vector<Value> body)
  : ValRep(VALREP_VECTOR)
  , m_body(body) {
}

ArrayVal::~ArrayVal() {
}

// TODO: implement member functions
Value ArrayVal::pop(){
  Value val = m_body.back();
  m_body.pop_back();
  return val;
}