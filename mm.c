/**
 * @file   mm.c
 * @Author 02335 team
 * @date   September, 2024
 * @brief  Memory management skeleton.
 * 
 */

#include <stdint.h>

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

static BlockHeader * first = NULL;
static BlockHeader * current = NULL;

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
    if (first == NULL) {
        if (aligned_memory_start + 2 * sizeof(BlockHeader) + MIN_SIZE <= aligned_memory_end) {
            first = (BlockHeader *)aligned_memory_start;
            SET_FREE(first, 1);
            BlockHeader *last = (BlockHeader *)(aligned_memory_end - sizeof(BlockHeader));
            last->next = NULL;
            SET_FREE(last, 0);
            SET_NEXT(first, last);
            current = first;
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
 *
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

void *simple_malloc(size_t size) {
    if (first == NULL) {
        simple_init();
        if (first == NULL) return NULL;
    }
    size_t aligned_size = (size + (sizeof(void*) - 1)) & ~(sizeof(void*) - 1);
    BlockHeader *search_start = current;
    do {
        if (GET_FREE(current)) {
            if (SIZE(current) >= aligned_size) {
                if (SIZE(current) - aligned_size < sizeof(BlockHeader) + MIN_SIZE) {
                    SET_FREE(current, 0);
                } else {
                    uintptr_t new_block_address = (uintptr_t)current + aligned_size + sizeof(BlockHeader);
                    BlockHeader *new_block = (BlockHeader *)new_block_address;
                    SET_NEXT(new_block, GET_NEXT(current));
                    SET_FREE(new_block, 1);
                    SET_NEXT(current, new_block);
                    SET_FREE(current, 0);
                }
                return (void *)(current + 1);
            }
        }
        current = GET_NEXT(current);
    } while (current != search_start);
    return NULL;
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


void simple_free(void *ptr) {
    if (ptr == NULL) return;
    BlockHeader *block = (BlockHeader *)((uintptr_t)ptr - sizeof(BlockHeader));
    if (GET_FREE(block)) return;
    SET_FREE(block, 1);
    BlockHeader *next_block = GET_NEXT(block);
    if (next_block != NULL && GET_FREE(next_block)) {
        SET_NEXT(block, GET_NEXT(next_block));
    }
    current = block;
}


/* Include test routines */

#include "mm_aux.c"
