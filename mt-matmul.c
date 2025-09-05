#include <riscv-pk/encoding.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include "marchid.h"
#include "util.h"
#include "dataset.h"

// EDIT THIS
static size_t n_cores = 2;

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

void matmul(const size_t coreid, const size_t ncores, const size_t lda,  const data_t A[], const data_t B[], data_t C[])
{
  size_t i, j, k;
  size_t block = lda / ncores;
  size_t start = block * coreid;
  //printf("starting hart %lu\n", coreid);
 
  for (i = 0; i < lda; i++) {
    for (j = start; j < (start+block); j++) {
      data_t sum = 0;
      for (k = 0; k < lda; k++)
        sum += A[j*lda + k] * B[k*lda + i];
      C[i + j*lda] = sum;
    }
  }
}


void thread_entry(int cid, int nc)
{
  static data_t results_data[ARRAY_SIZE];
  

  //stats(matmul(cid, nc, DIM_SIZE, input1_data, input2_data, results_data); barrier(nc), DIM_SIZE/DIM_SIZE/DIM_SIZE);


  matmul(cid, nc, DIM_SIZE, input1_data, input2_data, results_data);
  

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
