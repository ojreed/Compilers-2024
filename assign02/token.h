#ifndef TOKEN_H
#define TOKEN_H

// This header file defines the tags used for tokens (i.e., terminal
// symbols in the grammar.)

enum TokenKind {
  TOK_IDENTIFIER,
  TOK_INTEGER_LITERAL,
  TOK_PLUS,
  TOK_MINUS,
  TOK_TIMES,
  TOK_DIVIDE,
  TOK_LPAREN,
  TOK_RPAREN,
  TOK_SEMICOLON,
  //add members for additional kinds of tokens
  TOK_VAR,
  TOK_ASSIGN,
  TOK_LOR,
  TOK_LAND,
  TOK_LL,
  TOK_LLE,
  TOK_LG,
  TOK_LGE,
  TOK_LE,
  TOK_LNE,
  //add members for other AST node kinds HW2
  TOK_FUNC,
  TOK_IF,
  TOK_ELSE,
  TOK_WHILE,
  TOK_LBRACK,
  TOK_RBRACK,
  TOK_COMMA
};

#endif // TOKEN_H
