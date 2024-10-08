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
/**
 * @name    simple_init
 * @brief   Initialize the block structure within the available memory
 *
 */

// void simple_init() {
//   uintptr_t aligned_memory_start = memory_start;  /* TODO: Alignment */
//   uintptr_t aligned_memory_end   = memory_end;    /* TODO: Alignment */
//   BlockHeader * last;

//   /* Already initalized ? */
//   if (first == NULL) {
//     /* Check that we have room for at least one free block and an end header */
//     if (aligned_memory_start + 2*sizeof(BlockHeader) + MIN_SIZE <= aligned_memory_end) {
//       /* TODO: Place first and last blocks and set links and free flags properly */
//     }
//     current = first;     
//   } 
// }

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



/**
 * @name    simple_malloc
 * @brief   Allocate at least size contiguous bytes of memory and return a pointer to the first byte.
 *
 * This function should behave similar to a normal malloc implementation. 
 *
 * @param size_t size Number of bytes to allocate.
 * @retval Pointer to the start of the allocated memory or NULL if not possible.
 *simple_free_co
 */

// void* simple_malloc(size_t size) {
  
//   if (first == NULL) {
//     simple_init();
//     if (first == NULL) return NULL;
//   }

//   size_t aligned_size = size;  /* TODO: Alignment */

//   /* Search for a free block */
//   BlockHeader * search_start = current;
//   do {
 
//     if (GET_FREE(current)) {

//       /* Possibly coalesce consecutive free blocks here */

//       /* Check if free block is large enough */
//       if (SIZE(current) >= aligned_size) {
//         /* Will the remainder be large enough for a new block? */
//         if (SIZE(current) - aligned_size < sizeof(BlockHeader) + MIN_SIZE) {
//           /* TODO: Use block as is, marking it non-free*/
//         } else {
//           /* TODO: Carve aligned_size from block and allocate new free block for the rest */
//         }
        
//         return (void *) NULL; /* TODO: Return address of current's user_block and advance current */
//       }
//     }

//     current = GET_NEXT(current);
//   } while (current != search_start);

//  /* None found */
//   return NULL;
// }
//*************** */
// void *simple_malloc(size_t size) {
//     if (first == NULL) {
//         simple_init();
//         if (first == NULL) return NULL;
//     }
//     size_t aligned_size = (size + (sizeof(void*) - 1)) & ~(sizeof(void*) - 1);
//     BlockHeader *search_start = current;
//     do {
//         if (GET_FREE(current)) {
//             if (SIZE(current) >= aligned_size) {
//                 if (SIZE(current) - aligned_size < sizeof(BlockHeader) + MIN_SIZE) {
//                     SET_FREE(current, 0);
//                 } else {
//                     uintptr_t new_block_address = (uintptr_t)current + aligned_size + sizeof(BlockHeader);
//                     BlockHeader *new_block = (BlockHeader *)new_block_address;
//                     SET_NEXT(new_block, GET_NEXT(current));
//                     SET_FREE(new_block, 1);
//                     SET_NEXT(current, new_block);
//                     SET_FREE(current, 0);
//                 }
//                 return (void *)(current + 1);
//             }
//         }
//         current = GET_NEXT(current);
//     } while (current != search_start);
//     return NULL;
// }


void *simple_malloc(size_t size) {
    if (first == NULL) {
        simple_init();
        if (first == NULL) return NULL;
    }

    size_t aligned_size = (size + (sizeof(void*) - 1)) & ~(sizeof(void*) - 1);
    BlockHeader *search_start = current;
    BlockHeader *best_fit = NULL;
    size_t smallest_fit_size = (size_t)-1;

    printf("Malloc: Requesting %zu bytes (aligned to %zu bytes)\n", size, aligned_size);

    // Search for the best-fit block
    do {
        printf("Malloc: Inspecting block at %p, Free = %d, Size = %zu\n", current, GET_FREE(current), SIZE(current));

        if (GET_FREE(current)) {
            size_t current_block_size = SIZE(current);

            // Look for the smallest free block that can fit the requested size
            if (current_block_size >= aligned_size && current_block_size < smallest_fit_size) {
                best_fit = current;
                smallest_fit_size = current_block_size;
            }
        }

        current = GET_NEXT(current);
        
        // Check if the next block is NULL or corrupted
        if (current == NULL || (uintptr_t)current >= memory_end || (uintptr_t)current < memory_start) {
            printf("Error: next block is NULL or out of bounds at %p\n", current);
            return NULL;  // Prevent segfault by returning if invalid pointer
        }

    } while (current != search_start);

    // If no suitable block was found, return NULL
    if (best_fit == NULL) {
        printf("Malloc: No suitable block found\n");
        return NULL;
    }

    printf("Malloc: Using block at %p, Size = %zu\n", best_fit, SIZE(best_fit));

    // Use the best-fit block
    if (smallest_fit_size - aligned_size < sizeof(BlockHeader) + MIN_SIZE) {
        SET_FREE(best_fit, 0);
    } else {
        uintptr_t new_block_address = (uintptr_t)best_fit + aligned_size + sizeof(BlockHeader);
        if (new_block_address >= memory_end) {
            printf("Error: new block address out of bounds\n");
            return NULL;
        }
        BlockHeader *new_block = (BlockHeader *)new_block_address;
        SET_NEXT(new_block, GET_NEXT(best_fit));
        SET_FREE(new_block, 1);
        SET_NEXT(best_fit, new_block);
        SET_FREE(best_fit, 0);
    }

    return (void *)(best_fit + 1);
}






/**
 * @name    simple_free
 * @brief   Frees previously allocated memory and makes it available for subsequent calls to simple_malloc
 *
 * This function should behave similar to a normal free implementation. 
 *
 * @param void *ptr Pointer to the memory to free.
 *
 */
// void simple_free(void * ptr) {
//   BlockHeader * block = NULL; /* TODO: Find block corresponding to ptr */
//   if (GET_FREE(block)) {
//     /* Block is not in use -- probably an error */
//     return;
//   }

//   /* TODO: Free block */

//   /* Possibly coalesce consecutive free blocks here */
// }

//**************** */
// void simple_free(void *ptr) {
//     if (ptr == NULL) return;
//     BlockHeader *block = (BlockHeader *)((uintptr_t)ptr - sizeof(BlockHeader));
//     if (GET_FREE(block)) return;
//     SET_FREE(block, 1);
//     BlockHeader *next_block = GET_NEXT(block);
//     if (next_block != NULL && GET_FREE(next_block)) {
//         SET_NEXT(block, GET_NEXT(next_block));
//     }
//     current = block;
// }


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
