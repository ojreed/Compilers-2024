#include "function.h"
#include "arr.h"
#include "valrep.h"

ValRep::ValRep(ValRepKind kind)
  : m_kind(kind)
  , m_refcount(0) {
}

ValRep::~ValRep() {
}

Function *ValRep::as_function() {
  assert(m_kind == VALREP_FUNCTION);
  return static_cast<Function *>(this);
}
ArrayVal *ValRep::as_arr() {
  assert(m_kind == VALREP_VECTOR);
  return static_cast<ArrayVal *>(this);
}