#include <riscv-pk/encoding.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include "marchid.h"
#include "util.h"


// EDIT THIS

static size_t n_cores = 1;
static size_t num = 25;

struct fibfac {
    int fib;
    int fac;
};

static void __attribute__((noinline)) barrier_loop()
{
  static volatile int sense;
  static volatile int count;
  static __thread int threadsense;

  __sync_synchronize();

  threadsense = !threadsense;
  if (__sync_fetch_and_add(&count, 1) == n_cores-1)
  {
    count = 0;
    sense = threadsense;
  }
  else while(sense != threadsense)
    ;

  __sync_synchronize();
}

int factorial(int n) {
   //base case
   if(n == 0) {
      return 1;
   } else {
      return n * factorial(n-1);
   }
}


int fibbonacci(int n) {
   if(n == 0){
      return 0;
   } else if(n == 1) {
      return 1;
   } else {
      return (fibbonacci(n-1) + fibbonacci(n-2));
   }
}


void thread_entry(int cid, int nc, int* fib, int* fac)
{

  assert(fib);
  assert(fac);

  if(nc == 1){
    *fib = fibbonacci(num);
    *fac = factorial(num);
  }
  else{
    if(cid == 0){
      *fib = fibbonacci(num);
      *fac = 0;
    }
    else{
      *fib = 0;
      *fac = factorial(num);
    }
  }




   //exit(res);
}

void __main(void) {
  size_t mhartid = read_csr(mhartid);
  int cid = mhartid;
  int nc = n_cores;
  int start_time, end_time, total_time;
  int start_instr, end_instr, total_instr;
  

  if (mhartid >= n_cores) while (1);


  const char* march = get_march(read_csr(marchid));
  int fib;
  int fac;

  for (size_t i = 0; i < n_cores; i++) {
    if (mhartid == i) {
      printf("Starting hart = %lu\n", cid);
    }
    barrier(nc);
  }

  start_time = read_csr(mcycle);
  start_instr = read_csr(minstret);
  thread_entry(cid, nc, &fib, &fac);
  
  end_time = read_csr(mcycle);
  end_instr = read_csr(minstret);

  total_time = end_time - start_time;
  total_instr = end_instr - start_instr;
  //int res = verify(ARRAY_SIZE, results_data, verify_data);
  for (size_t i = 0; i < n_cores; i++) {
    if (mhartid == i) {
      printf("hart = %lu , total time = %lu , total instr = %lu\n", cid, total_time, total_instr);
    }
    barrier(nc);
  }

  for (size_t i = 0; i < n_cores; i++) {
  if(n_cores == 1){
    printf("Fibonacci from %lu is %lu\n", num, fib);
    printf("Factorial from %lu is %lu\n", num, fac);
  }
  else{
    if(mhartid == 0){
      printf("Fibonacci from %lu is %lu\n", num, fib);
    }
    else{
      printf("Factorial from %lu is %lu\n", num, fac);
    }
  }
    barrier(nc);
  }

  

  // Spin if not core 0
  if (mhartid > 0) while (1);
}


int main(void) {

  __main();

  return 0;
}
