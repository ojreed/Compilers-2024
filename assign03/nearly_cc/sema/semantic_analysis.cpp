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
#include <algorithm>
#include <utility>
#include <map>
#include <format>
#include "grammar_symbols.h"
#include "parse.tab.h"
#include "node.h"
#include "ast.h"
#include "exceptions.h"
#include "semantic_analysis.h"
#include "symtab.h"



SemanticAnalysis::SemanticAnalysis(const Options &options)
  : m_options(options)
  , m_global_symtab(new SymbolTable(nullptr, "global")) {
  m_cur_symtab = m_global_symtab;
  m_all_symtabs.push_back(m_global_symtab);
}

SemanticAnalysis::~SemanticAnalysis() {
  // The semantic analyzer owns the SymbolTables and their Symbols,
  // so delete them. Note that the AST has pointers to Symbol objects,
  // so the SemanticAnalysis object should live at least as long
  // as the AST.
  for (auto i = m_all_symtabs.begin(); i != m_all_symtabs.end(); ++i)
    delete *i;
}

void SemanticAnalysis::visit_struct_type(Node *n) {
  // TODO: implement
}

void SemanticAnalysis::visit_union_type(Node *n) {
  RuntimeError::raise("union types aren't supported");
}

void SemanticAnalysis::visit_variable_declaration(Node *n) {
  // visit the base type
  visit(n->get_kid(1));
  std::shared_ptr<Type> base_type = n->get_kid(1)->get_type();
  // iterate through declarators, adding variables
  // to the symbol table
  Node *decl_list = n->get_kid(2);
  for (auto i = decl_list->cbegin(); i != decl_list->cend(); ++i) {
    Node *declarator = *i;
    declarator->set_type(base_type);
    if (declarator->get_tag() == AST_NAMED_DECLARATOR) {
      visit_named_declarator(declarator);
    } else if (declarator->get_tag() == AST_ARRAY_DECLARATOR) {
      visit_array_declarator(declarator);
    } else if (declarator->get_tag() == AST_POINTER_DECLARATOR) {
      visit_pointer_declarator(declarator);
    }
    m_cur_symtab->add_entry(n->get_loc(),SymbolKind::VARIABLE,declarator->get_str(),declarator->get_type());
  }
}

void SemanticAnalysis::visit_basic_type(Node *n) {
  std::shared_ptr<Type> type; 
  std::set<std::string> type_spec;
  std::set<std::string> qual_spec;
  for (auto i = n->cbegin(); i != n->cend(); ++i) {
    Node *type_quality = *i;
    if (type_quality->get_str() == "int" || type_quality->get_str() == "char" || type_quality->get_str() == "void"){
      type_spec.insert(type_quality->get_str());
    } else {
      qual_spec.insert(type_quality->get_str());
      if (type_quality->get_str() == "long" || type_quality->get_str() == "short") {
        type_spec.insert("int");
      }
    }
  }
  if (type_spec.size()!=1){
    SemanticError::raise(n->get_loc(), "Improper combination of type specifications");
  }
  if (type_spec.find("char") != type_spec.end()){
    type = std::make_shared<BasicType>(BasicTypeKind::CHAR, false);
  } else if (type_spec.find("int") != type_spec.end()){
    if (qual_spec.find("long") != qual_spec.end()){ //long int
      if (qual_spec.find("unsigned") != qual_spec.end()) {
        type = std::make_shared<BasicType>(BasicTypeKind::LONG, false);
      } else {
        type = std::make_shared<BasicType>(BasicTypeKind::LONG, true);
      }
    } else if (qual_spec.find("short") != qual_spec.end()){ //short int
      if (qual_spec.find("unsigned") != qual_spec.end()) {
        type = std::make_shared<BasicType>(BasicTypeKind::SHORT, false);
      } else {
        type = std::make_shared<BasicType>(BasicTypeKind::SHORT, true);
      }
    } else { //pure int
      if (qual_spec.find("unsigned") != qual_spec.end()) {
        type = std::make_shared<BasicType>(BasicTypeKind::INT, false);
      } else {
        type = std::make_shared<BasicType>(BasicTypeKind::INT, true);
      }
    }
  } else if (type_spec.find("void") != type_spec.end()){
    type = std::make_shared<BasicType>(BasicTypeKind::VOID, true);
  } 
  if (qual_spec.find("const") != qual_spec.end()) {
    type = std::make_shared<QualifiedType>(type,TypeQualifier::CONST);
  }
  if (qual_spec.find("volatile") != qual_spec.end()) {
    type = std::make_shared<QualifiedType>(type,TypeQualifier::VOLATILE);
  }
  n->set_type(type);
}

void SemanticAnalysis::visit_named_declarator(Node *n) {
  n->set_str(n->get_kid(0)->get_str());
}

void SemanticAnalysis::visit_pointer_declarator(Node *n) {
  n->get_kid(0)->set_type(n->get_type());
  visit(n->get_kid(0));
  std::shared_ptr<Type> type(new PointerType(n->get_kid(0)->get_type()));
  n->reset_type(type);
  n->set_str(n->get_kid(0)->get_str());
}

void SemanticAnalysis::visit_array_declarator(Node *n) {
  n->get_kid(0)->set_type(n->get_type());
  visit(n->get_kid(0));
  std::shared_ptr<Type> type(new ArrayType(n->get_kid(0)->get_type(),std::stoi(n->get_kid(1)->get_str())));
  n->reset_type(type);
  n->set_str(n->get_kid(0)->get_str());
}

void SemanticAnalysis::visit_function_definition(Node *n) {
  // setup for body definition
  visit_function_declaration(n);
  std::shared_ptr<Type> base_type = n->get_kid(0)->get_type();
  std::string fn_name = n->get_kid(1)->get_str();

  //visit statement list
  m_cur_symtab = find_symbol_table_by_name("function " + fn_name);
  Node* stmt_list = n->get_kid(3);
  for (auto i = stmt_list->cbegin(); i != stmt_list->cend(); ++i) {
    Node *stmt = *i;
    visit(stmt);
  }
  leave_scope();
}

void SemanticAnalysis::visit_function_declaration(Node *n) {
  // visit the base type and setup function creation
  visit(n->get_kid(0));
  std::shared_ptr<Type> base_type = n->get_kid(0)->get_type();
  std::string fn_name = n->get_kid(1)->get_str();
  bool first_time = false;

  //enter function scope
  if (find_symbol_table_by_name("function " + fn_name)==nullptr) {
    enter_scope("function " + fn_name);
    first_time = true;
  } else {
    m_cur_symtab = find_symbol_table_by_name("function " + fn_name);
  }

  //visit parameters
  Node* param_list = n->get_kid(2);
  visit_function_parameter_list(param_list);
  
  //define function type
  std::shared_ptr<Type> type(new FunctionType(base_type));
  for (auto i = param_list->cbegin(); i != param_list->cend(); ++i) {
    Node *parameter = *i;
    type->add_member(Member(parameter->get_kid(1)->get_kid(0)->get_str(),parameter->get_type()));
  }
  
  //store fn information
  if (first_time){
    m_cur_symtab->set_fn_type(type);//store info about current function
    n->set_type(type);//annote function def node
  }
  leave_scope();//return to parent
  if (first_time){
    m_cur_symtab->add_entry(n->get_loc(),SymbolKind::FUNCTION,fn_name,n->get_type());//store new function in parent
  }
}

void SemanticAnalysis::visit_function_parameter_list(Node *n) {
  bool first_time = (m_cur_symtab->get_num_entries() == 0);
  int num_parameters = 0;
  if (!first_time) {
    num_parameters = m_cur_symtab->get_num_parameters();
  }
  if (!first_time && num_parameters != std::distance(n->cbegin(), n->cend())){
    SemanticError::raise(n->get_loc(), "Mismatch of number of parameters in redeclaration of funuction");
  }

  for (auto i = n->cbegin(); i != n->cend(); ++i) {
    //setup
    Node *parameter = *i;
    int index = std::distance(n->cbegin(), i);

    if (first_time){
      //basic computation
      visit_function_parameter(parameter);
    } else {
      //redef computation
      Symbol* original = m_cur_symtab->get_entry(0);
      m_cur_symtab->remove_entry(0);
      num_parameters-=1;
      visit_function_parameter(parameter);
      Symbol* updated = m_cur_symtab->get_entry(num_parameters + index -1);
      if (original->get_type()->as_str() != updated->get_type()->as_str()){
        std::string expected_type = original->get_type()->as_str();
        std::string actual_type = updated->get_type()->as_str();
        std::string error_message = "Mismatch of parameter type in redeclaration of function. Expected " + expected_type + ", but got " + actual_type + ".";
        SemanticError::raise(n->get_loc(), error_message.c_str());
      }
    }
  } 
}

void SemanticAnalysis::visit_function_parameter(Node *n) {
  // visit the base type
  visit(n->get_kid(0));
  std::shared_ptr<Type> base_type = n->get_kid(0)->get_type();
  Node* declarator = n->get_kid(1);
  declarator->set_type(base_type);
  if (declarator->get_tag() == AST_NAMED_DECLARATOR) {
    visit_named_declarator(declarator);
    n->set_type(declarator->get_type());
    m_cur_symtab->add_entry(n->get_loc(),SymbolKind::VARIABLE,declarator->get_str(),declarator->get_type());
  } else if (declarator->get_tag() == AST_ARRAY_DECLARATOR) {
    std::shared_ptr<Type> type(new PointerType(declarator->get_type()));
    declarator->reset_type(type);
    m_cur_symtab->add_entry(n->get_loc(),SymbolKind::VARIABLE,declarator->get_kid(0)->get_kid(0)->get_str(),declarator->get_type());
    n->set_type(type);
  } else if (declarator->get_tag() == AST_POINTER_DECLARATOR) {
    visit_pointer_declarator(declarator);
    n->set_type(declarator->get_type());
    m_cur_symtab->add_entry(n->get_loc(),SymbolKind::VARIABLE,declarator->get_str(),declarator->get_type());
  }
  
}

void SemanticAnalysis::visit_statement_list(Node *n) {
  enter_scope(std::format("block {}", n->get_loc().get_line()));
  for (auto i = n->cbegin(); i != n->cend(); ++i) {
    Node *stmt = *i;
    visit(stmt);
  }
  leave_scope();
}

void SemanticAnalysis::visit_return_expression_statement(Node *n) {
  std::shared_ptr<Type> return_type = m_cur_symtab->get_fn_type()->get_base_type();
  visit(n->get_kid(0)); 
  std::shared_ptr<Type> returned_type = n->get_kid(0)->get_type();
  if (!is_same_type(return_type,returned_type)){
    SemanticError::raise(n->get_loc(),"Invalid type of returned value");
  }
  n->set_type(return_type);
}

void SemanticAnalysis::visit_struct_type_definition(Node *n) {
  // TODO: implement
}

void SemanticAnalysis::visit_binary_expression(Node *n) {
  std::string op = n->get_kid(0)->get_str();
  visit(n->get_kid(1));
  visit(n->get_kid(2));
  std::shared_ptr<Type> lhs = n->get_kid(1)->get_type();
  std::shared_ptr<Type> rhs = n->get_kid(2)->get_type();
  std::shared_ptr<Type> final_type;
  if (op == "="){
    //assignment
    test_assignment(n, lhs, rhs);
    final_type = lhs;
  } else if (op == "+" || op == "-" || op == "*" || op == "/") {
    
    if (lhs->is_pointer() != rhs->is_pointer() && (op == "+" || op == "-")) {
      //pointer arithmatic
      if (lhs->is_pointer()) {
        final_type = lhs;
      } else {
        final_type = rhs;
      }
    } else if (!lhs->is_pointer() && !rhs->is_pointer()) {
      //arithmatic
      bool sign = (lhs->is_signed() || rhs->is_signed());
      if (lhs->get_basic_type_kind() == BasicTypeKind::CHAR || rhs->get_basic_type_kind() == BasicTypeKind::CHAR){
        SemanticError::raise(n->get_loc(),"Invalid type for arithmatic");
      }
      if (lhs->get_basic_type_kind() == BasicTypeKind::LONG || rhs->get_basic_type_kind() == BasicTypeKind::LONG ) {
        final_type = std::make_shared<BasicType>(BasicTypeKind::LONG, sign);
      } else if (lhs->get_basic_type_kind() == BasicTypeKind::INT || rhs->get_basic_type_kind() == BasicTypeKind::INT ) {
        final_type = std::make_shared<BasicType>(BasicTypeKind::INT, sign);
      } else if (lhs->get_basic_type_kind() == BasicTypeKind::SHORT || rhs->get_basic_type_kind() == BasicTypeKind::SHORT ) {
        final_type = std::make_shared<BasicType>(BasicTypeKind::INT, sign);
      }  
    } else {
      SemanticError::raise(n->get_loc(),"Invalid double pointer arithmatic");
    }
  } else {
    final_type = std::make_shared<BasicType>(BasicTypeKind::INT, true);
  }
  n->set_type(final_type);
}

void test_assignment(Node* n, std::shared_ptr<Type> lhs, std::shared_ptr<Type> rhs) {
  if (lhs->is_array() || lhs->is_function() || (lhs->is_struct() && !lhs->is_pointer())){//error: lhs is not lvalue
    SemanticError::raise(n->get_loc(),"LHS is not an lvalue");
  }
  if (lhs->is_const() && !lhs->is_pointer()) { //error: assign to a const value
    SemanticError::raise(n->get_loc(),"Invalid attempt to assign to a const variable");
  }
  if (!lhs->is_pointer() && rhs->is_pointer()) { //error: pointer and non-pointer assignment
    SemanticError::raise(n->get_loc(),"Improper assignment of pointer and non-pointer");
  } 
  if (lhs->is_pointer() && rhs->is_pointer()) {//pointer assignment
    if (!lhs->get_base_type()->get_unqualified_type()->is_same(rhs->get_base_type()->get_unqualified_type())) { //error: pointers with diff bases
      SemanticError::raise(n->get_loc(),"Improper assignment of non equivilant base type");
    }
    if (!lhs->is_const() && rhs->is_const()){
      SemanticError::raise(n->get_loc(),"LHS type missing qualifier");
    }
    if (!lhs->is_volatile() && rhs->is_volatile()){
      SemanticError::raise(n->get_loc(),"LHS type missing qualifier");
    }
  }
}

void SemanticAnalysis::visit_unary_expression(Node *n) {
  visit(n->get_kid(1));
  std::string op = n->get_kid(0)->get_str();
  std::shared_ptr<Type> original = n->get_kid(1)->get_type();
  if (op == "&") {
    std::shared_ptr<Type> new_type(new PointerType(original));
    n->set_type(new_type);
  } else if (op == "*") {
    if (!original->is_pointer()) {
      SemanticError::raise(n->get_loc(),"Attempting to dereference non-pointer");
    }
    n->set_type(original->get_base_type());
  } else if (op == "!") {
    n->set_type(original);
  } else if (op == "-") {
    if (original->get_basic_type_kind() == BasicTypeKind::CHAR) {
      SemanticError::raise(n->get_loc(),"Attempting to negate a character");
    }
    n->set_type(std::make_shared<BasicType>(original->get_basic_type_kind(), true));
  }
}

void SemanticAnalysis::visit_postfix_expression(Node *n) {
  // TODO: implement
}

void SemanticAnalysis::visit_conditional_expression(Node *n) {

}

void SemanticAnalysis::visit_cast_expression(Node *n) {
  // TODO: implement
}

void SemanticAnalysis::visit_function_call_expression(Node *n) {
  std::string fn_name = n->get_kid(0)->get_kid(0)->get_str();
  Symbol* function = m_cur_symtab->lookup_recursive(fn_name);
  if (function == nullptr) {
    SemanticError::raise(n->get_loc(),"Undefined Function");
  }
  std::shared_ptr<Type> fn_type = function->get_type();
  std::shared_ptr<Type> return_type = fn_type->get_base_type();
  visit(n->get_kid(1));
  Node *arg_list = n->get_kid(1);
  if (fn_type->get_num_members() != arg_list->get_num_kids()) {
    SemanticError::raise(n->get_loc(),"Improper number of arguments");
  }
  for (auto i = arg_list->cbegin(); i != arg_list->cend(); ++i) {
    //setup
    Node *argument = *i;
    int index = std::distance(arg_list->cbegin(), i);
    test_assignment(n,fn_type->get_member(index).get_type(),argument->get_type());
  }
  n->set_type(return_type);
}

void SemanticAnalysis::visit_field_ref_expression(Node *n) {
  // TODO: implement
}

void SemanticAnalysis::visit_indirect_field_ref_expression(Node *n) {
  // TODO: implement
}

void SemanticAnalysis::visit_array_element_ref_expression(Node *n) {
  visit(n->get_kid(0));
  std::string arr_name = n->get_kid(0)->get_kid(0)->get_str();
  Symbol* arr = m_cur_symtab->lookup_recursive(arr_name);
  if (arr == nullptr) {
    SemanticError::raise(n->get_loc(),"Undefined array");
  }
  std::shared_ptr<Type> arr_type = arr->get_type();
  visit(n->get_kid(1)); //visit index
  std::shared_ptr<Type> index_type = n->get_kid(1)->get_type();
  if (!index_type->is_basic() || index_type->get_basic_type_kind() == BasicTypeKind::CHAR) {
    SemanticError::raise(n->get_loc(),"Improper index type");
  }
  n->set_type(arr_type->get_base_type());
  n->set_str(arr_name);
}

void SemanticAnalysis::visit_variable_ref(Node *n) {
  std::string target_name = n->get_kid(0)->get_str();
  Symbol* var_symbol = m_cur_symtab->lookup_recursive(target_name);
  if (var_symbol == nullptr) {
    SemanticError::raise(n->get_loc(),"Undefined variable reference in this scope");
  }
  n->set_type(var_symbol->get_type());
  n->set_str(target_name);
}

void SemanticAnalysis::visit_literal_value(Node *n) {
  if (n->get_kid(0)->get_tag() == TOK_INT_LIT) {
    n->set_type(std::make_shared<BasicType>(BasicTypeKind::INT, true));
  } else {
    n->set_type(std::make_shared<BasicType>(BasicTypeKind::CHAR, false));
  }
}

SymbolTable *SemanticAnalysis::enter_scope(const std::string &name) {
  SymbolTable *symtab = new SymbolTable(m_cur_symtab, name);
  m_all_symtabs.push_back(symtab);
  m_cur_symtab = symtab;
  return symtab;
}

void SemanticAnalysis::leave_scope() {
  assert(m_cur_symtab->get_parent() != nullptr);
  m_cur_symtab = m_cur_symtab->get_parent();
}

// TODO: implement helper functions
SymbolTable* SemanticAnalysis::find_symbol_table_by_name(const std::string& name) {
  for (const auto& symtab : m_all_symtabs) {
      if (symtab->get_name() == name) {
          return symtab; // Found the SymbolTable with the given name
      }
  }
  return nullptr; // Return nullptr if no match is found
}

//family of functions for lazy type comp
bool is_same_type(Symbol* a, Symbol* b) {
  return (a->get_type()->get_basic_type_kind() == b->get_type()->get_basic_type_kind());
}

bool is_same_type(std::shared_ptr<Type> a, std::shared_ptr<Type> b) {
  return (a->get_basic_type_kind() == b->get_basic_type_kind());
}

bool is_same_type(Symbol* a, std::shared_ptr<Type> b) {
  return (a->get_type()->get_basic_type_kind() == b->get_basic_type_kind());
}

bool is_same_type(std::shared_ptr<Type> a, Symbol* b) {
  return (a->get_basic_type_kind() == b->get_type()->get_basic_type_kind());
}