//
// >>>> malloc challenge! <<<<
//
// Your task is to improve utilization and speed of the following malloc
// implementation.
// Initial implementation is the same as the one implemented in simple_malloc.c.
// For the detailed explanation, please refer to simple_malloc.c.

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// Interfaces to get memory pages from OS
//

void *mmap_from_system(size_t size);
void munmap_to_system(void *ptr, size_t size);

//
// Struct definitions
//

const int MIN_BIN_SCALE = 3;
const int MAX_BIN_SCALE = 12;

typedef struct my_metadata_t
{
  size_t size;
  struct my_metadata_t *next;
} my_metadata_t;

typedef struct my_heap_t
{
  // free_bins[i] contains the list of free spaces whose size <= (1 << (i + MIN_BIN_SCALE))
  my_metadata_t *free_bins[MAX_BIN_SCALE - MIN_BIN_SCALE + 1];
  my_metadata_t dummy;
} my_heap_t;

//
// Static variables (DO NOT ADD ANOTHER STATIC VARIABLES!)
//
my_heap_t my_heap;

//
// Helper functions (feel free to add/remove/edit!)
//

int find_bin_idx(size_t size)
{
  for (int i = MIN_BIN_SCALE; i < MAX_BIN_SCALE; i++)
  {
    if ((1 << i) >= size)
    {
      return i - MIN_BIN_SCALE;
    }
  }
  return MAX_BIN_SCALE - MIN_BIN_SCALE;
}

void my_add_to_free_list(my_metadata_t *metadata)
{
  assert(!metadata->next);
  int bin_idx = find_bin_idx(metadata->size);
  metadata->next = my_heap.free_bins[bin_idx];
  my_heap.free_bins[bin_idx] = metadata;
}

void my_remove_from_free_list(my_metadata_t *metadata, my_metadata_t *prev, int idx)
{
  if (prev)
  {
    prev->next = metadata->next;
  }
  else
  {
    my_heap.free_bins[idx] = metadata->next;
  }
  metadata->next = NULL;
}

//
// Interfaces of malloc (DO NOT RENAME FOLLOWING FUNCTIONS!)
//

// This is called at the beginning of each challenge.
void my_initialize()
{
  for (int i = 0; i < MAX_BIN_SCALE - MIN_BIN_SCALE; i++)
  {
    my_heap.free_bins[i] = &my_heap.dummy;
  }
  my_heap.dummy.size = 0;
  my_heap.dummy.next = NULL;
}

// my_malloc() is called every time an object is allocated.
// |size| is guaranteed to be a multiple of 8 bytes and meets 8 <= |size| <=
// 4000. You are not allowed to use any library functions other than
// mmap_from_system() / munmap_to_system().
void *my_malloc(size_t size)
{
  int bin_idx = find_bin_idx(size);
  my_metadata_t *metadata, *prev, *metadata_best, *prev_best;
  size_t size_best;
  int bin_idx_best = -1;
  for (int i = bin_idx; i <= MAX_BIN_SCALE - MIN_BIN_SCALE; i++)
  {
    metadata = my_heap.free_bins[i];
    prev = NULL;
    metadata_best = metadata;
    prev_best = prev;
    size_best = 0;

    while (metadata)
    {
      if (metadata->size >= size && (size_best == 0 || size_best > metadata->size))
      {
        size_best = metadata->size;
        metadata_best = metadata;
        prev_best = prev;
      }
      prev = metadata;
      metadata = metadata->next;
    }
    if (size_best > 0)
    {
      bin_idx_best = i;
      break;
    }
  }
  // now, metadata points to the first free slot
  // and prev is the previous entry.

  if (bin_idx_best == -1)
  {
    // There was no free slot available. We need to request a new memory region
    // from the system by calling mmap_from_system().
    //
    //     | metadata | free slot |
    //     ^
    //     metadata
    //     <---------------------->
    //            buffer_size
    size_t buffer_size = 4096;
    my_metadata_t *metadata = (my_metadata_t *)mmap_from_system(buffer_size);
    metadata->size = buffer_size - sizeof(my_metadata_t);
    metadata->next = NULL;
    // Add the memory region to the free list.
    my_add_to_free_list(metadata);
    // Now, try my_malloc() again. This should succeed.
    return my_malloc(size);
  }

  // |ptr| is the beginning of the allocated object.
  //
  // ... | metadata | object | ...
  //     ^          ^
  //     metadata   ptr
  void *ptr = metadata_best + 1;
  size_t remaining_size = metadata_best->size - size;
  metadata_best->size = size;
  // Remove the free slot from the free list.
  my_remove_from_free_list(metadata_best, prev_best, bin_idx_best);

  if (remaining_size > sizeof(my_metadata_t))
  {
    // Create a new metadata for the remaining free slot.
    //
    // ... | metadata | object | metadata | free slot | ...
    //     ^          ^        ^
    //     metadata   ptr      new_metadata
    //                 <------><---------------------->
    //                   size       remaining size
    my_metadata_t *new_metadata = (my_metadata_t *)((char *)ptr + size);
    new_metadata->size = remaining_size - sizeof(my_metadata_t);
    new_metadata->next = NULL;
    // Add the remaining free slot to the free list.
    my_add_to_free_list(new_metadata);
  }
  return ptr;
}

// This is called every time an object is freed.  You are not allowed to
// use any library functions other than mmap_from_system / munmap_to_system.
void my_free(void *ptr)
{
  // Look up the metadata. The metadata is placed just prior to the object.
  //
  // ... | metadata | object | ...
  //     ^          ^
  //     metadata   ptr
  my_metadata_t *metadata = (my_metadata_t *)ptr - 1;
  // Add the free slot to the free list.
  my_add_to_free_list(metadata);
}

// This is called at the end of each challenge.
void my_finalize()
{
  // Nothing is here for now.
  // feel free to add something if you want!
}

void test()
{
  // Implement here!
  assert(1 == 1); /* 1 is 1. That's always true! (You can remove this.) */
}
