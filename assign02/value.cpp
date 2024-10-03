#include "cpputil.h"
#include "exceptions.h"
#include "valrep.h"
#include "function.h"
#include "value.h"
#include "arr.h"

Value::Value(int ival)
  : m_kind(VALUE_INT) {
  m_atomic.ival = ival;
}

Value::Value(Function *fn)
  : m_kind(VALUE_FUNCTION)
  , m_rep(fn) {
  m_rep = fn;
  m_rep->add_ref();
}

Value::Value(IntrinsicFn intrinsic_fn)
  : m_kind(VALUE_INTRINSIC_FN) {
  m_atomic.intrinsic_fn = intrinsic_fn;
}

Value::Value(const Value &other)
  : m_kind(VALUE_INT) {
  // Just use the assignment operator to copy the other Value's data
  *this = other;
}

Value::Value(ArrayVal *arr)
  : m_kind(VALUE_ARR)
  , m_rep(arr) {
  m_rep = arr;
  m_rep->add_ref();
}

Value::~Value() {
  // TODO: handle reference counting (detach from ValRep, if any)
  if (is_dynamic()) {
    m_rep->remove_ref();
    if (m_rep->get_num_refs() == 0){
      delete m_rep;
    }
  }
}

Value &Value::operator=(const Value &rhs) {
  if (this != &rhs &&
      !(is_dynamic() && rhs.is_dynamic() && m_rep == rhs.m_rep)) {
    // TODO: handle reference counting (detach from previous ValRep, if any)
    if (is_dynamic()) {
      m_rep->remove_ref();
      if (m_rep->get_num_refs() == 0){
        delete m_rep;
      }
    }
    m_kind = rhs.m_kind;
    if (is_dynamic()) {
      // attach to rhs's dynamic representation
      m_rep = rhs.m_rep;
      rhs.m_rep->add_ref();
      // TODO: handle reference counting (attach to the new ValRep)
    } else {
      // copy rhs's atomic representation
      m_atomic = rhs.m_atomic;
    }
  }
  return *this;
}

Function *Value::get_function() const {
  assert(m_kind == VALUE_FUNCTION);
  return m_rep->as_function();
}
ArrayVal *Value::get_arr() {
  assert(m_kind == VALUE_ARR);
  return m_rep->as_arr();
}

std::string Value::as_str() const {
  switch (m_kind) {
  case VALUE_INT:
    return cpputil::format("%d", m_atomic.ival);
  case VALUE_FUNCTION:
    return cpputil::format("<function %s>", m_rep->as_function()->get_name().c_str());
  case VALUE_INTRINSIC_FN:
    return "<intrinsic function>";
  case VALUE_ARR:
    {
      ArrayVal* arr = m_rep->as_arr();
      int length = arr->len().get_ival();
      std::string arr_out = "[";
      for (int i = 0; i < length; i++){
        arr_out += std::to_string(arr->get(i).get_ival());
        if (i < length - 1){
          arr_out += ", ";
        }
      }
      return arr_out + "]";
    }
  default:
    // this should not happen
    RuntimeError::raise("Unknown value type %d", int(m_kind));
  }
}

// TODO: implementations of additional member functions
