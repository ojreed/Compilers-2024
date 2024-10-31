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
#include "instruction.h"
#include "highlevel.h"
#include "ast.h"
#include "parse.tab.h"
#include "grammar_symbols.h"
#include "exceptions.h"
#include "local_storage_allocation.h"
#include "highlevel_codegen.h"


// Adjust an opcode for a basic type
HighLevelOpcode get_opcode(HighLevelOpcode base_opcode, std::shared_ptr<Type> type) {
  if (type->is_basic())
    return static_cast<HighLevelOpcode>(int(base_opcode) + int(type->get_basic_type_kind()));
  else if (type->is_pointer())
    return static_cast<HighLevelOpcode>(int(base_opcode) + int(BasicTypeKind::LONG));
  else
    RuntimeError::raise("attempt to use type '%s' as data in opcode selection", type->as_str().c_str());
}

HighLevelCodegen::HighLevelCodegen(const Options &options, int next_label_num)
  : m_options(options)
  , m_next_label_num(next_label_num)
{
}

HighLevelCodegen::~HighLevelCodegen() {}

void HighLevelCodegen::generate(std::shared_ptr<Function> function) {
  assert(function->get_funcdef_ast() != nullptr);
  assert(!function->get_hl_iseq());

  // Create empty InstructionSequence to hold high-level instructions
  std::shared_ptr<InstructionSequence> hl_iseq(new InstructionSequence());
  function->set_hl_iseq(hl_iseq);

  // Member functions can use m_function to access the Function object
  m_function = function;

  // Visit function definition
  visit(function->get_funcdef_ast());
}

void HighLevelCodegen::visit_function_definition(Node *n) {
  // generate the name of the label that return instructions should target
  std::string fn_name = n->get_kid(1)->get_str();
  m_return_label_name = ".L" + fn_name + "_return";

  unsigned total_local_storage = 0U;
/*
  total_local_storage = n->get_total_local_storage();
*/

  get_hl_iseq()->append(new Instruction(HINS_enter, Operand(Operand::IMM_IVAL, total_local_storage)));

  // visit body
  visit(n->get_kid(3));

  get_hl_iseq()->define_label(m_return_label_name);
  get_hl_iseq()->append(new Instruction(HINS_leave, Operand(Operand::IMM_IVAL, total_local_storage)));
  get_hl_iseq()->append(new Instruction(HINS_ret));
}

void HighLevelCodegen::visit_statement_list(Node *n) {
  // TODO: implement
  for (auto i = n->cbegin(); i != n->cend(); ++i) {
    //setup
    Node *stmt = *i;
    visit(stmt);
  }
}

void HighLevelCodegen::visit_expression_statement(Node *n) {
  // TODO: implement
  visit(n->get_kid(0));
}

void HighLevelCodegen::visit_return_statement(Node *n) {
  // TODO: implement
}

void HighLevelCodegen::visit_return_expression_statement(Node *n) {
  // A possible implementation:
  Node *expr = n->get_kid(0);

  // generate code to evaluate the expression
  visit(expr);

  // move the computed value to the return value vreg
  HighLevelOpcode mov_opcode = get_opcode(HINS_mov_b, expr->get_type());
  get_hl_iseq()->append(new Instruction(mov_opcode, Operand(Operand::VREG, LocalStorageAllocation::VREG_RETVAL), expr->get_operand()));

  // jump to the return label
  visit_return_statement(n);
}

void HighLevelCodegen::visit_while_statement(Node *n) {
  //define label names
  std::string loop_label = next_label();
  std::string m_top_label_name = loop_label + "_while_loop";
  std::string m_bottom_label_name = loop_label + "_end_while_loop";

  //LOOP:
  get_hl_iseq()->define_label(m_top_label_name);
  //conditional statement
  Node* condition = n->get_kid(0);
  visit(condition);
  Operand loop = condition->get_operand();
  get_hl_iseq()->append(new Instruction(HINS_cjmp_f, loop, Operand(Operand::LABEL, m_bottom_label_name)));

  //loop body
  Node* body = n->get_kid(1);
  visit(body);

  get_hl_iseq()->append(new Instruction(HINS_jmp, Operand(Operand::LABEL, m_top_label_name)));   //continue loop
  get_hl_iseq()->define_label(m_bottom_label_name);
}

void HighLevelCodegen::visit_do_while_statement(Node *n) {
  //define label names
  std::string loop_label = next_label();
  std::string m_top_label_name = loop_label + "_do_while_loop";
  std::string m_bottom_label_name = loop_label + "_end_do_while_loop";

  //LOOP:
  get_hl_iseq()->define_label(m_top_label_name);

  //loop body
  Node* body = n->get_kid(0);
  visit(body);

  //conditional statement
  Node* condition = n->get_kid(1);
  visit(condition);
  Operand loop = condition->get_operand();
  get_hl_iseq()->append(new Instruction(HINS_cjmp_t, loop, Operand(Operand::LABEL, m_top_label_name)));

  //drop out of loop
  get_hl_iseq()->define_label(m_bottom_label_name);
}

void HighLevelCodegen::visit_for_statement(Node *n) {
  //define label names
  std::string if_label = next_label();
  std::string m_top_label_name = if_label + "_for_loop";
  std::string m_comp_label_name = if_label + "_for_loop_comp";
  std::string m_body_label_name = if_label + "_for_loop_body";
  std::string m_bottom_label_name = if_label + "_end_for_loop";

  //LOOP:
  get_hl_iseq()->define_label(m_top_label_name);
  
  //init conditional statement
  Node* def_loop_it = n->get_kid(0);
  visit(def_loop_it);
  get_hl_iseq()->define_label(m_comp_label_name);

  //conditional statement
  Node* loop_comp = n->get_kid(1);
  visit(loop_comp);
  Operand comp_res = loop_comp->get_operand();
  get_hl_iseq()->append(new Instruction(HINS_cjmp_f, comp_res, Operand(Operand::LABEL, m_bottom_label_name)));

  //if body
  get_hl_iseq()->define_label(m_body_label_name);
  Node* body = n->get_kid(3);
  visit(body);

  //continue loop
  Node* loop_inc = n->get_kid(2);
  visit(loop_inc);
  get_hl_iseq()->append(new Instruction(HINS_jmp, Operand(Operand::LABEL, m_comp_label_name)));   //continue loop
  
  get_hl_iseq()->define_label(m_bottom_label_name);
}

void HighLevelCodegen::visit_if_statement(Node *n) {
  //define label names
  std::string if_label = next_label();
  std::string m_top_label_name = if_label + "_if_stmt";
  std::string m_body_label_name = if_label + "_if_stmt_body";
  std::string m_bottom_label_name = if_label + "_end_if_stmt";

  //LOOP:
  get_hl_iseq()->define_label(m_top_label_name);
  //conditional statement
  Node* condition = n->get_kid(0);
  visit(condition);
  Operand loop = condition->get_operand();
  get_hl_iseq()->append(new Instruction(HINS_cjmp_f, loop, Operand(Operand::LABEL, m_bottom_label_name)));

  //if body
  get_hl_iseq()->define_label(m_body_label_name);
  Node* body = n->get_kid(1);
  visit(body);

  get_hl_iseq()->define_label(m_bottom_label_name);
}

void HighLevelCodegen::visit_if_else_statement(Node *n) {
  //define label names
  std::string if_label = next_label();
  std::string m_top_label_name = if_label + "_if_stmt";
  std::string m_body1_label_name = if_label + "_if_stmt_body";
  std::string m_body2_label_name = if_label + "_else_stmt_body";
  std::string m_bottom_label_name = if_label + "_end_if_stmt";

  //LOOP:
  get_hl_iseq()->define_label(m_top_label_name);
  //conditional statement
  Node* condition = n->get_kid(0);
  visit(condition);
  Operand loop = condition->get_operand();
  get_hl_iseq()->append(new Instruction(HINS_cjmp_f, loop, Operand(Operand::LABEL, m_body2_label_name))); //skip to else

  //if body
  get_hl_iseq()->define_label(m_body1_label_name);
  Node* if_body = n->get_kid(1);
  visit(if_body);
  get_hl_iseq()->append(new Instruction(HINS_jmp, Operand(Operand::LABEL, m_bottom_label_name)));//exit if

  //else body 
  get_hl_iseq()->define_label(m_body2_label_name);
  Node* else_body = n->get_kid(2);
  visit(else_body);  //TODO ELSE IF IS FAILING
  get_hl_iseq()->append(new Instruction(HINS_jmp, Operand(Operand::LABEL, m_bottom_label_name)));//exit if

  get_hl_iseq()->define_label(m_bottom_label_name);
}

void HighLevelCodegen::visit_binary_expression(Node *n) {
  //get operaton
  std::string op = n->get_kid(0)->get_str();
  HighLevelOpcode opcode;

  //process left and right operands
  Node* lhs = n->get_kid(1);
  Node* rhs = n->get_kid(2);
  visit(lhs);
  visit(rhs);
  Operand l_reg = lhs->get_operand();
  Operand r_reg = rhs->get_operand();

  if (op == "=") {//assignment
    opcode = get_opcode(HINS_mov_b, n->get_type());
    get_hl_iseq()->append(new Instruction(opcode, l_reg, r_reg));

    n->set_operand(l_reg);
  } else { //arithmatic  
    if (op == "+") {
      opcode = get_opcode(HINS_add_b, n->get_type());
    } else if (op == "-") {
      opcode = get_opcode(HINS_sub_b, n->get_type());
    } else if (op == "*") {
      opcode = get_opcode(HINS_mul_b, n->get_type());
    } else if (op == "/") {
      opcode = get_opcode(HINS_div_b, n->get_type());
    } else if (op == "<") {
      opcode = get_opcode(HINS_cmplt_b , n->get_type());
    } else if (op == "<=") {
      opcode = get_opcode(HINS_cmplte_b, n->get_type());
    } else if (op == ">") {
      opcode = get_opcode(HINS_cmpgt_b, n->get_type());
    } else if (op == ">=") {
      opcode = get_opcode(HINS_cmpgte_b, n->get_type());
    } else if (op == "==") {
      opcode = get_opcode(HINS_cmpeq_b, n->get_type());
    } else if (op == "!=") {
      opcode = get_opcode(HINS_cmpneq_b, n->get_type());
    }

    //setup temp destintation
    int i_v_temp = m_function->get_vra()->alloc_local();
    Operand v_temp = Operand(Operand::VREG, i_v_temp);


    get_hl_iseq()->append(new Instruction(opcode, v_temp, l_reg, r_reg));

    n->set_operand(v_temp);
  }
}

void HighLevelCodegen::visit_unary_expression(Node *n) {
  //get operaton
  std::string op = n->get_kid(0)->get_str();
  HighLevelOpcode opcode;

  //process left and right operands
  Node* value = n->get_kid(1);
  visit(value);
  Operand reg = value->get_operand();

  if (op == "!" || op == "-" ) { //arithmatic
    if (op == "-") {
      opcode = get_opcode(HINS_neg_b, n->get_type());
    } else if (op == "!") {
      opcode = get_opcode(HINS_not_b, n->get_type());
    } 
    
    //setup temp destintation
    int i_v_temp = m_function->get_vra()->alloc_local();
    Operand v_temp = Operand(Operand::VREG, i_v_temp);


    get_hl_iseq()->append(new Instruction(opcode, v_temp, reg));

    n->set_operand(v_temp);
  } else { //pointer computations

  }

}

void HighLevelCodegen::visit_function_call_expression(Node *n) {
  // TODO: implement
  std::string fn_name = n->get_kid(0)->get_kid(0)->get_str();
  Node* arg_list = n->get_kid(1);
  visit(arg_list);
  for (auto i = arg_list->cbegin(); i != arg_list->cend(); ++i) {
    //setup
    Node *arg = *i;
    int index = std::distance(arg_list->cbegin(), i) + 1;

    //get assignment op
    HighLevelOpcode opcode = get_opcode(HINS_mov_b, arg->get_type());

    //get fn register
    Operand f_reg = Operand(Operand::VREG, index);

    //get source register
    Operand s_reg = arg->get_operand();

    get_hl_iseq()->append(new Instruction(opcode, f_reg, s_reg));
  }
  get_hl_iseq()->append(new Instruction(HINS_call, Operand(Operand::LABEL,fn_name)));//function call
  n->set_operand(Operand(Operand::VREG,0));//return statement
}

void HighLevelCodegen::visit_field_ref_expression(Node *n) {
  // TODO: implement
}

void HighLevelCodegen::visit_indirect_field_ref_expression(Node *n) {
  // TODO: implement
}

void HighLevelCodegen::visit_array_element_ref_expression(Node *n) {
  // TODO: implement
}

void HighLevelCodegen::visit_variable_ref(Node *n) {
  // TODO: implement
  Symbol* s = n->get_symbol();
  if (s->get_reg() != -1) {
    n->set_operand(Operand(Operand::VREG, s->get_reg()));
  } else if (s->get_al() != -1) {
    n->set_operand(Operand(Operand::VREG_MEM, s->get_al()));
  } else {
    SemanticError::raise(n->get_loc(), "for some reason owen was very silly and did not allocate virtual storage for this variable");
  }
}

void HighLevelCodegen::visit_literal_value(Node *n) {
  // A partial implementation (note that this won't work correctly
  // for string constants!):
  LiteralValue val;
  if (n->get_type()->get_basic_type_kind() == BasicTypeKind::INT) {
    val = LiteralValue(std::stoi(n->get_kid(0)->get_str()),false,false);
  } else { // is char
    val = LiteralValue(n->get_kid(0)->get_str());
  }
  int vreg = m_function->get_vra()->alloc_local();
  Operand dest(Operand::VREG, vreg);
  HighLevelOpcode mov_opcode = get_opcode(HINS_mov_b, n->get_type());
  get_hl_iseq()->append(new Instruction(mov_opcode, dest, Operand(Operand::IMM_IVAL, val.get_int_value())));
  n->set_operand(dest);
}

void HighLevelCodegen::visit_implicit_conversion(Node *n) {
  // TODO: implement
}

std::string HighLevelCodegen::next_label() {
  std::string label = ".L" + std::to_string(m_next_label_num++);
  return label;
}

// TODO: additional private member functions
