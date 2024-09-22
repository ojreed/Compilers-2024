#include <cassert>
#include <iostream> 
#include <map>
#include <string>
#include <memory>
#include "token.h"
#include "ast.h"
#include "exceptions.h"
#include "parser2.h"
/*
TODO LIST
  0) fix optional input
  1) fix if/else
  2) convert all to smart pointers
*/

////////////////////////////////////////////////////////////////////////
// Parser2 implementation
// This version of the parser builds an AST directly,
// rather than first building a parse tree.
////////////////////////////////////////////////////////////////////////

// This is the grammar (Unit is the start symbol):
//
// Unit -> TStmt
// Unit -> TStmt Unit
// Stmt -> A ;
// Stmt → var ident ;
// E -> T E'
// E' -> + T E'
// E' -> - T E'
// E' -> epsilon
// T -> F T'
// T' -> * F T'
// T' -> / F T'
// T' -> epsilon
// F -> number
// F -> ident
// F -> ( E )
// A    → ident = A
// A    → L
// L    → R || R
// L    → R && R
// L    → R
// R    → E < E
// R    → E <= E
// R    → E > E
// R    → E >= E
// R    → E == E
// R    → E != E
// R    → E
// TStmt →      Stmt
// TStmt →      Func
// Stmt →       if ( A ) { SList }                     -- if stmt
// Stmt →       if ( A ) { SList } else { SList }      -- if/else stmt 
// Stmt →       while ( A ) { SList }                  -- while loop
// Func →       function ident ( OptPList ) { SList }  -- function def
// OptPList →   PList                                  -- opt. param list
// OptPList →   ε
// PList →      ident                            -- nonempty param list
// PList →      ident , PList
// SList →      Stmt                             -- stmt list
// SList →      Stmt SList
// F →          ident ( OptArgList )             -- function call
// OptArgList → ArgList                          -- opt. arg list
// OptArgList → ε
// ArgList →    L                                -- nonempty arg list
// ArgList →    L , ArgList

Parser2::Parser2(Lexer *lexer_to_adopt)
  : m_lexer(lexer_to_adopt)
  , m_next(nullptr) {
}

Parser2::~Parser2() {
  delete m_lexer;
}

Node *Parser2::parse() {
  return parse_Unit();
}
Node *Parser2::parse_Unit() {
  // note that this function produces a "flattened" representation
  // of the unit
  std::unique_ptr<Node> unit(new Node(AST_UNIT));
  for (;;) {
    unit->append_kid(parse_TStmt());
    if (m_lexer->peek() == nullptr)
      break;
  }

  return unit.release();
}

Node *Parser2::parse_TStmt() {
  Node *next_tok = m_lexer->peek(1);
  if (next_tok == nullptr) {
    SyntaxError::raise(m_lexer->get_current_loc(), "Unexpected end of input looking for statement");
  }
  int next_tok_tag = next_tok->get_tag();
  if (next_tok_tag == TOK_FUNC) {
    return parse_func();
  } else {
    return parse_Stmt();
  }
}


Node *Parser2::parse_Stmt() {
  // Stmt -> ^ A ;
  // Stmt -> ^ var ident ;
  //Stmt →       if ( A ) { SList }                     -- if stmt
  //Stmt →       if ( A ) { SList } else { SList }      -- if/else stmt 
  //Stmt →       while ( A ) { SList }                  -- while loop
  std::unique_ptr<Node> s(new Node(AST_STATEMENT));

  Node *next_tok = m_lexer->peek(1);
  Node *next_next_tok = m_lexer->peek(2);
  if (next_tok == nullptr || next_next_tok == nullptr) {
    SyntaxError::raise(m_lexer->get_current_loc(), "Unexpected end of input looking for statement");
  }
  int next_tok_tag = next_tok->get_tag();
  if (next_tok_tag == TOK_VAR) { //PARSE VAR ASSIGNMENT
      //add VAR and IDENT
      // Stmt -> ^ var ident ;
      Node* vardef(expect(static_cast<enum TokenKind>(next_tok_tag)));
      vardef->set_tag(AST_VARDEF);
      vardef->set_str("");
      Node* varref(expect(TOK_IDENTIFIER));
      varref->set_tag(AST_VARREF);
      vardef->append_kid(varref);
      s->append_kid(vardef);
      //Always look for semicolon
      expect_and_discard(TOK_SEMICOLON);
  } else if(next_tok_tag == TOK_IF) { //PARSE IF/ELSE
    //Stmt → if ( A ) { SList }                     -- if stmt
    //Stmt → if ( A ) { SList } else { SList } 
    Node* if_node(expect(static_cast<enum TokenKind>(next_tok_tag))); //CONSUME IF
    if_node->set_tag(AST_IF);
    if_node->set_str("");
    //CONSUME COMPARATIVE
    expect_and_discard(TOK_LPAREN);
    if_node->append_kid(parse_A());
    expect_and_discard(TOK_RPAREN);
    //CONSUME Statement List
    expect_and_discard(TOK_LBRACK);
    if_node->append_kid(parse_SList());
    expect_and_discard(TOK_RBRACK);
    //Stmt → if ( A ) { SList } ^                     -- if stmt
    //Stmt → if ( A ) { SList } ^ else { SList } 
    Node *internal_next_tok = m_lexer->peek(1);
    if (internal_next_tok != nullptr) {
      int internal_next_tok_tag = internal_next_tok->get_tag();
      if (internal_next_tok_tag == TOK_ELSE){
        Node* else_node(expect(static_cast<enum TokenKind>(internal_next_tok_tag))); //CONSUME IF
        else_node->set_str("");
        else_node->set_tag(AST_ELSE);
        expect_and_discard(TOK_LBRACK);
        else_node->append_kid(parse_SList());
        expect_and_discard(TOK_RBRACK);
        if_node->append_kid(else_node);
      }
    }
    s->append_kid(if_node);
  } else if(next_tok_tag == TOK_WHILE) { //PARSE WHILE
    //Stmt → while ( A ) { SList }                  -- while loop
    Node* wh_node(expect(static_cast<enum TokenKind>(next_tok_tag))); //CONSUME IF
    wh_node->set_tag(AST_WHILE);
    wh_node->set_str("");
    //CONSUME COMPARATIVE
    expect_and_discard(TOK_LPAREN);
    wh_node->append_kid(parse_A());
    expect_and_discard(TOK_RPAREN);
    //CONSUME Statement List
    expect_and_discard(TOK_LBRACK);
    wh_node->append_kid(parse_SList());
    expect_and_discard(TOK_RBRACK);
    s->append_kid(wh_node);
    //Stmt → while ( A ) { SList } ^                  -- while loop
  } else {
      //Add Parse A
      // Stmt -> ^ A ;
      s->append_kid(parse_A());
      //Always look for semicolon
      expect_and_discard(TOK_SEMICOLON);
  }
  s->set_str("");
  return s.release();
}

Node *Parser2::parse_func(){
  //Func → function ident ( OptPList ) { SList }  -- function def
  //Establish Function
  std::unique_ptr<Node> func_def(new Node(AST_FUNC));
  expect_and_discard(TOK_FUNC);
  Node* ID(expect(TOK_IDENTIFIER));
  ID->set_tag(AST_VARREF);
  func_def->append_kid(ID);

  //Setup Parameter List
  expect_and_discard(TOK_LPAREN);
  Node* PL(parse_OptPList());
  if (PL != nullptr) {  
    func_def->append_kid(PL); 
  }
  expect_and_discard(TOK_RPAREN);
  
  //Setup Statement list
  expect_and_discard(TOK_LBRACK);
  func_def->append_kid(parse_SList());
  expect_and_discard(TOK_RBRACK);
  
  return func_def.release();
}

Node *Parser2::parse_OptPList(){
  //OptPList →   PList                                  -- opt. param list
  //OptPList →   ε
  Node *next_tok = m_lexer->peek(1);
  if (next_tok == nullptr) {
    SyntaxError::raise(m_lexer->get_current_loc(), "Unexpected end of input looking for statement");
  }
  int next_tok_tag = next_tok->get_tag();
  if (next_tok_tag != TOK_RPAREN){
    return parse_PList();
  }
  return nullptr; 
}

Node *Parser2::parse_OptArgList(){
  //ArgList →    L                                -- nonempty arg list
  //ArgList →    L , ArgList
  Node *next_tok = m_lexer->peek(1);
  if (next_tok == nullptr) {
    SyntaxError::raise(m_lexer->get_current_loc(), "Unexpected end of input looking for statement");
  }
  int next_tok_tag = next_tok->get_tag();
  if (next_tok_tag != TOK_RPAREN){
    return parse_ArgList();
  }
  return nullptr; 
}

Node *Parser2::parse_PList(){
  //PList →      ident                            -- nonempty param list
  //PList →      ident , PList
  std::unique_ptr<Node> PL(new Node(AST_PARAMETER_LIST));

  Node *next_tok = m_lexer->peek(1);
  if (next_tok == nullptr) {
    SyntaxError::raise(m_lexer->get_current_loc(), "Unexpected end of input looking for statement");
  }
  int next_tok_tag = next_tok->get_tag();
  Node* ID(expect(static_cast<enum TokenKind>(next_tok_tag)));
  ID->set_tag(AST_VARREF);
  PL->append_kid(ID);
  //PList →      ident ^                            -- nonempty param list
  //PList →      ident ^ , PList
  //possible continuation after comma
  Node *potential_next_tok = m_lexer->peek(1);
  if (potential_next_tok == nullptr) {
    SyntaxError::raise(m_lexer->get_current_loc(), "Unexpected end of input looking for statement");
  }
  int potential_next_tok_tag = potential_next_tok->get_tag();
  while (potential_next_tok_tag == TOK_COMMA){
    expect_and_discard(TOK_COMMA);
    Node* ID(expect(static_cast<enum TokenKind>(next_tok_tag)));
    ID->set_tag(AST_VARREF);
    PL->append_kid(ID);
    potential_next_tok = m_lexer->peek(1);
    potential_next_tok_tag = potential_next_tok->get_tag();
  }
  return PL.release();
}

Node *Parser2::parse_ArgList(){
  //ArgList →    L                                -- nonempty arg list
  //ArgList →    L , ArgList
  std::unique_ptr<Node> AL(new Node(AST_ARG_LIST));

  Node *next_tok = m_lexer->peek(1);
  if (next_tok == nullptr) {
    SyntaxError::raise(m_lexer->get_current_loc(), "Unexpected end of input looking for statement");
  }
  AL->append_kid(parse_L());
  //ArgList →    L ^                               -- nonempty arg list
  //ArgList →    L ^, ArgList
  //possible continuation after comma
  Node *potential_next_tok = m_lexer->peek(1);
  if (potential_next_tok == nullptr) {
    SyntaxError::raise(m_lexer->get_current_loc(), "Unexpected end of input looking for statement");
  }
  int potential_next_tok_tag = potential_next_tok->get_tag();
  while (potential_next_tok_tag == TOK_COMMA){
    expect_and_discard(TOK_COMMA);
    AL->append_kid(parse_L());
    potential_next_tok = m_lexer->peek(1);
    potential_next_tok_tag = potential_next_tok->get_tag();
  }
  return AL.release();
}

Node *Parser2::parse_SList(){
  //SList →      Stmt                             -- stmt list
  //SList →      Stmt SList
  std::unique_ptr<Node> SL(new Node(AST_STATEMENT_LIST));

  Node *next_tok = m_lexer->peek(1);
  if (next_tok == nullptr) {
    SyntaxError::raise(m_lexer->get_current_loc(), "Unexpected end of input looking for statement");
  }
  SL->append_kid(parse_Stmt());
  //SList →      Stmt ^                           -- stmt list
  //SList →      Stmt ^ SList
  //possible continuation wihtout comma
  Node *potential_next_tok = m_lexer->peek(1);
  if (potential_next_tok == nullptr) {
    SyntaxError::raise(m_lexer->get_current_loc(), "Unexpected end of input looking for statement");
  }
  int potential_next_tok_tag = potential_next_tok->get_tag();
  while (potential_next_tok_tag != TOK_RBRACK){
    SL->append_kid(parse_Stmt());
    potential_next_tok = m_lexer->peek(1);
    potential_next_tok_tag = potential_next_tok->get_tag();
  }
  return SL.release();
}

Node *Parser2::parse_A() {
    //TODO: implement
    //A    → ident = A
    //A    → L

    Node *next_tok = m_lexer->peek(1);
    Node *next_next_tok = m_lexer->peek(2);

    int next_tok_tag = next_tok->get_tag();
    int next_next_tok_tag = next_next_tok->get_tag();
    
    if (next_tok == nullptr || next_next_tok == nullptr) {
        SyntaxError::raise(m_lexer->get_current_loc(), "Unexpected end of input looking for statement");
    }

    if (next_tok_tag == TOK_IDENTIFIER && next_next_tok_tag == TOK_ASSIGN) {
        //Assign <- ID and A
         Node* ID(expect(static_cast<enum TokenKind>(next_tok_tag)));
         Node* assign(expect(static_cast<enum TokenKind>(next_next_tok_tag)));
         assign->set_tag(AST_ASSIGN);
         ID->set_tag(AST_VARREF);
         assign->append_kid(ID);
         assign->append_kid(parse_A());
         assign->set_str("");
         return assign;
    } else {
        return parse_L();
    }
}


Node *Parser2::parse_L() {
    //L    → R || R
    //L    → R && R
    //L    → R

    Node *ast = parse_R(); //Always parse a front R

    Node *next_tok = m_lexer->peek();
    if (next_tok == nullptr) {
        SyntaxError::raise(m_lexer->get_current_loc(), "Unexpected end of input looking for statement");
    }
    //L    → R || R
    //L    → R && R
    int next_tok_tag = next_tok->get_tag();
    if (next_tok_tag == TOK_LOR || next_tok_tag == TOK_LAND){
        std::unique_ptr<Node> op(expect(static_cast<enum TokenKind>(next_tok_tag)));
        op->set_tag(next_tok_tag == TOK_LOR ? AST_LOR : AST_LAND);
        op->append_kid(ast);
        op->append_kid(parse_R());
        op->set_str("");
        return op.release();
    }
    return ast;
}

Node *Parser2::parse_R() {
    Node *ast = parse_E();
    Node *next_tok = m_lexer->peek();
    if (next_tok == nullptr) {
      SyntaxError::raise(m_lexer->get_current_loc(), "Unexpected end of input looking for statement");
    }
    int next_tok_tag = next_tok->get_tag();
    bool op_found = false;
    //R -> E [LOGICAL COMP] E
    std::unique_ptr<Node> op;
    if (next_tok_tag == TOK_LL) { //X = <
      op = std::unique_ptr<Node>(expect(static_cast<enum TokenKind>(next_tok_tag)));
      op->set_tag(AST_LL);
      op_found = true;
    } else if (next_tok_tag == TOK_LLE) { //X = <=
      op = std::unique_ptr<Node>(expect(static_cast<enum TokenKind>(next_tok_tag)));
      op->set_tag(AST_LLE);
      op_found = true;
    } else if (next_tok_tag == TOK_LG) { //X > 
      op = std::unique_ptr<Node>(expect(static_cast<enum TokenKind>(next_tok_tag)));
      op->set_tag(AST_LG);
      op_found = true;
    } else if (next_tok_tag == TOK_LGE) { //X = >=
      op = std::unique_ptr<Node>(expect(static_cast<enum TokenKind>(next_tok_tag)));
      op->set_tag(AST_LGE);
      op_found = true;
    } else if (next_tok_tag == TOK_LE) { //X = ==
      op = std::unique_ptr<Node>(expect(static_cast<enum TokenKind>(next_tok_tag)));
      op->set_tag(AST_LE);
      op_found = true;
    } else if (next_tok_tag == TOK_LNE) {//X = !=
      op = std::unique_ptr<Node>(expect(static_cast<enum TokenKind>(next_tok_tag)));
      op->set_tag(AST_LNE);
      op_found = true;
    }
    if (op_found){ //Shared work for all logical op types
        op->append_kid(ast);
        op->append_kid(parse_E());
        op->set_str("");
        return op.release();
    }
    //Base Case of R -> E
    return ast;
}

Node *Parser2::parse_E() {
  // E -> ^ T E'

  // Get the AST corresponding to the term (T)
  Node *ast = parse_T();

  // Recursively continue the additive expression
  return parse_EPrime(ast);
}

// This function is passed the "current" portion of the AST
// that has been built so far for the additive expression.
Node *Parser2::parse_EPrime(Node *ast_) {
  // E' -> ^ + T E'
  // E' -> ^ - T E'
  // E' -> ^ epsilon

  std::unique_ptr<Node> ast(ast_);

  // peek at next token
  Node *next_tok = m_lexer->peek();
  if (next_tok != nullptr) {
    int next_tok_tag = next_tok->get_tag();
    if (next_tok_tag == TOK_PLUS || next_tok_tag == TOK_MINUS)  {
      // E' -> ^ + T E'
      // E' -> ^ - T E'
      std::unique_ptr<Node> op(expect(static_cast<enum TokenKind>(next_tok_tag)));

      // build AST for next term, incorporate into current AST
      Node *term_ast = parse_T();
      ast.reset(new Node(next_tok_tag == TOK_PLUS ? AST_ADD : AST_SUB, {ast.release(), term_ast}));

      // copy source information from operator node
      ast->set_loc(op->get_loc());

      // continue recursively
      return parse_EPrime(ast.release());
    }
  }

  // E' -> ^ epsilon
  // No more additive operators, so just return the completed AST
  return ast.release();
}

Node *Parser2::parse_T() {
  // T -> F T'

  // Parse primary expression
  Node *ast = parse_F();

  // Recursively continue the multiplicative expression
  return parse_TPrime(ast);
}

Node *Parser2::parse_TPrime(Node *ast_) {
  // T' -> ^ * F T'
  // T' -> ^ / F T'
  // T' -> ^ epsilon

  std::unique_ptr<Node> ast(ast_);

  // peek at next token
  Node *next_tok = m_lexer->peek();
  if (next_tok != nullptr) {
    int next_tok_tag = next_tok->get_tag();
    if (next_tok_tag == TOK_TIMES || next_tok_tag == TOK_DIVIDE)  {
      // T' -> ^ * F T'
      // T' -> ^ / F T'
      std::unique_ptr<Node> op(expect(static_cast<enum TokenKind>(next_tok_tag)));

      // build AST for next primary expression, incorporate into current AST
      Node *primary_ast = parse_F();
      ast.reset(new Node(next_tok_tag == TOK_TIMES ? AST_MULTIPLY : AST_DIVIDE, {ast.release(), primary_ast}));

      // copy source information from operator node
      ast->set_loc(op->get_loc());

      // continue recursively
      return parse_TPrime(ast.release());
    }
  }

  // T' -> ^ epsilon
  // No more multiplicative operators, so just return the completed AST
  return ast.release();
}

Node *Parser2::parse_F() {
  // F -> ^ number
  // F -> ^ ident
  // F -> ^ ( E )
  //F →   ^ ident ( OptArgList )             -- function call

  Node *next_tok = m_lexer->peek(1);
  Node *next_next_tok = m_lexer->peek(2);
  if (next_tok == nullptr || next_next_tok == nullptr) {
    SyntaxError::raise(m_lexer->get_current_loc(), "Unexpected end of input looking for statement");
  }
  int tag = next_tok->get_tag();
  int tag2 = next_next_tok->get_tag();
  if (tag == TOK_IDENTIFIER && tag2 == TOK_LPAREN){
    std::unique_ptr<Node> fn_call(new Node(AST_FNCALL));
    //store identifier
    Node* ID(expect(TOK_IDENTIFIER));
    ID->set_tag(AST_VARREF);
    fn_call->append_kid(ID);
    //Store arguments
    expect_and_discard(TOK_LPAREN);
    Node* AL(parse_OptArgList());
    if (AL != nullptr) {  
      fn_call->append_kid(AL); 
    }
    expect_and_discard(TOK_RPAREN);
    return fn_call.release();
  } else if (tag == TOK_INTEGER_LITERAL || tag == TOK_IDENTIFIER) {
    // F -> ^ number
    // F -> ^ ident
    std::unique_ptr<Node> tok(expect(static_cast<enum TokenKind>(tag)));
    int ast_tag = tag == TOK_INTEGER_LITERAL ? AST_INT_LITERAL : AST_VARREF;
    std::unique_ptr<Node> ast(new Node(ast_tag));
    ast->set_str(tok->get_str());
    ast->set_loc(tok->get_loc());
    return ast.release();
  } else if (tag == TOK_LPAREN) {
    // F -> ^ ( A )
    expect_and_discard(TOK_LPAREN);
    std::unique_ptr<Node> ast(parse_A());
    expect_and_discard(TOK_RPAREN);
    return ast.release();
  } else {
    SyntaxError::raise(next_tok->get_loc(), "Invalid primary expression");
  }
}

Node *Parser2::expect(enum TokenKind tok_kind) {
  std::unique_ptr<Node> next_terminal(m_lexer->next());
  if (next_terminal->get_tag() != tok_kind) {
    SyntaxError::raise(next_terminal->get_loc(), "Unexpected token '%s'", next_terminal->get_str().c_str());
  }
  return next_terminal.release();
}

void Parser2::expect_and_discard(enum TokenKind tok_kind) {
  Node *tok = expect(tok_kind);
  delete tok;
}

void Parser2::error_at_current_loc(const std::string &msg) {
  SyntaxError::raise(m_lexer->get_current_loc(), "%s", msg.c_str());
}
