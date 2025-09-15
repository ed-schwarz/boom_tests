#include <riscv-pk/encoding.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include "marchid.h"
#include "util.h"


// EDIT THIS
#define ARRAY_SIZE 4 
static size_t n_cores = 1;
static size_t num = 25;




int fibbonacci(int n) {
   if(n == 0){
      return 0;
   } else if(n == 1) {
      return 1;
   } else {
      return (fibbonacci(n-1) + fibbonacci(n-2));
   }
}


void thread_entry(int cid, int nc)
{

  //stats(matmul(cid, nc, DIM_SIZE, input1_data, input2_data, results_data); barrier(nc), DIM_SIZE/DIM_SIZE/DIM_SIZE);
  int result;
  for(int i = cid; i < ARRAY_SIZE; i++){
    result = fibbonacci(num);
    //printf("Fibonacci of %lu is %lu\n", num, result);
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

  for (size_t i = 0; i < n_cores; i++) {
    if (mhartid == i) {
      printf("Starting hart = %lu\n", cid);
    }
    barrier(nc);
  }

  start_time = read_csr(mcycle);
  start_instr = read_csr(minstret);
  thread_entry(cid, nc);
  
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
  

  // Spin if not core 0
  if (mhartid > 0) while (1);
}


int main(void) {

  __main();

  return 0;
}
