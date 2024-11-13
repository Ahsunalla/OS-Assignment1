/**
 * @file   mm.c
 * @Author 02335 team
 * @date   September, 2024
 * @brief  Memory management skeleton.
 * 
 */

#include <stdint.h>
//#include "mm_aux.c"
#include "mm.h"



/* Proposed data structure elements */

typedef struct header {
  struct header * next;     // Bit 0 is used to indicate free block 
  uint64_t user_block[0];   // Standard trick: Empty array to make sure start of user block is aligned
} BlockHeader;

/* Macros to handle the free flag at bit 0 of the next pointer of header pointed at by p */
#define GET_NEXT(p) (BlockHeader *)((uintptr_t)(p->next) & ~1)
#define SET_NEXT(p, n) p->next = (BlockHeader *)((uintptr_t)(n) | ((uintptr_t)(p->next) & 1))
#define GET_FREE(p) (uint8_t)((uintptr_t)(p->next) & 0x1)
#define SET_FREE(p, f) p->next = (BlockHeader *)(((uintptr_t)(p->next) & ~1) | (f & 1))
#define SIZE(p) (size_t)((uintptr_t)GET_NEXT(p) - (uintptr_t)(p + 1))
#define MIN_SIZE (8)




BlockHeader *first = NULL;
BlockHeader *current = NULL;

// Gets the next block in the list from the given block
static BlockHeader *get_next_block(BlockHeader *block) {
    return GET_NEXT(block);
}

// Sets the next block pointer for the given block
static void set_next_block(BlockHeader *block, BlockHeader *next) {
    SET_NEXT(block, next);
}

// Checks if the block is free
static uint8_t is_block_free(BlockHeader *block) {
    return GET_FREE(block);
}

// Marks a block as free or allocated
static void mark_block_free(BlockHeader *block, uint8_t free) {
    SET_FREE(block, free);
}

// Gets the size of the given block
static size_t get_block_size(BlockHeader *block) {
    return SIZE(block);
}


void simple_init() {
    uintptr_t aligned_memory_start = (memory_start + (sizeof(void*) - 1)) & ~(sizeof(void*) - 1);
    uintptr_t aligned_memory_end = memory_end & ~(sizeof(void*) - 1);
    printf("Init: Aligned memory range: %p - %p\n", (void*)aligned_memory_start, (void*)aligned_memory_end);

    if (first == NULL) {
        if (aligned_memory_start + 2 * sizeof(BlockHeader) + MIN_SIZE <= aligned_memory_end) {
            first = (BlockHeader *)aligned_memory_start;
            SET_FREE(first, 1);
            BlockHeader *last = (BlockHeader *)(aligned_memory_end - sizeof(BlockHeader));
            last->next = first;  // Make it circular
            SET_FREE(last, 1);
            SET_NEXT(first, last);
            current = first;

            printf("Init: First block at %p, Last block at %p\n", first, last);
        } else {
            printf("Error: Not enough memory to initialize\n");
        }
    }
}




void *simple_malloc(size_t size) {
    if (first == NULL) {
        simple_init();
        if (first == NULL) return NULL;
    }

    // Align size to the nearest multiple of pointer size
    size_t aligned_size = (size + (sizeof(void*) - 1)) & ~(sizeof(void*) - 1);
    BlockHeader *search_start = current; // Start search from the last allocated block

    // Search for the next free block starting from `current`
    do {
        if (is_block_free(current) && get_block_size(current) >= aligned_size) {
            size_t block_size = get_block_size(current);

            // Check if there's enough space to split the block
            if (block_size - aligned_size < sizeof(BlockHeader) + MIN_SIZE) {
                mark_block_free(current, 0); // Mark entire block as used
            } else {
                // Split the block
                uintptr_t new_block_address = (uintptr_t)current + aligned_size + sizeof(BlockHeader);
                BlockHeader *new_block = (BlockHeader *)new_block_address;
                set_next_block(new_block, get_next_block(current));
                mark_block_free(new_block, 1);
                set_next_block(current, new_block);
                mark_block_free(current, 0);
            }

            void *allocated_memory = (void *)(current + 1);
            current = get_next_block(current); // Move `current` to the next block for next-fit
            return allocated_memory;
        }
        current = get_next_block(current);
    } while (current != search_start);

    // No suitable block found
    return NULL;
}


void simple_free(void *ptr) {
    if (ptr == NULL) return;

    BlockHeader *block = (BlockHeader *)((uintptr_t)ptr - sizeof(BlockHeader));
    if (block == NULL) {
        printf("Error: trying to free a NULL block\n");
        return;
    }
    
    if (GET_FREE(block)) return;  // Block already free, do nothing

    SET_FREE(block, 1);
    BlockHeader *next_block = GET_NEXT(block);
    if (next_block != NULL && GET_FREE(next_block)) {
        if ((uintptr_t)next_block >= memory_end || (uintptr_t)next_block < memory_start) {
            printf("Error: next block out of bounds\n");
            return;
        }
        SET_NEXT(block, GET_NEXT(next_block));
    }
    current = block;
}



/* Include test routines */

int simple_macro_test() {
  BlockHeader block;
  BlockHeader * p = &block;
  void * addr[2] = { (void *)  0x1234BABA, (void *) 0xFEDCBA981234BABA };
  int i;
  int ret = 0;

  /* Test separately for 32 and 64 bit addresses */
  for (i =0; i < 2; i++) {
    p->next = NULL;
    /* Check that next and free are properly separated */
    SET_NEXT(p, addr[i]);
    SET_FREE(p, 7);  /* only least bit should be used */

    if (GET_NEXT(p) != addr[i]) return 1 + i*10;  // Next pointer damaged
    if (GET_FREE(p) != 1)       return 2 + i*10;  // Free flag not set

    SET_NEXT(p, NULL);
    if (GET_FREE(p) != 1)       return 3 + i*10;  // Free flag damaged

    SET_NEXT(p, addr[i]);
    SET_FREE(p, 0);

    if (GET_FREE(p) != 0)       return 4 + i*10;  // Free flag not cleared
    if (GET_NEXT(p) != addr[i]) return 5 + i*10;  // Next pointer damaged

    /* Check size with and without flag */
    SET_FREE(p,i);

    /* Check size for forward next pointer */
    SET_NEXT(p, (void *) ((uintptr_t) p + sizeof(BlockHeader) + 0x100 ) );
    if (SIZE(p) !=  0x100)      return 6 + i*10;

    /* Check size for backward next pointer (dummy block) */
    SET_NEXT(p, (void *) ((uintptr_t) p + sizeof(BlockHeader) - 0x100 ) );
    if (SIZE(p) != 0 && SIZE(p) < 0x800000000000000 )   return 7 + i*10;
  
  }
  return ret;
} 


static void print_block(BlockHeader * p) {
  printf("Block at 0x%08lx next = 0x%08lx, free = %d\n",  (uintptr_t) p, (uintptr_t) GET_NEXT(p), GET_FREE(p));
}


/**
 * @name    simple_block_dump
 * @brief   Dumps the current list of blocks on standard out
 */
void simple_block_dump(void) {
  BlockHeader * p;

  if (first == NULL) {
    printf("Data structure is not initialized\n");
    return;
  }

  printf("first = 0x%08lx, current = 0x%08lx\n", (uintptr_t) first, (uintptr_t) current);

  p = first;

  do {
    if ((uintptr_t) p < memory_start || (uintptr_t) p >= memory_end) {
      printf("Block pointer 0x%08lx out of range\n", (uintptr_t) p);
      return;
    }

    print_block(p);

    p = GET_NEXT(p);
  } while (p != first);

}
