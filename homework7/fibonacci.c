#include <stdio.h>
#include <omp.h>

int fib(int n) {
  if (n <= 10) {
    if (n <= 1) {
      return n;
    }
    return fib(n - 1) + fib(n - 2);
  }

  int a, b;

  #pragma omp task shared(a)
  a = fib(n - 1);

  #pragma omp task shared(b)
  b = fib(n - 2);

  #pragma omp taskwait
  return a + b;
}

int main() {
  int num = 0;
  printf("Enter a number: ");
  scanf("%d", &num);

  int result = 0;

  #pragma omp parallel
  {
    #pragma omp single
    result = fib(num);
  }

  printf("F(%d) = %d\n", num, result);
  return 0;
}
