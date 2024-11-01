struct Foo {
  int x;
  int y;
};

int main(void) {
  struct Foo* f;
  f->y = 1;
  return 0;
}