#include "my_malloc.h"

const size_t PTRS = 10;

int main(int argc, char ** argv) {
  void * ptrs[PTRS];
  memstats();
  for (size_t i = 0; i < PTRS; ++i) {
    ptrs[i] = my_malloc(1 << i);
  }
  memstats();
  for (size_t i = 0; i < PTRS; i += 2) {
    my_free(ptrs[i]);
  }
  memstats();
  for (size_t i = 1; i < PTRS; i += 2) {
    my_free(ptrs[i]);
  }
  memstats();
  for (size_t i = 0; i < PTRS; ++i) {
    ptrs[i] = my_malloc(1 << (PTRS-i-1));
  }
  memstats();
  return 0;
}
// vim:set sw=2 ts=8 sts=2:
