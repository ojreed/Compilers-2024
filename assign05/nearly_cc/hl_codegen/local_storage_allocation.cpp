// Copyright (c) 2021-2024, David H. Hovemeyer <david.hovemeyer@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.

#include <cassert>
#include "node.h"
#include "symtab.h"
#include "local_storage_allocation.h"

LocalStorageAllocation::LocalStorageAllocation()
  : m_total_local_storage(0U)
  , m_next_vreg(VREG_FIRST_LOCAL) {
}

LocalStorageAllocation::~LocalStorageAllocation() {
}

void LocalStorageAllocation::allocate_storage(std::shared_ptr<Function> function) {
  // Any member function can use m_function to refer to the
  // Function object.
  m_function = function;

  visit(function->get_funcdef_ast());
}

void LocalStorageAllocation::visit_function_definition(Node *n) {
  Symbol *fn_id = n->get_kid(1)->get_symbol();
  SymbolTable *l_symtab = fn_id->get_symtab_k();
  m_function->get_vra()->alloc_local(); //burn return register
  int allocated = VREG_FIRST_ARG;
  assert(l_symtab->get_num_parameters()<VREG_FIRST_LOCAL);//ensures we do not have more inputs that input parameters
  for (auto i = l_symtab->cbegin(); i != l_symtab->cend(); ++i) {
    Symbol *s = *i;
    int index = std::distance(l_symtab->cbegin(), i);
    if (index >= (int) l_symtab->get_num_parameters()){ //function parameter to local var
      while (allocated < VREG_FIRST_LOCAL) {
        allocated = m_function->get_vra()->alloc_local();
      }
    }
    if (s->get_type()->is_array() || s->get_type()->is_struct()){ //needs mem_alloc
      //TODO: OR ITS ADDRESS IS TAKEN???
      s->set_al(m_storage_calc.add_field(s->get_type())); 
    } else { //can be stored in register
      s->set_reg(m_function->get_vra()->alloc_local());
      allocated+=1;
    }
    
    // printf("Register: %d, Offset: %d\n",s->get_reg(),s->get_al());
  }
  m_storage_calc.finish();
  n->set_total_local_storage(m_storage_calc.get_size());
  n->set_reg_used(allocated);
}

void LocalStorageAllocation::visit_statement_list(Node *n) {
  SymbolTable *l_symtab = n->get_kid(1)->get_symbol()->get_symtab();
  m_function->get_vra()->alloc_local(); //burn return register
  int allocated = VREG_FIRST_LOCAL;
  for (auto i = l_symtab->cbegin(); i != l_symtab->cend(); ++i) {
    Symbol *s = *i;
    if (s->get_type()->is_array() || s->get_type()->is_struct()){ //needs mem_alloc
      //TODO: OR ITS ADDRESS IS TAKEN???
      m_storage_calc.add_field(s->get_type());
      s->set_al(m_storage_calc.get_align()); 
    } else { //can be stored in register
      s->set_reg(m_function->get_vra()->alloc_local());
    }
    allocated+=1;
    m_storage_calc.finish();
    n->set_total_local_storage(m_storage_calc.get_size());
    // printf("Register: %d, Offset: %d\n",s->get_reg(),s->get_al());
  }
}

// TODO: implement private member functions
