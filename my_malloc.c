#include <unistd.h>

struct memory_block {
  char occupied;
  size_t length;
  char data[0];
};
struct free_memory_block {
  char occupied;
  size_t length;
  struct free_memory_block * next_free;
  struct free_memory_block * prev_free;
};

static struct memory_block * first = NULL;
static struct memory_block * last = NULL;
static struct free_memory_block * last_free = NULL;

static const size_t data_offset = sizeof(struct memory_block);
static const size_t min_size = 2*sizeof(struct free_memory_block *);

static struct memory_block * new_block(size_t sz) {
  struct memory_block * res = (struct memory_block *) sbrk(sz);
  res->occupied = 1;
  res->length = sz;
  return res;
}
#define next_block(cur) ((struct memory_block *) (((char *) cur) + cur->length))

void * my_malloc(size_t n) {
  if (n < min_size) n = min_size;
  if (first == NULL) {
    last = first = new_block(data_offset + n);
    return first->data;
  }
  struct memory_block * cur = first;
  while (cur->occupied || cur->length < n) {
    if (cur == last) break;
    cur = next_block(cur);
  }
  if (cur == last) {
    last = cur = new_block(data_offset + n);
  } else {
    cur->occupied = 1;
  }
  return cur->data;
}

void my_free(void * ptr) {
  struct free_memory_block * block = (struct free_memory_block *) (ptr - data_offset);
  block->occupied = 0;
  struct memory_block * cur = next_block(block);
  while (cur != last && cur->occupied) {
    cur = next_block(cur);
  }
  if (cur == last) {
    block->next_free = NULL;
    if (last_free) {
      last_free->next_free = block;
    }
    last_free = block;
    return;
  }
  struct free_memory_block * fcur = (struct free_memory_block *) cur;
  block->next_free = fcur;
  block->prev_free = fcur->prev_free;
  if (fcur->prev_free) {
    fcur->prev_free->next_free = block;
  }
  fcur->prev_free = block;
}
