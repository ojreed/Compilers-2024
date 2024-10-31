// A function declaration could use different
// parameter names than the corresponding function
// definition. This is legal, but the semantic
// analyzer will need to update the symbol table
// information so that the function's symbol
// table has entries reflecting the function
// definition's parameter names rather than the
// function declaration's parameter names.

int sum(int x, int y);

int sum(int a, int b) {
  return a + b;
}
