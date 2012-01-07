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
    struct free_memory_block * fcur = (struct free_memory_block *) cur;
    fcur->occupied = 1;
    if (fcur->prev_free)
      fcur->prev_free->next_free = fcur->next_free;
    if (fcur->next_free)
      fcur->next_free->prev_free = fcur->prev_free;
    else
      last_free = fcur->prev_free;
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

void memconsistency() {
#ifndef NDEBUG
  if (first == NULL) return;
  struct memory_block * cur = first;
  int saw_free = 0;
  struct free_memory_block * last_seen = NULL;
  struct free_memory_block * expect = NULL;
  char seen_any_free = 0;
  for (;;) {
    if (cur->occupied) {
      saw_free = 0;
    } else {
      struct free_memory_block * fcur = (struct free_memory_block *) cur;
      if (saw_free) {
	printf("%p: Seen %d free blocks in a row\n", fcur, saw_free);
      }
      if (fcur->prev_free != last_seen) {
	printf("%p: Expected prev_free = %p, got %p\n", fcur, last_seen, fcur->prev_free);
      }
      if (seen_any_free && fcur != expect) {
	printf("%p: Expected %p to be the next free block", fcur, expect);
      }
      ++saw_free;
      last_seen = fcur;
      expect = fcur->next_free;
      seen_any_free = 1;
    }
    if (cur == last) break;
    cur = next_block(cur);
  }
#endif // NDEBUG
}
// vim:set sw=2 ts=8 sts=2:
