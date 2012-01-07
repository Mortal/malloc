#include <unistd.h>
#ifndef NDEBUG
#include <stdio.h>
#endif

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
  /* We need a bit of space for the freelist pointers */
  if (n < min_size) n = min_size;

  /* First bit of memory allocated */
  if (first == NULL) {
    last = first = new_block(data_offset + n);
    return first->data;
  }

  /* Find a free piece of memory */
  struct memory_block * cur = first;
  while (cur->occupied || cur->length < n) {
    if (cur == last) break;
    cur = next_block(cur);
  }

  if (cur == last) {
    /* No free piece, allocate new thing */
    last = cur = new_block(data_offset + n);
  } else {
    cur->occupied = 1;
  }
  return cur->data;
}

void my_free(void * ptr) {
  struct free_memory_block * block = (struct free_memory_block *) (ptr - data_offset);
  /* don't set block->occupied = 0 yet */

  /* Find next free block from here */
  struct memory_block * cur = (struct memory_block *) block;
  while (cur != last && cur->occupied) {
    cur = next_block(cur);
  }
  if (cur->occupied) {
    /* We're the last free block */
    block->occupied = 0;
    block->next_free = NULL;
    if (last_free) {
      last_free->next_free = block;
    }
    block->prev_free = last_free;
    last_free = block;
    return;
  }
  block->occupied = 0;
  /* cur is free */
  struct free_memory_block * fcur = (struct free_memory_block *) cur;
  /* insert into freelist */
  block->next_free = fcur;
  block->prev_free = fcur->prev_free;
  if (fcur->prev_free) {
    fcur->prev_free->next_free = block;
  }
  fcur->prev_free = block;
}

void memstats() {
#ifndef NDEBUG
  printf("Memory stats\n");
  if (first == NULL) {
    printf("Nothing allocated\n");
  } else {
    struct memory_block * cur = first;
    for (;;) {
      if (cur->occupied) {
	printf("%p %db occupied\n", cur, cur->length);
      } else {
	struct free_memory_block * fcur = (struct free_memory_block *) cur;
	printf("%p %db free %p %p\n", fcur, fcur->length, fcur->next_free, fcur->prev_free);
      }
      if (cur == last) break;
      cur = next_block(cur);
    }
  }
  printf("End memory stats\n");
#endif // NDEBUG
}
// vim:set sw=2 ts=8 sts=2:
