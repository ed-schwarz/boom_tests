#include <riscv-pk/encoding.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include "marchid.h"
#include "util.h"


// EDIT THIS

static size_t n_cores = 2;
static float num = 100;


#pragma GCC optimize ("unroll-loops")
float factorial_it(float n) {
  double result = 1;
  for(double i = 1; i <= n; i++){
    result = result * i;
   }
  return result;
}


#pragma GCC optimize ("unroll-loops")
float fibbonacci_it (float n)
{
  float last = 1;   /* Initial value is fib (1).  */
  float prev = 0;   /* Initial value controls fib (2).  */
  float i;

  for (i = 1; i < n; ++i)
    /* If n is 1 or less, the loop runs zero times,  */
    /* since in that case i < n is false the first time.  */
    {
      /* Now last is fib (i)
         and prev is fib (i - 1).  */
      /* Compute fib (i + 1).  */
      double next = prev + last;
      /* Shift the values down.  */
      prev = last;
      last = next;
      /* Now last is fib (i + 1)
         and prev is fib (i).
         But that won’t stay true for long,
         because we are about to increment i.  */
    }

  return last;
}


void thread_entry(int cid, int nc, float* fib, float* fac)
{

  assert(fib);
  assert(fac);

  if(nc == 1){
    *fib = fibbonacci_it(num);
    *fac = factorial_it(num);
  }
  else{
    if(cid == 0){
      *fib = fibbonacci_it(num);
      *fac = 0;
    }
    else{
      *fib = 0;
      *fac = factorial_it(num);
    }
  }

}

void __main(void) {
  size_t mhartid = read_csr(mhartid);
  int cid = mhartid;
  int nc = n_cores;
  int start_time, end_time, total_time;
  int start_instr, end_instr, total_instr;
  

  if (mhartid >= n_cores) while (1);


  const char* march = get_march(read_csr(marchid));
  float fib;
  float fac;

  for (size_t i = 0; i < n_cores; i++) {
    barrier(nc);
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
    barrier(nc);
    if (mhartid == i) {
      printf("hart = %lu , total time rec = %lu , total instr = %lu\n", cid, total_time, total_instr);
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
