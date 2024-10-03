#ifndef ARR_H
#define ARR_H

#include <vector>
#include <string>
#include "valrep.h"
#include "value.h"

class Environment;
class Node;

class ArrayVal : public ValRep {
private:
  std::vector<Value> m_body;
  Environment *m_env;

  // value semantics prohibited
  ArrayVal(const ArrayVal &);
  ArrayVal &operator=(const ArrayVal &);

public:
  ArrayVal(std::vector<Value> body);
  virtual ~ArrayVal();

  Environment *get_env() const { return m_env; }
  Value len() {return Value(m_body.size());}
  Value get(int i) {return m_body[i];}
  void set(int i, Value val) {m_body[i] = val;}
  void push(Value val) {m_body.push_back(val);}
  Value pop(); 
};

#endif // ARR_H
