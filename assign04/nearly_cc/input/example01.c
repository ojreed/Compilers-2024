int add(int a, int b) {
  return a+b;
}

int main(void) {
  int i, n, sum;

  sum = 0;
  n = 11;

  for (i = 0; i <= n; i = i + 1) {
    sum = add(sum,1);
  }

  return sum;
}