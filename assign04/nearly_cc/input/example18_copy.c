struct Foo {
  int x;
  char y;
};


int readint(void);

int main(void) {
  struct Foo f;
  f.x = readint();
  f.y = '$';
  return 0;
}
