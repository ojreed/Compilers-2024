void print_i32(int n);
void print_nl(void);

int main() {
  int a, b, c, d, e, f;
  a = 10;
  b = 20;
  c = a + b;      // Expression 1
  d = a + b;      // Expression 2 (redundant with Expression 1)
  e = c * 2;      // Expression 3
  f = (a + b) * 2; // Expression 4 (redundant but more complex)

  print_i32(c);
  print_i32(d);
  print_i32(e);
  print_i32(f);

  int x;
  int y;
  x = 1 + 2;
  y = 1 + 2;
  x = 1 + 2;
  y = 1 + 2;
  x = 1 + 2;

  return 0;
}